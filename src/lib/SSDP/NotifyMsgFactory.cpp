/***************************************************************************
 *            NotifyMsgFactory.cpp
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
 
#include "NotifyMsgFactory.h"
#include "../SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

CNotifyMsgFactory::CNotifyMsgFactory(std::string p_sHTTPServerURL)
{
  m_sHTTPServerURL = p_sHTTPServerURL;
}

std::string CNotifyMsgFactory::msearch()
{
	stringstream result;
	
	result << "M-SEARCH * HTTP/1.1\r\n";
	result << "MX: 10\r\n";
	result << "ST: ssdp:all\r\n";
	result << "HOST: 239.255.255.250:1900\r\n"; // UPnP multicast address
	result << "MAN: \"ssdp:discover\"\r\n\r\n";
		
	return result.str();
}

std::string CNotifyMsgFactory::notify_alive(MESSAGE_TYPE a_type)
{
	stringstream result;
	
	result << "NOTIFY * HTTP/1.1\r\n";
  result << "HOST: 239.255.255.250:1900\r\n";
	result << "CACHE-CONTROL: max-age=1810\r\n";
	result << "LOCATION: http://" << m_sHTTPServerURL << "/description.xml\r\n";
  result << "NT: " << type_to_string(a_type) << "\r\n";
	result << "NTS: ssdp:alive\r\n";
	result << "SERVER: " << CSharedConfig::Shared()->GetOSName() << "/" << CSharedConfig::Shared()->GetOSVersion() << ", " <<
    "UPnP/1.0, " << CSharedConfig::Shared()->GetAppFullname() << "/" << CSharedConfig::Shared()->GetAppVersion() << "\r\n";  
	
	result << "USN: uuid:" << CSharedConfig::Shared()->GetUUID();
	if(a_type == MESSAGE_TYPE_USN)
		result << "\r\n";
	else
	  result << "::" << type_to_string(a_type) << "\r\n";
		
	result << "\r\n";
	
	return result.str();
}

std::string CNotifyMsgFactory::notify_bye_bye(MESSAGE_TYPE a_type)
{
	stringstream result;
	
	result << "NOTIFY * HTTP/1.1\r\n";
	result << "HOST: 239.255.255.250:1900\r\n";
  result << "NTS: ssdp:byebye\r\n";
	
	result << "USN: uuid:" << CSharedConfig::Shared()->GetUUID();
	if(a_type == MESSAGE_TYPE_USN)
	{
		result << "\r\n";
		result << "NT: " << CSharedConfig::Shared()->GetUUID() << "\r\n";
  }
	else
	{
	  result << "::" << type_to_string(a_type) << "\r\n";	
		result << "NT: " << type_to_string(a_type) << "\r\n";
  }

	//result << "Content-Length: 0\r\n\r\n";      
	result << "\r\n";      
	
	return result.str();	
}

std::string CNotifyMsgFactory::GetMSearchResponse(MESSAGE_TYPE p_MessageType)
{	
  stringstream result;
	
	char   szTime[30];
  time_t tTime = time(NULL);
  strftime(szTime, 30,"%a, %d %b %Y %H:%M:%S GMT" , gmtime(&tTime));   
		
	result << "HTTP/1.1 200 OK\r\n";
  result << "CACHE-CONTROL: max-age=1810\r\n";
  result << "DATE: " << szTime << "\r\n";
  result << "EXT: \r\n";
  result << "LOCATION: http://" << m_sHTTPServerURL << "/description.xml\r\n";
  result << "SERVER: " << CSharedConfig::Shared()->GetOSName() << "/" << CSharedConfig::Shared()->GetOSVersion() << ", " <<
    "UPnP/1.0, " << CSharedConfig::Shared()->GetAppFullname() << "/" << CSharedConfig::Shared()->GetAppVersion() << "\r\n";  
  result << "ST: " << type_to_string(p_MessageType) << "\r\n";  
  //result << "NTS: ssdp:alive\r\n";	
	result << "USN: uuid:" << CSharedConfig::Shared()->GetUUID();
	if(p_MessageType == MESSAGE_TYPE_USN)
		result << "\r\n";
	else
	  result << "::" << type_to_string(p_MessageType) << "\r\n";
		
	result << "Content-Length: 0\r\n";
	result << "\r\n";
	
	return result.str();
}

std::string CNotifyMsgFactory::type_to_string(MESSAGE_TYPE a_type)
{
	/* Convert message type to string */
  stringstream result;
	
	switch(a_type)
	{
		case MESSAGE_TYPE_USN:
			result << "uuid:" << CSharedConfig::Shared()->GetUUID();
			break;
		
		case MESSAGE_TYPE_ROOT_DEVICE:
			result.str("upnp:rootdevice");
		  break;
		
		case MESSAGE_TYPE_CONNECTION_MANAGER:
			result.str("urn:schemas-upnp-org:service:ConnectionManager:1");
		  break;
		
		case MESSAGE_TYPE_CONTENT_DIRECTORY:
			result.str("urn:schemas-upnp-org:service:ContentDirectory:1");
		  break;
		
		case MESSAGE_TYPE_MEDIA_SERVER:
			result.str("urn:schemas-upnp-org:device:MediaServer:1");
		  break;
		
		default:
			result.str("");	 
		  break;
	}
				
	return result.str();
}
