/***************************************************************************
 *            SSDPCtrl.cpp
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

#include "SSDPCtrl.h"
#include "SharedLog.h"
#include <iostream>
#include <sstream>
 
using namespace std;

/*===============================================================================
 CONSTANTS
===============================================================================*/

const std::string LOGNAME = "SSDPDCtrl";
 
/*===============================================================================
 CLASS CSSDPCtrl
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CSSDPCtrl::CSSDPCtrl(std::string p_sIPAddress, std::string p_sHTTPServerURL)
{
  m_pNotifyMsgFactory = new CNotifyMsgFactory(p_sHTTPServerURL);

  m_sIPAddress   = p_sIPAddress;
	msearch_thread = (fuppesThread)NULL;
  
  fuppesThreadInitMutex(&m_SessionReceiveMutex);
  fuppesThreadInitMutex(&m_SessionTimedOutMutex);
}

CSSDPCtrl::~CSSDPCtrl()
{
  fuppesThreadDestroyMutex(&m_SessionReceiveMutex);
  fuppesThreadDestroyMutex(&m_SessionTimedOutMutex);
  
  SAFE_DELETE(m_pNotifyMsgFactory);
}

/*===============================================================================
 INIT
===============================================================================*/

void CSSDPCtrl::Start()
{	
	m_Listener.SetupSocket(true, m_sIPAddress);
	m_Listener.SetReceiveHandler(this);
	m_Listener.BeginReceive();	
}

void CSSDPCtrl::Stop()
{	
  CleanupSessions();
	m_Listener.EndReceive();	
}

/*===============================================================================
 GET
===============================================================================*/

CUDPSocket* CSSDPCtrl::get_socket()
{
	return &m_Listener;
}

void CSSDPCtrl::CleanupSessions()
{
  fuppesThreadLockMutex(&m_SessionTimedOutMutex); 
     
  if(m_SessionList.size() == 0)
  {
    fuppesThreadUnlockMutex(&m_SessionTimedOutMutex); 
    return;  
  }
     
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "CleanupSessions");
  for(m_SessionListIterator = m_SessionList.begin(); m_SessionListIterator != m_SessionList.end(); m_SessionListIterator++)
  {
    if(m_SessionList.size() == 0)
      break;
                            
    CMSearchSession* pSession = *m_SessionListIterator;   
    m_SessionList.erase(m_SessionListIterator);
    delete pSession;
    m_SessionListIterator--;
  }
  
  fuppesThreadUnlockMutex(&m_SessionTimedOutMutex); 
}

/*===============================================================================
 SEND
===============================================================================*/

void CSSDPCtrl::send_msearch()
{
	CMSearchSession* pSession = new CMSearchSession(m_sIPAddress, this, m_pNotifyMsgFactory);
	m_LastMulticastEp = pSession->GetLocalEndPoint();
  pSession->Start();
  CleanupSessions();  
}

void CSSDPCtrl::send_alive()
{
  CleanupSessions();
  
  CUDPSocket Sock;
	Sock.SetupSocket(false, m_sIPAddress);
	
	//m_LastMulticastEp = Sock.GetLocalEndPoint();
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONNECTION_MANAGER));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONNECTION_MANAGER));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONTENT_DIRECTORY));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONTENT_DIRECTORY));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_MEDIA_SERVER));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_MEDIA_SERVER));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));
	
	Sock.TeardownSocket();
}

void CSSDPCtrl::send_byebye()
{
	CUDPSocket Sock;
	Sock.SetupSocket(false, m_sIPAddress);
	
  //m_LastMulticastEp = Sock.GetLocalEndPoint();	
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_ROOT_DEVICE));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_ROOT_DEVICE));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONNECTION_MANAGER));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONNECTION_MANAGER));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONTENT_DIRECTORY));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONTENT_DIRECTORY));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_MEDIA_SERVER));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_MEDIA_SERVER));
	fuppesSleep(200);
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_USN));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_USN));
	
	Sock.TeardownSocket();
}

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

void CSSDPCtrl::SetReceiveHandler(ISSDPCtrl* pHandler)
{ 
  m_pReceiveHandler = pHandler;
}

void CSSDPCtrl::OnUDPSocketReceive(CUDPSocket* pUDPSocket, CSSDPMessage* pSSDPMessage)
{
  stringstream sLog;
  sLog << "OnUDPSocketReceive() :: " << inet_ntoa(pSSDPMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pSSDPMessage->GetRemoteEndPoint().sin_port) << endl;
  sLog << inet_ntoa(m_LastMulticastEp.sin_addr) << ":" << ntohs(m_LastMulticastEp.sin_port);
 
  CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());
  
  if((m_LastMulticastEp.sin_addr.s_addr != pSSDPMessage->GetRemoteEndPoint().sin_addr.s_addr) ||
    (m_LastMulticastEp.sin_port != pSSDPMessage->GetRemoteEndPoint().sin_port))
  {	
    switch(pSSDPMessage->GetMessageType())
    {
      case SSDP_MESSAGE_TYPE_M_SEARCH:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_M_SEARCH");
        HandleMSearch(pSSDPMessage);
        break;
      default:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "default");
        if(m_pReceiveHandler != NULL)
          m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pSSDPMessage);
        else
          CSharedLog::Shared()->Warning(LOGNAME, "receive handler is null");          
        break;
    }    
  }
}

void CSSDPCtrl::OnSessionReceive(CMSearchSession* pSender, CSSDPMessage* pMessage)
{
  /* lock receive mutex */
  fuppesThreadLockMutex(&m_SessionReceiveMutex);  
  
  /* logging */
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "OnSessionReceive");
  CSharedLog::Shared()->DebugLog(LOGNAME, pMessage->GetMessage());
  
  /* pass message to the main fuppes instance */
  if(NULL != m_pReceiveHandler)
      m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pMessage);
  
  /* unlock receive mutex */
  fuppesThreadUnlockMutex(&m_SessionReceiveMutex);
}

void CSSDPCtrl::OnSessionTimeOut(CMSearchSession* pSender)
{
  CleanupSessions();
  
  /* lock timeout mutex */
  fuppesThreadLockMutex(&m_SessionTimedOutMutex); 
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "OnSessionTimeOut()");  
  m_SessionList.push_back(pSender);
  
  /* unlock timeout mutex */
  fuppesThreadUnlockMutex(&m_SessionTimedOutMutex); 
}

/* <\PUBLIC> */


/* <PRIVATE> */

void CSSDPCtrl::HandleMSearch(CSSDPMessage* pSSDPMessage)
{
  stringstream sLog;
  sLog << "received m-search from: \"" << inet_ntoa(pSSDPMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pSSDPMessage->GetRemoteEndPoint().sin_port) << "\"";      
  CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());
  CSharedLog::Shared()->DebugLog(LOGNAME, pSSDPMessage->GetMessage());
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "unicasting response");
  
  CUDPSocket Sock;
  Sock.SetupSocket(false, m_sIPAddress);

  Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_ROOT_DEVICE), pSSDPMessage->GetRemoteEndPoint());
  Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONNECTION_MANAGER), pSSDPMessage->GetRemoteEndPoint());
  Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONTENT_DIRECTORY), pSSDPMessage->GetRemoteEndPoint());
  Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_MEDIA_SERVER), pSSDPMessage->GetRemoteEndPoint());
  Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_USN), pSSDPMessage->GetRemoteEndPoint());

  Sock.TeardownSocket();
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "done");
  
  CleanupSessions();
}


/* <\PRIVATE> */
