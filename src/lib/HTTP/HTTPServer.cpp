/***************************************************************************
 *            HTTPServer.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "HTTPServer.h"
#include "HTTPMessage.h"
#include "HTTPRequestHandler.h"
#include "CommonFunctions.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"
#include "../Common/RegEx.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"

#include <iostream>
#include <sstream>
#ifndef WIN32
#include <errno.h>
#endif


// the max buffer size for files directly served
// from the local file system
#define MAX_BUFFER_SIZE 1048576 // 1 mb

// the max buffer size for transcoded files
#define MAX_TRANSCODING_BUFFER_SIZE 65536 // 64 kbyte



#ifndef WIN32

// mac os x has no MSG_NOSIGNAL 
// but >= 10.2 comes with SO_SIGPIPE
// SO_NOSIGPIPE is a setsockopt() option
// and not a send() parameter as MSG_NOSIGNAL
#ifndef MSG_NOSIGNAL
#define USE_SO_NOSIGPIPE
#define MSG_NOSIGNAL 0
#endif

#include <sys/errno.h>

#endif

using namespace std;

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
	
	#ifdef USE_SO_NOSIGPIPE
  int use_sigpipe = 1;
  int nOpt = setsockopt(m_Socket, SOL_SOCKET, SO_NOSIGPIPE, &use_sigpipe, sizeof(use_sigpipe));	  
	if(nOpt < 0)
	  throw EException("failed to setsockopt(SO_NOSIGPIPE)", __FILE__, __LINE__);
  #endif 

	
	// set socket option SO_REUSEADDR so restarting fuppes with
	// a fixed http port will not lead to a bind error
  int nRet  = 0;
  #ifdef WIN32  
  bool bOptVal = true;
  nRet = setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, sizeof(bool));
  #else
  int flag = 1;
  nRet = setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
  #endif
  if(nRet == -1) {
    throw EException("failed to setsockopt: SO_REUSEADDR", __FILE__, __LINE__);
  }

  // set local end point
	local_ep.sin_family      = AF_INET;
	local_ep.sin_addr.s_addr = inet_addr(p_sIPAddress.c_str());
	local_ep.sin_port				 = htons(CSharedConfig::Shared()->GetHTTPPort());
	memset(&(local_ep.sin_zero), '\0', 8);
	
  // bind the socket
	nRet = bind(m_Socket, (struct sockaddr*)&local_ep, sizeof(local_ep));	
  if(nRet == -1)
    throw EException(__FILE__, __LINE__, "failed to bind socket to : %s:%d", p_sIPAddress.c_str(), CSharedConfig::Shared()->GetHTTPPort());
  
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
    throw EException("failed to listen on socket", __FILE__, __LINE__);
  }

  // start accept thread
  fuppesThreadStart(accept_thread, AcceptLoop);  
  m_bIsRunning = true;
  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "HTTPServer started");
} // Start()


/** Stop() */
void CHTTPServer::Stop()
{   
  if(!m_bIsRunning)
    return;

  // stop accept thread
  m_bBreakAccept = true;
  if(accept_thread) {
    fuppesThreadCancel(accept_thread);
    fuppesThreadClose(accept_thread);
    accept_thread = (fuppesThread)NULL;    
  }
   
  // kill all remaining connections
  CleanupSessions();

  // close socket
  fuppesSocketClose(m_Socket);
  m_bIsRunning = false;
  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "HTTPServer stopped");
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
	CSharedLog::Log(L_EXT, __FILE__, __LINE__,
    "listening on %s", pHTTPServer->GetURL().c_str());
  
  // loop	
	while(!pHTTPServer->m_bBreakAccept)
	{
  
    // accept new connection
    nConnection = accept(nSocket, (struct sockaddr*)&remote_ep, &size);   
		if(nConnection == -1) {
		  #ifdef FUPPES_TARGET_MAC_OSX
      pthread_testcancel();
			fuppesSleep(10);
			#endif
			continue;
		}
  
		#ifdef USE_SO_NOSIGPIPE	
		int flag = 1;
    int nOpt = setsockopt(nConnection, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(flag));	  
	  if(nOpt < 0)
	    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "setsockopt(SO_NOSIGPIPE)");
		#endif
			
    // start session thread ...
    CHTTPSessionInfo* pSession = new CHTTPSessionInfo(pHTTPServer, nConnection, remote_ep, pHTTPServer->GetURL());      
    fuppesThread SessionThread = (fuppesThread)NULL;
    fuppesThreadStartArg(SessionThread, SessionLoop, *pSession);
    pSession->SetThreadHandle(SessionThread);
    
    // ... and store the thread in the session list
    pHTTPServer->m_ThreadList.push_back(pSession);
		
    // cleanup closed sessions
    pHTTPServer->CleanupSessions();
	}  

  CSharedLog::Log(L_DBG, __FILE__, __LINE__, "exiting accept loop");
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
  CHTTPRequestHandler* pHandler  = new CHTTPRequestHandler(pSession->GetHTTPServerURL());
  
  bool bKeepAlive = true;
  bool bResult    = false;
  
  stringstream sLog;
  
  while(bKeepAlive)
  {  
    // receive HTTP-request
		pRequest->SetRemoteEndPoint(pSession->GetRemoteEndPoint());
    bResult = ReceiveRequest(pSession, pRequest);  
    if(!bResult) {
      break;
    }

    CSharedLog::Log(L_DBG, __FILE__, __LINE__,
        "REQUEST %s", pRequest->GetMessage().c_str());
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
      CSharedLog::Log(L_DBG, __FILE__, __LINE__, " error sending HTTP message");
      break;
    }
    // end send response
    
    bKeepAlive = false;
  }  
  
  // close connection
  fuppesSocketClose(pSession->GetConnection());
  
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
      CSharedLog::Log(L_DBG, __FILE__, __LINE__, sLog.str().c_str());
      
      // WIN32 :: WSAEWOULDBLOCK handling
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
			// MAC OS X :: EAGAIN handling
			#ifdef FUPPES_TARGET_MAC_OSX
			if(errno == EAGAIN) {
				nLoopCnt++;
				fuppesSleep(10);
				continue;
			}
			else {
				bDoReceive = false;
        bRecvErr   = true;
        break;  
			}			
			// non blocking
      #else      
      bDoReceive = false;
      bRecvErr   = true;
      break;      
      #endif
      
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
      
      p_Request->SetHeader(sHeader);      
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
      
      // xbox 360: sends a content length of 3 but only 2 bytes of data
      if((p_Request->DeviceSettings()->Xbox360Support()) && (nContentLength == 3)) {
        bDoReceive = false;
      }
      else {      
        // content incomplete (continue receiving)
        continue;
      }
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
    //stringstream sMsg;
    //sMsg << "bytes received: " << nBytesReceived;
    //CSharedLog::Shared()->Log(L_EXTENDED, sMsg.str(), __FILE__, __LINE__);

    // create message
    bResult = p_Request->SetMessage(szMsg); 
  }
  
  return bResult;
} // ReceiveRequest


/** sends p_Response via the socket in p_Session */
bool SendResponse(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Response, CHTTPMessage* p_Request)
{
  // local vars
	int nRet = 0;       
  stringstream sLog;

  // send text message
  if(!p_Response->IsBinary())
  { 
	  // log
    CSharedLog::Log(L_DBG, __FILE__, __LINE__, "send response %s\n", p_Response->GetMessageAsString().c_str());
        
    // send
    nRet = fuppesSocketSend(p_Session->GetConnection(), p_Response->GetMessageAsString().c_str(), (int)strlen(p_Response->GetMessageAsString().c_str()));
    #ifdef WIN32 
    if(nRet == -1) {
      stringstream sLog;            
      sLog << "send error :: error no. " << WSAGetLastError() << " " << strerror(WSAGetLastError()) << endl;              
      CSharedLog::Log(L_DBG, __FILE__, __LINE__, sLog.str().c_str());
    }
    #else
    if(nRet == -1) {
      stringstream sLog;       
      sLog << "send error :: error no. " << errno << " " << strerror(errno) << endl;
      CSharedLog::Log(L_DBG, __FILE__, __LINE__, sLog.str().c_str());
    }          
    #endif
    
    return true;
  }
  // end send text message
  
  
  
  // send binary
  //unsigned int nOffset      = 0;
  fuppes_off_t  nOffset       = 0;
  char*         szChunk       = NULL;
  unsigned int  nRequestSize  = MAX_BUFFER_SIZE;
  int           nErr          = 0;    
    
    
  // calculate response ranges and request size
	
	// set response start range and the read offset
	// to the request's start range
	p_Response->SetRangeStart(p_Request->GetRangeStart());
	nOffset = p_Request->GetRangeStart();
	
	// partial request
  if((p_Request->GetRangeStart() > 0) || (p_Request->GetRangeEnd() > 0))
	{
    // not eof	
	  if(!p_Response->IsTranscoding() && (p_Request->GetRangeEnd() < p_Response->GetBinContentLength())) {
			// RANGE: BYTES=[n]-n+1
		  // the request contains a range end value
      if(p_Request->GetRangeEnd() > p_Request->GetRangeStart())
        nRequestSize = p_Request->GetRangeEnd() - p_Request->GetRangeStart() + 1;
      // RANGE: BYTES=n-
			// the request does NOT conatin a range and value
			else
        nRequestSize = p_Response->GetBinContentLength() - p_Request->GetRangeStart() + 1;
		}
		else if(p_Response->IsTranscoding()) {
     
      if(p_Request->GetRangeEnd() > p_Request->GetRangeStart())
        nRequestSize = p_Request->GetRangeEnd() - p_Request->GetRangeStart() + 1;
      // RANGE: BYTES=n-
			// the request does NOT conatin a range and value
			else
        nRequestSize = p_Response->GetBinContentLength() - p_Request->GetRangeStart() + 1;
    }    
    
		// set HTTP 206 partial content
    p_Response->SetMessageType(HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT);          
  }
	
	// set range end
  if(p_Request->GetRangeEnd() > 0) {
    p_Response->SetRangeEnd(p_Request->GetRangeEnd());
  }
	else {
    p_Response->SetRangeEnd(p_Response->GetBinContentLength());
  }

    
    
  // send header if it is a HEAD response 
  // or the start range is greater than the content and return
  if((nErr != -1) && 
     ((p_Request->GetMessageType() == HTTP_MESSAGE_TYPE_HEAD) ||
      ((p_Request->GetRangeStart() > 0) && 
       (p_Request->GetRangeStart() >= p_Response->GetBinContentLength())
      ))) 
  {
	  // log
		CSharedLog::Log(L_DBG, __FILE__, __LINE__, "send header: %s\n", p_Response->GetHeaderAsString().c_str());
    // send
    nErr = fuppesSocketSend(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), (int)strlen(p_Response->GetHeaderAsString().c_str()));
           
    return (nErr > 0);
  }   
       
    
  int          nCnt          = 0;
  int          nSend         = 0;    
  bool         bChunkLoop    = false;
  unsigned int nReqChunkSize = 0;
    
  if (p_Response->IsTranscoding()) {    
    nReqChunkSize = MAX_TRANSCODING_BUFFER_SIZE;    
  }
  else {    
    nReqChunkSize = MAX_BUFFER_SIZE;    
  }  
  
  // set chunk size
  if(nRequestSize > nReqChunkSize) {    
    szChunk       = new char[nReqChunkSize];
    bChunkLoop    = true;
  }
  else {  
    szChunk       = new char[nRequestSize];
    nReqChunkSize = nRequestSize;
    bChunkLoop    = false;
  }

  
  
  // send loop 
  while((nErr != -1) && 
        ((nRet = p_Response->GetBinContentChunk(szChunk, nReqChunkSize, nOffset)) > 0)
        )
  {
    // send HTTP header when the first package is ready
    if(nCnt == 0) {      
      // send
      nErr = fuppesSocketSend(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), (int)strlen(p_Response->GetHeaderAsString().c_str()));   
      CSharedLog::Log(L_DBG, __FILE__, __LINE__, "send header %s\n", p_Response->GetHeaderAsString().c_str());
    }

    
    if(nErr > 0) {
      CSharedLog::Log(L_DBG, __FILE__, __LINE__,
        "send binary data (bytes %llu to %llu from %llu)", nOffset, nOffset + nRet, p_Response->GetBinContentLength());
      
      if(p_Response->GetTransferEncoding() == HTTP_TRANSFER_ENCODING_CHUNKED) {
        char szSize[10];
        sprintf(szSize, "%X\r\n", nRet);
        fuppesSocketSend(p_Session->GetConnection(), szSize, strlen(szSize));
      }     

      nErr = fuppesSocketSend(p_Session->GetConnection(), szChunk, nRet);    

      if(p_Response->GetTransferEncoding() == HTTP_TRANSFER_ENCODING_CHUNKED) {
        char* szCRLF = "\r\n";
        fuppesSocketSend(p_Session->GetConnection(), szCRLF, strlen(szCRLF));
      }
      
    }        
   
    nSend += nRet; 
    nCnt++;
    nOffset += nRet;

    // calc next chunk size
    if(bChunkLoop && nErr >= 0) {      
      if(nRequestSize > nReqChunkSize)
        nRequestSize -= nReqChunkSize;      
      
      if(nRequestSize < nReqChunkSize)
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
        CSharedLog::Log(L_EXT, __FILE__, __LINE__, sLog.str().c_str());
        #else          
        sLog << "send error :: error no. " << errno << " " << strerror(errno) << endl;
        CSharedLog::Log(L_EXT, __FILE__, __LINE__, sLog.str().c_str());
        #endif  
      }
      
      // break transcoding
      if (p_Response->IsTranscoding()) {
        p_Response->BreakTranscoding();
      }
      
      break; 
    } 

  } // while
  
  // send last chunk
  if((nErr > 0) && (p_Response->GetTransferEncoding() == HTTP_TRANSFER_ENCODING_CHUNKED)) {
    char* szCRLF = "0\r\n\r\n";
    fuppesSocketSend(p_Session->GetConnection(), szCRLF, strlen(szCRLF));    
  }
  
  
  delete [] szChunk;    
  return true;  
}
