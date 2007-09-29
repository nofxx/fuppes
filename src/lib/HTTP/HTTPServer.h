/***************************************************************************
 *            HTTPServer.h
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
 
#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../Common/Common.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#endif

#include <string>
#include <iostream>
#include <list>

using namespace std;

class CHTTPServer;
class CHTTPMessage;

class IHTTPServer
{
  public:
    virtual ~IHTTPServer() {};
  
	  virtual bool OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut) = 0;
};

class CHTTPSessionInfo
{

  public:

    CHTTPSessionInfo(CHTTPServer* pHTTPServer, upnpSocket p_Connection, struct sockaddr_in p_RemoteEndPoint, std::string p_sHTTPServerURL)
    {
      m_pHTTPServer    = pHTTPServer;
      m_Connection     = p_Connection;    
      m_bIsTerminated  = false;
      m_RemoteEndPoint = p_RemoteEndPoint;
      m_sServerURL     = p_sHTTPServerURL;
    }

    fuppesSocket GetConnection() { return m_Connection;  }
    CHTTPServer* GetHTTPServer() { return m_pHTTPServer; }
    std::string  GetHTTPServerURL() { return m_sServerURL; }    
    fuppesThread GetThreadHandle() { return m_ThreadHandle; }
    void         SetThreadHandle(fuppesThread p_ThreadHandle) { m_ThreadHandle = p_ThreadHandle; }
    struct sockaddr_in GetRemoteEndPoint() { return m_RemoteEndPoint; }
  
    bool m_bIsTerminated;
       
  private:
  
    void CleanupSessions();

    CHTTPServer*        m_pHTTPServer;
    fuppesSocket        m_Connection;
    fuppesThread        m_ThreadHandle;
    struct sockaddr_in  m_RemoteEndPoint;
    std::string         m_sServerURL;

};

class CHTTPServer
{

  public:     
    CHTTPServer(std::string p_sIPAddress);	
    ~CHTTPServer();

    void          Start();		
    void          Stop();
    fuppesSocket  GetSocket() { return m_Socket; }
    std::string   GetURL();	
    void          CleanupSessions();

    bool				  SetReceiveHandler(IHTTPServer* pHandler);
    bool          CallOnReceive(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);
   
    bool m_bBreakAccept;

  private: 

    // Eventhandler 
    IHTTPServer* m_pReceiveHandler;

    sockaddr_in local_ep;
    bool				do_break;
    bool        m_bIsRunning;

    fuppesSocket      m_Socket;					      
    fuppesThread      accept_thread;
    fuppesThreadMutex m_ReceiveMutex;

  public:
    std::list<CHTTPSessionInfo*> m_ThreadList;
    std::list<CHTTPSessionInfo*>::iterator m_ThreadListIterator;

};

#endif // _HTTPSERVER_H
