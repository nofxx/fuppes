/***************************************************************************
 *            MSearchSession.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _MSEARCHSESSION_H
#define _MSEARCHSESSION_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>

#include "UDPSocket.h"
#include "../Common/Timer.h"
#include "SSDPMessage.h"
#include "NotifyMsgFactory.h"

/*===============================================================================
 FORWARD DECLARATIONS
===============================================================================*/

class CMSearchSession;

/*===============================================================================
 CLASS ISSDPSession
===============================================================================*/

class IMSearchSession
{

/* <PUBLIC> */

public:
    
  virtual void OnSessionReceive(CSSDPMessage* pMessage) = 0;
  virtual void OnSessionTimeOut(CMSearchSession* pSender) = 0;

/* <\PUBLIC> */

};

/*===============================================================================
 CLASS CSSDPSession
===============================================================================*/

class CMSearchSession: public IUDPSocket, ITimer
{

/* <PUBLIC> */
/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  //CSSDPSession(std::string p_sIPAddress, ISSDPSession* pEventHandler);

public:
  CMSearchSession(std::string p_sIPAddress, IMSearchSession* pReceiveHandler, CNotifyMsgFactory* pNotifyMsgFactory);
  virtual ~CMSearchSession();


/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/
  
  void OnUDPSocketReceive(CSSDPMessage* pMessage);

  void OnTimer();

/* <\PUBLIC> */
	
/*===============================================================================
 CONTROL
===============================================================================*/

  void Start();	  
  void Stop();

/*===============================================================================
 SEND/RECEIVE
===============================================================================*/

  void send_multicast(std::string);
  void send_unicast(std::string);

  void begin_receive_unicast();
  void end_receive_unicast();

  sockaddr_in  GetLocalEndPoint();
/*===============================================================================
 MEMBERS
===============================================================================*/
  private:
		int                m_nTimeout;
	  CUDPSocket         m_UdpSocket;
    std::string        m_sIPAddress;
    IMSearchSession*   m_pEventHandler;
    CTimer             m_Timer;    
    CNotifyMsgFactory* m_pNotifyMsgFactory;
};

/*===============================================================================
 CLASS CMSearchSession
===============================================================================*/

class CHandleMSearchSession;

class IHandleMSearchSession
{
  public:
    virtual void OnSessionEnd(CHandleMSearchSession* pSender) = 0;
};
  
class CHandleMSearchSession
{
  public:
    CHandleMSearchSession(CSSDPMessage* pSSDPMessage, std::string p_sIPAddress, std::string p_sHTTPServerURL);
    ~CHandleMSearchSession();
    void Start();
  
    bool m_bIsTerminated;
    std::string m_sIPAddress;
    std::string m_sHTTPServerURL;
    CNotifyMsgFactory* m_pNotifyMsgFactory;
    
    CSSDPMessage* GetSSDPMessage() { return m_pSSDPMessage; }
  
  private:
    fuppesThread m_Thread;
    CSSDPMessage* m_pSSDPMessage;
};

#endif /* _MSEARCHSESSION_H */
