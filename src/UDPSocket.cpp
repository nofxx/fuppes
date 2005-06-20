/***************************************************************************
 *            UDPSocket.cpp
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

#include "UDPSocket.h"
#include "SharedConfig.h"
#include "Common.h"

#include <iostream>
#include <sstream>


using namespace std;

#define MULTICAST_PORT 1900
#define MULTICAST_IP   "239.255.255.250"

//void *receive_loop(void *arg);
upnpThreadCallback receive_loop(void *arg);

CUDPSocket::CUDPSocket()
{	
    m_LocalEndpoint.sin_port = 0;
    m_ReceiveThread          = (upnpThread)NULL;
}	

CUDPSocket::~CUDPSocket()
{
}

upnpSocket CUDPSocket::get_socket_fd()
{
    return m_Socket;
}

void CUDPSocket::setup_socket(bool do_multicast)
{
    // create socket
    m_Socket = socket(PF_INET, SOCK_DGRAM, 0);
    if(m_Socket == -1)
        cout << "error: creating socket" << endl;

    int nRet  = 0;
    /*int flag = 1;
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));*/
    upnpSocketFlag(flag);
    nRet = setsockopt(m_Socket, SOL_SOCKET, SO_REUSEADDR, flag, sizeof(flag));
    if(-1 == nRet)	
        cout << "error: setsockopt" << endl;

    // set local endpoint		
    m_LocalEndpoint.sin_family = AF_INET;	
    if(do_multicast)
    {
        m_LocalEndpoint.sin_addr.s_addr = INADDR_ANY;
        m_LocalEndpoint.sin_port		= htons(MULTICAST_PORT);
    }
    else 
    {
        m_LocalEndpoint.sin_addr.s_addr = inet_addr(CSharedConfig::Shared()->GetIP().c_str());		
        m_LocalEndpoint.sin_port		= htons(0); // use random port
    }

    // bind socket
    nRet = bind(m_Socket, (struct sockaddr*)&m_LocalEndpoint, sizeof(m_LocalEndpoint)); 
    if(nRet == -1)
        cout << "error: bind" << endl;

    // get random port
    socklen_t nSize = sizeof(m_LocalEndpoint);
    getsockname(m_Socket, (struct sockaddr*)&m_LocalEndpoint, &nSize);

    // join multicast group
    m_fIsMulticast = do_multicast;
    if(do_multicast)
    {	
        struct ip_mreq stMreq; // Multicast interface structure  

        stMreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_IP); 
        stMreq.imr_interface.s_addr = INADDR_ANY; 	
        nRet = setsockopt(m_Socket, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&stMreq,sizeof(stMreq)); 	
        if(-1 == nRet)
            cout << "error: setsockopt multicast" << endl;
    }
}	

void CUDPSocket::teardown_socket()
{
    // leave multicast group
    if(m_fIsMulticast)
    {
        struct ip_mreq stMreq;

        stMreq.imr_multiaddr.s_addr = inet_addr("239.255.255.250"); 
        stMreq.imr_interface.s_addr = INADDR_ANY; 	
        setsockopt(m_Socket, IPPROTO_IP, IP_DROP_MEMBERSHIP,(char *)&stMreq,sizeof(stMreq)); 		
    }	
    upnpSocketClose(m_Socket);
}

void CUDPSocket::begin_receive()
{
    //  pthread_create(&receive_thread, NULL, &receive_loop, this);
    upnpThreadStart(m_ReceiveThread, receive_loop);
}

void CUDPSocket::send_multicast(std::string a_message)
{
    // create remote end point
    sockaddr_in remote_ep;	
    remote_ep.sin_family      = AF_INET;
    remote_ep.sin_addr.s_addr = inet_addr(MULTICAST_IP);
    remote_ep.sin_port		  = htons(MULTICAST_PORT);

    sendto(m_Socket, a_message.c_str(), (int)strlen(a_message.c_str()), 0, (struct sockaddr*)&remote_ep, sizeof(remote_ep));
}

int CUDPSocket::get_port()
{
    return ntohs(m_LocalEndpoint.sin_port);
}

std::string CUDPSocket::get_ip()
{
    return inet_ntoa(m_LocalEndpoint.sin_addr);
}

sockaddr_in CUDPSocket::get_local_ep()
{
    return m_LocalEndpoint;
}

//void *receive_loop(void *arg)
upnpThreadCallback receive_loop(void *arg)
{
    CUDPSocket* pUdpSocket = (CUDPSocket*)arg;
    cout << "[udp_socket] listening on " << pUdpSocket->get_ip() << ":" << pUdpSocket->get_port() << endl;

    char szBuffer[4096];	
    int  nBytesReceived = 0;

    stringstream msg;

    sockaddr_in remote_ep;	
    socklen_t   nSize = sizeof(remote_ep);	

    while((nBytesReceived = recvfrom(pUdpSocket->get_socket_fd(), szBuffer, sizeof(szBuffer), 0, (struct sockaddr*)&remote_ep, &nSize)) != -1)
    {
        szBuffer[nBytesReceived] = '\0';		
        msg << szBuffer;

        if((remote_ep.sin_addr.s_addr != pUdpSocket->get_local_ep().sin_addr.s_addr) || 
            (remote_ep.sin_port != pUdpSocket->get_local_ep().sin_port))		
        {
            CSSDPMessage Result(msg.str());		
            Result.SetRemoteEndPoint(remote_ep);
            Result.SetLocalEndPoint(pUdpSocket->get_local_ep());
            pUdpSocket->call_on_receive(&Result);
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

void CUDPSocket::call_on_receive(CSSDPMessage* pSSDPMessage)
{
    if(this->m_pReceiveHandler != NULL)
        m_pReceiveHandler->OnUDPSocketReceive(this, pSSDPMessage);
}