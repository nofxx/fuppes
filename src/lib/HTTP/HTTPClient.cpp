/***************************************************************************
 *            HTTPClient.cpp
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

#include "HTTPClient.h"
#include "CommonFunctions.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"

#ifndef WIN32
#include <pthread.h>
#endif

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

CHTTPClient::CHTTPClient(IHTTPClient* pAsyncReceiveHandler)
{
  m_AsyncThread = (fuppesThread)NULL;
  m_bIsAsync    = false;
	m_pAsyncReceiveHandler = pAsyncReceiveHandler;
}

CHTTPClient::~CHTTPClient()
{
  if(m_AsyncThread) {
    if(!m_bAsyncDone) {
      fuppesThreadCancel(m_AsyncThread);
    }
    fuppesThreadClose(m_AsyncThread);
  }

  upnpSocketClose(m_Socket);
}

fuppesThreadCallback AsyncThread(void* arg)
{
  CHTTPClient* pClient = (CHTTPClient*)arg;                  
  char buffer[16384];
  int nBytesReceived = 0;
  
  // connect socket
  if(connect(pClient->m_Socket, (struct sockaddr*)&pClient->m_RemoteEndpoint, sizeof(pClient->m_RemoteEndpoint)) == -1) {
    CSharedLog::Log(L_DBG, __FILE__, __LINE__, "failed to connect()");
    
    pClient->m_bAsyncDone = true;
    fuppesThreadExit();
  }
  
  // send message
	if(fuppesSocketSend(pClient->m_Socket, pClient->m_sMessage.c_str(), (int)strlen(pClient->m_sMessage.c_str())) <= 0) {
    CSharedLog::Log(L_DBG, __FILE__, __LINE__, "failed to send()");
    
    upnpSocketClose(pClient->m_Socket);
    pClient->m_bAsyncDone = true;
    fuppesThreadExit();
  }
  
  // receive answer
  char szTmpBuf[4096];
  int  nOffset = 0;
  
  while((nBytesReceived = recv(pClient->m_Socket, szTmpBuf, sizeof(szTmpBuf), 0)) > 0) {
    
    if((nOffset + nBytesReceived) > sizeof(buffer)) {      
      break;
    }
    
    memcpy(&buffer[nOffset], &szTmpBuf, nBytesReceived);
    nOffset += nBytesReceived;
  }
  buffer[nOffset] = '\0';
  
	pClient->m_sAsyncResult = buffer; //sReceived;
	CHTTPMessage Message;
	Message.SetMessage(buffer);
		
	if(pClient->m_pAsyncReceiveHandler != NULL) {
	  pClient->m_pAsyncReceiveHandler->OnAsyncReceiveMsg(&Message);
	}
					
  // clean up and exit
  upnpSocketClose(pClient->m_Socket);  
  pClient->m_bAsyncDone = true;
  fuppesThreadExit();
}

void CHTTPClient::AsyncNotify(CEventNotification* pNotification)
{
  // create socket
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(m_Socket == -1)    
    throw EException("failed to create socket", __FILE__, __LINE__);  
   
  // set local end point
	m_LocalEndpoint.sin_family      = AF_INET;
	m_LocalEndpoint.sin_addr.s_addr = inet_addr(CSharedConfig::Shared()->GetIPv4Address().c_str());
	m_LocalEndpoint.sin_port				= htons(0);
	memset(&(m_LocalEndpoint.sin_zero), '\0', 8);
  
  // bind the socket
	int nRet = bind(m_Socket, (struct sockaddr*)&m_LocalEndpoint, sizeof(m_LocalEndpoint));	
  if(nRet == -1)
    throw EException("failed to bind socket", __FILE__, __LINE__);
    
  // fetch local end point to get port number on random ports
	socklen_t size = sizeof(m_LocalEndpoint);
	getsockname(m_Socket, (struct sockaddr*)&m_LocalEndpoint, &size);
  
  // set remote end point
  m_RemoteEndpoint.sin_family      = AF_INET;
  m_RemoteEndpoint.sin_addr.s_addr = inet_addr(pNotification->GetSubscriberIP().c_str());
  m_RemoteEndpoint.sin_port        = htons(pNotification->GetSubscriberPort());
  memset(&(m_RemoteEndpoint.sin_zero), '\0', 8);
  
  // set the notification's host
  stringstream sHost;
  sHost << inet_ntoa(m_LocalEndpoint.sin_addr) << ":" << ntohs(m_LocalEndpoint.sin_port);  
  pNotification->SetHost(sHost.str());
  
  // start async send
  m_sMessage   = pNotification->BuildHeader() + pNotification->GetContent();
  m_bIsAsync   = true;
  m_bAsyncDone = false;
  fuppesThreadStartArg(m_AsyncThread, AsyncThread, *this);
}


/** HTTP GET implementation
 *  @param  p_sGetURL  the URL to GET
 *
 *  @return returns true on success otherwise false
 */
bool CHTTPClient::AsyncGet(std::string p_sGetURL)
{
  std::string  sGet;
	std::string  sIPAddress;
  unsigned int nPort;

  if(!SplitURL(p_sGetURL, &sIPAddress, &nPort))
    return false;

  RegEx rxGet("[http://]*[0-9|\\.]+:*[0-9]*([/|\\w|\\.]*)", PCRE_CASELESS);
  if(rxGet.Search(p_sGetURL.c_str())) {
		sGet = rxGet.Match(1);
  }
	else {
	  return false;
	}
	
  // create socket
	m_Socket = socket(AF_INET, SOCK_STREAM, 0);
  if(m_Socket == -1)    
    throw EException("failed to create socket", __FILE__, __LINE__);  
   
  // set local end point
	m_LocalEndpoint.sin_family      = AF_INET;
	m_LocalEndpoint.sin_addr.s_addr = inet_addr(CSharedConfig::Shared()->GetIPv4Address().c_str());
	m_LocalEndpoint.sin_port				= htons(0);
	memset(&(m_LocalEndpoint.sin_zero), '\0', 8);
  
  // bind the socket
	int nRet = bind(m_Socket, (struct sockaddr*)&m_LocalEndpoint, sizeof(m_LocalEndpoint));	
  if(nRet == -1)
    throw EException("failed to bind socket", __FILE__, __LINE__);
    
  // fetch local end point to get port number on random ports
	socklen_t size = sizeof(m_LocalEndpoint);
	getsockname(m_Socket, (struct sockaddr*)&m_LocalEndpoint, &size);
  
  // set remote end point
  m_RemoteEndpoint.sin_family      = AF_INET;
  m_RemoteEndpoint.sin_addr.s_addr = inet_addr(sIPAddress.c_str());
  m_RemoteEndpoint.sin_port        = htons(nPort);
  memset(&(m_RemoteEndpoint.sin_zero), '\0', 8);
  
  // build GET header
  std::string sMsg = BuildGetHeader(sGet, sIPAddress, nPort);  
  CSharedLog::Log(L_DBG, __FILE__, __LINE__, "send GET Header \n%s", sMsg.c_str());
	
  // start async send
  m_sMessage   = sMsg;
  m_bIsAsync   = true;
  m_bAsyncDone = false;
  fuppesThreadStartArg(m_AsyncThread, AsyncThread, *this);
	
	return true;
}

std::string CHTTPClient::BuildGetHeader(std::string p_sGet, std::string p_sTargetIPAddress, unsigned int p_nTargetPort)
{
  std::stringstream sHeader;
  
  // Build header
  sHeader << "GET " << p_sGet << " HTTP/1.0" << "\r\n";
  sHeader << "HOST: " << p_sTargetIPAddress << ":" << p_nTargetPort << "\r\n";
  sHeader << "CONTENT-LENGTH: 0" << "\r\n";
  sHeader << "USER-AGENT: " << CSharedConfig::Shared()->GetOSName() << "/" << CSharedConfig::Shared()->GetOSVersion() << ", " <<
             "UPnP/1.0, " << CSharedConfig::Shared()->GetAppFullname() << "/" << CSharedConfig::Shared()->GetAppVersion() << "\r\n";  
	
  sHeader << "\r\n";
  
  return sHeader.str();
}
