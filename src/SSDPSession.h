/***************************************************************************
 *            SSDPSession.h
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
 
#ifndef _SSDPSESSION_H
#define _SSDPSESSION_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>

#include "UDPSocket.h"
#include "Timer.h"
#include "SSDPMessage.h"
#include "NotifyMsgFactory.h"

/*===============================================================================
 FORWARD DECLARATIONS
===============================================================================*/

class CSSDPSession;

/*===============================================================================
 CLASS ISSDPSession
===============================================================================*/

class ISSDPSession
{

/* <PUBLIC> */

public:
    
  virtual void OnSessionReceive(CSSDPSession* pSender, CSSDPMessage* pMessage) = 0;
  virtual void OnSessionTimeOut(CSSDPSession* pSender) = 0;

/* <\PUBLIC> */

};

/*===============================================================================
 CLASS CSSDPSession
===============================================================================*/

class CSSDPSession: public IUDPSocket, ITimer
{

/* <PROTECTED> */

protected:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  CSSDPSession(std::string p_sIPAddress, ISSDPSession* pEventHandler);

public:
  virtual ~CSSDPSession();

/* <\PROTECTED> */

/* <PUBLIC> */

public:	

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/
  
  void OnUDPSocketReceive(CUDPSocket* pSocket, CSSDPMessage* pMessage);

  void OnTimer();

/* <\PUBLIC> */
	
/* <PROTECTED> */

/*===============================================================================
 CONTROL
===============================================================================*/

  virtual void Start();	  
  virtual void Stop();

/*===============================================================================
 SEND/RECEIVE
===============================================================================*/

  void send_multicast(std::string);
  void send_unicast(std::string);

  void begin_receive_unicast();
  void end_receive_unicast();

/*===============================================================================
 MEMBERS
===============================================================================*/
  protected:
		int           m_nTimeout;
	  CUDPSocket    m_UdpSocket;
    std::string   m_sIPAddress;
    ISSDPSession* m_pEventHandler;
    CTimer        m_Timer;

/* <\PROTECTED> */

};

/*===============================================================================
 CLASS CMSearchSession
===============================================================================*/

class CMSearchSession: public CSSDPSession
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR/DESTRUCTOR
===============================================================================*/

	  CMSearchSession(std::string p_sIPAddress, ISSDPSession* pReceiveHandler, CNotifyMsgFactory* pNotifyMsgFactory);
    ~CMSearchSession();
	
/*===============================================================================
 CONTROL
===============================================================================*/
    
  void Start();
  void Stop();

/*===============================================================================
 GET
===============================================================================*/    
    
  sockaddr_in  GetLocalEndPoint();

/* <\PUBLIC> */

/* <PRIVATE> */

private:

/*===============================================================================
 MEMBERS
===============================================================================*/    
  
  CNotifyMsgFactory* m_pNotifyMsgFactory;

/* <PRIVATE> */

};

#endif /* _SSDPSESSION_H */
