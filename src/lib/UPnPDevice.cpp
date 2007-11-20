/***************************************************************************
 *            UPnPDevice.cpp
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

#include "UPnPDevice.h"
#include "HTTP/HTTPMessage.h"
#include "Common/Common.h"
#include "Common/RegEx.h"
#include "SharedLog.h"

#include <sstream>
#include <iostream>
#include <libxml/xmlwriter.h>

using namespace std;

CUPnPDevice::CUPnPDevice(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL, IUPnPDevice* pEventHandler):
  CUPnPBase(nType, p_sHTTPServerURL)
{
  /* this constructor is for local devices only */
  m_bIsLocalDevice  = true;  
  m_pEventHandler   = pEventHandler;
  
  m_pTimer = new CTimer(this);
	m_pHTTPClient = NULL;
}


CUPnPDevice::CUPnPDevice(IUPnPDevice* pEventHandler, std::string p_sUUID):
  CUPnPBase(UPNP_DEVICE_UNKNOWN, "")
{
  /* this constructor is for remote devices only */
  m_bIsLocalDevice  = false;
  m_pEventHandler   = pEventHandler;
  m_sUUID						= p_sUUID;
	
  m_pTimer = new CTimer(this);
	m_pHTTPClient = NULL;
}

CUPnPDevice::~CUPnPDevice()
{
  if(m_pHTTPClient)
	  delete m_pHTTPClient;
  delete m_pTimer;
}


void CUPnPDevice::OnTimer()
{
  CSharedLog::Shared()->Log(L_DBG, "OnTimer()", __FILE__, __LINE__);
  if(m_pEventHandler != NULL)
    m_pEventHandler->OnTimer(this);
}

void CUPnPDevice::OnAsyncReceiveMsg(CHTTPMessage* pMessage)
{
 	if(ParseDescription(pMessage->GetContent())) {
		if(m_pEventHandler) {
		  m_pEventHandler->OnNewDevice(this);
    }
	}
	
	/*delete m_pHTTPClient;
	m_pHTTPClient = NULL;*/
}


/* BuildFromDescriptionURL */
void CUPnPDevice::BuildFromDescriptionURL(std::string p_sDescriptionURL)
{	
  if(!m_pHTTPClient)
	  m_pHTTPClient = new CHTTPClient(this);
		
	m_pHTTPClient->AsyncGet(p_sDescriptionURL);
}

/* AddUPnPService */
void CUPnPDevice::AddUPnPService(CUPnPService* pUPnPService)
{
	/* Add service to vector */
  m_vUPnPServices.push_back(pUPnPService);
}

/* GetUPnPServiceCount */
int CUPnPDevice::GetUPnPServiceCount()
{
	return (int)m_vUPnPServices.size();
}

/* GetUPnPService */
CUPnPService* CUPnPDevice::GetUPnPService(int p_nIndex)
{
  if(p_nIndex < 0)
    return NULL;

  return m_vUPnPServices[p_nIndex];
}

/* GetDeviceDescription */
std::string CUPnPDevice::GetDeviceDescription(CHTTPMessage* pRequest)
{		
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	
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

		// url base
		if(pRequest->DeviceSettings()->MediaServerSettings()->UseURLBase) {
			string sUrl = "http://" + m_sHTTPServerURL + "/";
			xmlTextWriterStartElement(writer, BAD_CAST "URLBase");
			xmlTextWriterWriteString(writer, BAD_CAST sUrl.c_str());
			xmlTextWriterEndElement(writer);
		}

		// device
		xmlTextWriterStartElement(writer, BAD_CAST "device");
		
			// UDN
			string sUDN = "uuid:" + m_sUUID;
			xmlTextWriterStartElement(writer, BAD_CAST "UDN");      
      xmlTextWriterWriteString(writer, BAD_CAST sUDN.c_str());
			xmlTextWriterEndElement(writer);
		  
			// friendlyName
			xmlTextWriterStartElement(writer, BAD_CAST "friendlyName");
			xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->FriendlyName.c_str());
      xmlTextWriterEndElement(writer);
			
			// manufacturer
			xmlTextWriterStartElement(writer, BAD_CAST "manufacturer");
			xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->Manufacturer.c_str());	
			xmlTextWriterEndElement(writer);
			
      // manufacturerURL
			xmlTextWriterStartElement(writer, BAD_CAST "manufacturerURL");
			xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->ManufacturerURL.c_str());
			xmlTextWriterEndElement(writer);
			
      // modelDescription
			if(pRequest->DeviceSettings()->MediaServerSettings()->UseModelDescription) {
				xmlTextWriterStartElement(writer, BAD_CAST "modelDescription");      
				xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->ModelDescription.c_str());
				xmlTextWriterEndElement(writer);
			}
			
			// modelName
			xmlTextWriterStartElement(writer, BAD_CAST "modelName");
			xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->ModelName.c_str());
			xmlTextWriterEndElement(writer);
		
			// modelNumber
			xmlTextWriterStartElement(writer, BAD_CAST "modelNumber");
			xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->ModelNumber.c_str());
			xmlTextWriterEndElement(writer);
      
			// modelURL
			xmlTextWriterStartElement(writer, BAD_CAST "modelURL");
			xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->ModelURL.c_str());
			xmlTextWriterEndElement(writer);
		
			// serialNumber
		  if(pRequest->DeviceSettings()->MediaServerSettings()->UseSerialNumber) {
				xmlTextWriterStartElement(writer, BAD_CAST "serialNumber");
				xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->SerialNumber.c_str());
				xmlTextWriterEndElement(writer);
			}
		
			// deviceType
			string sType = "urn:schemas-upnp-org:device:" + GetUPnPDeviceTypeAsString() + ":1";
			xmlTextWriterStartElement(writer, BAD_CAST "deviceType");
			xmlTextWriterWriteString(writer, BAD_CAST sType.c_str());			
      xmlTextWriterEndElement(writer);
		
			// UPC
		  if(pRequest->DeviceSettings()->MediaServerSettings()->UseUPC) {
				xmlTextWriterStartElement(writer, BAD_CAST "UPC");
				xmlTextWriterWriteString(writer, BAD_CAST pRequest->DeviceSettings()->MediaServerSettings()->UPC.c_str());
				xmlTextWriterEndElement(writer);
			}
		
			// DLNA
		  if(pRequest->DeviceSettings()->MediaServerSettings()->UseDLNA) {
				xmlTextWriterStartElementNS(writer, BAD_CAST "dlna", BAD_CAST "X_DLNADOC", BAD_CAST "urn:schemas-dlna-org:device-1-0");
      	xmlTextWriterWriteString(writer, BAD_CAST "DMS-1.00");
				xmlTextWriterEndElement(writer);
			}
		
      // iconList
      if(pRequest->DeviceSettings()->EnableDeviceIcon()) {
  
				xmlTextWriterStartElement(writer, BAD_CAST "iconList");
					xmlTextWriterStartElement(writer, BAD_CAST "icon");
						xmlTextWriterStartElement(writer, BAD_CAST "mimetype");
						xmlTextWriterWriteString(writer, BAD_CAST "image/png");
						xmlTextWriterEndElement(writer);
						xmlTextWriterStartElement(writer, BAD_CAST "width");
						xmlTextWriterWriteString(writer, BAD_CAST "50");
						xmlTextWriterEndElement(writer);
						xmlTextWriterStartElement(writer, BAD_CAST "height");
						xmlTextWriterWriteString(writer, BAD_CAST "50");
						xmlTextWriterEndElement(writer);
						xmlTextWriterStartElement(writer, BAD_CAST "height");
						xmlTextWriterWriteString(writer, BAD_CAST "32");
						xmlTextWriterEndElement(writer);
						xmlTextWriterStartElement(writer, BAD_CAST "url");
						xmlTextWriterWriteString(writer, BAD_CAST "/presentation/images/fuppes-small.png");
						xmlTextWriterEndElement(writer);
					xmlTextWriterEndElement(writer);
				xmlTextWriterEndElement(writer);
        
      } // icon list
  
  
		  // serviceList
			CUPnPService* pTmp;			
			xmlTextWriterStartElement(writer, BAD_CAST "serviceList");			
      for(unsigned int i = 0; i < m_vUPnPServices.size(); i++)
			{
				pTmp = m_vUPnPServices[i];				
				
			  if(pTmp->GetUPnPDeviceType() == UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR)
				{
				  if(!pRequest->DeviceSettings()->MediaServerSettings()->UseXMSMediaReceiverRegistrar)
			      continue;
				
				  stringstream sDescription;
					sDescription << 
						"<service>" <<
						"<serviceType>urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1</serviceType>" <<
						"<serviceId>urn:microsoft.com:serviceId:X_MS_MediaReceiverRegistrar</serviceId>" <<
						"<SCPDURL>/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/description.xml</SCPDURL>" <<
						"<controlURL>/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/control</controlURL>" <<
						"<eventSubURL>/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/event</eventSubURL>" <<
						"</service>";
												
					xmlTextWriterWriteRaw(writer, BAD_CAST sDescription.str().c_str());
					continue;
				}
				
				/* service */
				xmlTextWriterStartElement(writer, BAD_CAST "service");
				
					// serviceType
					string sTmp = "urn:schemas-upnp-org:service:" + pTmp->GetUPnPDeviceTypeAsString() + ":1";
					xmlTextWriterStartElement(writer, BAD_CAST "serviceType");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.c_str());
					xmlTextWriterEndElement(writer);
        
					// serviceId
					sTmp = "urn:upnp-org:serviceId:" + pTmp->GetUPnPDeviceTypeAsString();
					xmlTextWriterStartElement(writer, BAD_CAST "serviceId");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.c_str());					
					xmlTextWriterEndElement(writer);
				
					// SCPDURL
					sTmp = "/UPnPServices/" + pTmp->GetUPnPDeviceTypeAsString() + "/description.xml";
					xmlTextWriterStartElement(writer, BAD_CAST "SCPDURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.c_str());					
					xmlTextWriterEndElement(writer);					

					// controlURL
					sTmp = "/UPnPServices/" + pTmp->GetUPnPDeviceTypeAsString() + "/control/";
					xmlTextWriterStartElement(writer, BAD_CAST "controlURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.c_str());					
					xmlTextWriterEndElement(writer);

					// eventSubURL
					sTmp = "/UPnPServices/" + pTmp->GetUPnPDeviceTypeAsString() + "/event/";
					xmlTextWriterStartElement(writer, BAD_CAST "eventSubURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.c_str());					
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
	
	string output;
	output = (const char*)buf->content;
	
	xmlBufferFree(buf);
	
	//cout << "upnp description: " << output.str() << endl;
	return output;
}


xmlNode* FindNode(std::string p_sNodeName, xmlNode* pParentNode = NULL, bool p_bWalkSubnodes = false)
{
  if(!pParentNode || !pParentNode->children)
	  return NULL;

	xmlNode* pTmpNode = NULL;
	xmlNode* pSubNode = NULL;
	string   sNodeName;
	
	pTmpNode = pParentNode->children;
	do {
	  sNodeName = (char*)pTmpNode->name;
		//cout << "search: " << sNodeName << endl;
		
		if(ToLower(sNodeName).compare(ToLower(p_sNodeName)) == 0) {
		  return pTmpNode;
		}
		
		if(p_bWalkSubnodes && pTmpNode->children) {
		  pSubNode = FindNode(p_sNodeName, pTmpNode, true);
			if(pSubNode)
			  return pSubNode;
		}
		
	  pTmpNode = pTmpNode->next;
	}
	while(pTmpNode);
	
	
	return NULL;
}

/* ParseDescription */
bool CUPnPDevice::ParseDescription(std::string p_sDescription)
{
  //cout << __FILE__ << " parse: " << p_sDescription << endl;
	
  xmlDocPtr pDoc = NULL;
  pDoc = xmlReadMemory(p_sDescription.c_str(), p_sDescription.length(), "", NULL, 0);
  if(!pDoc) {
    CSharedLog::Shared()->Log(L_DBG, "xml parser error", __FILE__, __LINE__);
    return false;    
  }
    
  xmlNode* pRootNode = NULL;  
  xmlNode* pTmpNode  = NULL;   
  pRootNode = xmlDocGetRootElement(pDoc);  
  
	// friendlyName
	pTmpNode = FindNode("friendlyName", pRootNode, true);
	if(pTmpNode && pTmpNode->children)
	  m_sFriendlyName = (char*)pTmpNode->children->content;

	// UDN
	pTmpNode = FindNode("UDN", pRootNode, true);
	if(pTmpNode && pTmpNode->children) {
	  //m_sUUID = 
	}
	
	// deviceType
	pTmpNode = FindNode("deviceType", pRootNode, true);
	if(pTmpNode && pTmpNode->children) {
	  string sDevType = ToLower((char*)pTmpNode->children->content);

    if(sDevType.compare("urn:schemas-upnp-org:device:mediarenderer:1") == 0)    
      m_nUPnPDeviceType = UPNP_DEVICE_MEDIA_RENDERER;
    else if(sDevType.compare("urn:schemas-upnp-org:device:mediaserver:1") == 0)    
      m_nUPnPDeviceType = UPNP_DEVICE_MEDIA_SERVER;
	}
		
			  
				
	// presentationURL
	pTmpNode = FindNode("presentationURL", pRootNode, true);
	if(pTmpNode && pTmpNode->children) {
	  m_sPresentationURL = (char*)pTmpNode->children->content;
	}
				
	// manufacturer
	pTmpNode = FindNode("manufacturer", pRootNode, true);
	if(pTmpNode && pTmpNode->children) {
	  m_sManufacturer = (char*)pTmpNode->children->content;
	}			
				
	// manufacturerURL
	pTmpNode = FindNode("manufacturerURL", pRootNode, true);
	if(pTmpNode && pTmpNode->children) {
	  m_sManufacturerURL = (char*)pTmpNode->children->content;
	}
								
  xmlFreeDoc(pDoc);  

  /*#warning todo: uuid
  RegEx rxUUID("<UDN>uuid:(.+)</UDN>", PCRE_CASELESS);
  if(rxUUID.Search(p_sDescription.c_str())) {
    //m_sUUID = rxUUID.Match(1);
  }*/

  return true;
}
