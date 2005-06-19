/***************************************************************************
 *            StorageFolder.cpp
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

#include "StorageFolder.h"

#include "../SharedConfig.h"

#include <sstream> 
#include <libxml/xmlwriter.h>
#include <iostream>

using namespace std;

CStorageFolder::CStorageFolder()
{
}

CStorageFolder::~CStorageFolder()
{
}

void CStorageFolder::AddItem(CUPnPObject* pUPnPObject)
{
  m_vItems.push_back(pUPnPObject);
}

std::string CStorageFolder::GetContentAsString()
{
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
  
  CUPnPObject* pTmpObj = NULL; 
  
  // root  
  xmlTextWriterStartElementNS(writer, NULL, BAD_CAST "DIDL-Lite", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite");
        
  for(unsigned int i = 0; i < m_vItems.size(); i++)
  {
    pTmpObj = (CUPnPObject*)m_vItems[i];
    
    // item
    xmlTextWriterStartElement(writer, BAD_CAST "item");
    
      // id
      xmlTextWriterWriteAttribute(writer, BAD_CAST "id", BAD_CAST "0000000000000001");
      // parentID
      xmlTextWriterWriteAttribute(writer, BAD_CAST "parentID", BAD_CAST "0");
      // restricted
      xmlTextWriterWriteAttribute(writer, BAD_CAST "restricted", BAD_CAST "0");    
    
      // title
      xmlTextWriterStartElementNS(writer, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");    
      xmlTextWriterWriteString(writer, BAD_CAST "Alle meine Entchen");
      xmlTextWriterEndElement(writer);
    
      // class
      xmlTextWriterStartElementNS(writer, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(writer, BAD_CAST "object.item.audioItem.musicTrack");
      xmlTextWriterEndElement(writer);
    
      // creator
      xmlTextWriterStartElementNS(writer, BAD_CAST "dc", BAD_CAST "creator", BAD_CAST "http://purl.org/dc/elements/1.1/");    
      xmlTextWriterWriteString(writer, BAD_CAST "-Unknown-");
      xmlTextWriterEndElement(writer);
    
      // storageMedium
      xmlTextWriterStartElementNS(writer, BAD_CAST "upnp", BAD_CAST "storageMedium", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(writer, BAD_CAST "UNKNOWN");
      xmlTextWriterEndElement(writer);
    
      // date
      xmlTextWriterStartElementNS(writer, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
      xmlTextWriterWriteString(writer, BAD_CAST "2005-06-19");
      xmlTextWriterEndElement(writer);
      
      // writeStatus
      xmlTextWriterStartElementNS(writer, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(writer, BAD_CAST "UNKNOWN");
      xmlTextWriterEndElement(writer);
      
      // get url
      xmlTextWriterStartElement(writer, BAD_CAST "res");
      xmlTextWriterWriteAttribute(writer, BAD_CAST "protocolInfo", BAD_CAST "http-get:*:audio/mpeg:*");
      stringstream sTmp;
      sTmp << "http://" << CSharedConfig::Shared()->GetHTTPServerURL() << "/test.mp3";
      xmlTextWriterWriteAttribute(writer, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      xmlTextWriterEndElement(writer);                  
    
    // end iten
    xmlTextWriterEndElement(writer);    
  }  
  
  /*Result += "&lt;res protocolInfo=\"http-get:*:audio/mpeg:*\" ";
  Result += "importUri=\"http://" + HTTPServer.Shared.Address + ":" + HTTPServer.Shared.Port.ToString() + "/MediaServer0/" + AItem.Id.ToString("X16") + "/\"&gt;" +
   "http://" + HTTPServer.Shared.Address + ":" + HTTPServer.Shared.Port.ToString() + "/MediaServer0/" + AItem.Id.ToString("X16") + "/" + AItem.Title + "&lt;/res&gt;";*/  
   
	// end root
	xmlTextWriterEndElement(writer);
  xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	std::stringstream output;
	output << (const char*)buf->content;
	
	xmlBufferFree(buf);
  
  //cout << output.str().substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> ")) << endl;  
	return output.str().substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> "));
}
