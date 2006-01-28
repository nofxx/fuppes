/***************************************************************************
 *            MSearchSession.cpp
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
 
/* todo: create expiration timer */

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "MSearchSession.h"
#include "NotifyMsgFactory.h"
#include "SharedLog.h"

#include <iostream>
#include <sstream>

using namespace std;

/*===============================================================================
 CLASS CSSDPSession
===============================================================================*/

/* <PROTECTED> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CMSearchSession::CMSearchSession(std::string p_sIPAddress, IMSearchSession* pReceiveHandler, CNotifyMsgFactory* pNotifyMsgFactory)
  :m_Timer(this)
{
  ASSERT(NULL != pReceiveHandler);
  ASSERT(NULL != pNotifyMsgFactory);
  
  m_sIPAddress        = p_sIPAddress;
  m_pEventHandler     = pReceiveHandler;
  m_pNotifyMsgFactory = pNotifyMsgFactory;
  
  m_Timer.SetInterval(30);
  m_UdpSocket.SetupSocket(false, m_sIPAddress);	
  m_UdpSocket.SetTTL(4);
}

CMSearchSession::~CMSearchSession()
{
  m_UdpSocket.TeardownSocket();  
}

/* <\PROTECTED> */

/* <PUBLIC> */

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

void CMSearchSession::OnUDPSocketReceive(CUDPSocket* pSocket, CSSDPMessage* pSSDPMessage)
{
  if(m_pEventHandler != NULL)
    m_pEventHandler->OnSessionReceive(this, pSSDPMessage);
}

void CMSearchSession::OnTimer()
{
  Stop();
  if(m_pEventHandler != NULL)
  {
    m_pEventHandler->OnSessionTimeOut(this);
  }  
}

/*===============================================================================
 CONTROL
===============================================================================*/

void CMSearchSession::Start()
{
	begin_receive_unicast();
	fuppesSleep(200);
	send_multicast(m_pNotifyMsgFactory->msearch());
  m_Timer.Start();
}

void CMSearchSession::Stop()
{
  m_Timer.Stop();
	end_receive_unicast();
}

/*===============================================================================
 SEND/RECEIVE
===============================================================================*/

void CMSearchSession::send_multicast(std::string a_message)
{
	/* Send message twice */
  m_UdpSocket.SendMulticast(a_message);
	fuppesSleep(200);
	m_UdpSocket.SendMulticast(a_message);	
}

void CMSearchSession::send_unicast(std::string)
{
}

void CMSearchSession::begin_receive_unicast()
{	
	/* Start receiving messages */
  m_UdpSocket.SetReceiveHandler(this);
	m_UdpSocket.BeginReceive();
}

void CMSearchSession::end_receive_unicast()
{
  /* End receiving messages */
  m_UdpSocket.EndReceive();
}

sockaddr_in CMSearchSession:: GetLocalEndPoint()
{
	return m_UdpSocket.GetLocalEndPoint();
}

/* <\PUBLIC> */

fuppesThreadCallback HandleMSearchThread(void *arg);

CHandleMSearchSession::CHandleMSearchSession(CSSDPMessage* pSSDPMessage, std::string p_sIPAddress, std::string p_sHTTPServerURL)
{
  m_bIsTerminated     = false;
  m_sIPAddress        = p_sIPAddress;
  m_sHTTPServerURL    = p_sHTTPServerURL;
  m_pSSDPMessage      = pSSDPMessage;
  m_pNotifyMsgFactory = new CNotifyMsgFactory(m_sHTTPServerURL);
}
   
CHandleMSearchSession::~CHandleMSearchSession()
{
  delete m_pNotifyMsgFactory;
}

void CHandleMSearchSession::Start()
{
  m_bIsTerminated = false;
  fuppesThreadStartArg(m_Thread, HandleMSearchThread, *this);
}

fuppesThreadCallback HandleMSearchThread(void *arg)
{
  cout << "HandleMSearchThread" << endl;
  fflush(stdout);
  
  CHandleMSearchSession* pSession = (CHandleMSearchSession*) arg;
    
  cout << "sess: " << pSession->m_bIsTerminated << endl;
  fflush(stdout);
  
  
  if(pSession->GetSSDPMessage()->GetMSearchST() != M_SEARCH_ST_UNSUPPORTED)
  {    
    CSharedLog::Shared()->ExtendedLog("CHandleMSearchSession", "unicasting response");
     
    cout << "HandleMSearch - MX = " << pSession->GetSSDPMessage()->GetMX() << endl;
    
    CUDPSocket Sock;
    Sock.SetupSocket(false, pSession->m_sIPAddress);
  
    /* calculate mx delay */
    int nSleepMS = pSession->GetSSDPMessage()->GetMX() * 1000;
    if(pSession->GetSSDPMessage()->GetMSearchST() == M_SEARCH_ST_ALL)
      nSleepMS /= 6;
    
    if(pSession->GetSSDPMessage()->GetMSearchST() == M_SEARCH_ST_ALL)
    {
      fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_ROOT_DEVICE), pSession->GetSSDPMessage()->GetRemoteEndPoint());
      fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONNECTION_MANAGER), pSession->GetSSDPMessage()->GetRemoteEndPoint());
      fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONTENT_DIRECTORY), pSession->GetSSDPMessage()->GetRemoteEndPoint());
      fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_MEDIA_SERVER), pSession->GetSSDPMessage()->GetRemoteEndPoint());
      fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_USN), pSession->GetSSDPMessage()->GetRemoteEndPoint());
    }
    else
    {
      fuppesSleep(nSleepMS);
    }
      
    Sock.TeardownSocket();
    CSharedLog::Shared()->ExtendedLog("CHandleMSearchSession", "done");
  }  
  
  pSession->m_bIsTerminated = true;
}
