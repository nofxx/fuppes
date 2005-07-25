/***************************************************************************
 *            HTTPClient.cpp
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "HTTPClient.h"
#include "Common.h"
#include "SharedLog.h"
#include "RegEx.h"

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

/*===============================================================================
 CONSTANTS
===============================================================================*/

const std::string LOGNAME = "HTTPClient";

/*===============================================================================
 CLASS CHTTPClient
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 MESSAGES
===============================================================================*/

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
    CSharedLog::Shared()->Error(LOGNAME, "connect()");
    return false;
  }
  
  /* Send */
  std::string sMsg = pMessage->GetMessageAsString();  
  CSharedLog::Shared()->Log(LOGNAME, sMsg);  
  if(send(sock, sMsg.c_str(), (int)strlen(sMsg.c_str()), 0) == -1)
  {
    CSharedLog::Shared()->Error(LOGNAME, "send()");
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
    CSharedLog::Shared()->Error(LOGNAME, "connect()");
    return false;
  }

  /* Get header */
  std::string sMsg = BuildGetHeader(p_sGet, p_sTargetIPAddress, p_nTargetPort);  
  CSharedLog::Shared()->Log(LOGNAME, "send GET Header");

  /* Send */
  if(-1 == send(sock, sMsg.c_str(), (int)strlen(sMsg.c_str()), 0))
  {
    CSharedLog::Shared()->Error(LOGNAME, "send()");    
    return false;
  }
  else
  {
    CSharedLog::Shared()->Log(LOGNAME, "receive answer");

    char buffer[4096];
    int nBytesReceived;
    stringstream sReceived;

    while((nBytesReceived = recv(sock, buffer, sizeof(buffer), 0)) > 0)
    {
      stringstream sMsg;
      sMsg << "received " << nBytesReceived << " bytes";
      CSharedLog::Shared()->Log(LOGNAME, sMsg.str());
      
      buffer[nBytesReceived] = '\0';
      sReceived << buffer;
    }

    if(sReceived.str().length() > 0)
    {
      CSharedLog::Shared()->Log(LOGNAME, "done receive");      
      pResult->BuildFromString(sReceived.str());
      CSharedLog::Shared()->Log(LOGNAME, "done build msg");      
      return true;
    }
    else
    {
      CSharedLog::Shared()->Error(LOGNAME, "recv()");
      return false;
    }
  }

  return false;
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

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
