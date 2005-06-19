/***************************************************************************
 *            SharedConfig.cpp
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
 
#include <iostream>

#include "win32.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/param.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

//#include <string>
#include "SharedConfig.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN     64
#endif

//using namespace std;

CSharedConfig* CSharedConfig::m_Instance = 0;

CSharedConfig* CSharedConfig::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedConfig();
	return m_Instance;
}

CSharedConfig::CSharedConfig()
{
in_addr* addr;

#ifdef WIN32
  // Init windows sockets
  WSADATA wsaData;
  int nRet = WSAStartup(MAKEWORD(1, 1), &wsaData);
  if(0 == nRet)
  {
    // Get hostname
    char name[64] = "";
    nRet = gethostname(name, sizeof(name));
    if(0 == nRet)
    {
      m_sHostname = name;

      // Get host
      struct hostent* host;
      host = gethostbyname(name);

      // Get host address
      //in_addr* addr;
      addr = (struct in_addr*)host->h_addr;
    }
    WSACleanup();
  }
    
#else

  char* name;
  gethostname(name, 64);	
  m_sHostname = name;
	
  struct hostent* host;
  host = gethostbyname(name);
	
  //in_addr* addr;
  addr = (struct in_addr*)host->h_addr;
#endif

  m_sIP = inet_ntoa(*addr);
}

string CSharedConfig::GetAppName()
{
	return "fuppes";
}

string CSharedConfig::GetAppFullname()
{
	return "Free UPnP Entertainment Service";
}

string CSharedConfig::GetAppVersion()
{
	return "0.1";
}

string CSharedConfig::GetHostname()
{
	return m_sHostname;
}

string CSharedConfig::GetIP()
{
	return m_sIP;
}

string CSharedConfig::GetUDN()
{	
	return "12345678-aabb-0000-ccdd-1234eeff0000";
}

void CSharedConfig::SetHTTPServerURL(string p_sURL)
{
  m_sHTTPServerURL = p_sURL;
}

string CSharedConfig::GetHTTPServerURL()
{
  return m_sHTTPServerURL;
}
