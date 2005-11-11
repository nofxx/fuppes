/***************************************************************************
 *            MSearchSession.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
/* todo: create expiration timer */

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "MSearchSession.h"
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

CMSearchSession::CMSearchSession(std::string p_sIPAddress, IMSearchSession* pReceiveHandler, CNotifyMsgFactory* pNotifyMsgFactory)
  :m_Timer(this)
{
  ASSERT(NULL != pReceiveHandler);
  ASSERT(NULL != pNotifyMsgFactory);
  
  m_sIPAddress        = p_sIPAddress;
  m_pEventHandler     = pReceiveHandler;
  m_pNotifyMsgFactory = pNotifyMsgFactory;
  
  m_Timer.SetInterval(30);
  m_UdpSocket.SetupSocket(false, m_sIPAddress);	
}

CMSearchSession::~CMSearchSession()
{
  m_UdpSocket.TeardownSocket();  
}

/* <\PROTECTED> */

/* <PUBLIC> */

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

void CMSearchSession::OnUDPSocketReceive(CUDPSocket* pSocket, CSSDPMessage* pSSDPMessage)
{
  if(m_pEventHandler != NULL)
    m_pEventHandler->OnSessionReceive(this, pSSDPMessage);
}

void CMSearchSession::OnTimer()
{
  Stop();
  if(m_pEventHandler != NULL)
  {
    m_pEventHandler->OnSessionTimeOut(this);
  }  
}

/*===============================================================================
 CONTROL
===============================================================================*/

void CMSearchSession::Start()
{
	begin_receive_unicast();
	fuppesSleep(200);
	send_multicast(m_pNotifyMsgFactory->msearch());
  m_Timer.Start();
}

void CMSearchSession::Stop()
{
  m_Timer.Stop();
	end_receive_unicast();
}

/*===============================================================================
 SEND/RECEIVE
===============================================================================*/

void CMSearchSession::send_multicast(std::string a_message)
{
	/* Send message twice */
  m_UdpSocket.SendMulticast(a_message);
	fuppesSleep(200);
	m_UdpSocket.SendMulticast(a_message);	
}

void CMSearchSession::send_unicast(std::string)
{
}

void CMSearchSession::begin_receive_unicast()
{	
	/* Start receiving messages */
  m_UdpSocket.SetReceiveHandler(this);
	m_UdpSocket.BeginReceive();
}

void CMSearchSession::end_receive_unicast()
{
  /* End receiving messages */
  m_UdpSocket.EndReceive();
}

sockaddr_in CMSearchSession:: GetLocalEndPoint()
{
	return m_UdpSocket.GetLocalEndPoint();
}

/* <\PUBLIC> */
