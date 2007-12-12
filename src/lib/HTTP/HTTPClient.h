/***************************************************************************
 *            HTTPClient.h
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
 
#ifndef _HTTPCLIENT_H
#define _HTTPCLIENT_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "HTTPMessage.h"
#include "../GENA/EventNotification.h"
#include "../Common/Common.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

class IHTTPClient
{
  public:
    virtual ~IHTTPClient() {};
  
	  virtual void OnAsyncReceiveMsg(CHTTPMessage* pMessage) = 0;
};

class CHTTPClient
{
  public:
    CHTTPClient(IHTTPClient* pAsyncReceiveHandler = NULL);
    ~CHTTPClient();

    bool AsyncGet(std::string p_sGetURL);
    void AsyncNotify(CEventNotification* pNotification);
 
    fuppesThread m_AsyncThread;
    std::string  m_sAsyncResult;
    std::string  m_sMessage;
    bool         m_bAsyncDone;
    bool         m_bIsAsync;
 	  IHTTPClient* m_pAsyncReceiveHandler;

    sockaddr_in  m_LocalEndpoint;
    sockaddr_in  m_RemoteEndpoint;

    upnpSocket   m_Socket;

  private:
    std::string BuildGetHeader(std::string  p_sGet,
                               std::string  p_sTargetIPAddress,
                               unsigned int p_nTargetPort); 
};

#endif // _HTTPCLIENT_H
