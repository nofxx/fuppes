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
}

CSSDPCtrl::~CSSDPCtrl()
{
  SAFE_DELETE(m_pNotifyMsgFactory);
}

/*===============================================================================
 INIT
===============================================================================*/

void CSSDPCtrl::Start()
{	
	m_Listener.SetupSocket(true, m_sIPAddress);
	m_Listener.SetReceiveHandler(this);
	m_Listener.begin_receive();	
}

void CSSDPCtrl::Stop()
{	
	m_Listener.end_receive();	
}

/*===============================================================================
 GET
===============================================================================*/

CUDPSocket* CSSDPCtrl::get_socket()
{
	return &m_Listener;
}

/*===============================================================================
 SEND
===============================================================================*/

void CSSDPCtrl::send_msearch()
{
	CMSearchSession* msearch = new CMSearchSession(m_sIPAddress, this, m_pNotifyMsgFactory);
	m_LastMulticastEp = msearch->GetLocalEndPoint();
	/* T.S.TODO: Where could we call CMSearchSession::Stop() to terminate thread??? */
  /* uv :: UPnP says that remote devices have to answer within iirc 30 seconds
           so let's start a timer and kill the thread when the time is over */
  msearch->start();	
}

void CSSDPCtrl::send_alive()
{
	CUDPSocket Sock;
	Sock.SetupSocket(false, m_sIPAddress);
	
	m_LastMulticastEp = Sock.get_local_ep();
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONNECTION_MANAGER));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONNECTION_MANAGER));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONTENT_DIRECTORY));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONTENT_DIRECTORY));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_MEDIA_SERVER));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_MEDIA_SERVER));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));
	
	Sock.teardown_socket();
}

void CSSDPCtrl::send_byebye()
{
	CUDPSocket Sock;
	Sock.SetupSocket(false, m_sIPAddress);
	
  m_LastMulticastEp = Sock.get_local_ep();	
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_ROOT_DEVICE));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_ROOT_DEVICE));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONNECTION_MANAGER));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONNECTION_MANAGER));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONTENT_DIRECTORY));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONTENT_DIRECTORY));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_MEDIA_SERVER));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_MEDIA_SERVER));
	upnpSleep(200);
	
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_USN));
	Sock.send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_USN));
	
	Sock.teardown_socket();
}

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

bool CSSDPCtrl::SetReceiveHandler(ISSDPCtrl* pHandler)
{
  ASSERT(NULL != pHandler);
  if(NULL == pHandler)
    return false;
  
  m_pReceiveHandler = pHandler;

  return true;
}

bool CSSDPCtrl::OnUDPSocketReceive(CUDPSocket* pUDPSocket, CSSDPMessage* pSSDPMessage)
{		
  ASSERT(NULL != pUDPSocket);
  if(NULL == pUDPSocket)
    return false;
  ASSERT(NULL != pSSDPMessage);
  if(NULL == pSSDPMessage)
    return false;
  
  if((m_LastMulticastEp.sin_addr.s_addr != pSSDPMessage->GetRemoteEndPoint().sin_addr.s_addr) ||
    (m_LastMulticastEp.sin_port != pSSDPMessage->GetRemoteEndPoint().sin_port))
  {	
    if(pSSDPMessage->GetContent().substr(0, 8).compare("M-SEARCH") == 0)
    {
      CSharedLog::Shared()->Log(LOGNAME, "[SSDPCtrl] received m-search. unicasting response");
      CUDPSocket Sock;
      Sock.SetupSocket(false, m_sIPAddress);

      Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_ROOT_DEVICE), pSSDPMessage->GetRemoteEndPoint());
      Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONNECTION_MANAGER), pSSDPMessage->GetRemoteEndPoint());
      Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONTENT_DIRECTORY), pSSDPMessage->GetRemoteEndPoint());
      Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_MEDIA_SERVER), pSSDPMessage->GetRemoteEndPoint());
      Sock.SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_USN), pSSDPMessage->GetRemoteEndPoint());

      Sock.teardown_socket();
      CSharedLog::Shared()->Log(LOGNAME, "[SSDPCtrl] done");
    }
    else if(NULL != m_pReceiveHandler)
    {
      m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pSSDPMessage);
    }
  }	
  
  return true;
}

bool CSSDPCtrl::OnSessionReceive(CSSDPSession* pSender, CSSDPMessage* pMessage)
{
  ASSERT(NULL != pSender);
  if(NULL == pSender)
    return false;
  ASSERT(NULL != pMessage);
  if(NULL == pMessage)
    return false;
  
  //CSharedLog::Shared()->Log(LOGNAME, "OnSessionReceive");
  if(NULL != m_pReceiveHandler)
      m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pMessage);
  
  //cout << pMessage->GetContent() << endl;

  return true;
}

/* <\PUBLIC> */
