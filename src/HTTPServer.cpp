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
#include "SharedConfig.h"
#include "RegEx.h"

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
    /*cout << "call on receive" << endl;
    fflush(stdout);*/
  
    /* Parse message */
    return m_pReceiveHandler->OnHTTPServerReceiveMsg(pMessageIn, pMessageOut);
  }
  else return false;  
}

/**
 * closes finished session threads
 */
void CHTTPServer::CleanupSessions()
{
  if(m_ThreadList.empty())
    return;
 
  /* iterate session list */  
  for(m_ThreadListIterator = m_ThreadList.begin(); m_ThreadListIterator != m_ThreadList.end(); m_ThreadListIterator++)
  {
    if(m_ThreadList.empty())
      break;
    
    /* and close terminated threads */
    CHTTPSessionInfo* pInfo = *m_ThreadListIterator;   
    if(pInfo && pInfo->m_bIsTerminated)
    {
      if(fuppesThreadClose(pInfo->GetThreadHandle()))
      {
        m_ThreadListIterator = m_ThreadList.erase(m_ThreadListIterator);
        delete pInfo;
        //m_ThreadListIterator--;
      }
      else
      {
          cout << "ERROR " << endl;
          fflush(stdout);
          break;
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
  int   nTryCnt  = 0;
  int   nRecvCnt = 0;
  char  szBuffer[4096];    
  char* szMsg;
    
  int nContentLength = 0;
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
    cout << "new: " << nTmpRecv << " have: " << nBytesReceived << endl;
    fflush(stdout);
    if(nTmpRecv == -1)
    {
      cout << "ERROR :: lost connection" << endl;
      bDoReceive = false;
      break;
    }                  
    
    /* append received buffer */
    if(nBytesReceived > 0)
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
    else
    {
      szMsg = new char[nTmpRecv + 1];
      memcpy(szMsg, szBuffer, nTmpRecv);
      szMsg[nTmpRecv] = '\0';
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
      cout << "full header" << endl;
      fflush(stdout);              
            
      sHeader = sMsg.substr(0, nPos);
      nPos += string("\r\n\r\n").length();
      sContent = sMsg.substr(nPos, sMsg.length() - nPos);
    }
    else
    {
      cout << "WARNING :: did not received the full header. try again. " << nRecvCnt << endl;
      fflush(stdout);
      fuppesSleep(100);
      nRecvCnt++;
      continue;
    }
                  
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
      cout << "received less data then given in CONTENT-LENGTH" << endl;
      continue;
    }
    else
    {
      cout << "received full content - should: " << nContentLength << " have: " << sContent.length() << endl;
      if (sContent.length() < 10)
        cout << sContent << endl;
        
      fflush(stdout);
      bDoReceive = false;
    }
                  
    if(nTmpRecv == 0)      
      nRecvCnt++;                
                  
    cout << "recv: "  << nRecvCnt << " - "<< nBytesReceived << endl;
    fflush(stdout);
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
  }
 	else
 	{
	  cout << "ALERT :: bytes Received == 0" << endl;
	  fflush(stdout);
  }

  /* build response message and send it */
  if(bResult)
  {
    /* build response */
    CHTTPMessage ResponseMsg;
  	
  	//fuppesThreadLockMutex(&ReceiveMutex);
    bResult = pSession->GetHTTPServer()->CallOnReceive(&ReceivedMessage, &ResponseMsg);
  	//fuppesThreadUnlockMutex(&ReceiveMutex);    
    if(!bResult)
      cout << "ERROR parsing HTTP Message" << endl;
    if(bResult)
    {
      //CSharedLog::Shared()->Log(LOGNAME, "send response");     
      /*cout << "SEND RESPONSE" << endl;
      fflush(stdout);*/
      
      /* send binary content */
      if(ResponseMsg.GetBinContentLength() > 0)
      {
        //CSharedLog::Shared()->Log(LOGNAME, "send BIN response");     
        send(pSession->GetConnection(), ResponseMsg.GetHeaderAsString().c_str(), (int)strlen(ResponseMsg.GetHeaderAsString().c_str()), 0);            
        send(pSession->GetConnection(), ResponseMsg.GetBinContent(), ResponseMsg.GetBinContentLength(), 0);                        
      }
      /* send text content */
      else
      {
        //CSharedLog::Shared()->Log(LOGNAME, "send TEXT response"); 
        //cout << ResponseMsg.GetHeaderAsString() << endl;    
        //cout << ResponseMsg.GetMessageAsString().length() - ResponseMsg.GetHeaderAsString().length() << endl;
        /*cout << "BEGIN SEND: " << ResponseMsg.GetMessageAsString().length() << endl;
        fflush(stdout);*/
        
        int nSend = 0;
        nSend = send(pSession->GetConnection(), ResponseMsg.GetMessageAsString().c_str(), (int)strlen(ResponseMsg.GetMessageAsString().c_str()), 0);
        /*cout << "END SEND " << nSend << endl;
        fflush(stdout);*/
      }
      
      //CSharedLog::Shared()->Log(LOGNAME, "DONE send response");     
    }
  }
  
  cout << "done send" << endl << endl;
  fflush(stdout);  
  
  /* close connection */
  upnpSocketClose(pSession->GetConnection());    
  
  /* exit thread */
  pSession->m_bIsTerminated = true;
  fuppesThreadExit();  
}

/* <\PUBLIC> */
