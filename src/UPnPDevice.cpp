/***************************************************************************
 *            UPnPDevice.cpp
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "UPnPDevice.h"
#include "HTTP/HTTPMessage.h"
#include "HTTP/HTTPClient.h"
#include "Common/Common.h"
#include "Common/RegEx.h"
#include "SharedLog.h"


#include <sstream>
#include <iostream>
#include <libxml/xmlwriter.h>

using namespace std;

/*===============================================================================
 CONSTANTS
===============================================================================*/

const string LOGNAME = "UPNPDevice";

/*===============================================================================
 CLASS CUPnPDevice
===============================================================================*/

/* <PROTECTED> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

/* constructor */
CUPnPDevice::CUPnPDevice(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL, IUPnPDevice* pOnTimerHandler):
  CUPnPBase(nType, p_sHTTPServerURL)
{
  /* this constructor is for local devices only */
  m_bIsLocalDevice  = true;  
  m_pOnTimerHandler = pOnTimerHandler;
  
  m_pTimer = new CTimer(this);
}

/* <\PROTECTED> */

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CUPnPDevice::CUPnPDevice(IUPnPDevice* pOnTimerHandler):
  CUPnPBase(UPNP_DEVICE_TYPE_UNKNOWN, "")
{
  /* this constructor is for remote devices only */
  m_bIsLocalDevice  = false;
  m_pOnTimerHandler = pOnTimerHandler;
  
  m_pTimer = new CTimer(this);
}

CUPnPDevice::~CUPnPDevice()
{
  delete m_pTimer;
}


/*===============================================================================
 TIMER
===============================================================================*/

void CUPnPDevice::OnTimer()
{
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "OnTimer()");
  if(m_pOnTimerHandler != NULL)
    m_pOnTimerHandler->OnTimer(this);
}

/*===============================================================================
 BUILD DEVICE
===============================================================================*/

/* BuildFromDescriptionURL */
bool CUPnPDevice::BuildFromDescriptionURL(std::string p_sDescriptionURL)
{    
  CHTTPClient  HTTPClient;
  CHTTPMessage HTTPMessage;
    
  /* Get description message */
  if(true == HTTPClient.Get(p_sDescriptionURL, &HTTPMessage))
  {    
    return ParseDescription(HTTPMessage.GetContent());
  }
  else
  {
    /* We could not get the device description */
    CSharedLog::Shared()->Error(LOGNAME, "BuildFromDescriptionURL");
    return false;
  }
}

/* AddUPnPService */
void CUPnPDevice::AddUPnPService(CUPnPService* pUPnPService)
{
	/* Add service to vector */
  m_vUPnPServices.push_back(pUPnPService);
}

/*===============================================================================
 GET
===============================================================================*/

/* GetUPnPServiceCount */
int CUPnPDevice::GetUPnPServiceCount()
{
	return (int)m_vUPnPServices.size();
}

/* GetUPnPService */
CUPnPService* CUPnPDevice::GetUPnPService(int p_nIndex)
{
	ASSERT(p_nIndex >= 0);
  if(p_nIndex < 0)
    return NULL;

  return m_vUPnPServices[p_nIndex];
}

/* GetDeviceType */
UPNP_DEVICE_TYPE CUPnPDevice::GetDeviceType()
{
	return m_nUPnPDeviceType;
}

std::string CUPnPDevice::GetDeviceTypeAsString()
{
  switch(m_nUPnPDeviceType)
  {
    case UPNP_DEVICE_TYPE_MEDIA_SERVER :
      return "MediaServer";
    case UPNP_DEVICE_TYPE_MEDIA_RENDERER :
      return "MediaRenderer";
    default:
      return "Unknown";
  }
}

/* GetDeviceDescription */
std::string CUPnPDevice::GetDeviceDescription()
{		
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
	/*xmlChar *szTmp;	
	  tmp = ConvertInput("<äöü>", "UTF-8");
    xmlTextWriterWriteComment(writer, tmp);*/
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

	/* root */
	xmlTextWriterStartElementNS(writer, NULL, BAD_CAST "root", BAD_CAST "urn:schemas-upnp-org:device-1-0");
	
		/* specVersion */
		xmlTextWriterStartElement(writer, BAD_CAST "specVersion");
		
			/* major */
			xmlTextWriterStartElement(writer, BAD_CAST "major");  	
			xmlTextWriterWriteString(writer, BAD_CAST "1");
			xmlTextWriterEndElement(writer);		
			/* minor */
			xmlTextWriterStartElement(writer, BAD_CAST "minor");  	
			xmlTextWriterWriteString(writer, BAD_CAST "0");
			xmlTextWriterEndElement(writer);		
	
		/* end specVersion */
		xmlTextWriterEndElement(writer);

		/* url base */
		sTmp << "http://" << m_sHTTPServerURL << "/";
		xmlTextWriterStartElement(writer, BAD_CAST "URLBase");
		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
		sTmp.str("");
		xmlTextWriterEndElement(writer);

		/* device */
		xmlTextWriterStartElement(writer, BAD_CAST "device");
		
		  /* deviceType */
			sTmp << "urn:schemas-upnp-org:device:" << GetUPnPDeviceTypeAsString() << ":1";
			xmlTextWriterStartElement(writer, BAD_CAST "deviceType");
			xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
			sTmp.str("");
      xmlTextWriterEndElement(writer);
		
			/* friendlyName */
			xmlTextWriterStartElement(writer, BAD_CAST "friendlyName");
			xmlTextWriterWriteString(writer, BAD_CAST m_sFriendlyName.c_str());
      xmlTextWriterEndElement(writer);
			
			/* manufacturer */
			xmlTextWriterStartElement(writer, BAD_CAST "manufacturer");
      xmlTextWriterWriteString(writer, BAD_CAST m_sManufacturer.c_str());
			xmlTextWriterEndElement(writer);
			
      /* manufacturerURL */
			xmlTextWriterStartElement(writer, BAD_CAST "manufacturerURL");
      xmlTextWriterWriteString(writer, BAD_CAST m_sManufacturerURL.c_str());
			xmlTextWriterEndElement(writer);
			
      /* modelDescription */
			xmlTextWriterStartElement(writer, BAD_CAST "modelDescription");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelDescription.c_str());
			xmlTextWriterEndElement(writer);
			
			/* modelName */
			xmlTextWriterStartElement(writer, BAD_CAST "modelName");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelName.c_str());
			xmlTextWriterEndElement(writer);
			
			/* modelNumber */
			xmlTextWriterStartElement(writer, BAD_CAST "modelNumber");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelNumber.c_str());
			xmlTextWriterEndElement(writer);
      
			/* modelURL */
			xmlTextWriterStartElement(writer, BAD_CAST "modelURL");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelURL.c_str());
			xmlTextWriterEndElement(writer);
			
			/* serialNumber */
			xmlTextWriterStartElement(writer, BAD_CAST "serialNumber");
      xmlTextWriterWriteString(writer, BAD_CAST m_sSerialNumber.c_str());
			xmlTextWriterEndElement(writer);
			
			/* UDN */
			xmlTextWriterStartElement(writer, BAD_CAST "UDN");
			sTmp << "uuid:" << m_sUUID;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
			xmlTextWriterEndElement(writer);
			
			/* UPC */
			xmlTextWriterStartElement(writer, BAD_CAST "UPC");
      xmlTextWriterWriteString(writer, BAD_CAST m_sUPC.c_str());
			xmlTextWriterEndElement(writer);		
		
		  /* serviceList */
			xmlTextWriterStartElement(writer, BAD_CAST "serviceList");			
      for(unsigned int i = 0; i < m_vUPnPServices.size(); i++)
			{
				CUPnPService* pTmp = m_vUPnPServices[i];				
				/* service */
				xmlTextWriterStartElement(writer, BAD_CAST "service");
				
					/* serviceType */
					sTmp << "urn:schemas-upnp-org:service:" << pTmp->GetUPnPDeviceTypeAsString() << ":1";
					xmlTextWriterStartElement(writer, BAD_CAST "serviceType");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
        
					/* serviceId */
					sTmp << "urn:upnp-org:serviceId:" << pTmp->GetUPnPDeviceTypeAsString();
					xmlTextWriterStartElement(writer, BAD_CAST "serviceId");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
				
					/* SCPDURL */
					sTmp << "/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/description.xml";
					xmlTextWriterStartElement(writer, BAD_CAST "SCPDURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);					

					/* controlURL */
					sTmp << "/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/control/";
					xmlTextWriterStartElement(writer, BAD_CAST "controlURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);					

					/* eventSubURL */
					sTmp << "/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/event/";
					xmlTextWriterStartElement(writer, BAD_CAST "eventSubURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
				
				/* end service */
				xmlTextWriterEndElement(writer);				
			}			
			/* end serviceList */
			xmlTextWriterEndElement(writer);
			
      /* presentationURL */
      xmlTextWriterStartElement(writer, BAD_CAST "presentationURL");
      xmlTextWriterWriteString(writer, BAD_CAST m_sPresentationURL.c_str());
			xmlTextWriterEndElement(writer);
      
		/* end device */
		xmlTextWriterEndElement(writer);

	/* end root */
	xmlTextWriterEndElement(writer);	
	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	std::stringstream output;
	output << (const char*)buf->content;
	
	xmlBufferFree(buf);
	
	//cout << "upnp description: " << output.str() << endl;
	return output.str();
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

/* ParseDescription */
bool CUPnPDevice::ParseDescription(std::string p_sDescription)
{
  #warning todo: parse description
  
  /*xmlDocPtr pDoc = NULL;
  pDoc = xmlReadMemory(p_sDescription.c_str(), p_sDescription.length(), "", NULL, 0);
  if(!pDoc) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "xml parser error", __FILE__, __LINE__);
    return false;    
  }
    
  xmlNode* pRootNode = NULL;  
  xmlNode* pTmpNode  = NULL;   
  pRootNode = xmlDocGetRootElement(pDoc);  
  
  
  xmlFreeDoc(pDoc);
  xmlCleanupParser(); */  
  
  
  //cout << p_sDescription << endl;
  
  RegEx rxFriendlyName("<friendlyName>(.*)</friendlyName>", PCRE_CASELESS);
  if(rxFriendlyName.Search(p_sDescription.c_str())) {
    m_sFriendlyName = rxFriendlyName.Match(1);
  }
  
  RegEx rxUUID("<UDN>uuid:(.+)</UDN>", PCRE_CASELESS);
  if(rxUUID.Search(p_sDescription.c_str())) {
    m_sUUID = rxUUID.Match(1);
  }
  
  RegEx rxDeviceType("<deviceType>(.*)</deviceType>", PCRE_CASELESS);
  if(rxDeviceType.Search(p_sDescription.c_str())) {
    string sDevType = ToLower(rxDeviceType.Match(1));    
    if(sDevType.compare("urn:schemas-upnp-org:device:mediarenderer:1") == 0)    
      m_nUPnPDeviceType = UPNP_DEVICE_TYPE_MEDIA_RENDERER;
    else if(sDevType.compare("urn:schemas-upnp-org:device:mediaserver:1") == 0)    
      m_nUPnPDeviceType = UPNP_DEVICE_TYPE_MEDIA_SERVER;
  }
  
  /*<?xml version="1.0"?>
<root
  xmlns="urn:schemas-upnp-org:device-1-0">
        <specVersion>
                <major>1</major>
                <minor>0</minor>
        </specVersion>
        <INMPR03>1.0</INMPR03>
        <device>
                <deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>
                <friendlyName>NOXON audio</friendlyName>
                <manufacturer>TerraTec GmbH</manufacturer>
                <manufacturerURL>www.TerraTec.de</manufacturerURL>
                <modelDescription>NOXON audio</modelDescription>
                <modelName>NOXON 1.0</modelName>
                <modelNumber>1.0</modelNumber>
                <modelURL>http://www.TerraTec.de</modelURL>
                <serialNumber>0010C64ABF59</serialNumber>
                <UDN>uuid:00000000-0000-0000-0000-08000E200000</UDN>
                <UPC>123810928305556upc</UPC>
                <serviceList>
                        <service>
                                <serviceType>urn:schemas-upnp-org:service:RenderingControl:1</serviceType>
                                <serviceId>urn:upnp-org:serviceId:RenderingControlServiceID</serviceId>
                                <SCPDURL>/RenderingControl/desc.xml</SCPDURL>
                                <controlURL>/RenderingControl/ctrl</controlURL>
                                <eventSubURL>/RenderingControl/evt</eventSubURL>                        </service>
                        <service>
                                <serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>
                                <serviceId>urn:upnp-org:serviceId:ConnectionManagerServiceID</serviceId>
                                <SCPDURL>/ConnectionManager/desc.xml</SCPDURL>
                                <controlURL>/ConnectionManager/ctrl</controlURL>                                <eventSubURL>/ConnectionManager/evt</eventSubURL>
                        </service>
                        <service>
                                <serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>
                                <serviceId>urn:upnp-org:serviceId:AVTransportServiceID</serviceId>
                                <SCPDURL>/AVTransport/desc.xml</SCPDURL>
                                <controlURL>/AVTransport/ctrl</controlURL>
                                <eventSubURL>/AVTransport/evt</eventSubURL>
                        </service>
                        <service>
                                <serviceType>urn:schemas-upnp-org:service:HtmlPageHandler:1</serviceType>
                                <serviceId>urn:upnp-org:serviceId:HtmlPageServiceID</serviceId>
                                <SCPDURL>/HtmlPageHandler/desc.xml</SCPDURL>
                                <controlURL>/HtmlPageHandler/ctrl</controlURL>
                                <eventSubURL>/HtmlPageHandler/evt</eventSubURL>
                        </service>
                </serviceList>
                <presentationURL>/index.html</presentationURL>
        </device>
</root>*/

  return true;
}

/* <\PRIVATE> */
