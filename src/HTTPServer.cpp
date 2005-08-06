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
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	local_ep.sin_family      = PF_INET;
	local_ep.sin_addr.s_addr = inet_addr(p_sIPAddress.c_str());
	local_ep.sin_port				 = htons(0);
	
	bind(sock, (struct sockaddr*)&local_ep, sizeof(local_ep));	
	socklen_t size = sizeof(local_ep);
	getsockname(sock, (struct sockaddr*)&local_ep, &size);	
}

CHTTPServer::~CHTTPServer()
{
}

/*===============================================================================
 COMMON
===============================================================================*/

void CHTTPServer::Start()
{
  do_break = false;			
  listen(sock, 3);

  /* Start thread */
  fuppesThreadStart(accept_thread, AcceptLoop);
}

void CHTTPServer::Stop()
{
  /* Stop thread */
  fuppesThreadClose(accept_thread, 2000);
}

upnpSocket CHTTPServer::GetSocket()
{
  return sock;
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
	ASSERT(NULL != pHandler);
  if(NULL == pHandler)
    return false;
  
  /* Save pointer to message handler */
  m_pReceiveHandler = pHandler;

  return true;
}

bool CHTTPServer::CallOnReceive(std::string p_sMessage, CHTTPMessage* pMessageOut)
{
  ASSERT(NULL != pMessageOut);
  if(NULL == pMessageOut)
    return false;

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
	
	for(;;)
	{
		/* T.S.TODO: Error on exit here */
    nConnection = accept(nSocket, (struct sockaddr*)&remote_ep, &size);
		if(nConnection != -1)      
		{	
      stringstream sMsg;
      sMsg << "new connection from " << inet_ntoa(remote_ep.sin_addr) << ":" << ntohs(remote_ep.sin_port);
			CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());
			
      /* Start session thread */
      CHTTPSessionInfo pSession(pHTTPServer, nConnection);      
      fuppesThread Session = (fuppesThread)NULL;
      /* T.S.TODO: Where do we need to exit thread??? */
      /* uv :: we put the thread handles in a list (e.g vector)
               and build a garbage-collecting thread, that
               closes all finished threads */
      fuppesThreadStartArg(Session, SessionLoop, pSession);
		}
	}	
	
	return 0;
}

fuppesThreadCallback SessionLoop(void *arg)
{
  CHTTPSessionInfo pSession = *(CHTTPSessionInfo*)arg;  
  int  nBytesReceived = 0;
  char szBuffer[4096];
  
  nBytesReceived = recv(pSession.GetConnection(), szBuffer, 4096, 0); /* MSG_DONTWAIT */
  if(nBytesReceived != -1)			
  {
    stringstream sMsg;
    sMsg << "bytes received: " << nBytesReceived;
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());
    szBuffer[nBytesReceived] = '\0';
    //cout << szBuffer << endl;
    
    CHTTPMessage ResponseMsg;
    bool fRet = pSession.GetHTTPServer()->CallOnReceive(szBuffer, &ResponseMsg);
    if(true == fRet)				
    {
      CSharedLog::Shared()->ExtendedLog(LOGNAME, "sending response");
      //cout << pResponse->GetMessageAsString() << endl;
      
      if(ResponseMsg.GetBinContentLength() > 0)
      {
        send(pSession.GetConnection(), ResponseMsg.GetHeaderAsString().c_str(), (int)strlen(ResponseMsg.GetMessageAsString().c_str()), 0);            
        send(pSession.GetConnection(), ResponseMsg.GetBinContent(), ResponseMsg.GetBinContentLength(), 0);                        
      }
      else            
        send(pSession.GetConnection(), ResponseMsg.GetMessageAsString().c_str(), (int)strlen(ResponseMsg.GetMessageAsString().c_str()), 0);
      
      
      upnpSocketClose(pSession.GetConnection());      
      CSharedLog::Shared()->ExtendedLog(LOGNAME, "done");
    }
    
  }
  
  return 0;  
}

/* <\PUBLIC> */
