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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "NotifyMsgFactory.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

/*===============================================================================
 CLASS CNotifyMsgFactory
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CNotifyMsgFactory::CNotifyMsgFactory(std::string p_sHTTPServerURL)
{
  m_sHTTPServerURL = p_sHTTPServerURL;
}

/*===============================================================================
 NOTIFICATIONS
===============================================================================*/

std::string CNotifyMsgFactory::msearch()
{
	stringstream result;
	
	result << "M-SEARCH * HTTP/1.1\r\n";
	result << "MX: 10\r\n";
	result << "ST: ssdp:all\r\n";
	result << "HOST: 239.255.255.250:1900\r\n"; // UPnP broadcast address
	result << "MAN: \"ssdp:discover\"\r\n\r\n";
		
	return result.str();
}

std::string CNotifyMsgFactory::notify_alive(MESSAGE_TYPE a_type)
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
	if(a_type == MESSAGE_TYPE_USN)
		result << "\r\n";
	else
	  result << "::" << type_to_string(a_type) << "\r\n";
	
	result << "CACHE-CONTROL: max-age=1800\r\n";
	result << "NT: " << type_to_string(a_type) << "\r\n";
	result << "Content-Length: 0\r\n\r\n";      
	
	return result.str();
}

std::string CNotifyMsgFactory::notify_bye_bye(MESSAGE_TYPE a_type)
{
	stringstream result;
	
	result << "NOTIFY * HTTP/1.1\r\n";
	result << "HOST: 239.255.255.250:1900\r\n";
  result << "NTS: ssdp:byebye\r\n";
	
	result << "USN: uuid:" << CSharedConfig::Shared()->GetUDN();
	if(a_type == MESSAGE_TYPE_USN)
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

std::string CNotifyMsgFactory::GetMSearchResponse(MESSAGE_TYPE p_MessageType)
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
	if(p_MessageType == MESSAGE_TYPE_USN)
		result << "\r\n";
	else
	  result << "::" << type_to_string(p_MessageType) << "\r\n";
	result << "Content-Length: 0\r\n\r\n";      
	
	return result.str();
}

/* <\PUBLIC> */

/*===============================================================================
 HELPER
===============================================================================*/

/* <PRIVATE> */

std::string CNotifyMsgFactory::type_to_string(MESSAGE_TYPE a_type)
{
	/* Convert message type to string */
  stringstream result;
	
	switch(a_type)
	{
		case MESSAGE_TYPE_USN:
			result << "uuid:" << CSharedConfig::Shared()->GetUDN();
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

/* <\PRIVATE> */
