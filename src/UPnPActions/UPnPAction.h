/***************************************************************************
 *            UPnPAction.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _UPNPACTION_H
#define _UPNPACTION_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "../UPnPBase.h"
#include <string>

/*===============================================================================
 DEFINITIONS
===============================================================================*/

typedef enum tagUPNP_ACTION_TYPE
{
  UPNP_ACTION_TYPE_UNKNOWN,
        
  UPNP_ACTION_TYPE_CONTENT_DIRECTORY_BROWSE,
  
	/*
POST /UPnPServices/ContentDirectory/control/ HTTP/1.1
Host: 192.168.0.3:60230
User-Agent: UPnP/1.0 DLNADOC/1.00
SOAPACTION: "urn:schemas-upnp-org:service:ContentDirectory:1#Search"
Content-Type: text/xml; charset="utf-8"
Content-Length: 517

<?xml version="1.0" encoding="utf-8"?><s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/"><s:Body><u:Search xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1"><ContainerID>0</ContainerID><SearchCriteria>(upnp:class contains "object.item.imageItem") and (dc:title contains "")</SearchCriteria><Filter>*</Filter><StartingIndex>0</StartingIndex><RequestedCount>7</RequestedCount><SortCriteria></SortCriteria></u:Search></s:Body></s:Envelope>
*/
	
	UPNP_ACTION_TYPE_CONTENT_DIRECTORY_SEARCH,
	
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
  
  UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SEARCH_CAPABILITIES,
  
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
  
  UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SORT_CAPABILITIES,
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
  
  UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SYSTEM_UPDATE_ID,
  
  /*
  POST /UPnPServices/ConnectionManager/control/ HTTP/1.1
  HOST: 192.168.0.3:1117
  SOAPACTION: "urn:schemas-upnp-org:service:ConnectionManager:1#GetProtocolInfo"
  CONTENT-TYPE: text/xml ; charset="utf-8"
  Content-Length: 294
  
  <?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s=
  "http://schemas.xmlsoap.org/soap/envelope/">
     <s:Body>
        <u:GetProtocolInfo xmlns:u="urn:schemas-upnp-org:service:ConnectionManager
  :1" />
     </s:Body>
  </s:Envelope> */
  UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_PROTOCOL_INFO  
  
}UPNP_ACTION_TYPE;

/*===============================================================================
 CLASS CUPnPAction
===============================================================================*/

class CUPnPAction
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
  */
  CUPnPAction(UPNP_ACTION_TYPE p_nType, std::string p_sMessage);

  /** destructor
  */
  virtual ~CUPnPAction();
  
  UPNP_ACTION_TYPE GetActionType() { return m_nActionType; }

/*===============================================================================
 MEMBERS
===============================================================================*/

//  protected:
    UPNP_DEVICE_TYPE m_nTargetDevice;
    UPNP_ACTION_TYPE m_nActionType;
    std::string      m_sMessage;

/* <\PUBLIC> */

};

#endif /* _UPNPACTION_H */
