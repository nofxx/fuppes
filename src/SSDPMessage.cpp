/***************************************************************************
 *            SSDPMessage.cpp
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
 
#include "SSDPMessage.h"
#include "RegEx.h"
#include "SharedLog.h"
#include <sstream>

const std::string LOGNAME = "SSDPMessage";

CSSDPMessage::CSSDPMessage()
{
  

  
}

CSSDPMessage::~CSSDPMessage()
{
}

void CSSDPMessage::SetMessage(std::string p_sMessage)
{
  CMessageBase::SetMessage(p_sMessage);
  /*
  HTTP/1.1 200 OK
  CACHE-CONTROL: max-age=1800
  EXT:
  LOCATION: http://192.168.0.2:47224/
  SERVER: Free UPnP Entertainment Service/0.1.1 UPnP/1.0 libfuppes/0.1
  ST: urn:schemas-upnp-org:service:ContentDirectory:1
  NTS: ssdp:alive
  USN: uuid:45645678-aabb-0000-ccdd-1234eeff0000::urn:schemas-upnp-org:service:ContentDirectory:1
  Content-Length: 0 */
  
	RegEx rxLocation("LOCATION: +(http://.+)", PCRE_CASELESS);
	if(rxLocation.Search(m_sMessage.c_str()))
	{
    m_sLocation = rxLocation.Match(1);
    //CSharedLog::Shared()->Log(LOGNAME, m_sLocation);    
	}
  
  RegEx rxServer("SERVER: +(.*)", PCRE_CASELESS);
	if(rxServer.Search(m_sMessage.c_str()))
	{
    m_sServer = rxServer.Match(1);
    //CSharedLog::Shared()->Log(LOGNAME, m_sServer);
	}
  
  RegEx rxUSN("USN: +(.*)", PCRE_CASELESS);
  if(rxUSN.Search(m_sMessage.c_str()))
  {
    m_sUSN = rxUSN.Match(1);
    //CSharedLog::Shared()->Log(LOGNAME, m_sUSN);
    
    RegEx rxUUID("uuid:([A-Z|0-9|-]+)", PCRE_CASELESS);
    if(rxUUID.Search(m_sUSN.c_str()))
    {
      m_sUUID = rxUUID.Match(1);
      //CSharedLog::Shared()->Log(LOGNAME, m_sUUID);
    }    
  }
}

std::string CSSDPMessage::GetDeviceID()
{
  std::stringstream sDeviceID;
  sDeviceID << this->GetRemoteIPAddress() << "::" << this->GetUUID();
  return sDeviceID.str();  
}
