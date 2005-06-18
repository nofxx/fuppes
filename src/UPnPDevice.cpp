/***************************************************************************
 *            UPnPDevice.cpp
 * 
 *  Copyright  2005  Ulrich Völkel
 *  mail@ulrich-voelkel.de
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
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>

using namespace std;

xmlChar *ConvertInput(const char *in, const char *encoding);

CUPnPDevice::CUPnPDevice(eUPnPDeviceType p_UPnPDeviceType)
{
	m_UPnPDeviceType = p_UPnPDeviceType;
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
	return m_vUPnPServices.size();
}

CUPnPService* CUPnPDevice::GetUPnPService(int p_nIndex)
{
	return m_vUPnPServices[p_nIndex];
}

eUPnPDeviceType CUPnPDevice::GetDeviceType()
{
	return m_UPnPDeviceType;
}

std::string	CUPnPDevice::GetDeviceTypeAsString()
{
	std::string sResult;
	
	switch(m_UPnPDeviceType)
	{
		case dt_root_device:
			sResult = "RootDevice";
		  break;
		case dt_media_server:
			sResult = "MediaServer";
		  break;
	}
	
	return sResult;
}

void CUPnPDevice::SetHTTPServerURL(std::string p_sHTTPServerURL)
{
	m_sHTTPServerURL = p_sHTTPServerURL;
}

std::string CUPnPDevice::GetDescription()
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
			sTmp << "urn:schemas-upnp-org:device:" << GetDeviceTypeAsString() << ":1";
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
      xmlTextWriterWriteString(writer, BAD_CAST m_sUDN.c_str());
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
					sTmp << "urn:schemas-upnp-org:service:" << pTmp->GetUPnPServiceTypeAsString() << ":1";
					xmlTextWriterStartElement(writer, BAD_CAST "serviceType");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
				
					// serviceId
					sTmp << "urn:upnp-org:serviceId:" << pTmp->GetUPnPServiceTypeAsString() << "ServiceID";
					xmlTextWriterStartElement(writer, BAD_CAST "serviceId");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);
				
					// SCPDURL
					sTmp << "/UPnPServices/" << pTmp->GetUPnPServiceTypeAsString() << "/description.xml";
					xmlTextWriterStartElement(writer, BAD_CAST "SCPDURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);					

					// controlURL
					sTmp << "/UPnPServices/" << pTmp->GetUPnPServiceTypeAsString() << "/control/";
					xmlTextWriterStartElement(writer, BAD_CAST "controlURL");
      		xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
					sTmp.str("");
					xmlTextWriterEndElement(writer);					

					// eventSubURL
					sTmp << "/UPnPServices/" << pTmp->GetUPnPServiceTypeAsString() << "/event/";
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

// from: http://xmlsoft.org/examples/testWriter.c
xmlChar *ConvertInput(const char *in, const char *encoding)
{
    xmlChar *out;
    int ret;
    int size;
    int out_size;
    int temp;
    xmlCharEncodingHandlerPtr handler;

    if (in == 0)
        return 0;

    handler = xmlFindCharEncodingHandler(encoding);

    if (!handler) {
        printf("ConvertInput: no encoding handler found for '%s'\n",
               encoding ? encoding : "");
        return 0;
    }

    size = (int) strlen(in) + 1;
    out_size = size * 2 - 1;
    out = (unsigned char *) xmlMalloc((size_t) out_size);

    if (out != 0) {
        temp = size - 1;
        ret = handler->input(out, &out_size, (const xmlChar *) in, &temp);
        if ((ret < 0) || (temp - size + 1)) {
            if (ret < 0) {
                printf("ConvertInput: conversion wasn't successful.\n");
            } else {
                printf
                    ("ConvertInput: conversion wasn't successful. converted: %i octets.\n",
                     temp);
            }

            xmlFree(out);
            out = 0;
        } else {
            out = (unsigned char *) xmlRealloc(out, out_size + 1);
            out[out_size] = 0;  /*null terminating out */
        }
    } else {
        printf("ConvertInput: no mem\n");
    }

    return out;
}
