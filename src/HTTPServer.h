/***************************************************************************
 *            HTTPServer.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 
#ifndef _HTTPSERVER_H
#define _HTTPSERVER_H

#include "win32.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#endif

#include <string>
#include "HTTPMessage.h"

#include <iostream>
using namespace std;

class IHTTPServer
{
  public:
	  virtual CHTTPMessage* OnHTTPServerReceiveMsg(CHTTPMessage*) = 0;
};

class CHTTPServer;

class CHTTPSessionInfo
{
  public:
    CHTTPSessionInfo(CHTTPServer* pHTTPServer, upnpSocket p_Connection)
    {
      m_pHTTPServer = pHTTPServer;
      m_Connection  = p_Connection;
    }
    
    upnpSocket GetConnection() { return m_Connection; }
    CHTTPServer* GetHTTPServer() { return m_pHTTPServer; }
  
  private:
    CHTTPServer* m_pHTTPServer;
    upnpSocket   m_Connection;
};

class CHTTPServer
{
	public:
	  CHTTPServer();	
	  ~CHTTPServer();
		  
		void          Start();		
	  upnpSocket    GetSocket();
	  std::string   GetURL();	
		void				  SetReceiveHandler(IHTTPServer*);		
		CHTTPMessage* CallOnReceive(std::string);		
	
	private:
	  sockaddr_in local_ep;
		bool				do_break;
	
    upnpSocket sock;					      
    upnpThread accept_thread;			  

		// eventhandler
		IHTTPServer* m_pReceiveHandler;
};

#endif /* _HTTPSERVER_H */
