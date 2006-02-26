/***************************************************************************
 *            HTTPServer.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "HTTPServer.h"
#include "HTTPMessage.h"
#include "SharedLog.h"
#include "SharedConfig.h"
#include "RegEx.h"

#include <iostream>
#include <sstream>
#ifndef WIN32
#include <errno.h>
#endif

using namespace std;

/*===============================================================================
 CONSTANTS
===============================================================================*/

const string LOGNAME = "HTTPServer";

/*===============================================================================
 CALLBACKS
===============================================================================*/

fuppesThreadCallback AcceptLoop(void *arg);
fuppesThreadCallback SessionLoop(void *arg);

fuppesThreadMutex ReceiveMutex;

/*===============================================================================
 CLASS CHTTPServer
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CHTTPServer::CHTTPServer(std::string p_sIPAddress)
{
	accept_thread = (fuppesThread)NULL;
	fuppesThreadInitMutex(&ReceiveMutex);
  	
  /* create socket */
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(m_Socket == -1)  
    CSharedLog::Shared()->Error(LOGNAME, "creating socket");	
  
  /* set socket non blocking */
  if(!fuppesSocketSetNonBlocking(m_Socket))
    CSharedLog::Shared()->Error(LOGNAME, "fuppesSocketSetNonBlocking");
  
  /* set loacl end point */
	local_ep.sin_family      = AF_INET;
	local_ep.sin_addr.s_addr = inet_addr(p_sIPAddress.c_str());
	local_ep.sin_port				 = htons(CSharedConfig::Shared()->GetHTTPPort());
	memset(&(local_ep.sin_zero), '\0', 8); // fill the rest of the structure with zero
	
  /* try to bind the socket */
	int nRet = bind(m_Socket, (struct sockaddr*)&local_ep, sizeof(local_ep));	
  if(nRet == -1)
    CSharedLog::Shared()->Error(LOGNAME, "bind()");  
  
  /* get local end point to retreive port number on random ports */
	socklen_t size = sizeof(local_ep);
	getsockname(m_Socket, (struct sockaddr*)&local_ep, &size);	
}

CHTTPServer::~CHTTPServer()
{
  Stop();
	fuppesThreadDestroyMutex(&ReceiveMutex);
}

/*===============================================================================
 COMMON
===============================================================================*/

void CHTTPServer::Start()
{
  m_bBreakAccept = false;
  
  /* listen on socket */
  int nRet = listen(m_Socket, 0);
  if(nRet == -1)
    CSharedLog::Shared()->Error(LOGNAME, "listen()");    

  /* start accept thread */
  fuppesThreadStart(accept_thread, AcceptLoop);
}

void CHTTPServer::Stop()
{   
  /* stop accept thread */
  m_bBreakAccept = true;  
  fuppesThreadClose(accept_thread);
  accept_thread = (fuppesThread)NULL;
    
  /* close socket */
  upnpSocketClose(m_Socket);
}

upnpSocket CHTTPServer::GetSocket()
{
  return m_Socket;
}

std::string CHTTPServer::GetURL()
{
  stringstream result;
  result << inet_ntoa(local_ep.sin_addr) << ":" << ntohs(local_ep.sin_port);
  return result.str();
}

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

bool CHTTPServer::SetReceiveHandler(IHTTPServer* pHandler)
{
	BOOL_CHK_RET_POINTER(pHandler);
  
  /* Save pointer to message handler */
  m_pReceiveHandler = pHandler;

  return true;
}

bool CHTTPServer::CallOnReceive(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  BOOL_CHK_RET_POINTER(pMessageOut);

  if(m_pReceiveHandler != NULL)
  {
    /* Parse message */
    return m_pReceiveHandler->OnHTTPServerReceiveMsg(pMessageIn, pMessageOut);
  }
  else 
    return false;  
}

/**
 * closes finished session threads
 */
void CHTTPServer::CleanupSessions()
{
  if(m_ThreadList.empty())
    return;
 
  /* iterate session list */  
  for(m_ThreadListIterator = m_ThreadList.begin(); m_ThreadListIterator != m_ThreadList.end();)
  {
    if(m_ThreadList.empty())
      break;
    
    /* and close terminated threads */
    CHTTPSessionInfo* pInfo = *m_ThreadListIterator;   
    if(pInfo && pInfo->m_bIsTerminated)
    {        
      if(fuppesThreadClose(pInfo->GetThreadHandle()))     
      {       
        std::list<CHTTPSessionInfo*>::iterator tmpIt = m_ThreadListIterator;      
        ++tmpIt;                 
        m_ThreadList.erase(m_ThreadListIterator);
        m_ThreadListIterator = tmpIt;
        delete pInfo;        
      }      
    }
    else
    {
       m_ThreadListIterator++;
    }
  }    
}

/*===============================================================================
 CALLBACKS
===============================================================================*/

fuppesThreadCallback AcceptLoop(void *arg)
{
	CHTTPServer* pHTTPServer = (CHTTPServer*)arg;
  stringstream sLog;
  sLog << "listening on" << pHTTPServer->GetURL();  
	CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());
  
	upnpSocket nSocket     = pHTTPServer->GetSocket();			
	upnpSocket nConnection = 0;
  
	struct sockaddr_in remote_ep;
	socklen_t size = sizeof(remote_ep);
	  
	while(!pHTTPServer->m_bBreakAccept)
	{
    /* accept new connections */
    nConnection = accept(nSocket, (struct sockaddr*)&remote_ep, &size);   
		if(nConnection != -1)      
		{	
      stringstream sMsg;
      sMsg << "new connection from " << inet_ntoa(remote_ep.sin_addr) << ":" << ntohs(remote_ep.sin_port);
			CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());
      //cout << sMsg.str() << endl;
      
      /* start session thread */
      CHTTPSessionInfo* pSession = new CHTTPSessionInfo(pHTTPServer, nConnection);      
      fuppesThread SessionThread = (fuppesThread)NULL;      
      fuppesThreadStartArg(SessionThread, SessionLoop, *pSession);      
      pSession->SetThreadHandle(SessionThread);
      /* and store the thread in the session list */
      pHTTPServer->m_ThreadList.push_back(pSession);
		} 
    
    /* cleanup closed sessions and sleep am moment */
    pHTTPServer->CleanupSessions();     
    fuppesSleep(50);
	}  
	  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "exiting accept loop");
  pHTTPServer->CleanupSessions();
	fuppesThreadExit();
}

fuppesThreadCallback SessionLoop(void *arg)
{
  CHTTPSessionInfo* pSession = (CHTTPSessionInfo*)arg;  
  int   nBytesReceived = 0;
  int   nRecvCnt = 0;
  char  szBuffer[4096];    
  char* szMsg = NULL;
    
  unsigned int nContentLength = 0;
  CHTTPMessage ReceivedMessage;        
  bool bDoReceive = true;
  
  /* receive message */
  int nTmpRecv = 0;
  while(bDoReceive)
  {           
    if(nRecvCnt == 30)
      break;
                   
    /* receive */     
    nTmpRecv = recv(pSession->GetConnection(), szBuffer, 4096, 0);
    //cout << "new: " << nTmpRecv << " have: " << nBytesReceived << endl;
    //fflush(stdout);
    if(nTmpRecv < 0)
    {
      stringstream sLog;
      #ifdef WIN32      
      sLog << "error no. " << WSAGetLastError() << " " << strerror(WSAGetLastError()) << endl;
      CSharedLog::Shared()->Error(LOGNAME, sLog.str());           
      cout << "bytes received: " << nBytesReceived << endl;
      if(WSAGetLastError() != WSAEWOULDBLOCK)
      {
        bDoReceive = false;
        break;      
      }      
      #else
      sLog << "error no. " << errno << " " << strerror(errno) << endl;
      cout << "bytes received: " << nBytesReceived << endl;
      CSharedLog::Shared()->Error(LOGNAME, sLog.str());     
      bDoReceive = false;
      break;      
      #endif
    }                  
    
    /* append received buffer */
    if((nBytesReceived > 0) && (nTmpRecv > 0))
    {
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
    else if ((nBytesReceived == 0) && (nTmpRecv > 0))
    {
      szMsg = new char[nTmpRecv + 1];
      memcpy(szMsg, szBuffer, nTmpRecv);
      szMsg[nTmpRecv] = '\0';
    }
    else if(nTmpRecv == 0)
    {
      cout << "connection gracefully closed" << endl;
      fflush(stdout);
      /* close connection */
      upnpSocketClose(pSession->GetConnection()); 
      bDoReceive = false;
      break;
    }
    
    /* clear receive buffer */
    memset(szBuffer, '\0', 4096);
    
    nBytesReceived += nTmpRecv; 
    string sMsg = szMsg;     
    
    /* split header and content */
    std::string sHeader  = "";
    std::string sContent = "";
    unsigned int nPos = sMsg.find("\r\n\r\n");  
    if(nPos != string::npos)
    {
      /*cout << "full header" << endl;
      fflush(stdout);*/
            
      sHeader = sMsg.substr(0, nPos);
      nPos += string("\r\n\r\n").length();
      sContent = sMsg.substr(nPos, sMsg.length() - nPos);
    }
    else
    {      
      /*CSharedLog::Shared()->Warning(LOGNAME, "did not received the full header.");
      cout << sMsg << endl;
      cout << "pos: " << nPos << " end: " << string::npos << endl;*/
      //fuppesSleep(10);
      nRecvCnt++;
      continue;
    }
                
    /*cout << sHeader << endl << endl;
    fflush(stdout);*/

    
    /* read content length */
    RegEx rxContentLength("CONTENT-LENGTH: *(\\d+)", PCRE_CASELESS);
    if(rxContentLength.Search(sHeader.c_str()))
    {
      string sContentLength = rxContentLength.Match(1);    
      nContentLength = std::atoi(sContentLength.c_str());
    }      

    /* check if we received the full content */
    if(sContent.length() < nContentLength)
    {
      /*stringstream sLog;
      sLog << "received less data then given in CONTENT-LENGTH. should: " << nContentLength << " have: " << sContent.length();
      CSharedLog::Shared()->Warning(LOGNAME, sLog.str());*/
      continue;
    }
    else
    {
      /* full content */
      bDoReceive = false;
      /*cout << "full request" << endl;
      cout << sMsg << endl;*/
    }
                  
    if(nTmpRecv == 0)      
      nRecvCnt++;    
  }
  /* end receive */

  /* build received message */
  bool bResult = false;  
  if(nBytesReceived > 0)			
  {
    /* logging */
    stringstream sMsg;
    sMsg << "bytes received: " << nBytesReceived;
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());

    /* Create message */    
    bResult = ReceivedMessage.SetMessage(szMsg);
    //cout << szMsg << endl << endl;
  }

  /* build response message and send it */
  if(bResult)
  {
    /* build response */
    CHTTPMessage ResponseMsg;  	  	
    bResult = pSession->GetHTTPServer()->CallOnReceive(&ReceivedMessage, &ResponseMsg);  	
    if(!bResult)
      CSharedLog::Shared()->Error(LOGNAME, "handling HTTP message");      
    if(bResult)
    {
      //cout << ResponseMsg.GetMessageAsString() << endl << endl;
      
      /*cout << ResponseMsg.GetHeaderAsString() << endl;
      fflush(stdout);*/
      if(!ResponseMsg.IsChunked())
      { 
        /* send complete binary stream */
        if(ResponseMsg.GetBinContentLength() > 0) 
        { 
          CSharedLog::Shared()->ExtendedLog(LOGNAME, "sending non chunked binary");
          send(pSession->GetConnection(), ResponseMsg.GetHeaderAsString().c_str(), (int)strlen(ResponseMsg.GetHeaderAsString().c_str()), 0);             
          CSharedLog::Shared()->DebugLog(LOGNAME, ResponseMsg.GetHeaderAsString());
          CSharedLog::Shared()->ExtendedLog(LOGNAME, "header send");
          #ifdef WIN32
          send(pSession->GetConnection(), ResponseMsg.GetBinContent(), ResponseMsg.GetBinContentLength(), 0);          
          #else
          send(pSession->GetConnection(), ResponseMsg.GetBinContent(), ResponseMsg.GetBinContentLength(), MSG_NOSIGNAL);
          #endif
          CSharedLog::Shared()->ExtendedLog(LOGNAME, "content send");
        } 
        /* send text message */
        else 
        {           
          CSharedLog::Shared()->ExtendedLog(LOGNAME, "sending plain text");          
          send(pSession->GetConnection(), ResponseMsg.GetMessageAsString().c_str(), (int)strlen(ResponseMsg.GetMessageAsString().c_str()), 0); 
          CSharedLog::Shared()->DebugLog(LOGNAME, ResponseMsg.GetMessageAsString());
        } 
      }
      /* send chunked message */
      else 
      {         
        
        unsigned int nOffset = ReceivedMessage.GetRangeStart(); 
        char szChunk[65536];
        unsigned int nRequestSize = 65536;
        int nErr = 0;
        if(ReceivedMessage.GetRangeEnd() > 0)
        {          
          nRequestSize = ReceivedMessage.GetRangeEnd() - ReceivedMessage.GetRangeStart() + 1;
          ResponseMsg.SetMessageType(HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT);          
        }
        //cout << "LENG: " << ResponseMsg.GetBinContentLength() << endl;
        
        ResponseMsg.SetRangeStart(ReceivedMessage.GetRangeStart());
        ResponseMsg.SetRangeEnd(ReceivedMessage.GetRangeEnd());
      
        
     /*   cout << ResponseMsg.GetHeaderAsString() << endl;
        fflush(stdout);*/
        
        unsigned int nRet = 0; 

        CSharedLog::Shared()->ExtendedLog(LOGNAME, "sending chunked binary");
        CSharedLog::Shared()->DebugLog(LOGNAME, ResponseMsg.GetHeaderAsString());
        
        /* send header */        
        if(nErr != -1)
          nErr = send(pSession->GetConnection(), ResponseMsg.GetHeaderAsString().c_str(), (int)strlen(ResponseMsg.GetHeaderAsString().c_str()), 0);             
       
        //cout << ResponseMsg.GetHeaderAsString() << endl;
        
        if(nErr == -1)
          cout << "[ERROR] send header" << endl;
        
        int nCnt = 0;
        int nSend = 0;
        /*cout << "get content" << endl;
        fflush(stdout);*/
        while((nErr != -1) && ((nRet = ResponseMsg.GetBinContentChunk(szChunk, nRequestSize, nOffset)) > 0)) 
        {             
          //cout << "got content" << endl;          

          /*cout << "read binary" << endl;
          cout << "start: " << nOffset << endl;
          cout << "requested: " << nRequestSize << endl;
          cout << "end: " << nRet << endl;
          fflush(stdout);        */
          
          #ifdef WIN32
          nErr = send(pSession->GetConnection(), szChunk, nRet, 0);    
          int nWSAErr = WSAGetLastError();    
          while(nErr < 0 && nWSAErr == 10035)
          {
            nErr    = send(pSession->GetConnection(), szChunk, nRet, 0);
            nWSAErr = WSAGetLastError();
          }            
          #else
          nErr = send(pSession->GetConnection(), szChunk, nRet, MSG_NOSIGNAL);
          #endif
                 

          nSend += nRet;            
          nCnt++;
          nOffset += nRet;            
         
          if((nErr < 0) || ((ResponseMsg.GetMessageType() == HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT) && (ReceivedMessage.GetHTTPConnection() == HTTP_CONNECTION_CLOSE)))
          {
            if(nErr < 0)
            {
              stringstream sLog;
              #ifdef WIN32
              sLog << "error no. " << WSAGetLastError() << " " << strerror(WSAGetLastError()) << endl;
              #else              
              sLog << "error no. " << errno << " " << strerror(errno) << endl;
              #endif
              CSharedLog::Shared()->Error(LOGNAME, sLog.str());              
            }
            /*else
            {
              cout << "connection closed  (send done)" << endl;
            }*/
            /*cout << "offset: " << nOffset << endl;
            cout << "send: " << nSend << endl;
            cout << "length: " << ResponseMsg.GetBinContentLength() << endl;*/
            
            //fflush(stdout); 
            
            if (ResponseMsg.m_bIsTranscoding)
            {
              ResponseMsg.m_bBreakTranscoding = true; 
              fuppesSleep(500); /* wait for the transcoding thread to end */
            }
            break; 
          }                         
 
            /*cout << "offset: " << nOffset << endl;
            cout << "send: " << nSend << endl;
            cout << "length: " << ResponseMsg.GetBinContentLength() << endl;            */
            //cout << "send no.: " << nCnt << endl;
            
          } /* while */
            
      } /* else */
            
    }
  }  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "done sending response");
  
  
  /* close connection */
  upnpSocketClose(pSession->GetConnection());    
  //cout << "connection closed" << endl;
  
  /* exit thread */
  pSession->m_bIsTerminated = true;
  fuppesThreadExit();  
}

/* <\PUBLIC> */
