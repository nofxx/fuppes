/***************************************************************************
 *            HTTPServer.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "HTTPServer.h"
#include "HTTPMessage.h"
#include "HTTPRequestHandler.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"
#include "../Common/RegEx.h"

#include <iostream>
#include <sstream>
#ifndef WIN32
#include <errno.h>
#endif

using namespace std;

const string LOGNAME = "HTTPServer";

#ifndef WIN32
// mac os x has no MSG_NOSIGNAL 
// but >= 10.2 comes with SO_SIGPIPE
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif
#endif


fuppesThreadCallback AcceptLoop(void *arg);
fuppesThreadCallback SessionLoop(void *arg);

bool ReceiveRequest(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Request);
bool SendResponse(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Response, CHTTPMessage* p_Request);


/** Constructor */
CHTTPServer::CHTTPServer(std::string p_sIPAddress)
{
  // init member vars
  m_bIsRunning  = false;
	accept_thread = (fuppesThread)NULL;
	fuppesThreadInitMutex(&m_ReceiveMutex);  	
  	
  // create socket
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(m_Socket == -1)
	  throw EException("failed to create socket", __FILE__, __LINE__);
  
	#ifdef FUPPES_TARGET_MAC_OSX
	/* OS X does not support pthread_cancel
	   so we need to set the socket to non blocking and
		 constantly poll the cancellation state.
		 otherwise fuppes will hang on shutdown
		 same for UDPSocket */
	fuppesSocketSetNonBlocking(m_Socket);
	#endif
   
  // set local end point
	local_ep.sin_family      = AF_INET;
	local_ep.sin_addr.s_addr = inet_addr(p_sIPAddress.c_str());
	local_ep.sin_port				 = htons(CSharedConfig::Shared()->GetHTTPPort());
	memset(&(local_ep.sin_zero), '\0', 8);
	
  // bind the socket
	int nRet = bind(m_Socket, (struct sockaddr*)&local_ep, sizeof(local_ep));	
  if(nRet == -1)   
    throw EException("failed to bind socket", __FILE__, __LINE__);
  
  // fetch local end point to get port number on random ports
	socklen_t size = sizeof(local_ep);
	getsockname(m_Socket, (struct sockaddr*)&local_ep, &size);
    
} // CHTTPServer()


/** Destructor */
CHTTPServer::~CHTTPServer()
{
  Stop();
	fuppesThreadDestroyMutex(&m_ReceiveMutex);  
} // ~CHTTPServer()


/** Start() */
void CHTTPServer::Start()
{
  m_bBreakAccept = false;
  
  // listen on socket
  int nRet = listen(m_Socket, 0);
  if(nRet == -1) {
    CSharedLog::Shared()->Log(L_ERROR, "failed to listen on socket", __FILE__, __LINE__);
    throw EException("failed to listen on socket", __FILE__, __LINE__);
  }

  // start accept thread
  fuppesThreadStart(accept_thread, AcceptLoop);  
  m_bIsRunning = true;
  
  CSharedLog::Shared()->Log(L_EXTENDED, "HTTPServer started", __FILE__, __LINE__);
} // Start()


/** Stop() */
void CHTTPServer::Stop()
{   
  if(!m_bIsRunning)
    return;

  // stop accept thread
  m_bBreakAccept = true;
  if(accept_thread) {
    int nExitCode;    
    fuppesThreadCancel(accept_thread, nExitCode);
    fuppesThreadClose(accept_thread);
    accept_thread = (fuppesThread)NULL;    
  }
    
  CleanupSessions();
  
  // close socket
  upnpSocketClose(m_Socket);
  m_bIsRunning = false;
  
  CSharedLog::Shared()->Log(L_EXTENDED, "HTTPServer stopped", __FILE__, __LINE__);
} // Stop()

std::string CHTTPServer::GetURL()
{
  stringstream result;
  result << inet_ntoa(local_ep.sin_addr) << ":" << ntohs(local_ep.sin_port);
  return result.str();
}

bool CHTTPServer::SetReceiveHandler(IHTTPServer* pHandler)
{
  m_pReceiveHandler = pHandler;
  return true;
}

// deprecated :: request are handled asynchronous by CHHTTPRequestHandler
bool CHTTPServer::CallOnReceive(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  fuppesThreadLockMutex(&m_ReceiveMutex);  

  bool bResult = false;
  if(m_pReceiveHandler != NULL) {
    // Parse message
    bResult = m_pReceiveHandler->OnHTTPServerReceiveMsg(pMessageIn, pMessageOut);
  }
    
  fuppesThreadUnlockMutex(&m_ReceiveMutex);    
  return bResult;
}

/**
 * closes finished session threads
 */
void CHTTPServer::CleanupSessions()
{
  if(m_ThreadList.empty())
    return;
 
  // iterate session list ...
  for(m_ThreadListIterator = m_ThreadList.begin(); m_ThreadListIterator != m_ThreadList.end();)
  {
    if(m_ThreadList.empty())
      break;
    
    // ... and close terminated threads
    CHTTPSessionInfo* pInfo = *m_ThreadListIterator;   
    if(pInfo && pInfo->m_bIsTerminated && fuppesThreadClose(pInfo->GetThreadHandle()))
    {       
      std::list<CHTTPSessionInfo*>::iterator tmpIt = m_ThreadListIterator;      
      ++tmpIt;                 
      m_ThreadList.erase(m_ThreadListIterator);
      m_ThreadListIterator = tmpIt;
      delete pInfo; 
      continue;
    }    
    
    m_ThreadListIterator++;    
  }
}

/**
 * the HTTPServer's AcceptLoop constantly
 * looks for new incoming connections, 
 * starts a new Session Thread for each
 * new connection and stores them in the session list
 */
fuppesThreadCallback AcceptLoop(void *arg)
{                     
  #ifndef FUPPES_TARGET_WIN32                     
  //set thread cancel state
  int nRetVal = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  if (nRetVal != 0) {
    perror("Thread pthread_setcancelstate Failed...\n");
    exit(EXIT_FAILURE);
  }

  // set thread cancel type
  nRetVal = pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  if (nRetVal != 0) {
    perror("Thread pthread_setcanceltype failed...");
    exit(EXIT_FAILURE);
  }
  #endif // FUPPES_TARGET_WIN32


  // local vars
	CHTTPServer* pHTTPServer = (CHTTPServer*)arg; 
  
	upnpSocket nSocket     = pHTTPServer->GetSocket();			
	upnpSocket nConnection = 0;
  
	struct sockaddr_in remote_ep;
	socklen_t size = sizeof(remote_ep);
  
  // log
  stringstream sLog;
  sLog << "listening on" << pHTTPServer->GetURL();  
	CSharedLog::Shared()->Log(L_EXTENDED, sLog.str(), __FILE__, __LINE__);
  
  // loop	
	while(!pHTTPServer->m_bBreakAccept)
	{
    // accept new connection
    nConnection = accept(nSocket, (struct sockaddr*)&remote_ep, &size);   
		if(nConnection == -1) {
		  #ifdef FUPPES_TARGET_MAC_OSX
      pthread_testcancel();
			fuppesSleep(100);
			#endif
			continue;
		}
			
    // log
    stringstream sMsg;
    sMsg << "new connection from " << inet_ntoa(remote_ep.sin_addr) << ":" << ntohs(remote_ep.sin_port);
		CSharedLog::Shared()->Log(L_EXTENDED, sMsg.str(), __FILE__, __LINE__);
      
    // start session thread ...
    CHTTPSessionInfo* pSession = new CHTTPSessionInfo(pHTTPServer, nConnection, remote_ep);      
    fuppesThread SessionThread = (fuppesThread)NULL;
    
    fuppesThreadStartArg(SessionThread, SessionLoop, *pSession);
    pSession->SetThreadHandle(SessionThread);
    // ... and store the thread in the session list
    pHTTPServer->m_ThreadList.push_back(pSession);
		
    // cleanup closed sessions
    pHTTPServer->CleanupSessions();
	}  
	  
  CSharedLog::Shared()->Log(L_EXTENDED, "exiting accept loop", __FILE__, __LINE__);
  pHTTPServer->CleanupSessions();
	fuppesThreadExit();
} // AcceptLoop

/** Session-loop
  */
fuppesThreadCallback SessionLoop(void *arg)
{
  CHTTPSessionInfo*    pSession  = (CHTTPSessionInfo*)arg;    
  CHTTPMessage*        pRequest  = new CHTTPMessage();   
  CHTTPMessage*        pResponse = new CHTTPMessage();
  CHTTPRequestHandler* pHandler  = new CHTTPRequestHandler();
  
  bool bKeepAlive = true;
  bool bResult    = false;
  
  stringstream sLog;
  
  while(bKeepAlive)
  {  
    // receive HTTP-request
    bResult = ReceiveRequest(pSession, pRequest);  
    if(!bResult)
      break;
    
    sLog << "REQUEST:" << endl << pRequest->GetMessage();    
    CSharedLog::Shared()->Log(L_DEBUG, sLog.str(), __FILE__, __LINE__);
    sLog.str("");
    // end receive
    
    // check if requesting IP is allowed to access
    std::string sIP = inet_ntoa(pSession->GetRemoteEndPoint().sin_addr);    
    if(CSharedConfig::Shared()->IsAllowedIP(sIP)) {    
      // build response
      bResult = pHandler->HandleRequest(pRequest, pResponse);      
      if(!bResult)
        bResult = pSession->GetHTTPServer()->CallOnReceive(pRequest, pResponse);      
    }
    // otherwise create a "403 (forbidden)" response
    else {
      pResponse->SetVersion(HTTP_VERSION_1_0);
      pResponse->SetMessageType(HTTP_MESSAGE_TYPE_403_FORBIDDEN);
      pResponse->SetMessage("403 Forbidden");
    }
    
    // send response
    bResult = SendResponse(pSession, pResponse, pRequest);
    if(!bResult) {
      CSharedLog::Shared()->Error(LOGNAME, "sending HTTP message");    
      break;
    }
    // end send response
    
    bKeepAlive = false;
  }  
  
  // close connection
  upnpSocketClose(pSession->GetConnection());
  
  // clean up
  delete pRequest;
  delete pResponse;
  delete pHandler;
  
  // exit thread
  pSession->m_bIsTerminated = true;
  fuppesThreadExit();  
}


/** receives a HTTP messag from p_Session's socket and stores it in p_Request */
bool ReceiveRequest(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Request)
{
  int   nBytesReceived = 0;
  int   nRecvCnt = 0;
  char  szBuffer[4096];    
  char* szMsg = NULL;
  unsigned int nContentLength = 0;   
 
  bool bDoReceive = true;
  bool bRecvErr   = false;
  
  unsigned int nLoopCnt = 0;  
  
  // receive loop
  int nTmpRecv = 0;
  while(bDoReceive)
  {           
    if(nRecvCnt == 30)
      break;
                   
    // receive
    nTmpRecv = recv(p_Session->GetConnection(), szBuffer, 4096, 0);    
    
    // error handling
    if(nTmpRecv < 0) {
      // log
      stringstream sLog;
      #ifdef WIN32    
      sLog << "error no. " << WSAGetLastError() << " " << strerror(WSAGetLastError()) << endl;      
      #else
      sLog << "error no. " << errno << " " << strerror(errno) << endl;            
      #endif
      CSharedLog::Shared()->Log(L_EXTENDED_ERR, sLog.str(), __FILE__, __LINE__);
      
      #ifdef WIN32
      if(WSAGetLastError() != WSAEWOULDBLOCK) {
        bDoReceive = false;
        bRecvErr   = true;
        break;      
      }
      else {
        nLoopCnt++;        
        fuppesSleep(10);
        continue;
      }
      #else      
      bDoReceive = false;
      bRecvErr   = true;
      break;      
      #endif
    } // if(nTmpRecv < 0)                 
    
    // create new buffer or ...
    if ((nBytesReceived == 0) && (nTmpRecv > 0)) {
      szMsg = new char[nTmpRecv + 1];
      memcpy(szMsg, szBuffer, nTmpRecv);
      szMsg[nTmpRecv] = '\0';
    }    
    // ... append received buffer or ...
    else if((nBytesReceived > 0) && (nTmpRecv > 0)) {
      char* szTmp = new char[nBytesReceived + 1];
      memcpy(szTmp, szMsg, nBytesReceived);
      szTmp[nBytesReceived] = '\0';
      
      delete[] szMsg;        
      szMsg = new char[nBytesReceived + nTmpRecv + 1];
      memcpy(szMsg, szTmp, nBytesReceived);
      memcpy(&szMsg[nBytesReceived], szBuffer, nTmpRecv);
      szMsg[nBytesReceived + nTmpRecv] = '\0';
      
      delete[] szTmp;
    }    
    // ... close connection gracefully
    else if(nTmpRecv == 0) {      
      upnpSocketClose(p_Session->GetConnection()); 
      bDoReceive = false;
      break;
    }
    
    // clear receive buffer
    memset(szBuffer, '\0', 4096);
    
    nBytesReceived += nTmpRecv; 
    string sMsg = szMsg;     
    
    // split header and content
    string sHeader  = "";
    string sContent = "";
    string::size_type nPos = sMsg.find("\r\n\r\n");  
        
    // got the full header
    if(nPos != string::npos) {            
      sHeader = sMsg.substr(0, nPos);
      nPos += string("\r\n\r\n").length();
      sContent = sMsg.substr(nPos, sMsg.length() - nPos);
    }
    // header is incomplete (continue receiving)
    else {
      nRecvCnt++;      
      continue;
    }
    
    
    // check content length
    RegEx rxContentLength("CONTENT-LENGTH: *(\\d+)", PCRE_CASELESS);
    if(rxContentLength.Search(sHeader.c_str())) {
      string sContentLength = rxContentLength.Match(1);    
      nContentLength = std::atoi(sContentLength.c_str());
    }

    // check if we received the full content
    if(sContent.length() < nContentLength) {
      // content incomplete (continue receiving)
      continue;
    }
    else {
      // full content. message is complete
      bDoReceive = false;      
    }
                  
    if(nTmpRecv == 0)      
      nRecvCnt++;

  } // while(bDoReceive) 
  // end receive


  // build received message
  bool bResult = false;  
  if((nBytesReceived > 0) && !bRecvErr)
  {
    // log
    stringstream sMsg;
    sMsg << "bytes received: " << nBytesReceived;
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());

    // create message
    bResult = p_Request->SetMessage(szMsg);    
  }
  
  return bResult;
} // ReceiveRequest


/** sends p_Response via the socket in p_Session */
bool SendResponse(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Response, CHTTPMessage* p_Request)
{
  // local vars
  unsigned int nRet = 0;       
  stringstream sLog;  
  

  // send complete binary stream
  /*if(!p_Response->IsChunked() && (p_Response->GetBinContentLength() > 0))
  { 
    // log
    sLog << p_Response->GetHeaderAsString() << "complete binary";
    CSharedLog::Shared()->Log(L_DEBUG, sLog.str(), __FILE__, __LINE__);
    sLog.str("");
    
    // send header
    send(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), (int)strlen(p_Response->GetHeaderAsString().c_str()), 0);      
    
    // send complete bin content
    #ifdef WIN32
    send(p_Session->GetConnection(), p_Response->GetBinContent(), p_Response->GetBinContentLength(), 0);          
    #else      
    send(p_Session->GetConnection(), p_Response->GetBinContent(), p_Response->GetBinContentLength(), MSG_NOSIGNAL);      
    #endif
    return true;
  }*/
  // send complete binary


  // send text message
  if(!p_Response->IsBinary())
  { 
    CSharedLog::Shared()->Log(L_DEBUG, p_Response->GetMessageAsString(), __FILE__, __LINE__);      
    
    #ifdef WIN32          
    send(p_Session->GetConnection(), p_Response->GetMessageAsString().c_str(), (int)strlen(p_Response->GetMessageAsString().c_str()), 0); 
    if(nRet == -1) {
      stringstream sLog;            
      sLog << "send error :: error no. " << WSAGetLastError() << " " << strerror(WSAGetLastError()) << endl;              
      CSharedLog::Shared()->Log(L_EXTENDED_ERR, sLog.str(), __FILE__, __LINE__);
    }
    #else
    nRet = send(p_Session->GetConnection(), p_Response->GetMessageAsString().c_str(), (int)strlen(p_Response->GetMessageAsString().c_str()), MSG_NOSIGNAL); 
    if(nRet == -1) {
      stringstream sLog;       
      sLog << "send error :: error no. " << errno << " " << strerror(errno) << endl;
      CSharedLog::Shared()->Log(L_EXTENDED_ERR, sLog.str(), __FILE__, __LINE__);
    }          
    #endif
    
    return true;
  }
  // end send text message
  
  
  
  // send binary
  unsigned int nOffset      = 0;
  char*        szChunk      = NULL;
  unsigned int nRequestSize = 64000;
  int          nErr         = 0;    
    
    
  // set ranges
  if((p_Request->GetRangeStart() > 0) ||
     ((p_Request->GetRangeEnd() > 0) && (p_Request->GetRangeEnd() < p_Response->GetBinContentLength()))
    )
  {          
    if(p_Request->GetRangeEnd() > p_Request->GetRangeStart())
      nRequestSize = p_Request->GetRangeEnd() - p_Request->GetRangeStart() + 1;
    else
      nRequestSize = p_Response->GetBinContentLength() - p_Request->GetRangeStart() + 1;
          
    nOffset = p_Request->GetRangeStart();      
    p_Response->SetMessageType(HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT);          
  }    
  
  p_Response->SetRangeStart(p_Request->GetRangeStart());
  if(p_Request->GetRangeEnd() > 0)
    p_Response->SetRangeEnd(p_Request->GetRangeEnd());
  else
    p_Response->SetRangeEnd(p_Response->GetBinContentLength());
  
 
    
  // send header if it is a HEAD response 
  // or the start range is greater than the content and return
  if((nErr != -1) && ((p_Request->GetMessageType() == HTTP_MESSAGE_TYPE_HEAD) ||
     ((p_Request->GetRangeStart() > 0) && (p_Request->GetRangeStart() >= p_Response->GetBinContentLength()))))
  {
    CSharedLog::Shared()->Log(L_DEBUG, p_Response->GetHeaderAsString(), __FILE__, __LINE__);      
    
    #ifdef WIN32
    nErr = send(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), (int)strlen(p_Response->GetHeaderAsString().c_str()), 0);             
    #else
    nErr = send(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), (int)strlen(p_Response->GetHeaderAsString().c_str()), MSG_NOSIGNAL);
    #endif    
    return true;
  }   
        
    
  int          nCnt          = 0;
  int          nSend         = 0;    
  bool         bChunkLoop    = false;
  unsigned int nReqChunkSize = 0;
  
  // set chunk size
  if(nRequestSize > 1048576) { // 1 mb  
    nReqChunkSize = 1048576;
    szChunk       = new char[nReqChunkSize];
    bChunkLoop    = true;
  }
  else {  
    szChunk       = new char[nRequestSize];
    nReqChunkSize = nRequestSize;
    bChunkLoop    = false;
  }
    
     
  // send loop
  while((nErr != -1) && ((nRet = p_Response->GetBinContentChunk(szChunk, nReqChunkSize, nOffset)) > 0)) 
  { 
    // send HTTP header when the first package is ready
    if(nCnt == 0) {
      sLog << p_Response->GetHeaderAsString() << "partial binary (" << nOffset << " - " << nOffset + nRet << ")";
      CSharedLog::Shared()->Log(L_DEBUG, sLog.str(), __FILE__, __LINE__);
      sLog.str("");
      
      
      #ifdef WIN32
      nErr = send(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), (int)strlen(p_Response->GetHeaderAsString().c_str()), 0);             
      #else
      nErr = send(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), (int)strlen(p_Response->GetHeaderAsString().c_str()), MSG_NOSIGNAL);
      #endif       
    }
    
    // send chunk
    #ifdef WIN32
    nErr = send(p_Session->GetConnection(), szChunk, nRet, 0);      
    int nWSAErr = WSAGetLastError();    
    while(nErr < 0 && nWSAErr == 10035) {
      fuppesSleep(10);
      nErr    = send(p_Session->GetConnection(), szChunk, nRet, 0);        
      nWSAErr = WSAGetLastError();
    }            
    #else
    nErr = send(p_Session->GetConnection(), szChunk, nRet, MSG_NOSIGNAL);
    #endif
           
    nSend += nRet; 
    nCnt++;
    nOffset += nRet;
    
    
    // calc next chunk size
    if(bChunkLoop && nErr >= 0) {
      nRequestSize -= nReqChunkSize;
      if(nRequestSize > 1048576)
        nReqChunkSize = 1048576; // 1 mb
      else
        nReqChunkSize = nRequestSize;        
    }
    
    
    // error/connection handling
    if((nErr < 0) || 
       (
        (p_Response->GetMessageType() == HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT) &&
        (p_Request->GetHTTPConnection() == HTTP_CONNECTION_CLOSE) && 
         !bChunkLoop
       )
      )
    {
      
      // error handling
      if(nErr < 0)
      {
        stringstream sLog;
        #ifdef WIN32
        sLog << "send error :: error no. " << WSAGetLastError() << " " << strerror(WSAGetLastError()) << endl;              
        CSharedLog::Shared()->Error(LOGNAME, sLog.str());     
        #else          
        sLog << "send error :: error no. " << errno << " " << strerror(errno) << endl;
        CSharedLog::Shared()->Error(LOGNAME, sLog.str());
        #endif  
      }
      
      // break transcoding
      if (p_Response->IsTranscoding()) {
        p_Response->BreakTranscoding();
      }
      
      break; 
    } 
      
  } // while
          
  delete [] szChunk;    
  return true;  
}
