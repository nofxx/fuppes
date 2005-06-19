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
 
 
#include <iostream>

#include "SSDPCtrl.h"
#include "NotifyMsgFactory.h"
 
#include "SSDPSession.h"
 
using namespace std;
 
CSSDPCtrl::CSSDPCtrl()
{
	msearch_thread = (upnpThread)NULL;
	listener       = new CUDPSocket();
}

CSSDPCtrl::~CSSDPCtrl()
{
	delete listener;
}

CUDPSocket* CSSDPCtrl::get_socket()
{
	return listener;
}

void CSSDPCtrl::Start()
{	
	listener->setup_socket(true);
	listener->SetReceiveHandler(this);
	listener->begin_receive();	
}

void CSSDPCtrl::send_msearch()
{
	CMSearchSession* msearch = new CMSearchSession();
	this->last_multicast_ep = msearch->GetLocalEndPoint();
	msearch->start();	
}

void CSSDPCtrl::send_alive()
{
	cout << "[ssdp_ctrl] sending notify alive" << endl;
	
	CUDPSocket* sock = new CUDPSocket();
	sock->setup_socket(false);
	
	this->last_multicast_ep = sock->get_local_ep();
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_root_device));	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_root_device));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_connection_manager));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_connection_manager));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_content_directory));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_content_directory));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_media_server));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_media_server));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_usn));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_alive(mt_usn));
	
	sock->teardown_socket();
	delete sock;
	
	cout << "[ssdp_ctrl] done" << endl;
}

void CSSDPCtrl::send_byebye()
{
	cout << "[ssdp_ctrl] sending notify byebye" << endl;
	
	CUDPSocket* sock = new CUDPSocket();
	sock->setup_socket(false);
	
  this->last_multicast_ep = sock->get_local_ep();	
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_root_device));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_root_device));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_connection_manager));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_connection_manager));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_content_directory));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_content_directory));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_media_server));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_media_server));
	upnpSleep(200);
	
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_usn));
	sock->send_multicast(CNotifyMsgFactory::shared()->notify_bye_bye(mt_usn));
	
	sock->teardown_socket();
	delete sock;
	
	cout << "[ssdp_ctrl] done" << endl;	
}

void CSSDPCtrl::SetReceiveHandler(ISSDPCtrl* pHandler)
{
	m_pReceiveHandler = pHandler;
}

void CSSDPCtrl::OnUDPSocketReceive(CUDPSocket* pUDPSocket, CSSDPMessage* pSSDPMessage)
{  
	//cout << "[ssdp_ctrl] received message from " << inet_ntoa(pMessage->get_remote_ep().sin_addr) << ":" << ntohs(pMessage->get_remote_ep().sin_port) << endl;		  
		
	if(((this->last_multicast_ep.sin_addr.s_addr != pSSDPMessage->GetRemoteEndPoint().sin_addr.s_addr) ||
		 (this->last_multicast_ep.sin_port != pSSDPMessage->GetRemoteEndPoint().sin_port)) &&
	   (m_pReceiveHandler != NULL))
	{		
	  m_pReceiveHandler->OnSSDPCtrlReceiveMsg(pSSDPMessage);
	}	
}
