/***************************************************************************
 *            HTTPServer.h
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
 
#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

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
#include <iostream>
#include <list>

using namespace std;

/*===============================================================================
 FORWARD DECLARATIONS
===============================================================================*/

class CHTTPServer;
class CHTTPMessage;

/*===============================================================================
 CLASS IHTTPServer
===============================================================================*/

class IHTTPServer
{

/* <PUBLIC> */

  public:

	  virtual bool OnHTTPServerReceiveMsg(
      CHTTPMessage* pMessageIn,
      CHTTPMessage* pMessageOut) = 0;

/* <\PUBLIC> */

};

/*===============================================================================
 CLASS CHTTPSessionInfo
===============================================================================*/

class CHTTPSessionInfo
{

/* <PUBLIC> */

public:
    
/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  CHTTPSessionInfo(CHTTPServer* pHTTPServer, upnpSocket p_Connection)
  {
    m_pHTTPServer   = pHTTPServer;
    m_Connection    = p_Connection;    
    m_bIsTerminated = false;
  }

/*===============================================================================
 GET
===============================================================================*/

    fuppesSocket GetConnection() { return m_Connection;  }
    CHTTPServer* GetHTTPServer() { return m_pHTTPServer; }
    fuppesThread GetThreadHandle() { return m_ThreadHandle; }
    void         SetThreadHandle(fuppesThread p_ThreadHandle) { m_ThreadHandle = p_ThreadHandle; }
    
  
/* <\PUBLIC> */

    bool m_bIsTerminated;
    
/* <PRIVATE> */    
    
private:
  
  void CleanupSessions();

/*===============================================================================
 MEMBERS
===============================================================================*/
  
  CHTTPServer* m_pHTTPServer;
  fuppesSocket m_Connection;
  fuppesThread m_ThreadHandle;

/* <\PRIVATE> */

};

/*===============================================================================
 CLASS CHTTPServer
===============================================================================*/

class CHTTPServer
{

/* <PUBLIC> */

public:
    
/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/
	  
  CHTTPServer(std::string p_sIPAddress);	
  ~CHTTPServer();

/*===============================================================================
 COMMON
===============================================================================*/

  void          Start();		
  void          Stop();
  upnpSocket    GetSocket();
  std::string   GetURL();	
  void          CleanupSessions();

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

  bool				  SetReceiveHandler(IHTTPServer* pHandler);
  bool          CallOnReceive(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);
   
/* <\PUBLIC> */

  bool m_bBreakAccept;

/* <PRIVATE> */

private: 

/*===============================================================================
 MEMBERS
===============================================================================*/

  /* Eventhandler */
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

/* <\PRIVATE> */

};

#endif /* _HTTPSERVER_H */
