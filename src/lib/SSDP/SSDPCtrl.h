/***************************************************************************
 *            SSDPCtrl.h
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
 
#ifndef _SSDPCTRL_H
#define _SSDPCTRL_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../Common/Common.h"

#ifndef WIN32
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>
#include <list>

#include "UDPSocket.h"
#include "SSDPMessage.h"
#include "MSearchSession.h"
#include "NotifyMsgFactory.h"

class ISSDPCtrl
{
  public:
	  virtual ~ISSDPCtrl() {};
    virtual void OnSSDPCtrlReceiveMsg(CSSDPMessage*) = 0;
};

class CSSDPCtrl: public IUDPSocket, IMSearchSession
{
  public:
		CSSDPCtrl(std::string p_sIPAddress, std::string p_sHTTPServerURL);
		virtual ~CSSDPCtrl();

		void Start();
    void Stop();

		CUDPSocket* get_socket();

		void send_msearch();
	  void send_alive();
	  void send_byebye();


	  void SetReceiveHandler(ISSDPCtrl* pHandler);
	  void OnUDPSocketReceive(CSSDPMessage* pSSDPMessage);
   	void OnSessionReceive(CSSDPMessage* pMessage);
	
    void OnSessionTimeOut(CMSearchSession* pSender);

	private:

    void HandleMSearch(CSSDPMessage* pSSDPMessage);  
  
    void CleanupSessions();
  
    CUDPSocket         m_Listener;	
    CNotifyMsgFactory* m_pNotifyMsgFactory;
    fuppesThread       msearch_thread;
    sockaddr_in        m_LastMulticastEp;  
    std::string        m_sIPAddress;    
    std::string        m_sHTTPServerURL;
    ISSDPCtrl*         m_pReceiveHandler;
    fuppesThreadMutex  m_SessionReceiveMutex;
    fuppesThreadMutex  m_SessionTimedOutMutex;
  
    std::list<CMSearchSession*> m_SessionList;    
    std::list<CMSearchSession*>::iterator m_SessionListIterator;
    
    std::list<CHandleMSearchSession*> m_HandleMSearchThreadList;
    std::list<CHandleMSearchSession*>::iterator m_HandleMSearchThreadListIterator;
};

#endif // _SSDPCTRL_H
