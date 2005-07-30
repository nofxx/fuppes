/***************************************************************************
 *            UDPSocket.cpp
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

#include "UDPSocket.h"
#include "SharedLog.h"

#include <iostream>
#include <sstream>


using namespace std;

const string LOGNAME = "UDPSocket";

#define MULTICAST_PORT 1900
#define MULTICAST_IP   "239.255.255.250"

fuppesThreadCallback receive_loop(void *arg);

/* constructor */
CUDPSocket::CUDPSocket()
{	
	local_ep.sin_port = 0;
	receive_thread = (fuppesThread)NULL;
}	
	
/* destructor */
CUDPSocket::~CUDPSocket()
{
}

/* SetupSocket */
bool CUDPSocket::SetupSocket(bool p_bDoMulticast, std::string p_sIPAddress)
{
  /* Create socket */
	sock = socket(PF_INET, SOCK_DGRAM, 0);
	if(sock == -1)
  {
    CSharedLog::Shared()->Error(LOGNAME, "creating socket");
    return false;
  }
	
	int ret  = 0;
  #ifdef WIN32
  upnpSocketFlag(flag);  
  ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, flag, sizeof(flag));
  #else
  int flag = 1;
  ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
  #endif
	if(ret == -1)
  {
    CSharedLog::Shared()->Error(LOGNAME, "setsockopt: SO_REUSEADDR");
    return false;
  }
	
	/* Set local endpoint */
  local_ep.sin_family = AF_INET;	
	if(p_bDoMulticast)
	{
    local_ep.sin_addr.s_addr = INADDR_ANY;
    local_ep.sin_port		     = htons(MULTICAST_PORT);
	}
  else 
	{
    local_ep.sin_addr.s_addr = inet_addr(p_sIPAddress.c_str());		
    local_ep.sin_port		     = htons(0); /* use random port */
	}
	
	/* Bind socket */
	ret = bind(sock, (struct sockaddr*)&local_ep, sizeof(local_ep)); 
  if(ret == -1)
  {
    CSharedLog::Shared()->Error(LOGNAME, "bind");	
    return false;
  }
	
	/* Get random port */
	socklen_t size = sizeof(local_ep);
	getsockname(sock, (struct sockaddr*)&local_ep, &size);
	
	/* Join multicast group */
	m_bDoMulticast = p_bDoMulticast;
	if(m_bDoMulticast)
	{	
		struct ip_mreq stMreq; /* Multicast interface structure */
			
		stMreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP); 
		stMreq.imr_interface.s_addr = INADDR_ANY; 	
		ret = setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq,sizeof(stMreq)); 	
		if(ret == -1)
    {
      CSharedLog::Shared()->Error(LOGNAME, "setsockopt: multicast");
      return false;
    }
	}
  
  return true;
}	

/* TeardownSocket */
void CUDPSocket::TeardownSocket()
{
	/* Leave multicast group */
	if(m_bDoMulticast)
	{
		struct ip_mreq stMreq;
		
	  stMreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP); 
		stMreq.imr_interface.s_addr = INADDR_ANY; 	
		setsockopt(sock, IPPROTO_IP, IP_DROP_MEMBERSHIP,(char *)&stMreq,sizeof(stMreq)); 		
	}	
	upnpSocketClose(sock);
}

/* BeginReceive */
void CUDPSocket::BeginReceive()
{
  /* Start thread */
  fuppesThreadStart(receive_thread, receive_loop);
}

/* EndReceive */
void CUDPSocket::EndReceive()
{
  /* Exit thread */
  fuppesThreadClose(receive_thread, 2000);
}

/* SendMulticast */
void CUDPSocket::SendMulticast(std::string p_sMessage)
{
	/* Create remote end point */
	sockaddr_in remote_ep;	
	remote_ep.sin_family      = AF_INET;
  remote_ep.sin_addr.s_addr = inet_addr(MULTICAST_IP);
  remote_ep.sin_port				= htons(MULTICAST_PORT);
		
	sendto(sock, p_sMessage.c_str(), (int)strlen(p_sMessage.c_str()), 0, (struct sockaddr*)&remote_ep, sizeof(remote_ep));
}

/* SendUnicast */
void CUDPSocket::SendUnicast(std::string p_sMessage, sockaddr_in p_RemoteEndPoint)
{
  sendto(sock, p_sMessage.c_str(), (int)strlen(p_sMessage.c_str()), 0, (struct sockaddr*)&p_RemoteEndPoint, sizeof(p_RemoteEndPoint));  
}

/* GetSocketFd */
upnpSocket CUDPSocket::GetSocketFd()
{
	return sock;
}

/* GetPort */
int CUDPSocket::GetPort()
{
	return ntohs(local_ep.sin_port);
}

/* GetIPAddress */
std::string CUDPSocket::GetIPAddress()
{
	return inet_ntoa(local_ep.sin_addr);
}

/* GetLocalEp */
sockaddr_in CUDPSocket::GetLocalEndPoint()
{
	return local_ep;
}

fuppesThreadCallback receive_loop(void *arg)
{
  CUDPSocket* udp_sock = (CUDPSocket*)arg;
  stringstream sMsg;
  sMsg << "listening on " << udp_sock->GetIPAddress() << ":" << udp_sock->GetPort();
	CSharedLog::Shared()->Log(LOGNAME, sMsg.str());
	
	char buffer[4096];	
	int  bytes_received = 0;
	
	stringstream msg;
	
	sockaddr_in remote_ep;	
	socklen_t   size = sizeof(remote_ep);	
		/* T.S. TODO: Error on exit here */
	while((bytes_received = recvfrom(udp_sock->GetSocketFd(), buffer, sizeof(buffer), 0, (struct sockaddr*)&remote_ep, &size)) != -1)
	{
		buffer[bytes_received] = '\0';		
		msg << buffer;   
    
    //cout << msg.str();    
    /*cout << inet_ntoa(remote_ep.sin_addr) << ":" << ntohs(remote_ep.sin_port) << endl;
    cout << inet_ntoa(udp_sock->get_local_ep().sin_addr) << ":" << ntohs(udp_sock->get_local_ep().sin_port) << endl;*/
    
		if((remote_ep.sin_addr.s_addr != udp_sock->GetLocalEndPoint().sin_addr.s_addr) || 
			 (remote_ep.sin_port != udp_sock->GetLocalEndPoint().sin_port))		
		{
			CSSDPMessage SSDPMessage;
      SSDPMessage.SetMessage(msg.str());
			SSDPMessage.SetRemoteEndPoint(remote_ep);
			SSDPMessage.SetLocalEndPoint(udp_sock->GetLocalEndPoint());
			udp_sock->CallOnReceive(&SSDPMessage);
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

void CUDPSocket::CallOnReceive(CSSDPMessage* pSSDPMessage)
{
	if(this->m_pReceiveHandler != NULL)
	  m_pReceiveHandler->OnUDPSocketReceive(this, pSSDPMessage);
}
