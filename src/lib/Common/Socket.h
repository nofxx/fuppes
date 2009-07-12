/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Socket.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef _SOCKET_H
#define _SOCKET_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifdef WIN32

#pragma comment(lib,"Wsock32.lib") 
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <shlwapi.h>

#else

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#endif

#include "Common.h"
#include <string>

namespace fuppes {

class SocketBase
{
	protected:
		SocketBase();
		~SocketBase() { }
		
	public:
		bool setNonBlocking();
		bool close();

	protected:		
		#ifdef WIN32
		SOCKET		m_socket;
		#else
		int				m_socket;
		#endif
	
	private:
		bool			m_nonBlocking;
};	

	
class UDPSocket: private SocketBase
{
	public:
		UDPSocket() { }
		~UDPSocket() { }    
};

class TCPSocket: public SocketBase
{
	public:
		TCPSocket();
		~TCPSocket();
		
		bool					connect();
		fuppes_off_t	send(std::string message);
		fuppes_off_t	send(const unsigned char* buffer, fuppes_off_t size);
		// timeout works only on nonblocking sockets and if "select()" is available
		fuppes_off_t	receive(int timeout = 0);
		
		char*					buffer() { return m_buffer; }
		fuppes_off_t	bufferSize() { return m_bufferSize; }
		fuppes_off_t	bufferFill() { return m_bufferFill; }
		
		std::string		localAddress() { return inet_ntoa(m_localEndpoint.sin_addr); }
		int						localPort() { return ntohs(m_localEndpoint.sin_port); }
		
		void					remoteAddress(std::string address) { m_remoteAddress = address; }
		void					remotePort(unsigned int port) { m_remotePort = port; }
		
	private:
    sockaddr_in  m_localEndpoint;
    sockaddr_in  m_remoteEndpoint;

		std::string	 m_remoteAddress;
		unsigned int m_remotePort;

		char*				 m_buffer;
		fuppes_off_t m_bufferSize;
		fuppes_off_t m_bufferFill;
};

class TCPServer: private SocketBase
{
	public:
		TCPServer() { }
		~TCPServer() { }   
};

}

#endif // _SOCKET_H
