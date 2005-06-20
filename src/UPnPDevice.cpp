/***************************************************************************
 *            UPnPDevice.cpp
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
 
#include "UPnPDevice.h"

#include <sstream>
#include <iostream>
#include <libxml/xmlwriter.h>

using namespace std;

CUPnPDevice::CUPnPDevice(eUPnPDeviceType p_UPnPDeviceType): CUPnPBase(p_UPnPDeviceType)
{
}

CUPnPDevice::~CUPnPDevice()
{
}

void CUPnPDevice::AddUPnPService(CUPnPService* pUPnPService)
{
	m_vUPnPServices.push_back(pUPnPService);
}

int  CUPnPDevice::GetUPnPServiceCount()
{
	return (int)m_vUPnPServices.size();
}

CUPnPService* CUPnPDevice::GetUPnPService(int p_nIndex)
{
	return m_vUPnPServices[p_nIndex];
}

eUPnPDeviceType CUPnPDevice::GetDeviceType()
{
	return m_UPnPDeviceType;
}

void CUPnPDevice::SetHTTPServerURL(std::string p_sHTTPServerURL)
{
	m_sHTTPServerURL = p_sHTTPServerURL;
}

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

	// root
	xmlTextWriterStartElementNS(writer, NULL, BAD_CAST "root", BAD_CAST "urn:schemas-upnp-org:device-1-0");
	
		// specVersion
		xmlTextWriterStartElement(writer, BAD_CAST "specVersion");
		
			// major
			xmlTextWriterStartElement(writer, BAD_CAST "major");  	
			xmlTextWriterWriteString(writer, BAD_CAST "1");
			xmlTextWriterEndElement(writer);		
			// minor
			xmlTextWriterStartElement(writer, BAD_CAST "minor");  	
			xmlTextWriterWriteString(writer, BAD_CAST "0");
			xmlTextWriterEndElement(writer);		
	
		// end specVersion
		xmlTextWriterEndElement(writer);

		// url base
		sTmp << "http://" << m_sHTTPServerURL << "/";
		xmlTextWriterStartElement(writer, BAD_CAST "URLBase");
		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
		sTmp.str("");
		xmlTextWriterEndElement(writer);

		// device
		xmlTextWriterStartElement(writer, BAD_CAST "device");
		
		  // deviceType			
			sTmp << "urn:schemas-upnp-org:device:" << GetUPnPDeviceTypeAsString() << ":1";
			xmlTextWriterStartElement(writer, BAD_CAST "deviceType");
			xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
			sTmp.str("");
      xmlTextWriterEndElement(writer);
		
			// friendlyName			
			xmlTextWriterStartElement(writer, BAD_CAST "friendlyName");
			xmlTextWriterWriteString(writer, BAD_CAST m_sFriendlyName.c_str());
      xmlTextWriterEndElement(writer);
			
			// manufacturer
			xmlTextWriterStartElement(writer, BAD_CAST "manufacturer");
      xmlTextWriterWriteString(writer, BAD_CAST m_sManufacturer.c_str());
			xmlTextWriterEndElement(writer);
			
      // manufacturerURL
			xmlTextWriterStartElement(writer, BAD_CAST "manufacturerURL");
      xmlTextWriterWriteString(writer, BAD_CAST m_sManufacturerURL.c_str());
			xmlTextWriterEndElement(writer);
			
      // modelDescription
			xmlTextWriterStartElement(writer, BAD_CAST "modelDescription");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelDescription.c_str());
			xmlTextWriterEndElement(writer);
			
			// modelName
			xmlTextWriterStartElement(writer, BAD_CAST "modelName");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelName.c_str());
			xmlTextWriterEndElement(writer);
			
			// modelNumber
			xmlTextWriterStartElement(writer, BAD_CAST "modelNumber");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelNumber.c_str());
			xmlTextWriterEndElement(writer);
      
			// modelURL
			xmlTextWriterStartElement(writer, BAD_CAST "modelURL");
      xmlTextWriterWriteString(writer, BAD_CAST m_sModelURL.c_str());
			xmlTextWriterEndElement(writer);
			
			// serialNumber
			xmlTextWriterStartElement(writer, BAD_CAST "serialNumber");
      xmlTextWriterWriteString(writer, BAD_CAST m_sSerialNumber.c_str());
			xmlTextWriterEndElement(writer);
			
			// UDN
			xmlTextWriterStartElement(writer, BAD_CAST "UDN");
			sTmp << "uuid:" << m_sUDN;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
			xmlTextWriterEndElement(writer);
			
			// UPC
			xmlTextWriterStartElement(writer, BAD_CAST "UPC");
      xmlTextWriterWriteString(writer, BAD_CAST m_sUPC.c_str());
			xmlTextWriterEndElement(writer);		
		
		  // serviceList
			xmlTextWriterStartElement(writer, BAD_CAST "serviceList");			
      for(unsigned int i = 0; i < m_vUPnPServices.size(); i++)
			{
				CUPnPService* pTmp = m_vUPnPServices[i];				
				// service
				xmlTextWriterStartElement(writer, BAD_CAST "service");
				
					// serviceType
					sTmp << "urn:schemas-upnp-org:service:" << pTmp->GetUPnPDeviceTypeAsString() << ":1";
					xmlTextWriterStartElement(writer, BAD_CAST "serviceType");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
				
					// serviceId
					sTmp << "urn:upnp-org:serviceId:" << pTmp->GetUPnPDeviceTypeAsString() << "ServiceID";
					xmlTextWriterStartElement(writer, BAD_CAST "serviceId");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
				
					// SCPDURL
					sTmp << "/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/description.xml";
					xmlTextWriterStartElement(writer, BAD_CAST "SCPDURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);					

					// controlURL
					sTmp << "/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/control/";
					xmlTextWriterStartElement(writer, BAD_CAST "controlURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);					

					// eventSubURL
					sTmp << "/UPnPServices/" << pTmp->GetUPnPDeviceTypeAsString() << "/event/";
					xmlTextWriterStartElement(writer, BAD_CAST "eventSubURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
				
				// end service
				xmlTextWriterEndElement(writer);				
			}			
			// end serviceList			
			xmlTextWriterEndElement(writer);
			
		// end device
		xmlTextWriterEndElement(writer);

	// end root
	xmlTextWriterEndElement(writer);	
	xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	std::stringstream output;
	output << (const char*)buf->content;
	
	xmlBufferFree(buf);
	
	//cout << "upnp description: " << output.str() << endl;
	return output.str();
}