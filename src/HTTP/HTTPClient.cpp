/***************************************************************************
 *            HTTPClient.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "HTTPClient.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../SharedLog.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#endif

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

CHTTPClient::CHTTPClient()
{
  m_AsyncThread = (fuppesThread)NULL;
  m_bIsAsync    = false;
}

CHTTPClient::~CHTTPClient()
{
  if(m_AsyncThread)
  {
    int nExitCode;
    fuppesThreadCancel(m_AsyncThread, nExitCode);
    fuppesThreadClose(m_AsyncThread);
  }
}

bool CHTTPClient::Send(CHTTPMessage* pMessage, std::string p_sTargetIPAddress, unsigned int p_nTargetPort)
{
  /* Init socket */
  upnpSocket sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr;
  addr.sin_family      = PF_INET;
  addr.sin_addr.s_addr = inet_addr(p_sTargetIPAddress.c_str());
  addr.sin_port        = htons(p_nTargetPort);
  
  /* Connect */
  if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
    CSharedLog::Shared()->Log(L_ERROR, "connect()", __FILE__, __LINE__);
    return false;
  }
  
  /* Send */
  std::string sMsg = pMessage->GetMessageAsString();  
  CSharedLog::Shared()->Log(L_DEBUG, sMsg, __FILE__, __LINE__);  
  if(send(sock, sMsg.c_str(), (int)strlen(sMsg.c_str()), 0) == -1)
  {
    CSharedLog::Shared()->Log(L_ERROR, "send()", __FILE__, __LINE__);
    return false;
  }
  
  return true;
}

/** HTTP GET implementation
 *  @param  p_sGetURL  the URL to GET
 *  @param  pResult pointer to a CHTTPMessage object that will be filled with the received values
 *
 *  @return returns true on success otherwise false
 */
bool CHTTPClient::Get(std::string p_sGetURL, CHTTPMessage* pResult)
{
  std::string   sIPAddress;
  unsigned int  nPort;

  if(!SplitURL(p_sGetURL, &sIPAddress, &nPort))
    return false;

  RegEx rxGet("[http://]*[0-9|\\.]+:*[0-9]*([/|\\w|\\.]*)", PCRE_CASELESS);
  if(rxGet.Search(p_sGetURL.c_str()))
  {
    return Get(rxGet.Match(1), pResult, sIPAddress, nPort);
  }
  else return false;    
}

/** HTTP GET implementation
 *  @param  p_sGet  the GET path plus file
 *  @param  pResult pointer to a CHTTPMessage object that will be filled with the received values
 *  @param  p_sTargetIPAddress  the IP-Address of the remote host
 *  @param  p_nTargetPort the port of the remote host (default = 80)
 *
 *  @return returns true on success otherwise false
 *
 *  @todo check if received content's length corresponds to the header's CONTENT-LENGTH value
 *  @todo implement HTTP 1.1 functionality
 */
bool CHTTPClient::Get(std::string p_sGet, CHTTPMessage* pResult, std::string p_sTargetIPAddress, unsigned int p_nTargetPort)
{
  /* Init socket */
  upnpSocket sock = socket(AF_INET, SOCK_STREAM, 0);
  sockaddr_in addr;
  addr.sin_family      = PF_INET;
  addr.sin_addr.s_addr = inet_addr(p_sTargetIPAddress.c_str());
  addr.sin_port        = htons(p_nTargetPort);

  /* Connect */
  if(connect(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1)
  {
    CSharedLog::Shared()->Log(L_ERROR, "connect()",__FILE__, __LINE__);
    return false;
  }

  /* Get header */
  std::string sMsg = BuildGetHeader(p_sGet, p_sTargetIPAddress, p_nTargetPort);  
  CSharedLog::Shared()->Log(L_EXTENDED, "send GET Header", __FILE__, __LINE__);

  /* Send */
  if(-1 == send(sock, sMsg.c_str(), (int)strlen(sMsg.c_str()), 0))
  {
    CSharedLog::Shared()->Log(L_ERROR, "send()", __FILE__, __LINE__);    
    return false;
  }
  else
  {
    CSharedLog::Shared()->Log(L_EXTENDED, "receive answer", __FILE__, __LINE__);

    char buffer[4096];
    int nBytesReceived;
    stringstream sReceived;

    while((nBytesReceived = recv(sock, buffer, sizeof(buffer), 0)) > 0)
    {
      stringstream sMsg;
      sMsg << "received " << nBytesReceived << " bytes";
      CSharedLog::Shared()->Log(L_DEBUG, sMsg.str(), __FILE__, __LINE__);
      
      buffer[nBytesReceived] = '\0';
      sReceived << buffer;
    }

    if(sReceived.str().length() > 0)
    {
      CSharedLog::Shared()->Log(L_EXTENDED, "done receive", __FILE__, __LINE__);      
      pResult->BuildFromString(sReceived.str());
      return true;
    }
    else
    {
      CSharedLog::Shared()->Log(L_ERROR, "recv()", __FILE__, __LINE__);
      return false;
    }
  }

  return false;
}


fuppesThreadCallback AsyncThread(void* arg)
{
  CHTTPClient* pClient = (CHTTPClient*)arg;                  

  std::string   sIPAddress;
  unsigned int  nPort;

  // split URL
  if(!SplitURL(pClient->m_sNotifyCallback, &sIPAddress, &nPort)) {
    pClient->m_bAsyncDone = true;
    fuppesThreadExit();
  }




  pClient->m_bAsyncDone = true;
  fuppesThreadExit();
}

void CHTTPClient::AsyncNotify(std::string p_sCallback)
{
  m_sNotifyCallback = p_sCallback;
  m_bIsAsync   = true;
  m_bAsyncDone = false;
  fuppesThreadStartArg(m_AsyncThread, AsyncThread, *this);
}


std::string CHTTPClient::BuildGetHeader(std::string p_sGet, std::string p_sTargetIPAddress, unsigned int p_nTargetPort)
{
  std::stringstream sHeader;
  
  /* Build header */
  sHeader << "GET " << p_sGet << " HTTP/1.0" << "\r\n";
  sHeader << "HOST: " << p_sTargetIPAddress << ":" << p_nTargetPort << "\r\n";
  //sHeader << "USER_AGENT: " << "FUPPES-HTTPClient Mozilla/4.0 (compatible)" << "\r\n";
  sHeader << "CONTENT-LENGTH: 0" << "\r\n";
  sHeader << "\r\n";
  
  return sHeader.str();
}

/* <\PRIVATE> */
