/***************************************************************************
 *            UDPSocket.cpp
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

#include "UDPSocket.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>


using namespace std;

#define MULTICAST_PORT 1900
#define MULTICAST_IP   "239.255.255.250"

//void *receive_loop(void *arg);
upnpThreadCallback receive_loop(void *arg);

CUDPSocket::CUDPSocket()
{	
	local_ep.sin_port = 0;
	receive_thread = (upnpThread)NULL;
}	
	

CUDPSocket::~CUDPSocket()
{
}

upnpSocket CUDPSocket::get_socket_fd()
{
	return sock;
}

void CUDPSocket::setup_socket(bool do_multicast)
{
  // create socket
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
		cout << "error: creating socket" << endl;
	
	int ret  = 0;
	/*int flag = 1;
  ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));*/
  upnpSocketFlag(flag);
  ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, flag, sizeof(flag));
	if(ret == -1)	
		cout << "error: setsockopt" << endl;
	
	// set local endpoint		
  local_ep.sin_family = AF_INET;	
	if(do_multicast)
	{
    local_ep.sin_addr.s_addr = INADDR_ANY;
    local_ep.sin_port		     = htons(MULTICAST_PORT);
	}
  else 
	{
    local_ep.sin_addr.s_addr = inet_addr(shared_config::shared()->get_ip().c_str());		
    local_ep.sin_port		     = htons(0); // use random port
	}
	
	// bind socket
	ret = bind(sock, (struct sockaddr*)&local_ep, sizeof(local_ep)); 
  if(ret == -1)
		cout << "error: bind" << endl;
	
	// get random port
	socklen_t size = sizeof(local_ep);
	getsockname(sock, (struct sockaddr*)&local_ep, &size);
	
	// join multicast group
	is_multicast = do_multicast;
	if(do_multicast)
	{	
		struct ip_mreq stMreq; // Multicast interface structure  
			
		stMreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP); 
		stMreq.imr_interface.s_addr = INADDR_ANY; 	
		ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq,sizeof(stMreq)); 	
		if(ret == -1)
			cout << "error: setsockopt multicast" << endl;
	}
}	


void CUDPSocket::teardown_socket()
{
	// leave multicast group
	if(is_multicast)
	{
		struct ip_mreq stMreq;
		
	  stMreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250"); 
		stMreq.imr_interface.s_addr = INADDR_ANY; 	
		setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,(char *)&stMreq,sizeof(stMreq)); 		
	}	
	upnpSocketClose(sock);
}

void CUDPSocket::begin_receive()
{
//  pthread_create(&receive_thread, NULL, &receive_loop, this);
  upnpThreadStart(receive_thread, receive_loop);
}

void CUDPSocket::send_multicast(std::string a_message)
{
	// create remote end point
	sockaddr_in remote_ep;	
	remote_ep.sin_family      = AF_INET;
  remote_ep.sin_addr.s_addr = inet_addr(MULTICAST_IP);
  remote_ep.sin_port				= htons(MULTICAST_PORT);
	
	/*char message[strlen(a_message.c_str())];
	strcpy(message, a_message.c_str());	*/
	
	/*char* message = new char[strlen(a_message.c_str())];
	strcpy(message, a_message.c_str());	*/
	
	//sendto(tmp_sock, message, sizeof(message), 0, (struct sockaddr*)&remote_ep, sizeof(remote_ep));
	//sendto(sock, message, sizeof(message), 0, (struct sockaddr*)&remote_ep, sizeof(remote_ep));
		
	sendto(sock, a_message.c_str(), strlen(a_message.c_str()), 0, (struct sockaddr*)&remote_ep, sizeof(remote_ep));
}

int CUDPSocket::get_port()
{
	return ntohs(local_ep.sin_port);
}

std::string CUDPSocket::get_ip()
{
	return inet_ntoa(local_ep.sin_addr);
}

sockaddr_in CUDPSocket::get_local_ep()
{
	return local_ep;
}

//void *receive_loop(void *arg)
upnpThreadCallback receive_loop(void *arg)
{
  CUDPSocket* udp_sock = (CUDPSocket*)arg;
	cout << "[udp_socket] listening on " << udp_sock->get_ip() << ":" << udp_sock->get_port() << endl;
	
	char buffer[4096];	
	int  bytes_received = 0;
	
	stringstream msg;
	
	sockaddr_in remote_ep;	
	socklen_t   size = sizeof(remote_ep);	
		
	while((bytes_received = recvfrom(udp_sock->get_socket_fd(), buffer, sizeof(buffer), 0, (struct sockaddr*)&remote_ep, &size)) != -1)
	{
		buffer[bytes_received] = '\0';		
		msg << buffer;
			
		if((remote_ep.sin_addr.s_addr != udp_sock->get_local_ep().sin_addr.s_addr) || 
			 (remote_ep.sin_port != udp_sock->get_local_ep().sin_port))		
		{
			CMessage* result = new CMessage(msg.str());		
			result->set_remote_ep(remote_ep);
			result->set_local_ep(udp_sock->get_local_ep());
			udp_sock->call_on_receive(result);			
		}
		
		msg.str("");
		msg.clear();
		
		upnpSleep(100);
	}

  return 0;
}


/**
 * on_receive
 */
void CUDPSocket::SetReceiveHandler(IUDPSocket* pHandler)
{
	this->m_pReceiveHandler = pHandler;
}

void CUDPSocket::call_on_receive(CMessage* pMessage)
{
	if(this->m_pReceiveHandler != NULL)
	  m_pReceiveHandler->OnUDPSocketReceive(this, pMessage);
}
