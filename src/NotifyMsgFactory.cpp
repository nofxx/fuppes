/***************************************************************************
 *            NotifyMsgFactory.cpp
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "NotifyMsgFactory.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

CNotifyMsgFactory* CNotifyMsgFactory::instance = 0;

CNotifyMsgFactory* CNotifyMsgFactory::shared()
{
	if (instance == 0)
		instance = new CNotifyMsgFactory();
	return instance;
}

CNotifyMsgFactory::CNotifyMsgFactory()
{
}

void CNotifyMsgFactory::SetHTTPServerURL(std::string sURL)
{
	m_sHTTPServerURL = sURL;
}

std::string CNotifyMsgFactory::type_to_string(msg_type a_type)
{
	stringstream result;
	
	switch(a_type)
	{
		case mt_usn:
			result << "uuid:" << CSharedConfig::Shared()->GetUDN();
			break;
		
		case mt_root_device:
			result.str("upnp:rootdevice");
		  break;
		
		case mt_connection_manager:
			result.str("urn:schemas-upnp-org:service:ConnectionManager:1");
		  break;
		
		case mt_content_directory:
			result.str("urn:schemas-upnp-org:service:ContentDirectory:1");
		  break;
		
		case mt_media_server:
			result.str("urn:schemas-upnp-org:device:MediaServer:1");
		  break;
		
		default:
			result.str("");	 
		  break;
	}
				
	return result.str();
}

std::string CNotifyMsgFactory::msearch()
{
	stringstream result;
	
	result << "M-SEARCH * HTTP/1.1\r\n";
	result << "MX: 10\r\n";
	result << "ST: ssdp:all\r\n";
	result << "HOST: 239.255.255.250:1900\r\n";
	result << "MAN: \"ssdp:discover\"\r\n\r\n";
		
	return result.str();
}

std::string CNotifyMsgFactory::notify_alive(msg_type a_type)
{
	stringstream result;
	
	result << "NOTIFY * HTTP/1.1\r\n";
  result << "LOCATION: http://" << m_sHTTPServerURL << "/\r\n";
  result << "HOST: 239.255.255.250:1900\r\n";
  result << "SERVER: " << CSharedConfig::Shared()->GetAppFullname() << "/" << CSharedConfig::Shared()->GetAppVersion() << " ";
		result << "UPnP/1.0 ";
	  result << "libfuppes/0.1\r\n";
  result << "NTS: ssdp:alive\r\n";
	
	result << "USN: uuid:" << CSharedConfig::Shared()->GetUDN();
	if(a_type == mt_usn)
		result << "\r\n";
	else
	  result << "::" << type_to_string(a_type) << "\r\n";
	
	result << "CACHE-CONTROL: max-age=1800\r\n";
	result << "NT: " << type_to_string(a_type) << "\r\n";
	result << "Content-Length: 0\r\n\r\n";      
	
	return result.str();
}

std::string CNotifyMsgFactory::notify_bye_bye(msg_type a_type)
{
	stringstream result;
	
	result << "NOTIFY * HTTP/1.1\r\n";
	result << "HOST: 239.255.255.250:1900\r\n";
  result << "NTS: ssdp:byebye\r\n";
	
	result << "USN: uuid:" << CSharedConfig::Shared()->GetUDN();
	if(a_type == mt_usn)
	{
		result << "\r\n";
		result << "NT: " << CSharedConfig::Shared()->GetUDN() << "\r\n";
  }
	else
	{
	  result << "::" << type_to_string(a_type) << "\r\n";	
		result << "NT: " << type_to_string(a_type) << "\r\n";
  }

	result << "Content-Length: 0\r\n\r\n";      
	
	return result.str();	
}

std::string CNotifyMsgFactory::GetMSearchResponse(msg_type p_MessageType)
{	
  stringstream result;
	
	result << "HTTP/1.1 200 OK\r\n";
  result << "CACHE-CONTROL: max-age=1800\r\n";
  result << "EXT: \r\n";
  result << "LOCATION: http://" << m_sHTTPServerURL << "/\r\n";
  result << "SERVER: " << CSharedConfig::Shared()->GetAppFullname() << "/" << CSharedConfig::Shared()->GetAppVersion() << " ";
		result << "UPnP/1.0 ";
	  result << "libfuppes/0.1\r\n";
  result << "ST: " << type_to_string(p_MessageType) << "\r\n";  
  result << "NTS: ssdp:alive\r\n";	
	result << "USN: uuid:" << CSharedConfig::Shared()->GetUDN();
	if(p_MessageType == mt_usn)
		result << "\r\n";
	else
	  result << "::" << type_to_string(p_MessageType) << "\r\n";
	result << "Content-Length: 0\r\n\r\n";      
	
	return result.str();
}
