/***************************************************************************
 *            UDPSocket.h
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
 
#ifndef _UDPSOCKET_H
#define _UDPSOCKET_H

#include "win32.h"

#ifndef WIN32
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>

#include "SSDPMessage.h"

class CUDPSocket;

class IUDPSocket
{
public:
    virtual void OnUDPSocketReceive(CUDPSocket*, CSSDPMessage*) = 0;
};

class CUDPSocket
{
public:
    CUDPSocket();
    ~CUDPSocket();

    void send_multicast(std::string a_message);

    void begin_receive();
    void setup_socket(bool do_multicast);
    void teardown_socket();
    void setup_random_port();

    upnpSocket get_socket_fd();

    void SetReceiveHandler(IUDPSocket*);
    void call_on_receive(CSSDPMessage*);

    int  get_port();
    std::string get_ip();
    sockaddr_in get_local_ep();	

private:
    upnpSocket  m_Socket;
    upnpThread  m_ReceiveThread;
    bool        m_fIsMulticast;
    sockaddr_in m_LocalEndpoint;	// local end point

    IUDPSocket* m_pReceiveHandler;

};

#endif /* _UDPSOCKET_H */