/***************************************************************************
 *            UDPSocket.h
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
 
#ifndef _UDPSOCKET_H
#define _UDPSOCKET_H

#include "Common.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#endif

#include <string>

#include "SSDPMessage.h"

class CUDPSocket;

class IUDPSocket
{
	public:
		virtual bool OnUDPSocketReceive(CUDPSocket*, CSSDPMessage*) = 0;
};

class CUDPSocket
{
	public:
    /** constructor
     */
		CUDPSocket();
  
    /** destructor
     */
		~CUDPSocket();
	
    /** initializes the socket
     *  @param  p_bDoMulticast  indicates whether to multicast or not
     *  @param  p_sIPAddress    the ip address. irrelevant if p_bDoMulticast is true
     *  @return returns true on success otherwise false
     */
		bool SetupSocket(bool p_bDoMulticast, std::string p_sIPAddress = "");
  
    /** finalizes and closes the socket
     */
	  void TeardownSocket();	  

    /** multicasts a message
     *  @param  p_sMessage  the message to send
     */
    void SendMulticast(std::string p_sMessage);
    
    /** unicastcasts a message
     *  @param  p_sMessage  the message to send
     *  @param  p_RemoteEndPoint  the receiver
     */
    void SendUnicast(std::string p_sMessage, sockaddr_in p_RemoteEndPoint);
  
    /** starts the receive thread
     */
		void BeginReceive();
    
    /** ends the receive thread
     */
    void EndReceive();
	
    /** returns the socket's descriptor
     *  @return the descriptor
     */
		upnpSocket GetSocketFd();

    /** returns the socket's port
     *  @return the port number
     */
		int  GetPort();
    
    /** returns the socket's IP-Address as string
     *  @return the address
     */
	  std::string GetIPAddress();
    
    /** returns the local end point
     *  @return the end point structure
     */
	  sockaddr_in GetLocalEndPoint();	

	  void SetReceiveHandler(IUDPSocket*);
	  void CallOnReceive(CSSDPMessage*);
  
	private:
    upnpSocket    sock;					
  	fuppesThread  receive_thread;					  
		bool          m_bDoMulticast;	
	  sockaddr_in   local_ep;	// local end point
	
	  IUDPSocket*   m_pReceiveHandler;
	
};

#endif /* _UDPSOCKET_H */
