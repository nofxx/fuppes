/***************************************************************************
 *            SSDPCtrl.cpp
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
 
#include "SSDPCtrl.h"
#include "../SharedLog.h"
#include <iostream>
#include <sstream>
 
using namespace std;

const std::string LOGNAME = "SSDPDCtrl";
 
CSSDPCtrl::CSSDPCtrl(std::string p_sIPAddress, std::string p_sHTTPServerURL)
{
  m_sHTTPServerURL = p_sHTTPServerURL;
  m_pNotifyMsgFactory = new CNotifyMsgFactory(m_sHTTPServerURL);

  m_sIPAddress   = p_sIPAddress;
	msearch_thread = (fuppesThread)NULL;
  
  fuppesThreadInitMutex(&m_SessionReceiveMutex);
  fuppesThreadInitMutex(&m_SessionTimedOutMutex);
}

CSSDPCtrl::~CSSDPCtrl()
{
  fuppesThreadDestroyMutex(&m_SessionReceiveMutex);
  fuppesThreadDestroyMutex(&m_SessionTimedOutMutex);
  
  delete m_pNotifyMsgFactory;
}

void CSSDPCtrl::Start()
{	
	try {
    m_Listener.SetupSocket(true, m_sIPAddress);
    m_Listener.SetTTL(4);
	  m_Listener.SetReceiveHandler(this);
	  m_Listener.BeginReceive();	
  }
  catch(...) {
    throw;
  }
  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "SSDPController started");
}

void CSSDPCtrl::Stop()
{	
  CleanupSessions();
	m_Listener.EndReceive();  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "SSDPController stopped");
}

CUDPSocket* CSSDPCtrl::get_socket()
{
	return &m_Listener;
}

void CSSDPCtrl::CleanupSessions()
{
  CSharedLog::Log(L_DBG, __FILE__, __LINE__, "CleanupSessions");
  fuppesThreadLockMutex(&m_SessionTimedOutMutex); 
      
  if(m_HandleMSearchThreadList.size() > 0)
  {    
    for(m_HandleMSearchThreadListIterator = m_HandleMSearchThreadList.begin();
        m_HandleMSearchThreadListIterator != m_HandleMSearchThreadList.end(); )
    {     
      if(m_HandleMSearchThreadList.size() == 0)
        break;
      
      CHandleMSearchSession* pMSession = *m_HandleMSearchThreadListIterator;
      if(pMSession->m_bIsTerminated)
      {
        std::list<CHandleMSearchSession*>::iterator tmpIt = m_HandleMSearchThreadListIterator;
        ++tmpIt;        
        m_HandleMSearchThreadList.erase(m_HandleMSearchThreadListIterator);
        m_HandleMSearchThreadListIterator = tmpIt;
        delete pMSession;        
      }
      else
      {
        ++m_HandleMSearchThreadListIterator;
      }  
    }
  }
     
  if(m_SessionList.size() > 0)
  {  
    for(m_SessionListIterator = m_SessionList.begin();
        m_SessionListIterator != m_SessionList.end(); )
    {
      if(m_SessionList.size() == 0)
        break;
                  
      std::list<CMSearchSession*>::iterator tmpIt = m_SessionListIterator;
      ++tmpIt;      
      CMSearchSession* pSession = *m_SessionListIterator;   
      m_SessionList.erase(m_SessionListIterator);
      m_SessionListIterator = tmpIt;
      delete pSession;
    }  
  }

  fuppesThreadUnlockMutex(&m_SessionTimedOutMutex); 
}

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
	Sock.SetTTL(4);
	
	//m_LastMulticastEp = Sock.GetLocalEndPoint();
	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));	
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_MEDIA_SERVER));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONTENT_DIRECTORY));
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONNECTION_MANAGER));
		
	/*Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));	
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
	Sock.SendMulticast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));*/
	
	Sock.TeardownSocket();
}

void CSSDPCtrl::send_byebye()
{
	CUDPSocket Sock;
	Sock.SetupSocket(false, m_sIPAddress);
	Sock.SetTTL(4);
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

void CSSDPCtrl::SetReceiveHandler(ISSDPCtrl* pHandler)
{ 
  m_pReceiveHandler = pHandler;
}

void CSSDPCtrl::OnUDPSocketReceive(CSSDPMessage* pSSDPMessage)
{
  stringstream sLog;
  sLog << "OnUDPSocketReceive() :: " << inet_ntoa(pSSDPMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pSSDPMessage->GetRemoteEndPoint().sin_port) << endl;
  //sLog << inet_ntoa(m_LastMulticastEp.sin_addr) << ":" << ntohs(m_LastMulticastEp.sin_port);
 
  CSharedLog::Log(L_DBG, __FILE__, __LINE__, sLog.str().c_str());
  
  if((m_LastMulticastEp.sin_addr.s_addr != pSSDPMessage->GetRemoteEndPoint().sin_addr.s_addr) ||
    (m_LastMulticastEp.sin_port != pSSDPMessage->GetRemoteEndPoint().sin_port))
  {	
    switch(pSSDPMessage->GetMessageType())
    {
      case SSDP_MESSAGE_TYPE_M_SEARCH:
        //CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_M_SEARCH");
        HandleMSearch(pSSDPMessage);
        break;
      
      default:        
        if(m_pReceiveHandler != NULL) {
          fuppesThreadLockMutex(&m_SessionReceiveMutex);  
          m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pSSDPMessage);
          fuppesThreadUnlockMutex(&m_SessionReceiveMutex);  
        }
        /*else
          CSharedLog::Shared()->Warning(LOGNAME, "receive handler is null"); */
        break;
    }
  }
}

void CSSDPCtrl::OnSessionReceive(CSSDPMessage* pMessage)
{
  /* lock receive mutex */
  fuppesThreadLockMutex(&m_SessionReceiveMutex);  
  
  /* logging */
  CSharedLog::Log(L_DBG, __FILE__, __LINE__, pMessage->GetMessage().c_str());
  
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
  
  //CSharedLog::Shared()->ExtendedLog(LOGNAME, "OnSessionTimeOut()");  
  m_SessionList.push_back(pSender);
  
  /* unlock timeout mutex */
  fuppesThreadUnlockMutex(&m_SessionTimedOutMutex); 
}


void CSSDPCtrl::HandleMSearch(CSSDPMessage* pSSDPMessage)
{
  fuppesThreadLockMutex(&m_SessionTimedOutMutex);      
     
  CSharedLog::Shared()->DebugLog(LOGNAME, pSSDPMessage->GetMessage());
  stringstream sLog;
  sLog << "received m-search from: \"" << inet_ntoa(pSSDPMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pSSDPMessage->GetRemoteEndPoint().sin_port) << "\"";      
  CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());      
  
  //cout << pSSDPMessage->GetMSearchST() << " - " << M_SEARCH_ST_UNSUPPORTED << endl;
  
  if(pSSDPMessage->GetMSearchST() != M_SEARCH_ST_UNSUPPORTED)
  {
    CHandleMSearchSession* pHandleMSearch = new CHandleMSearchSession(pSSDPMessage, m_sIPAddress, m_sHTTPServerURL);
    m_HandleMSearchThreadList.push_back(pHandleMSearch);
    pHandleMSearch->Start();
  }
 
  fuppesThreadUnlockMutex(&m_SessionTimedOutMutex);   
  
  CleanupSessions();
}
