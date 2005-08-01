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
#include <iostream>

using namespace std;

CStorageFolder::CStorageFolder(std::string p_sHTTPServerURL):
  CUPnPObject(uotStorageFolder, p_sHTTPServerURL)
{
}

CStorageFolder::~CStorageFolder()
{
}

void CStorageFolder::AddUPnPObject(CUPnPObject* pUPnPObject)
{
  m_vObjects.push_back(pUPnPObject);
}

string CStorageFolder::GetChildCountAsString()
{
  stringstream sTmp;
  sTmp << (int)m_vObjects.size();
  return sTmp.str();
}

std::string CStorageFolder::GetContentAsString(CUPnPBrowse* pBrowseAction, 
                                               unsigned int* p_nNumberReturned, 
                                               unsigned int* p_nTotalMatches)
{
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
  
  CUPnPObject* pTmpObj = NULL;  
  
  *p_nTotalMatches = (int)m_vObjects.size();
  /* if requestCount is 0 then show everything */
  if(pBrowseAction->m_nRequestedCount == 0)
  {
    *p_nNumberReturned = (int)m_vObjects.size();
  }
  else
  {
    if((pBrowseAction->m_nStartingIndex + pBrowseAction->m_nRequestedCount) > m_vObjects.size())
      *p_nNumberReturned = pBrowseAction->m_nStartingIndex + (m_vObjects.size() - pBrowseAction->m_nStartingIndex);
    else
      *p_nNumberReturned = pBrowseAction->m_nStartingIndex + pBrowseAction->m_nRequestedCount;
  }
  
  /* root */
  xmlTextWriterStartElementNS(writer, NULL, BAD_CAST "DIDL-Lite", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite");
        
  std::vector<CUPnPObject*> vTmp = m_vObjects;
  SortContent(&vTmp, scNone);
  
  for(unsigned int i = pBrowseAction->m_nStartingIndex; i < *p_nNumberReturned; i++)
  { 
    pTmpObj = (CUPnPObject*)vTmp[i]; //m_vObjects[i];
    
    switch(pTmpObj->GetObjectType())
    {
      case uotStorageFolder:        
        pTmpObj->GetDescription(writer);
        break;
      case uotAudioItem:        
        pTmpObj->GetDescription(writer);
        break;
      case uotItem:
        break;
    }
  }  
  
	/* end root */
	xmlTextWriterEndElement(writer);
  xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	std::stringstream output;
	output << (const char*)buf->content;
	
	xmlBufferFree(buf);  
	return output.str().substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> "));
}

void CStorageFolder::GetDescription(xmlTextWriterPtr pWriter)
{
  /* container  */
  xmlTextWriterStartElement(pWriter, BAD_CAST "container"); 
     
    /* id */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST this->GetObjectID().c_str()); 
    /* searchable  */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "searchable", BAD_CAST "0"); 
    /* parentID  */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST this->GetParent()->GetObjectID().c_str()); 
    /* restricted */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");     
    /* childCount */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "childCount", BAD_CAST this->GetChildCountAsString().c_str()); 
     
    /* title */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");     
    xmlTextWriterWriteString(pWriter, BAD_CAST this->GetName().c_str()); 
    xmlTextWriterEndElement(pWriter); 
   
    /* class */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
    xmlTextWriterWriteString(pWriter, BAD_CAST "object.container");  //"object.container.musicContainer"
    xmlTextWriterEndElement(pWriter); 
     
    /* writeStatus */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN"); 
    xmlTextWriterEndElement(pWriter);            
   
  /* end container */
  xmlTextWriterEndElement(pWriter);
}

void CStorageFolder::SortContent(std::vector<CUPnPObject*>* pObjList, eSortCriteria p_SortCriteria)
{
  /* ACHTUNG: Billiger Bubblesort :) */
  
  CUPnPObject* pTmpObj;
  // descending by name
  /*for(unsigned int i = 0; i < pObjList->size(); i++)
  {
    for(unsigned int j = 0; j < pObjList->size(); j++)
    {
      if((*pObjList)[i]->GetName() > (*pObjList)[j]->GetName())
      {
        pTmpObj        = (*pObjList)[i];
        (*pObjList)[i] = (*pObjList)[j];
        (*pObjList)[j] = pTmpObj;
      }
    }
  }*/
    
  /* ascending by name */
  for(unsigned int i = 0; i < pObjList->size(); i++)
  {
    for(unsigned int j = 0; j < pObjList->size(); j++)
    {
      if((*pObjList)[j]->GetName() > (*pObjList)[i]->GetName())
      {
        pTmpObj        = (*pObjList)[j];
        (*pObjList)[j] = (*pObjList)[i];
        (*pObjList)[i] = pTmpObj;
      }
    }
  }  
}
