/***************************************************************************
 *            SSDPCtrl.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 
 
#include "SSDPCtrl.h"
#include "SharedLog.h"
#include <iostream>
 
using namespace std;

const std::string LOGNAME = "SSDPDCtrl";
 
CSSDPCtrl::CSSDPCtrl(std::string p_sIPAddress, std::string p_sHTTPServerURL)
{
  m_pNotifyMsgFactory = new CNotifyMsgFactory(p_sHTTPServerURL);

  m_sIPAddress   = p_sIPAddress;
	msearch_thread = (fuppesThread)NULL;
	listener       = new CUDPSocket();
}

CSSDPCtrl::~CSSDPCtrl()
{
  delete m_pNotifyMsgFactory;
	delete listener;
}

CUDPSocket* CSSDPCtrl::get_socket()
{
	return listener;
}

void CSSDPCtrl::Start()
{	
	listener->SetupSocket(true, m_sIPAddress);
	listener->SetReceiveHandler(this);
	listener->begin_receive();	
}

void CSSDPCtrl::Stop()
{	
	listener->end_receive();	
}

void CSSDPCtrl::send_msearch()
{
	CMSearchSession* msearch = new CMSearchSession(m_sIPAddress, this, m_pNotifyMsgFactory);
	this->last_multicast_ep = msearch->GetLocalEndPoint();
	// T.S.TODO: Where could we call CMSearchSession::Stop() to terminate thread???
  /* uv :: UPnP says that remote devices have to answer within iirc 30 seconds
           so let's start a timer and kill the thread when the time is over */
  msearch->start();	
}

void CSSDPCtrl::send_alive()
{
	CUDPSocket* pSock = new CUDPSocket();
	pSock->SetupSocket(false, m_sIPAddress);
	
	this->last_multicast_ep = pSock->get_local_ep();
	
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));	
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_ROOT_DEVICE));
	upnpSleep(200);
	
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONNECTION_MANAGER));
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONNECTION_MANAGER));
	upnpSleep(200);
	
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONTENT_DIRECTORY));
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_CONTENT_DIRECTORY));
	upnpSleep(200);
	
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_MEDIA_SERVER));
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_MEDIA_SERVER));
	upnpSleep(200);
	
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));
	pSock->send_multicast(m_pNotifyMsgFactory->notify_alive(MESSAGE_TYPE_USN));
	
	pSock->teardown_socket();
	delete pSock;
}

void CSSDPCtrl::send_byebye()
{
	CUDPSocket* sock = new CUDPSocket();
	sock->SetupSocket(false, m_sIPAddress);
	
  this->last_multicast_ep = sock->get_local_ep();	
	
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_ROOT_DEVICE));
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_ROOT_DEVICE));
	upnpSleep(200);
	
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONNECTION_MANAGER));
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONNECTION_MANAGER));
	upnpSleep(200);
	
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONTENT_DIRECTORY));
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_CONTENT_DIRECTORY));
	upnpSleep(200);
	
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_MEDIA_SERVER));
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_MEDIA_SERVER));
	upnpSleep(200);
	
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_USN));
	sock->send_multicast(m_pNotifyMsgFactory->notify_bye_bye(MESSAGE_TYPE_USN));
	
	sock->teardown_socket();
	delete sock;
}

void CSSDPCtrl::SetReceiveHandler(ISSDPCtrl* pHandler)
{
	m_pReceiveHandler = pHandler;
}

void CSSDPCtrl::OnUDPSocketReceive(CUDPSocket* pUDPSocket, CSSDPMessage* pSSDPMessage)
{		
	if((this->last_multicast_ep.sin_addr.s_addr != pSSDPMessage->GetRemoteEndPoint().sin_addr.s_addr) ||
		 (this->last_multicast_ep.sin_port != pSSDPMessage->GetRemoteEndPoint().sin_port))
	{	
    if(pSSDPMessage->GetContent().substr(0, 8).compare("M-SEARCH") == 0)
    {
      CSharedLog::Shared()->Log(LOGNAME, "[SSDPCtrl] received m-search. unicasting response");
      CUDPSocket* pSock = new CUDPSocket();
      pSock->SetupSocket(false, m_sIPAddress);
      
      pSock->SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_ROOT_DEVICE), pSSDPMessage->GetRemoteEndPoint());
      pSock->SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONNECTION_MANAGER), pSSDPMessage->GetRemoteEndPoint());
      pSock->SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONTENT_DIRECTORY), pSSDPMessage->GetRemoteEndPoint());
      pSock->SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_MEDIA_SERVER), pSSDPMessage->GetRemoteEndPoint());
      pSock->SendUnicast(m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_USN), pSSDPMessage->GetRemoteEndPoint());
      
      pSock->teardown_socket();
      delete pSock;
      CSharedLog::Shared()->Log(LOGNAME, "[SSDPCtrl] done");
    }
    else if(m_pReceiveHandler !=NULL)
      m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pSSDPMessage);
	}	
}

void CSSDPCtrl::OnSessionReceive(CSSDPSession* pSender, CSSDPMessage* pMessage)
{
  //CSharedLog::Shared()->Log(LOGNAME, "OnSessionReceive");
  if(m_pReceiveHandler !=NULL)
      m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pMessage);
  
  //cout << pMessage->GetContent() << endl;
}
