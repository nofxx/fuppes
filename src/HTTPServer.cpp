/***************************************************************************
 *            HTTPServer.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <iostream>
#include <sstream>


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
	local_ep.sin_port				 = htons(0); // htons(5080);	
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
  fuppesThreadClose("HTTPSERVER accept", accept_thread);
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

bool CHTTPServer::CallOnReceive(std::string p_sMessage, CHTTPMessage* pMessageOut)
{
  BOOL_CHK_RET_POINTER(pMessageOut);

  if(m_pReceiveHandler != NULL)
  {
    /* Create message */
    CHTTPMessage Message;
    Message.SetMessage(p_sMessage);

    /* Parse message */
    return m_pReceiveHandler->OnHTTPServerReceiveMsg(&Message, pMessageOut);
  }
  else return false;
}

/**
 * closes finished session threads
 */
void CHTTPServer::CleanupSessions()
{
  if(m_ThreadList.size() == 0)
    return;
 
  /* iterate session list */            
  int i = 0;
  for(m_ThreadListIterator = m_ThreadList.begin(); m_ThreadListIterator != m_ThreadList.end(); m_ThreadListIterator++)
  {
    cout << "size = " << m_ThreadList.size() << endl;
    if(m_ThreadList.size() == 0)
      break;
                           
    cout << "i = " << i << endl;
    fflush(stdout);
    i++;
                           
    /* and close terminated threads */
    CHTTPSessionInfo* pInfo = *m_ThreadListIterator;   
    if(pInfo && pInfo->m_bIsTerminated)
    {
      cout << "closing: " << pInfo->GetThreadHandle() << endl;
      fflush(stdout);
      if(fuppesThreadClose("HTTPSERVER - Session", pInfo->GetThreadHandle()))
      {
        m_ThreadList.erase(m_ThreadListIterator);
        delete pInfo;
        m_ThreadListIterator--;
      }
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
			     
      /* start session thread */
      CHTTPSessionInfo* pSession = new CHTTPSessionInfo(pHTTPServer, nConnection);      
      fuppesThread SessionThread = (fuppesThread)NULL;      
      fuppesThreadStartArg(SessionThread, SessionLoop, *pSession);      
      pSession->SetThreadHandle(SessionThread);
      /* and store the thread in the session list */
      pHTTPServer->m_ThreadList.push_back(pSession);
     
      cout << "started: " << pSession->GetThreadHandle() << endl;
      fflush(stdout);
		} 
    
    /* cleanup closed sessions and sleep am moment */
    pHTTPServer->CleanupSessions();
    fuppesSleep(100);
	}  
	  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "exiting accept loop");
  pHTTPServer->CleanupSessions();
	fuppesThreadExit();
}

fuppesThreadCallback SessionLoop(void *arg)
{
  CHTTPSessionInfo* pSession = (CHTTPSessionInfo*)arg;  
  int  nBytesReceived = 0;
  char szBuffer[4096];  
  
  /* receive message */
  nBytesReceived = recv(pSession->GetConnection(), szBuffer, 4096, 0);  
  if(nBytesReceived != -1)			
  {
    /* logging */
    stringstream sMsg;
    sMsg << "bytes received: " << nBytesReceived;
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());
    
    /* zero-terminate buffer */
    szBuffer[nBytesReceived] = '\0';    
    
    /* send response */
    CHTTPMessage ResponseMsg;
    bool bResult = pSession->GetHTTPServer()->CallOnReceive(szBuffer, &ResponseMsg);
    if(bResult)	
    {
      CSharedLog::Shared()->ExtendedLog(LOGNAME, "send response");     
      
      /* send binary content */
      if(ResponseMsg.GetBinContentLength() > 0)
      {
        send(pSession->GetConnection(), ResponseMsg.GetHeaderAsString().c_str(), (int)strlen(ResponseMsg.GetHeaderAsString().c_str()), 0);            
        send(pSession->GetConnection(), ResponseMsg.GetBinContent(), ResponseMsg.GetBinContentLength(), 0);                        
      }
      /* send text content */
      else
      {
        send(pSession->GetConnection(), ResponseMsg.GetMessageAsString().c_str(), (int)strlen(ResponseMsg.GetMessageAsString().c_str()), 0);
      }
      
      /* close connection */
      upnpSocketClose(pSession->GetConnection());
    }
    
  }
  
  /* exit thread */
  pSession->m_bIsTerminated = true;
  fuppesThreadExit();  
}

/* <\PUBLIC> */
