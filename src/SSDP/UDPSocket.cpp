/***************************************************************************
 *            UDPSocket.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "UDPSocket.h"
#include "../SharedLog.h"
#include "../Common/Common.h"

#include <iostream>
#include <sstream>

using namespace std;

/*===============================================================================
 CONSTANTS
===============================================================================*/

const string LOGNAME = "UDPSocket";

/*===============================================================================
 DEFINITIONS
===============================================================================*/

#define MULTICAST_PORT 1900
#define MULTICAST_IP   "239.255.255.250"

/*===============================================================================
 THREAD
===============================================================================*/

fuppesThreadCallback ReceiveLoop(void *arg);

/*===============================================================================
 CLASS CUDPSocket
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

/* constructor */
CUDPSocket::CUDPSocket()
{	
	/* Init members */
  m_LocalEndpoint.sin_port = 0;
	m_ReceiveThread          = (fuppesThread)NULL;
  m_pReceiveHandler        = NULL;
}	
	
/* destructor */
CUDPSocket::~CUDPSocket()
{
  /* Cleanup */
  //TeardownSocket();
}

/*===============================================================================
 CONTROL SOCKET
===============================================================================*/

/* SetupSocket */
bool CUDPSocket::SetupSocket(bool p_bDoMulticast, std::string p_sIPAddress /* = "" */)
{
  /* Create socket */
	m_Socket = socket(AF_INET, SOCK_DGRAM, 0);
	if(m_Socket == -1) {
    CSharedLog::Shared()->Log(L_ERROR, "failed to create socket", __FILE__, __LINE__);    
    throw EException("failed to create socket", __FILE__, __LINE__);    
  }
  
  /* Set socket options */
  int ret  = 0;
  #ifdef WIN32  
  bool bOptVal = true;
  ret = setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, (char*)&bOptVal, sizeof(bool));
  #else
  int flag = 1;
  ret = setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
  #endif
  if(ret == -1) {    
    CSharedLog::Shared()->Log(L_ERROR, "failed to setsockopt: SO_REUSEADDR", __FILE__, __LINE__);
    throw EException("failed to setsockopt: SO_REUSEADDR", __FILE__, __LINE__);
  }

  // set nonblocking
  if (!fuppesSocketSetNonBlocking(m_Socket)) {
    CSharedLog::Shared()->Log(L_ERROR, "failed to setsockopt: fuppesSocketSetNonBlocking", __FILE__, __LINE__);
    throw EException("failed to setsockopt: fuppesSocketSetNonBlocking", __FILE__, __LINE__);
  }
	
	/* Set local endpoint */
  m_LocalEndpoint.sin_family = AF_INET;	
	if(p_bDoMulticast) {
    m_LocalEndpoint.sin_addr.s_addr  = INADDR_ANY;
    m_LocalEndpoint.sin_port		     = htons(MULTICAST_PORT);
	}
  else {
    m_LocalEndpoint.sin_addr.s_addr  = inet_addr(p_sIPAddress.c_str());		
    m_LocalEndpoint.sin_port		     = htons(0); /* use random port */
	}	
	memset(&(m_LocalEndpoint.sin_zero), '\0', 8); // fill the rest of the structure with zero
	
	/* Bind socket */
	ret = bind(m_Socket, (struct sockaddr*)&m_LocalEndpoint, sizeof(m_LocalEndpoint)); 
  if(ret == -1) {
    CSharedLog::Shared()->Log(L_ERROR, "failed to bind socket", __FILE__, __LINE__);
    throw EException("failed to bind socket", __FILE__, __LINE__);
  }
	
	/* Get random port */
	socklen_t size = sizeof(m_LocalEndpoint);
	getsockname(m_Socket, (struct sockaddr*)&m_LocalEndpoint, &size);
	
	/* Join multicast group */
	m_bDoMulticast = p_bDoMulticast;
	if(m_bDoMulticast)
	{	
		struct ip_mreq stMreq; /* Multicast interface structure */
			
		stMreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP); 
		stMreq.imr_interface.s_addr  = INADDR_ANY; 	
		ret = setsockopt(m_Socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq,sizeof(stMreq)); 	
		if(ret == -1) {      
      CSharedLog::Shared()->Log(L_ERROR, "failed to setsockopt: multicast", __FILE__, __LINE__);
      throw EException("failed to setsockopt: multicast", __FILE__, __LINE__);      
    }
	}
  
  return true;
}	

void CUDPSocket::SetTTL(int p_nTTL)
{
  int ret = setsockopt(m_Socket, IPPROTO_IP, IP_TTL, (char *)&p_nTTL, sizeof(p_nTTL)); 	
  if (ret == -1)
    CSharedLog::Shared()->Log(L_ERROR, "setsockopt: TTL", __FILE__, __LINE__); 
}

/* TeardownSocket */
void CUDPSocket::TeardownSocket()
{
    /* Exit thread */  
  if(m_ReceiveThread != (fuppesThread)NULL)
    EndReceive();
  
	/* Leave multicast group */
	if(m_bDoMulticast)
	{
		struct ip_mreq stMreq;
		
	  stMreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP); 
		stMreq.imr_interface.s_addr = INADDR_ANY; 	
		setsockopt(m_Socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,(char *)&stMreq,sizeof(stMreq)); 		
	}

  /* Close socket */
  upnpSocketClose(m_Socket);
}

/*===============================================================================
 SEND MESSAGES
===============================================================================*/

/* SendMulticast */
void CUDPSocket::SendMulticast(std::string p_sMessage)
{
	/* Create remote end point */
	sockaddr_in remote_ep;	
	remote_ep.sin_family      = AF_INET;
  remote_ep.sin_addr.s_addr = inet_addr(MULTICAST_IP);
  remote_ep.sin_port				= htons(MULTICAST_PORT);
		
  /* Send message */
	sendto(m_Socket, p_sMessage.c_str(), (int)strlen(p_sMessage.c_str()), 0, (struct sockaddr*)&remote_ep, sizeof(remote_ep));
}

/* SendUnicast */
void CUDPSocket::SendUnicast(std::string p_sMessage, sockaddr_in p_RemoteEndPoint)
{
  /* Send message */
  sendto(m_Socket, p_sMessage.c_str(), (int)strlen(p_sMessage.c_str()), 0, (struct sockaddr*)&p_RemoteEndPoint, sizeof(p_RemoteEndPoint));  
}

/*===============================================================================
 RECEIVE MESSAGES
===============================================================================*/

/* BeginReceive */
void CUDPSocket::BeginReceive()
{
  /* Start thread */
  m_bBreakReceive = false;
  fuppesThreadStart(m_ReceiveThread, ReceiveLoop);
}

/* EndReceive */
void CUDPSocket::EndReceive()
{
  /* Exit thread */  
  m_bBreakReceive = true;  
  if(m_ReceiveThread)
  {
    fuppesThreadClose(m_ReceiveThread);
    m_ReceiveThread = (fuppesThread)NULL;
  }
}

/* SetReceiveHandler */
void CUDPSocket::SetReceiveHandler(IUDPSocket* pHandler)
{
  /* Save pointer to the receive handler */
  m_pReceiveHandler = pHandler;
}

/* CallOnReceive */
void CUDPSocket::CallOnReceive(CSSDPMessage* pSSDPMessage)
{ 
  /* Call receive handler */  
  if(NULL != m_pReceiveHandler)
	  m_pReceiveHandler->OnUDPSocketReceive(this, pSSDPMessage);
}

/*===============================================================================
 GET
===============================================================================*/

/* GetSocketFd */
upnpSocket CUDPSocket::GetSocketFd()
{
	return m_Socket;
}

/* GetPort */
int CUDPSocket::GetPort()
{
	return ntohs(m_LocalEndpoint.sin_port);
}

/* GetIPAddress */
std::string CUDPSocket::GetIPAddress()
{
	return inet_ntoa(m_LocalEndpoint.sin_addr);
}

/* GetLocalEp */
sockaddr_in CUDPSocket::GetLocalEndPoint()
{
	return m_LocalEndpoint;
}

/*===============================================================================
 THREAD
===============================================================================*/

fuppesThreadCallback ReceiveLoop(void *arg)
{
  CUDPSocket* udp_sock = (CUDPSocket*)arg;
  stringstream sLog;
  sLog << "listening on " << udp_sock->GetIPAddress() << ":" << udp_sock->GetPort();
	CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());
	
	char buffer[4096];	
	int  bytes_received = 0;
	
	stringstream msg;
	
	sockaddr_in remote_ep;	
	socklen_t   size = sizeof(remote_ep);	

  while(!udp_sock->m_bBreakReceive)
	{
    bytes_received = recvfrom(udp_sock->GetSocketFd(), buffer, sizeof(buffer), 0, (struct sockaddr*)&remote_ep, &size);
    if(bytes_received < 0)   
    {  
      fuppesSleep(100);
      continue;
    }
    
		buffer[bytes_received] = '\0';		
		msg << buffer;
    
    //cout << msg.str() << endl;    
    /*cout << inet_ntoa(remote_ep.sin_addr) << ":" << ntohs(remote_ep.sin_port) << endl;
    cout << inet_ntoa(udp_sock->GetLocalEndPoint().sin_addr) << ":" << ntohs(udp_sock->GetLocalEndPoint().sin_port) << endl;*/
    
    /* ensure we don't receive our own message */
		if((remote_ep.sin_addr.s_addr != udp_sock->GetLocalEndPoint().sin_addr.s_addr) || 
			 (remote_ep.sin_port != udp_sock->GetLocalEndPoint().sin_port))		
		{
			CSSDPMessage pSSDPMessage; // = new CSSDPMessage();
      if(pSSDPMessage.SetMessage(msg.str()))
      {
  			pSSDPMessage.SetRemoteEndPoint(remote_ep);
	  		pSSDPMessage.SetLocalEndPoint(udp_sock->GetLocalEndPoint());
		  	udp_sock->CallOnReceive(&pSSDPMessage);
      }
      else
        CSharedLog::Shared()->Error(LOGNAME, "parsing UDP-message");
		}
		
		msg.str("");
		msg.clear();
		fuppesSleep(100);	
	}
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "exiting ReceiveLoop()");
  fuppesThreadExit();
}
