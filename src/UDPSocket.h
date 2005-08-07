/***************************************************************************
 *            UDPSocket.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

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

/*===============================================================================
 FORWARD DECLARATIONS
===============================================================================*/

class CUDPSocket;

/*===============================================================================
 CLASS IUDPSocket
===============================================================================*/

class IUDPSocket
{

/* <PUBLIC> */

public:

  virtual void OnUDPSocketReceive(
    CUDPSocket*   pSocket,
    CSSDPMessage* pMessage) = 0;

/* <\PUBLIC> */

};

/*===============================================================================
 CLASS CUDPSocket
===============================================================================*/

class CUDPSocket
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
   */
  CUDPSocket();

  /** destructor
   */
  ~CUDPSocket();

/*===============================================================================
 CONTROL SOCKET
===============================================================================*/

  /** initializes the socket
   *  @param  p_bDoMulticast  indicates whether to multicast or not
   *  @param  p_sIPAddress    the ip address. irrelevant if p_bDoMulticast is true
   *  @return returns true on success otherwise false
   */
  bool SetupSocket(bool p_bDoMulticast, std::string p_sIPAddress = "");

  /** finalizes and closes the socket
   */
  void TeardownSocket();

/*===============================================================================
 SEND MESSAGES
===============================================================================*/

  /** multicasts a message
   *  @param  p_sMessage  the message to send
   */
  void SendMulticast(std::string p_sMessage);

  /** unicasts a message
   *  @param  p_sMessage  the message to send
   *  @param  p_RemoteEndPoint  the receiver
   */
  void SendUnicast(std::string p_sMessage, sockaddr_in p_RemoteEndPoint);

/*===============================================================================
 RECEIVE MESSAGES
===============================================================================*/

  /** starts the receive thread
   */
  void BeginReceive();

  /** ends the receive thread
   */
  void EndReceive();

  /** sets the receive handler
   *  @param  pISocket  the receiver handler
   */	
  void SetReceiveHandler(IUDPSocket* pISocket);

  /** lets the receiver handler handle a message
   *  @param  pMessage  the message to handle
   */	    
  void CallOnReceive(CSSDPMessage* pMessage);

/*===============================================================================
 GET
===============================================================================*/

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
 
/* <\PUBLIC> */

/* <PRIVATE> */

private:

/*===============================================================================
 MEMBERS
===============================================================================*/

  /* Flags */
  bool          m_bDoMulticast;	

  /* Socket */
  upnpSocket    m_Socket;					
  sockaddr_in   m_LocalEndpoint;

  /* Message handling */
  fuppesThread  m_ReceiveThread;
  IUDPSocket*   m_pReceiveHandler;
	
/* <\PRIVATE> */

};

#endif /* _UDPSOCKET_H */
