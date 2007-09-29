/***************************************************************************
 *            UPnPAction.h
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
 
#ifndef _UPNPACTION_H
#define _UPNPACTION_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../UPnPBase.h"
#include "../DeviceSettings/DeviceSettings.h"
#include <string>

#define UPNP_UNKNOWN 0

typedef enum UPNP_CONTENT_DIRECTORY_ACTIONS {
  UPNP_UNKNWON                  = 0,
  UPNP_BROWSE									  = 1,
	UPNP_SEARCH									  = 2,
	UPNP_GET_SEARCH_CAPABILITIES  = 3,
	UPNP_GET_SORT_CAPABILITIES    = 4,
  UPNP_GET_SYSTEM_UPDATE_ID		  = 5,
	UPNP_GET_PROTOCOL_INFO		    = 6
} UPNP_CONTENT_DIRECTORY_ACTIONS;

typedef enum UPNP_AV_TRANSPORT_ACTIONS {
} UPNP_AV_TRANSPORT_ACTIONS;

typedef enum UPNP_CONNECTION_MANAGER_ACTIONS {	
  CMA_UNKNOWN                     = 0,  
  CMA_GET_PROTOCOL_INFO           = 1,
  CMA_PREPARE_FOR_CONNECTION      = 2,
  CMA_CONNECTION_COMPLETE         = 3,
  CMA_GET_CURRENT_CONNECTION_IDS  = 4,
  CMA_GET_CURRENT_CONNECTION_INFO = 5	
} UPNP_CONNECTION_MANAGER_ACTIONS;

typedef enum UPNP_X_MS_MEDIA_RECEIVER_REGISTRAR_ACTIONS {
  UPNP_IS_AUTHORIZED = 1,
  UPNP_IS_VALIDATED  = 2
} UPNP_X_MS_MEDIA_RECEIVER_REGISTRAR_ACTIONS;


class CUPnPAction
{
  public:

    /** constructor */
	  CUPnPAction(UPNP_DEVICE_TYPE p_nTargetDeviceType, int p_nActionType, std::string p_sContent) 
	  {
	    m_nActionType       = p_nActionType;
      m_sContent          = p_sContent;
			m_nTargetDeviceType = p_nTargetDeviceType;
	  }

    /** destructor */
    virtual ~CUPnPAction() {};
  
    int GetActionType() { return m_nActionType; }
		std::string GetContent() { return m_sContent; }
		UPNP_DEVICE_TYPE GetTargetDeviceType() { return m_nTargetDeviceType; }

    CDeviceSettings* DeviceSettings() { return m_pDeviceSettings; }
	  void DeviceSettings(CDeviceSettings* pSettings) { m_pDeviceSettings = pSettings; }

  private:
    UPNP_DEVICE_TYPE m_nTargetDeviceType;
    int              m_nActionType;
    std::string      m_sContent;
		
		CDeviceSettings*   m_pDeviceSettings;
};

class CUPnPBrowseSearchBase: public CUPnPAction
{
  protected:
    CUPnPBrowseSearchBase(UPNP_DEVICE_TYPE p_nTargetDeviceType, int p_nActionType, std::string p_sContent);
    
  public:
    bool IncludeProperty(std::string p_sProperty);  
  
    std::string      m_sFilter;
    unsigned int     m_nStartingIndex;
    unsigned int     m_nRequestedCount;
    std::string      m_sSortCriteria;   
};

#endif // _UPNPACTION_H

/*typedef enum tagUPNP_ACTION_TYPE
{
  UPNP_ACTION_TYPE_UNKNOWN,
        
  UPNP_ACTION_TYPE_CONTENT_DIRECTORY_BROWSE,
  */
	/*
POST /UPnPServices/ContentDirectory/control/ HTTP/1.1
Host: 192.168.0.3:60230
User-Agent: UPnP/1.0 DLNADOC/1.00
SOAPACTION: "urn:schemas-upnp-org:service:ContentDirectory:1#Search"
Content-Type: text/xml; charset="utf-8"
Content-Length: 517

<?xml version="1.0" encoding="utf-8"?><s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:Search xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1"><ContainerID>0</ContainerID><SearchCriteria>(upnp:class contains "object.item.imageItem") and (dc:title contains "")</SearchCriteria><Filter>*</Filter><StartingIndex>0</StartingIndex><RequestedCount>7</RequestedCount><SortCriteria></SortCriteria></u:Search></s:Body></s:Envelope>
*/
	
	//UPNP_ACTION_TYPE_CONTENT_DIRECTORY_SEARCH,
	
  /* POST /UPnPServices/ContentDirectory/control/ HTTP/1.1
  HOST: 192.168.0.3:1117
  SOAPACTION: "urn:schemas-upnp-org:service:ContentDirectory:1#GetSearchCapabilities"
  CONTENT-TYPE: text/xml ; charset="utf-8"
  Content-Length: 299
  
  <?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s=
  "http://schemas.xmlsoap.org/soap/envelope/">
     <s:Body>
        <u:GetSearchCapabilities xmlns:u="urn:schemas-upnp-org:service:ContentDire
  ctory:1" />
     </s:Body>
  </s:Envelope> */
  
  //UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SEARCH_CAPABILITIES,
  
  /*
  POST /UPnPServices/ContentDirectory/control/ HTTP/1.1
  HOST: 192.168.0.3:1117
  SOAPACTION: "urn:schemas-upnp-org:service:ContentDirectory:1#GetSortCapabilities
  "
  CONTENT-TYPE: text/xml ; charset="utf-8"
  Content-Length: 297
  
  <?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s=
  "http://schemas.xmlsoap.org/soap/envelope/">
     <s:Body>
        <u:GetSortCapabilities xmlns:u="urn:schemas-upnp-org:service:ContentDirect
  ory:1" />
     </s:Body>
  </s:Envelope> */
  
  //UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SORT_CAPABILITIES,
  /*
  POST /UPnPServices/ContentDirectory/control/ HTTP/1.1
  HOST: 192.168.0.3:1117
  SOAPACTION: "urn:schemas-upnp-org:service:ContentDirectory:1#GetSystemUpdateID"
  CONTENT-TYPE: text/xml ; charset="utf-8"
  Content-Length: 295
  
  <?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s=
  "http://schemas.xmlsoap.org/soap/envelope/">
     <s:Body>
        <u:GetSystemUpdateID xmlns:u="urn:schemas-upnp-org:service:ContentDirector
  y:1" />
     </s:Body>
  </s:Envelope> */
  
  //UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SYSTEM_UPDATE_ID,
  
  /*
  POST /UPnPServices/ConnectionManager/control/ HTTP/1.1
  HOST: 192.168.0.3:1117
  SOAPACTION: "urn:schemas-upnp-org:service:ConnectionManager:1#GetProtocolInfo"
  CONTENT-TYPE: text/xml ; charset="utf-8"
  Content-Length: 294
  
  <?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" 
						  xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
     <s:Body>
        <u:GetProtocolInfo xmlns:u="urn:schemas-upnp-org:service:ConnectionManager
  :1" />
     </s:Body>
  </s:Envelope> */
  //UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_PROTOCOL_INFO,
	
	/*
	POST /web/msr_control HTTP/1.1
	User-Agent: Xbox/2.0.4552.0 UPnP/1.0 Xbox/2.0.4552.0
	SOAPACTION: "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1#IsAuthorized"
	CONTENT-TYPE: text/xml; charset="utf-8"
	Content-Length: 304

	<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" 
						  s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
   <s:Body>
      <u:IsAuthorized xmlns:u="urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1">
			  <DeviceID></DeviceID>
			</u:IsAuthorized>
   </s:Body>
  </s:Envelope> */
  //UPNP_ACTION_TYPE_X_MS_MEDIA_RECEIVER_REGISTRAR_IS_AUTHORIZED,
	
	/*
	POST /web/msr_control HTTP/1.1
	User-Agent: Xbox/2.0.4552.0 UPnP/1.0 Xbox/2.0.4552.0
	SOAPACTION: "urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1#IsValidated"
	CONTENT-TYPE: text/xml; charset="utf-8"
	Content-Length: 302

	<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"
							s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">
    <s:Body>
      <u:IsValidated xmlns:u="urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1">
			  <DeviceID></DeviceID>
			</u:IsValidated>
    </s:Body>
  </s:Envelope>
	*/
 /* UPNP_ACTION_TYPE_X_MS_MEDIA_RECEIVER_REGISTRAR_IS_VALIDATED
	
  
}UPNP_ACTION_TYPE; */

