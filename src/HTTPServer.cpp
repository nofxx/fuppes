/***************************************************************************
 *            HTTPServer.cpp
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

#include "HTTPServer.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

upnpThreadCallback accept_loop(void *arg);

CHTTPServer::CHTTPServer()
{
	accept_thread = (upnpThread)NULL;
	
	sock = socket(AF_INET, SOCK_STREAM, 0);
	
	local_ep.sin_family      = PF_INET;
	local_ep.sin_addr.s_addr = inet_addr(CSharedConfig::Shared()->GetIP().c_str());
	local_ep.sin_port				 = htons(0);
	
	bind(sock, (struct sockaddr*)&local_ep, sizeof(local_ep));	
	socklen_t size = sizeof(local_ep);
	getsockname(sock, (struct sockaddr*)&local_ep, &size);	
}

CHTTPServer::~CHTTPServer()
{
}

upnpSocket CHTTPServer::GetSocket()
{
	return sock;
}

std::string CHTTPServer::GetURL()
{
	stringstream result;
	result << inet_ntoa(local_ep.sin_addr) << ":" << ntohs(local_ep.sin_port);
	return result.str();
}

void CHTTPServer::Start()
{
	do_break = false;			
	listen(sock, 3);
	
	upnpThreadStart(accept_thread, accept_loop);
}

upnpThreadCallback accept_loop(void *arg)
{
	CHTTPServer* pHTTPServer = (CHTTPServer*)arg;
	cout << "[HTTPServer] listening on " << pHTTPServer->GetURL() << endl;	
	
	upnpSocket sock       = pHTTPServer->GetSocket();			
	upnpSocket connection = 0;	
	
	struct sockaddr_in remote_ep;
	socklen_t size = sizeof(remote_ep);
	
	for(;;)
	{
		connection = accept(sock, (struct sockaddr*)&remote_ep, &size);
		if(connection != -1)
		{	
			cout << "[HTTPServer] new connection from " <<  inet_ntoa(remote_ep.sin_addr) << ":" << ntohs(remote_ep.sin_port) << endl;
			
			int  nBytesReceived = 0;
			char szBuffer[4096];
						
			size = sizeof(remote_ep);
			nBytesReceived = recv(connection, szBuffer, 4096, 0); // MSG_DONTWAIT
			if(nBytesReceived != -1)			
			{
				cout << "[HTTPServer] bytes received: " << nBytesReceived << endl;
				szBuffer[nBytesReceived] = '\0';
				cout << szBuffer << endl;
				
				CHTTPMessage* pResponse = pHTTPServer->CallOnReceive(szBuffer);
				if(pResponse != NULL)				
				{
					cout << "[HTTPServer] sending response" << endl;
			    //cout << pResponse->GetMessageAsString() << endl;
					send(connection, pResponse->GetMessageAsString().c_str(), strlen(pResponse->GetMessageAsString().c_str()), 0);
					cout << "[HTTPServer] done" << endl;
				}
				
				delete pResponse;
			}			
			upnpSocketClose(connection);
		}
	}	
	
	return 0;
}

void CHTTPServer::SetReceiveHandler(IHTTPServer* pHandler)
{
	m_pReceiveHandler = pHandler;
}

CHTTPMessage* CHTTPServer::CallOnReceive(std::string p_sMessage)
{
	if(m_pReceiveHandler != NULL)
	{
		// parse message
		CHTTPMessage* pMsg = new CHTTPMessage(p_sMessage);		
		return m_pReceiveHandler->OnHTTPServerReceiveMsg(pMsg);
	}
	else
	{
		return NULL;
	}
}
