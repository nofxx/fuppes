/***************************************************************************
 *            UPnPService.cpp
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
 
#include "UPnPService.h"

//#include "xmlencode.h"

#include <sstream>
#include <iostream>
#include <libxml/xmlwriter.h>

CUPnPService::CUPnPService(eUPnPDeviceType p_UPnPDeviceType, std::string p_sHTTPServerURL):
  CUPnPBase(p_UPnPDeviceType, p_sHTTPServerURL)
{
  
}

CUPnPService::~CUPnPService()
{
}

std::string CUPnPService::GetServiceDescription()
{
	xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;
	
  buf    = xmlBufferCreate();
	writer = xmlNewTextWriterMemory(buf, 0);
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);

	/* root */
	xmlTextWriterStartElementNS(writer, NULL, BAD_CAST "scpd", BAD_CAST "urn:schemas-upnp-org:service-1-0");

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
