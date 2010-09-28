/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            HTTPServer.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../Common/Common.h"
#include "../Common/Thread.h"
#include "../Common/Socket.h"

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

class HTTPSession: public fuppes::Thread
{
	public:
    virtual ~HTTPSession();
		void run();
		
    HTTPSession(CHTTPServer* pHTTPServer, fuppes::TCPRemoteSocket* remoteSocket, std::string p_sHTTPServerURL)
			:Thread("httpsession")
    {
      m_pHTTPServer    = pHTTPServer;
      //m_Connection     = p_Connection;    
      m_bIsTerminated  = false;
      //m_RemoteEndPoint = p_RemoteEndPoint;
      m_sServerURL     = p_sHTTPServerURL;
      m_remoteSocket   = remoteSocket;
    }

    fuppesSocket GetConnection() { return m_remoteSocket->socket();  }
    CHTTPServer* GetHTTPServer() { return m_pHTTPServer; }
    std::string  GetHTTPServerURL() { return m_sServerURL; }    
    //fuppesThread GetThreadHandle() { return m_ThreadHandle; }
    //void         SetThreadHandle(fuppesThread p_ThreadHandle) { m_ThreadHandle = p_ThreadHandle; }
    struct sockaddr_in GetRemoteEndPoint() { return m_remoteSocket->remoteEndpoint(); }

    fuppes::TCPRemoteSocket*  socket() { return m_remoteSocket; }
    
    bool m_bIsTerminated;

	private:
  
    //void CleanupSessions();

    CHTTPServer*        m_pHTTPServer;
    //fuppesSocket        m_Connection;
    //fuppesThread        m_ThreadHandle;
    //struct sockaddr_in  m_RemoteEndPoint;
    std::string         m_sServerURL;
    fuppes::TCPRemoteSocket*    m_remoteSocket;
		
};

class HTTPSessionStore: public fuppes::Thread
{
  public:
    HTTPSessionStore():Thread("HTTPSessionStore") {
    }
    static void append(HTTPSession* session);
    static void finished(HTTPSession* session);

    static void init();
    static void uninit();
    
  private:
    static HTTPSessionStore* m_instance;
  	void run();

    fuppes::Mutex                        m_mutex;
    std::list<HTTPSession*>              m_sessions;
    std::list<HTTPSession*>::iterator    m_sessionsIterator;
    std::list<HTTPSession*>              m_finishedSessions;
    std::list<HTTPSession*>::iterator    m_finishedSessionsIterator;
};

class CHTTPServer: public fuppes::Thread
{

  public:     
    CHTTPServer(std::string p_sIPAddress);	
    ~CHTTPServer();

    void          Start();		
    void          Stop();
    //fuppesSocket  GetSocket() { return m_Socket; }
    std::string   GetURL();	
    //void          CleanupSessions();

    bool				  SetReceiveHandler(IHTTPServer* pHandler);
    bool          CallOnReceive(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);
		bool					isStarted() { return m_isStarted; }
	
    bool 					m_bBreakAccept;
		bool					m_isStarted;
	
  private: 

    // Eventhandler 
    IHTTPServer* m_pReceiveHandler;

    fuppes::TCPServer   m_listenSocket;
    //sockaddr_in local_ep;
    bool				do_break;
    bool        m_bIsRunning;

    //fuppesSocket      m_Socket;					      
    //fuppesThread      accept_thread;
		void run();
    //fuppesThreadMutex m_ReceiveMutex;
		fuppes::Mutex	 m_receiveMutex;

  public:
    /*std::list<CHTTPSessionInfo*> m_ThreadList;
    std::list<CHTTPSessionInfo*>::iterator m_ThreadListIterator;*/
		std::list<HTTPSession*> m_ThreadList;
    std::list<HTTPSession*>::iterator m_ThreadListIterator;

};

#endif // _HTTPSERVER_H
