/***************************************************************************
 *            VideoItem.cpp
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
#include "VideoItem.h"
#include <sstream>
#include <string>

using namespace std;

CVideoItem::CVideoItem(std::string p_sHTTPServerURL):
  CUPnPItem(UPNP_OBJECT_TYPE_IMAGE_ITEM, p_sHTTPServerURL)
{
  m_nVideoType = VIDEO_TYPE_UNKNOWN;
  m_nSize      = 0;
}

/* GetDescription */
void CVideoItem::GetDescription(xmlTextWriterPtr pWriter)
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
    xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.videoItem.movie");
    xmlTextWriterEndElement(pWriter);
    
    /* res */
    xmlTextWriterStartElement(pWriter, BAD_CAST "res");
    
    std::stringstream sTmp;
    sTmp << "http-get:*:" << this->GetMimeType() << ":*";
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
    sTmp.str("");
    
    sTmp << "http://" << GetHTTPServerURL() << "/MediaServer/VideoItems/" << GetObjectID();
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
    xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
    xmlTextWriterEndElement(pWriter);                  
  
  /* end item */
  xmlTextWriterEndElement(pWriter);
}

std::string CVideoItem::GetMimeType()
{
  string sResult = "";
  
  switch(m_nVideoType)
  {
    case VIDEO_TYPE_UNKNOWN:
      break;
    case VIDEO_TYPE_AVI:
      sResult = MIME_TYPE_VIDEO_X_MSVIDEO;
      break;
    case VIDEO_TYPE_MPEG:
      sResult = MIME_TYPE_VIDEO_MPEG;
      break;
  }  
  
  return sResult;
}

/* TODO: nach UPnPItem verlagern */
unsigned int CVideoItem::GetSize()
{
  if(m_nSize == 0)
  {
    fstream fFile;    
    fFile.open(m_sFileName.c_str(), ios::binary|ios::in);
    if(fFile.fail() != 1)
    { 
      fFile.seekg(0, ios::end); 
      m_nSize = streamoff(fFile.tellg());  
      fFile.close();
    }
  }
  
  return m_nSize;  
}
