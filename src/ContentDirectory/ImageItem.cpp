/***************************************************************************
 *            ImageItem.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "../Common.h"
#include "ImageItem.h"
#include <sstream>
#include <string>

using namespace std;

CImageItem::CImageItem(std::string p_sHTTPServerURL):
  CUPnPItem(UPNP_OBJECT_TYPE_IMAGE_ITEM, p_sHTTPServerURL)
{
  m_nImageType = IMAGE_TYPE_UNKNOWN;
}

/* GetDescription */
void CImageItem::GetDescription(xmlTextWriterPtr pWriter)
{
  VOID_CHK_RET_POINTER(pWriter);

  /* item */
  xmlTextWriterStartElement(pWriter, BAD_CAST "item");

    /* id */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST GetObjectID().c_str());
    /* parentID */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST GetParent()->GetObjectID().c_str());
    /* restricted */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");    
  
    /* title */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST GetName().c_str());
    xmlTextWriterEndElement(pWriter);
  
    /* class */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.imageItem");
    xmlTextWriterEndElement(pWriter);
  
/* longDescription
upnp
No */

    /* storageMedium */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "storageMedium", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);

/* rating
upnp
No

description
dc
No

publisher
dc
No */

    /* date */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "2005-11-11");
    xmlTextWriterEndElement(pWriter);   
    
/* rights
dc
No */  
    
    /* res */
    xmlTextWriterStartElement(pWriter, BAD_CAST "res");
    
    std::stringstream sTmp;
    sTmp << "http-get:*:" << this->GetMimeType() << ":*";
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
    sTmp.str("");
    
    sTmp << "http://" << GetHTTPServerURL() << "/MediaServer/ImageItems/" << GetObjectID();
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
    xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
    xmlTextWriterEndElement(pWriter);                  
  
  /* end item */
  xmlTextWriterEndElement(pWriter);
}

std::string CImageItem::GetMimeType()
{
  string sResult = "";
  
  switch(m_nImageType)
  {
    case IMAGE_TYPE_UNKNOWN:
      break;
    case IMAGE_TYPE_PNG:
      sResult = "image/png";
      break;
    case IMAGE_TYPE_BMP:
      sResult = "image/bmp";
      break;
    case IMAGE_TYPE_JPEG:
      sResult = "image/jpeg";
      break;
  }  
  
  return sResult;
}
