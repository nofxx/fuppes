/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            HTTPServer.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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
#include "../Common/Exception.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"
#include "../DeviceSettings/MacAddressTable.h"

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
// but >= 10.2 comes with SO_NOSIGPIPE
// SO_NOSIGPIPE is a setsockopt() option
// and not a send() parameter as MSG_NOSIGNAL
// OpenBSD has none of the options so we
// need to ignore SIGPIPE events
// the signal is created in fuppes_start() (libmain.cpp)
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#ifdef SO_NOSIGPIPE
  #define USE_SO_NOSIGPIPE
#endif
#endif

#include <sys/errno.h>

#endif

using namespace std;
using namespace fuppes;

//fuppesThreadCallback AcceptLoop(void *arg);
//fuppesThreadCallback SessionLoop(void *arg);

/*bool ReceiveRequest(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Request);
bool SendResponse(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Response, CHTTPMessage* p_Request);*/

bool ReceiveRequest(HTTPSession* p_Session, CHTTPMessage* p_Request);
bool SendResponse(HTTPSession* p_Session, CHTTPMessage* p_Response, CHTTPMessage* p_Request);


/** Constructor */
CHTTPServer::CHTTPServer(std::string p_sIPAddress)
:Thread("httpserver")
{
  // init member vars
  m_bIsRunning  = false;
	m_isStarted   = false;
	//accept_thread = (fuppesThread)NULL;
	//fuppesThreadInitMutex(&m_ReceiveMutex);  	
  	
  // create socket
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(m_Socket == -1)
	  throw fuppes::Exception(__FILE__, __LINE__, "failed to create socket");
  
	//#ifdef FUPPES_TARGET_MAC_OSX
	#if defined(BSD)
	/* OS X does not support pthread_cancel
	   so we need to set the socket to non blocking and
		 constantly poll the cancellation state.
		 otherwise fuppes will hang on shutdown
		 same for UDPSocket */
	fuppesSocketSetNonBlocking(m_Socket);
	#endif
	
	#ifdef HAVE_SELECT
	fuppesSocketSetNonBlocking(m_Socket);
	#endif
	
	
	#ifdef USE_SO_NOSIGPIPE
  int use_sigpipe = 1;
  int nOpt = setsockopt(m_Socket, SOL_SOCKET, SO_NOSIGPIPE, &use_sigpipe, sizeof(use_sigpipe));	  
	if(nOpt < 0)
	  throw fuppes::Exception(__FILE__, __LINE__, "failed to setsockopt(SO_NOSIGPIPE)");
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
    throw fuppes::Exception(__FILE__, __LINE__, "failed to setsockopt: SO_REUSEADDR");
  }

  // set local end point
	local_ep.sin_family      = AF_INET;
	local_ep.sin_addr.s_addr = inet_addr(p_sIPAddress.c_str());
	local_ep.sin_port				 = htons(CSharedConfig::Shared()->GetHTTPPort());
	memset(&(local_ep.sin_zero), '\0', 8);
	
  // bind the socket
	nRet = bind(m_Socket, (struct sockaddr*)&local_ep, sizeof(local_ep));	
  if(nRet == -1)
    throw fuppes::Exception(__FILE__, __LINE__, "failed to bind socket to : %s:%d", p_sIPAddress.c_str(), CSharedConfig::Shared()->GetHTTPPort());
  
  // fetch local end point to get port number on random ports
	socklen_t size = sizeof(local_ep);
	getsockname(m_Socket, (struct sockaddr*)&local_ep, &size);

  MacAddressTable::init();
} // CHTTPServer()


/** Destructor */
CHTTPServer::~CHTTPServer()
{
  Stop();
  MacAddressTable::uninit();
} // ~CHTTPServer()


/** Start() */
void CHTTPServer::Start()
{
  m_bBreakAccept = false;
  
  // listen on socket
  int nRet = listen(m_Socket, 0);
  if(nRet == -1) {
    throw fuppes::Exception(__FILE__, __LINE__, "failed to listen on socket");
  }

  HTTPSessionStore::init();
  
  // start accept thread
	start();
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
	stop();
	//wait();
  /*if(accept_thread) {
    fuppesThreadCancel(accept_thread);
    fuppesThreadClose(accept_thread);
    accept_thread = (fuppesThread)NULL;    
  }*/
   
  // kill all remaining connections
  //CleanupSessions();

  // close socket
  fuppesSocketClose(m_Socket);
  m_bIsRunning = false;

  HTTPSessionStore::uninit();
  
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
  //fuppesThreadLockMutex(&m_ReceiveMutex);  
	m_receiveMutex.lock();

  bool bResult = false;
  if(m_pReceiveHandler != NULL) {
    // Parse message
    bResult = m_pReceiveHandler->OnHTTPServerReceiveMsg(pMessageIn, pMessageOut);
  }
    
  //fuppesThreadUnlockMutex(&m_ReceiveMutex);    
	m_receiveMutex.unlock();
  return bResult;
}

/**
 * closes finished session threads
 */
/*void CHTTPServer::CleanupSessions()
{
  if(m_ThreadList.empty())
    return;
 
  // iterate session list ...
  for(m_ThreadListIterator = m_ThreadList.begin(); m_ThreadListIterator != m_ThreadList.end();)
  {
    if(m_ThreadList.empty())
      break;
    */
    // ... and close terminated threads
    /*CHTTPSessionInfo* pInfo = *m_ThreadListIterator;   
    if(pInfo && pInfo->m_bIsTerminated && fuppesThreadClose(pInfo->GetThreadHandle()))
    {           
      std::list<CHTTPSessionInfo*>::iterator tmpIt = m_ThreadListIterator;      
      ++tmpIt;                 
      m_ThreadList.erase(m_ThreadListIterator);
      m_ThreadListIterator = tmpIt;
      delete pInfo; 
      continue;
    } */       
/*
    HTTPSession* pInfo = *m_ThreadListIterator;   
    if(pInfo && pInfo->m_bIsTerminated)
    {           
      std::list<HTTPSession*>::iterator tmpIt = m_ThreadListIterator;      
      ++tmpIt;                 
      m_ThreadList.erase(m_ThreadListIterator);
      m_ThreadListIterator = tmpIt;
      delete pInfo; 
      continue;
    }
		
    m_ThreadListIterator++;    
  }
}*/

/**
 * the HTTPServer's AcceptLoop constantly
 * looks for new incoming connections, 
 * starts a new Session Thread for each
 * new connection and stores them in the session list
 */

//fuppesThreadCallback AcceptLoop(void *arg)
void CHTTPServer::run()
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
	CHTTPServer* pHTTPServer = this; //(CHTTPServer*)arg; 
  
	fuppesSocket nSocket     = pHTTPServer->GetSocket();
	fuppesSocket nConnection = 0;
  
	struct sockaddr_in remote_ep;
	socklen_t size = sizeof(remote_ep);
  
  // log  
	CSharedLog::Log(L_EXT, __FILE__, __LINE__,
    "listening on %s", pHTTPServer->GetURL().c_str());

	pHTTPServer->m_isStarted = true;

	
	#ifdef HAVE_SELECT
	fd_set fds;	
	struct timeval timeout;
	#endif
	
  // loop	
	while(!this->stopRequested()) //!pHTTPServer->m_bBreakAccept)
	{
		#ifdef HAVE_SELECT
		FD_ZERO(&fds);
		FD_SET(nSocket, &fds);
		
		timeout.tv_sec = 2;
		timeout.tv_usec = 0;
 		int sel = select(nSocket + 1, &fds, NULL, NULL, &timeout);

		if(!FD_ISSET(nSocket, &fds)  || sel <= 0)
			continue;
		#endif

    // accept new connection
    nConnection = accept(nSocket, (struct sockaddr*)&remote_ep, &size);   
		if(nConnection == -1) {
		  //#ifdef FUPPES_TARGET_MAC_OSX
			#if defined(BSD)
      pthread_testcancel();
			msleep(10);
			#endif
			continue;
		}
			
    // start session thread ...
    /*CHTTPSessionInfo* pSession = new CHTTPSessionInfo(pHTTPServer, nConnection, remote_ep, pHTTPServer->GetURL());      
    fuppesThread SessionThread = (fuppesThread)NULL;
    fuppesThreadStartArg(SessionThread, SessionLoop, *pSession);
    pSession->SetThreadHandle(SessionThread);*/
		HTTPSession* session = new HTTPSession(pHTTPServer, nConnection, remote_ep, pHTTPServer->GetURL());
		session->start();
    
    // ... and store the thread in the session list
    //pHTTPServer->m_ThreadList.push_back(pSession);
		//pHTTPServer->m_ThreadList.push_back(session);
		
    // cleanup closed sessions
    //pHTTPServer->CleanupSessions();
	}  

	pHTTPServer->m_isStarted = false;
	
  CSharedLog::Log(L_DBG, __FILE__, __LINE__, "exiting accept loop");
  //pHTTPServer->CleanupSessions();
	//fuppesThreadExit();
	
} // AcceptLoop


HTTPSessionStore* HTTPSessionStore::m_instance = NULL;

void HTTPSessionStore::init() // static
{
  if(m_instance != NULL)
    return;

  m_instance = new HTTPSessionStore();
  m_instance->start();  
}

void HTTPSessionStore::uninit() // static
{
  if(m_instance == NULL)
    return;

#warning todo: clear unfinished sessions
  m_instance->stop();  
  cout << m_instance->m_finishedSessions.size() << " finished sessions left" << endl;
  cout << m_instance->m_sessions.size() << " running sessions left" << endl;
  
  delete m_instance;
  m_instance = NULL;
}

void HTTPSessionStore::append(HTTPSession* session) // static
{
  if(m_instance == NULL)
    return;
  
  MutexLocker locker(&m_instance->m_mutex);
  m_instance->m_sessions.push_back(session);
}

void HTTPSessionStore::finished(HTTPSession* session) // static
{
  if(m_instance == NULL)
    return;
  
  MutexLocker locker(&m_instance->m_mutex);
  m_instance->m_sessions.remove(session);
  m_instance->m_finishedSessions.push_back(session);
}


void HTTPSessionStore::run()
{
  while(!stopRequested()) {
    m_mutex.lock();
    
    for(m_finishedSessionsIterator = m_finishedSessions.begin();
        m_finishedSessionsIterator != m_finishedSessions.end();
        m_finishedSessionsIterator++) {
      delete *m_finishedSessionsIterator;          
    }
    m_finishedSessions.clear();

    m_mutex.unlock();
    msleep(2000);
  }
}


/** Session-loop
  */
//fuppesThreadCallback SessionLoop(void *arg)
void HTTPSession::run()
{
  HTTPSessionStore::append(this);
  
	#ifdef USE_SO_NOSIGPIPE	
	int flag = 1;
  int nOpt = setsockopt(m_Connection, SOL_SOCKET, SO_NOSIGPIPE, &flag, sizeof(flag));
  if(nOpt < 0) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "setsockopt(SO_NOSIGPIPE)");
    HTTPSessionStore::finished(this);
    return;
  }
	#endif
  
  
	HTTPSession*          pSession   = this;    
  CHTTPMessage*         pRequest   = new CHTTPMessage();   
  CHTTPMessage*         pResponse  = new CHTTPMessage();
  CHTTPRequestHandler*  pHandler   = new CHTTPRequestHandler(pSession->GetHTTPServerURL());
  
  bool bKeepAlive = true;
  bool bResult    = false;
  
  stringstream sLog;
  
	pRequest->SetRemoteEndPoint(pSession->GetRemoteEndPoint());
  std::string ip = inet_ntoa(pSession->GetRemoteEndPoint().sin_addr);    

  /*
	string mac;
	MacAddressTable::mac(ip, mac);
	cout << "IP: " << ip << " MAC: " << mac << endl;
  */
	
  while(bKeepAlive)
  {  
    // receive HTTP-request
    bResult = ReceiveRequest(pSession, pRequest);  
    if(!bResult) {
      break;
    }

    CSharedLog::Log(L_DBG, __FILE__, __LINE__,
        "REQUEST %s", pRequest->GetMessage().c_str());
    // end receive
    
    // check if requesting IP is allowed to access
    if(CSharedConfig::Shared()->IsAllowedIP(ip)) {
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
  //fuppesThreadExit();
  HTTPSessionStore::finished(this);
}


/** receives a HTTP messag from p_Session's socket and stores it in p_Request */
//bool ReceiveRequest(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Request)
bool ReceiveRequest(HTTPSession* p_Session, CHTTPMessage* p_Request)
{
  int   nBytesReceived = 0;
  int   nRecvCnt = 0;
  char  szBuffer[4096];    
  char* szMsg = NULL;
	char* szHeader = NULL;
	bool	bGotHeader = false;
	size_t	nHeaderPos	= 0;
	
  unsigned int nContentLength = 0;   
 
  bool bDoReceive = true;
  bool bRecvErr   = false; 
  
  //unsigned int nLoopCnt = 0;

#ifdef HAVE_SELECT
	fd_set fds;
#endif
	
  // receive loop
  int nTmpRecv = 0;
  while(bDoReceive)
  {           
    if(nRecvCnt == 30)
      break;
    
#ifdef HAVE_SELECT
		FD_ZERO(&fds);
		FD_SET(p_Session->GetConnection(), &fds);
		
 		int sel = select(p_Session->GetConnection() + 1, &fds, NULL, NULL, NULL);
		if(!FD_ISSET(p_Session->GetConnection(), &fds)  || sel <= 0)
			continue;
		//cout << "SELECT: " << sel << endl;
#endif
		
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
        //nLoopCnt++;        
        fuppesSleep(10);
        continue;
      }
      #else			
			// MAC OS X :: EAGAIN handling
			//#ifdef FUPPES_TARGET_MAC_OSX
			#if defined(BSD)
			if(errno == EAGAIN) {
				//nLoopCnt++;
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
      szMsg = (char*)malloc((nTmpRecv + 1) * sizeof(char*));
      memcpy(szMsg, szBuffer, nTmpRecv);
    }
    // ... append received buffer or ...
    else if((nBytesReceived > 0) && (nTmpRecv > 0)) {
			szMsg = (char*)realloc(szMsg, (nBytesReceived + nTmpRecv + 1) * sizeof(char*));
			memcpy(&szMsg[nBytesReceived], szBuffer, nTmpRecv);
    }    
    // ... close connection gracefully
    else if(nTmpRecv == 0) {      
      fuppesSocketClose(p_Session->GetConnection()); 
      bDoReceive = false;
      break;
    }
		
    // clear receive buffer
    memset(szBuffer, '\0', 4096);
		
    nBytesReceived += nTmpRecv; 
		szMsg[nBytesReceived] = '\0';
		
    // split header and content		
		char*	szPos	= NULL;
		
		// got the full header
		if(((szPos = strstr(szMsg, "\r\n\r\n")) != NULL) && !bGotHeader) {
			
			nHeaderPos = szPos - szMsg;
			szHeader = (char*)malloc((nHeaderPos + 1) * sizeof(char*));
			strncpy(szHeader, szMsg, nHeaderPos);
			szHeader[nHeaderPos] = '\0';

			p_Request->SetHeader(szHeader);
			
			free(szHeader);
			szHeader = NULL;
			bGotHeader = true;
			nHeaderPos += 4;
		}
    // header is incomplete (continue receiving)
    else if(!bGotHeader) {
      nRecvCnt++;      
      continue;
    }

    // check content length
    RegEx rxContentLength("CONTENT-LENGTH: *(\\d+)", PCRE_CASELESS);
    if(rxContentLength.Search(p_Request->GetHeader().c_str())) {
      string sContentLength = rxContentLength.Match(1);    
      nContentLength = ::atoi(sContentLength.c_str());
    }
		
    // check if we received the full content
    if((nBytesReceived - nHeaderPos) < nContentLength) {
      
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
  if((nBytesReceived > 0) && !bRecvErr) {
    // create message
    bResult = p_Request->SetMessage(szMsg);
  }

	if(szMsg) {
		free(szMsg);
	}
	
  return bResult;
} // ReceiveRequest


/** sends p_Response via the socket in p_Session */
//bool SendResponse(CHTTPSessionInfo* p_Session, CHTTPMessage* p_Response, CHTTPMessage* p_Request)
bool SendResponse(HTTPSession* p_Session, CHTTPMessage* p_Response, CHTTPMessage* p_Request)
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
      nErr = fuppesSocketSend(p_Session->GetConnection(), p_Response->GetHeaderAsString().c_str(), p_Response->GetHeaderAsString().length());
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
        string szCRLF = "\r\n";
        fuppesSocketSend(p_Session->GetConnection(), szCRLF.c_str(), strlen(szCRLF.c_str()));
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
    string szCRLF = "0\r\n\r\n";
    fuppesSocketSend(p_Session->GetConnection(), szCRLF.c_str(), strlen(szCRLF.c_str()));    
  }
  
  
  delete [] szChunk;    
  return true;  
}
