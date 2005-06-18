/***************************************************************************
 *            SSDPSession.cpp
 * 
 *  Copyright  2005  Ulrich Völkel
 *  mail@ulrich-voelkel.de
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
 
// todo: create expiration timer

#include "SSDPSession.h"
#include "NotifyMsgFactory.h"

#include <iostream>

using namespace std;

CSSDPSession::CSSDPSession()
{
	udp = new CUDPSocket();
}

CSSDPSession::~CSSDPSession()
{
	delete udp;
}

void CSSDPSession::send_multicast(std::string a_message)
{
	udp->send_multicast(a_message);
	upnpSleep(200);
	udp->send_multicast(a_message);	
}

void CSSDPSession::send_unicast(std::string)
{
}
	  
void CSSDPSession::begin_receive_unicast()
{	
	udp->SetReceiveHandler(this);
	udp->begin_receive();
}

void CSSDPSession::OnUDPSocketReceive(CUDPSocket* pSocket, CSSDPMessage* pSSDPMessage)
{
	cout << "ssdp_session::OnUDPSocketReceive" << endl << pSSDPMessage->GetContent() << endl;
}

void CSSDPSession::start()
{
}


CMSearchSession::CMSearchSession(): CSSDPSession()
{
	udp->setup_socket(false);	
}

CMSearchSession::~CMSearchSession()
{
}

void CMSearchSession::start()
{
	cout << "msearch_session::start" << endl;
	begin_receive_unicast();
	upnpSleep(200);
	send_multicast(CNotifyMsgFactory::shared()->msearch());
}

sockaddr_in CMSearchSession:: GetLocalEndPoint()
{
	return udp->get_local_ep();
}
