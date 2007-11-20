/***************************************************************************
 *            MessageBase.cpp
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
 
#include "MessageBase.h"
#include "Common/RegEx.h"

#include <iostream>
using namespace std;

CMessageBase::CMessageBase()
{	
  m_sMessage = "";
  m_sHeader  = "";
  m_sContent = "";
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
  m_LocalEp = p_EndPoint;
}

void CMessageBase::SetRemoteEndPoint(sockaddr_in p_EndPoint)
{
  m_RemoteEp = p_EndPoint;
}

