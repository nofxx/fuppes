/***************************************************************************
 *            SSDPSession.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "SSDPSession.h"
#include "NotifyMsgFactory.h"

#include <iostream>

using namespace std;

CSSDPSession::CSSDPSession(std::string p_sIPAddress, ISSDPSession* pReceiveHandler)
{
  m_sIPAddress      = p_sIPAddress;
  m_pReceiveHandler = pReceiveHandler;
	udp = new CUDPSocket();
}

CSSDPSession::~CSSDPSession()
{
	delete udp;
}

void CSSDPSession::send_multicast(std::string a_message)
{
	udp->SendMulticast(a_message);
	upnpSleep(200);
	udp->SendMulticast(a_message);	
}

void CSSDPSession::send_unicast(std::string)
{
}
	  
void CSSDPSession::begin_receive_unicast()
{	
	udp->SetReceiveHandler(this);
	udp->BeginReceive();
}

void CSSDPSession::end_receive_unicast()
{
  udp->EndReceive();
}

bool CSSDPSession::OnUDPSocketReceive(CUDPSocket* pSocket, CSSDPMessage* pSSDPMessage)
{
  ASSERT(NULL != pSocket);
  if(NULL == pSocket)
    return false;
  ASSERT(NULL != pSSDPMessage);
  if(NULL == pSSDPMessage)
    return false;
  
  //cout << "ssdp_session::OnUDPSocketReceive" << endl << pSSDPMessage->GetContent() << endl;
	if(m_pReceiveHandler != NULL)
	  m_pReceiveHandler->OnSessionReceive(this, pSSDPMessage);

  return true;
}

void CSSDPSession::start()
{
}


CMSearchSession::CMSearchSession(std::string p_sIPAddress, ISSDPSession* pReceiveHandler, CNotifyMsgFactory* pNotifyMsgFactory):
  CSSDPSession(p_sIPAddress, pReceiveHandler)
{
  m_pNotifyMsgFactory = pNotifyMsgFactory;
	udp->SetupSocket(false, m_sIPAddress);	
}

CMSearchSession::~CMSearchSession()
{
}

void CMSearchSession::start()
{
	//cout << "msearch_session::start" << endl;
	begin_receive_unicast();
	upnpSleep(200);
	send_multicast(m_pNotifyMsgFactory->msearch());
}

void CMSearchSession::Stop()
{
	end_receive_unicast();
}

sockaddr_in CMSearchSession:: GetLocalEndPoint()
{
	return udp->GetLocalEndPoint();
}
