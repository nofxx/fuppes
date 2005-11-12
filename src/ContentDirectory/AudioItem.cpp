/***************************************************************************
 *            AudioItem.cpp
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "../Common.h"
#include "AudioItem.h"
#include <sstream>
#include <string>

/*===============================================================================
 CLASS CAudioItem
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

/* constructor */
CAudioItem::CAudioItem(std::string p_sHTTPServerURL):
  CUPnPItem(UPNP_OBJECT_TYPE_AUDIO_ITEM, p_sHTTPServerURL)
{
  m_bDoTranscode = false;   
  m_nAudioFormat = AUDIO_FORMAT_UNKNOWN;
  m_nDecoderType = AUDIO_DECODER_UNKNOWN;
  m_nEncoderType = AUDIO_ENCODER_UNKNOWN;
}

/*===============================================================================
 GET
===============================================================================*/

/* GetDescription */
void CAudioItem::GetDescription(xmlTextWriterPtr pWriter)
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
    xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.audioItem.musicTrack");
    xmlTextWriterEndElement(pWriter);
  
    /* creator */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "creator", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "-Unknown-");
    xmlTextWriterEndElement(pWriter);
  
    /* storageMedium */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "storageMedium", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);
  
    /* date */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "2005-10-15");
    xmlTextWriterEndElement(pWriter);
    
    /* writeStatus */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);
    
    /* res */
    xmlTextWriterStartElement(pWriter, BAD_CAST "res");
    
    std::stringstream sTmp;
    sTmp << "http-get:*:" << this->GetMimeType() << ":*";
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
    sTmp.str("");
    
    sTmp << "http://" << GetHTTPServerURL() << "/MediaServer/AudioItems/" << GetObjectID();
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
    xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
    xmlTextWriterEndElement(pWriter);                  
  
  /* end item */
  xmlTextWriterEndElement(pWriter);
}

bool CAudioItem::SetupTranscoding()
{
  std::string sExt = ExtractFileExt(m_sFileName);  
  if (ToLower(sExt).compare("mp3") == 0)
  {
    m_bDoTranscode = false;
    m_nEncoderType = AUDIO_ENCODER_NONE;
    m_nDecoderType = AUDIO_DECODER_NONE;    
    return true;
  }
  else if (ToLower(sExt).compare("ogg") == 0)
  {
    m_bDoTranscode = true;
    m_nEncoderType = AUDIO_ENCODER_LAME;
    m_nDecoderType = AUDIO_DECODER_VORBIS;    
    return true;
  }
  else if (ToLower(sExt).compare("mpc") == 0)
  {
    m_bDoTranscode = true;
    m_nEncoderType = AUDIO_ENCODER_LAME;
    m_nDecoderType = AUDIO_DECODER_MUSEPACK;    
    return true;
  }
  else if (ToLower(sExt).compare("flac") == 0)
  {
    m_bDoTranscode = true;
    m_nEncoderType = AUDIO_ENCODER_LAME;
    m_nDecoderType = AUDIO_DECODER_FLAC;
    return true;
  }
  else
    return false;
}  

/* <\PUBLIC> */

std::string CAudioItem::GetMimeType()
{
  return "audio/mpeg";
}

unsigned int CAudioItem::GetSize()
{
  return 0;
}
