/***************************************************************************
 *            SSDPSession.cpp
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
 
/* todo: create expiration timer */

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "SSDPSession.h"
#include "NotifyMsgFactory.h"

#include <iostream>

using namespace std;

/*===============================================================================
 CLASS CSSDPSession
===============================================================================*/

/* <PROTECTED> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CSSDPSession::CSSDPSession(std::string p_sIPAddress, ISSDPSession* pEventHandler)
{
  m_sIPAddress    = p_sIPAddress;
  m_pEventHandler = pEventHandler;
  m_Timer         = new CTimer(this);
}

CSSDPSession::~CSSDPSession()
{
  delete m_Timer;
}

/* <\PROTECTED> */

/* <PUBLIC> */

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

void CSSDPSession::OnUDPSocketReceive(CUDPSocket* pSocket, CSSDPMessage* pSSDPMessage)
{
  //cout << "ssdp_session::OnUDPSocketReceive" << endl << pSSDPMessage->GetContent() << endl;
  if(m_pEventHandler != NULL)
    m_pEventHandler->OnSessionReceive(this, pSSDPMessage);
}

void CSSDPSession::OnTimer()
{
  Stop();
  if(m_pEventHandler != NULL)
    m_pEventHandler->OnSessionTimeOut(this);
}

/* <\PUBLIC> */

/* <PROTECTED> */

/*===============================================================================
 CONTROL
===============================================================================*/

void CSSDPSession::Start()
{
}

void CSSDPSession::Stop()
{
}

/*===============================================================================
 SEND/RECEIVE
===============================================================================*/

void CSSDPSession::send_multicast(std::string a_message)
{
	/* Send message twice */
  m_UdpSocket.SendMulticast(a_message);
	upnpSleep(200);
	m_UdpSocket.SendMulticast(a_message);	
}

void CSSDPSession::send_unicast(std::string)
{
}

void CSSDPSession::begin_receive_unicast()
{	
	/* Start receiving messages */
  m_UdpSocket.SetReceiveHandler(this);
	m_UdpSocket.BeginReceive();
}

void CSSDPSession::end_receive_unicast()
{
  /* End receiving messages */
  m_UdpSocket.EndReceive();
}

/* <\PROTECTED> */

/*===============================================================================
 CLASS CMSearchSession
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR/DESTRUCTOR
===============================================================================*/

CMSearchSession::CMSearchSession(std::string p_sIPAddress, ISSDPSession* pReceiveHandler, CNotifyMsgFactory* pNotifyMsgFactory):
  CSSDPSession(p_sIPAddress, pReceiveHandler)
{
  ASSERT(NULL != pReceiveHandler);
  ASSERT(NULL != pNotifyMsgFactory);
  
  m_pNotifyMsgFactory = pNotifyMsgFactory;
	m_UdpSocket.SetupSocket(false, m_sIPAddress);	
  
  m_Timer->SetInterval(30);
}

CMSearchSession::~CMSearchSession()
{
}

/*===============================================================================
 CONTROL
===============================================================================*/

void CMSearchSession::Start()
{
	//cout << "msearch_session::start" << endl;
	begin_receive_unicast();
	upnpSleep(200);
	send_multicast(m_pNotifyMsgFactory->msearch());
  //m_Timer->Start();  
}

void CMSearchSession::Stop()
{
  //m_Timer->Stop();
	end_receive_unicast();
}

/*===============================================================================
 GET
===============================================================================*/

sockaddr_in CMSearchSession:: GetLocalEndPoint()
{
	return m_UdpSocket.GetLocalEndPoint();
}

/* <\PUBLIC> */
