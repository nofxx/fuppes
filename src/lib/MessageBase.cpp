/***************************************************************************
 *            MessageBase.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "MessageBase.h"
#include "Common/RegEx.h"

#include <iostream>
using namespace std;

CMessageBase::CMessageBase()
{	
  m_sMessage = "";
  m_sHeader  = "";
  m_sContent = "";
	
	bzero(&m_LocalEp, sizeof(struct sockaddr_in));
	bzero(&m_RemoteEp, sizeof(struct sockaddr_in));
}

CMessageBase::~CMessageBase()
{
}

bool CMessageBase::SetMessage(std::string p_sMessage)
{
  m_sMessage = p_sMessage;

  std::string::size_type nPos = m_sMessage.find("\r\n\r\n");  
  if(nPos != string::npos) {	
    m_sHeader  = m_sMessage.substr(0, nPos + strlen("\r\n"));
    m_sContent = m_sMessage.substr(nPos + 4, m_sMessage.length() - nPos - 4);    

    return true;
  }
  else {
		return false;
	}  
}

bool CMessageBase::SetHeader(std::string p_sHeader)
{
  m_sHeader = p_sHeader;
  return true;
}

void CMessageBase::SetLocalEndPoint(sockaddr_in p_EndPoint)
{	
	m_LocalEp.sin_family	= p_EndPoint.sin_family;
	m_LocalEp.sin_addr 		= p_EndPoint.sin_addr;
	m_LocalEp.sin_port		= p_EndPoint.sin_port;
	memset(&m_LocalEp.sin_zero, 0, sizeof(m_LocalEp.sin_zero));	
}

void CMessageBase::SetRemoteEndPoint(sockaddr_in p_EndPoint)
{
	m_RemoteEp.sin_family	= p_EndPoint.sin_family;
	m_RemoteEp.sin_addr		= p_EndPoint.sin_addr;
	m_RemoteEp.sin_port		= p_EndPoint.sin_port;
	memset(&m_RemoteEp.sin_zero, 0, sizeof(m_RemoteEp.sin_zero));	
}

std::string CMessageBase::GetRemoteIPAddress() 
{ 
	string ip = inet_ntoa(m_RemoteEp.sin_addr);
	return ip.c_str();
}
