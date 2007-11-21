/***************************************************************************
 *            MSearchSession.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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
 
#include "MSearchSession.h"
#include "NotifyMsgFactory.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"

#include <iostream>
#include <sstream>
#include <time.h>

using namespace std;

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

void CMSearchSession::OnUDPSocketReceive(CSSDPMessage* pSSDPMessage)
{
  if(m_pEventHandler != NULL)
    m_pEventHandler->OnSessionReceive(pSSDPMessage);
}

void CMSearchSession::OnTimer()
{
  Stop();
  if(m_pEventHandler != NULL)
  {
    m_pEventHandler->OnSessionTimeOut(this);
  }  
}

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

fuppesThreadCallback HandleMSearchThread(void *arg);

CHandleMSearchSession::CHandleMSearchSession(CSSDPMessage* pSSDPMessage, std::string p_sIPAddress, std::string p_sHTTPServerURL)
{
  m_bIsTerminated     = false;
  m_sIPAddress        = p_sIPAddress;
  m_sHTTPServerURL    = p_sHTTPServerURL;
  m_pSSDPMessage      = new CSSDPMessage();
  pSSDPMessage->Assign(m_pSSDPMessage);
  m_pNotifyMsgFactory = new CNotifyMsgFactory(m_sHTTPServerURL);
  m_Thread            = (fuppesThread)NULL;
}
   
CHandleMSearchSession::~CHandleMSearchSession()
{
  if(m_Thread)
    fuppesThreadClose(m_Thread);
  delete m_pSSDPMessage;
  delete m_pNotifyMsgFactory;
}

void CHandleMSearchSession::Start()
{
  m_bIsTerminated = false;
  fuppesThreadStartArg(m_Thread, HandleMSearchThread, *this);
}

fuppesThreadCallback HandleMSearchThread(void *arg)
{
  /*cout << "HandleMSearchThread" << endl;
  fflush(stdout);*/
  
  CHandleMSearchSession* pSession = (CHandleMSearchSession*) arg;
 
  if(pSession->GetSSDPMessage()->GetMSearchST() != M_SEARCH_ST_UNSUPPORTED)
  {    
    CSharedLog::Shared()->ExtendedLog("CHandleMSearchSession", "unicasting response");
     
    /*cout << "HandleMSearch - MX = " << pSession->GetSSDPMessage()->GetMX() << endl;
    fflush(stdout);*/
    CUDPSocket Sock;
    Sock.SetupSocket(false, pSession->m_sIPAddress);
  
    /* calculate mx delay */
    /* initialize random generator */
    srand (time(NULL));    
    
    int nRand = rand(); // % pSession->GetSSDPMessage()->GetMX();
    //cout << nRand << endl;
    if(pSession->GetSSDPMessage()->GetMX() > 0)
      nRand %= pSession->GetSSDPMessage()->GetMX() + 1;
    else
      nRand = 0;
    /*cout << nRand << endl;    
    fflush(stdout);    */
    int nSleepMS = nRand * 1000;
    if(nRand == pSession->GetSSDPMessage()->GetMX())
      nSleepMS -= 500;      
    //cout << "SLEEP MS: " << nSleepMS << endl;
    //nSleepMS *= 1000;
    if((pSession->GetSSDPMessage()->GetMSearchST() == M_SEARCH_ST_ALL) && nSleepMS > 0)
      nSleepMS /= 6;
    
		std::string sUUID = "";
		
    if(pSession->GetSSDPMessage()->GetMSearchST() == M_SEARCH_ST_ALL)
    {
      fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_USN), pSession->GetSSDPMessage()->GetRemoteEndPoint());
			fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_ROOT_DEVICE), pSession->GetSSDPMessage()->GetRemoteEndPoint());
      fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_MEDIA_SERVER), pSession->GetSSDPMessage()->GetRemoteEndPoint());
			fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONTENT_DIRECTORY), pSession->GetSSDPMessage()->GetRemoteEndPoint());
			fuppesSleep(nSleepMS);
      Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONNECTION_MANAGER), pSession->GetSSDPMessage()->GetRemoteEndPoint());
    }
    else
    {
      //cout << "handling special search: " << pSession->GetSSDPMessage()->GetMSearchST() << endl;      
      fuppesSleep(nSleepMS);
			switch(pSession->GetSSDPMessage()->GetMSearchST())      
      {
        case M_SEARCH_ST_ROOT:          
          Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_ROOT_DEVICE), pSession->GetSSDPMessage()->GetRemoteEndPoint());          
          break;
        case M_SEARCH_ST_DEVICE_MEDIA_SERVER:
          Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_MEDIA_SERVER), pSession->GetSSDPMessage()->GetRemoteEndPoint());          
          break;
        case M_SEARCH_ST_SERVICE_CONTENT_DIRECTORY:
          Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONTENT_DIRECTORY), pSession->GetSSDPMessage()->GetRemoteEndPoint());          
          break;
        case M_SEARCH_ST_SERVICE_CONNECTION_MANAGER:
          Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_CONNECTION_MANAGER), pSession->GetSSDPMessage()->GetRemoteEndPoint());             
          break;          
        case M_SEARCH_ST_UUID:
          sUUID = pSession->GetSSDPMessage()->GetSTAsString().substr(5);
          if(sUUID.compare(ToLower(CSharedConfig::Shared()->GetUUID())) == 0) {
            Sock.SendUnicast(pSession->m_pNotifyMsgFactory->GetMSearchResponse(MESSAGE_TYPE_USN), pSession->GetSSDPMessage()->GetRemoteEndPoint());            
          }
          break;
				default:
				  break;
       }
    }
      
    Sock.TeardownSocket();
    CSharedLog::Shared()->ExtendedLog("CHandleMSearchSession", "done");
  }  
  
  /*cout << "HandleMSearchThread DONE" << endl;
  fflush(stdout);  */
  
  pSession->m_bIsTerminated = true;
  fuppesThreadExit();
}
