/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ContentDirectory.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#include "ContentDirectory.h" 
#include "ContentDirectoryDescription.cpp"
#include "../UPnPActions/UPnPBrowse.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../DLNA/DLNA.h"
#include "VirtualContainerMgr.h"

#include "ContentDatabase.h"

//#include <iostream>
#include <sstream>
#include <libxml/xmlwriter.h>
#include <time.h>

using namespace std;
using namespace fuppes;

CContentDirectory::CContentDirectory(std::string p_sHTTPServerURL):
CUPnPService(UPNP_SERVICE_CONTENT_DIRECTORY, 1, p_sHTTPServerURL)
{
  m_hasSubtitles = false;
}

CContentDirectory::~CContentDirectory()
{
}

std::string CContentDirectory::GetServiceDescription()
{
  return sContentDirectoryDescription;
}


/* HandleUPnPAction */
void CContentDirectory::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
  string sContent;
  
  switch((UPNP_CONTENT_DIRECTORY_ACTIONS)pUPnPAction->GetActionType())
  {
    // Browse
    case UPNP_BROWSE:
      DbHandleUPnPBrowse((CUPnPBrowse*)pUPnPAction, &sContent);      
      break;
		// Search
		case UPNP_SEARCH:
		  HandleUPnPSearch((CUPnPSearch*)pUPnPAction, &sContent);
			break;
    // GetSearchCapabilities
    case UPNP_GET_SEARCH_CAPABILITIES:
      HandleUPnPGetSearchCapabilities(pUPnPAction, &sContent);
      break;      
    // GetSortCapabilities
    case UPNP_GET_SORT_CAPABILITIES:
      HandleUPnPGetSortCapabilities(pUPnPAction, &sContent);
      break;
		// GetSortExtensionsCapabilities
    case UPNP_GET_SORT_EXTENSION_CAPABILITIES:
      HandleUPnPGetSortExtensionCapabilities(pUPnPAction, &sContent);
      break;
    // GetSystemUpdateID  
    case UPNP_GET_SYSTEM_UPDATE_ID:
      HandleUPnPGetSystemUpdateID(pUPnPAction, &sContent);
      break;
    // DestroyObject
    case UPNP_DESTROY_OBJECT:
      HandeUPnPDestroyObject(pUPnPAction, &sContent);
    default:
      ASSERT(true == false);
      break;
  }
  
  if(!sContent.empty()) {    
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml; charset=\"utf-8\"");
    pMessageOut->SetContent(sContent);
  }
  else {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR, "text/xml; charset=\"utf-8\"");            

    sContent = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"  
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <s:Fault>"
    "      <faultcode>s:Client</faultcode>"
    "      <faultstring>UPnPError</faultstring>"
    "      <detail>"
    "        <UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">"
    "          <errorCode>401</errorCode>"
    "          <errorDescription>Invalid Action</errorDescription>"
    "        </UPnPError>"
    "      </detail>"
    "    </s:Fault>"
    "  </s:Body>"
    "</s:Envelope>";
    
    pMessageOut->SetContent(sContent);        
  }  
}

  

/* HandleUPnPBrowse */
void CContentDirectory::DbHandleUPnPBrowse(CUPnPBrowse* pUPnPBrowse, std::string* p_psResult)
{ 
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
  
  // root
  xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Envelope", NULL);    
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "s", 
    BAD_CAST "encodingStyle", 
    BAD_CAST  "http://schemas.xmlsoap.org/soap/envelope/", 
    BAD_CAST "http://schemas.xmlsoap.org/soap/encoding/");
   
    // body
    xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Body", NULL);    
  
      // browse response
      xmlTextWriterStartElementNS(writer, BAD_CAST "u",        
        BAD_CAST "BrowseResponse", 
        BAD_CAST "urn:schemas-upnp-org:service:ContentDirectory:1");
          
      // result
      xmlTextWriterStartElement(writer, BAD_CAST "Result");
          
      // build result xml
      xmlTextWriterPtr resWriter;
      xmlBufferPtr resBuf;
      resBuf    = xmlBufferCreate();   
      resWriter = xmlNewTextWriterMemory(resBuf, 0);    
      xmlTextWriterStartDocument(resWriter, NULL, "UTF-8", NULL);
  
      xmlTextWriterStartElementNS(resWriter, NULL, BAD_CAST "DIDL-Lite", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");    
      xmlTextWriterWriteAttribute(resWriter, BAD_CAST "xmlns:dc", BAD_CAST "http://purl.org/dc/elements/1.1/");
      xmlTextWriterWriteAttribute(resWriter, BAD_CAST "xmlns:upnp", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");
      xmlTextWriterWriteAttribute(resWriter, BAD_CAST "xmlns:sec", BAD_CAST "http://www.sec.co.kr/");

                                  
      unsigned int nNumberReturned = 0;
      unsigned int nTotalMatches   = 0;

      bool valid = true;
      switch(pUPnPBrowse->browseFlag()) {
        
        case UPNP_BROWSE_FLAG_METADATA:          
          BrowseMetadata(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);
          break;
        case UPNP_BROWSE_FLAG_DIRECT_CHILDREN:                    
          BrowseDirectChildren(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);
          break;
        default:
          valid = false;
          break;
      }

      if(m_hasSubtitles) {
        //xmlTextWriterWriteAttribute(resWriter, BAD_CAST "xmlns:sec", BAD_CAST "http://www.sec.co.kr/");
      }
              
      // finalize result xml
      xmlTextWriterEndElement(resWriter);
      xmlTextWriterEndDocument(resWriter);
      xmlFreeTextWriter(resWriter);

      std::string sResOutput;
      sResOutput = (const char*)resBuf->content;

      xmlBufferFree(resBuf);        
      sResOutput = sResOutput.substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> "));
      xmlTextWriterWriteString(writer, BAD_CAST sResOutput.c_str());
      
      // end result
      xmlTextWriterEndElement(writer);
        
      // number returned
      xmlTextWriterStartElement(writer, BAD_CAST "NumberReturned");      
      xmlTextWriterWriteFormatString(writer, "%u", nNumberReturned);      
      xmlTextWriterEndElement(writer);
      
      // total matches
      xmlTextWriterStartElement(writer, BAD_CAST "TotalMatches");      
      xmlTextWriterWriteFormatString(writer, "%u", nTotalMatches);      
      xmlTextWriterEndElement(writer);
      
      // update id
      xmlTextWriterStartElement(writer, BAD_CAST "UpdateID");
      xmlTextWriterWriteFormatString(writer, "%u", CContentDatabase::systemUpdateId()); 
      xmlTextWriterEndElement(writer);
  
      // end browse response
      xmlTextWriterEndElement(writer);
      
    // end body
    xmlTextWriterEndElement(writer);
   
	// end root
	xmlTextWriterEndElement(writer);
  xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	//std::stringstream output;
	//string sResult
  if(valid)
    *p_psResult = (const char*)buf->content;
	//output << (const char*)buf->content;  
	xmlBufferFree(buf);  

//cout << *p_psResult << endl;
			  
  /**p_psResult = sResult;  
  CSharedLog::Shared()->Log(L_DEBUG, sResult, __FILE__, __LINE__);*/
}

void CContentDirectory::BrowseMetadata(xmlTextWriterPtr pWriter, 
                        unsigned int* p_pnTotalMatches,
                        unsigned int* p_pnNumberReturned,
                        CUPnPBrowse*  pUPnPBrowse)
{

	//cout << "BrowseMetadata VIRTUAL LAYOUT: " << pUPnPBrowse->virtualFolderLayout() << ":" << endl;

  // total matches and
  // number returned are always 1
  // on metadata browse
  *p_pnTotalMatches   = 1;
  *p_pnNumberReturned = 1;

	SQLQuery qry;
	string sql;
	
  
  // get container type
  OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
  if(pUPnPBrowse->GetObjectIDAsUInt() > 0) {
		sql = qry.build(SQL_GET_OBJECT_TYPE, pUPnPBrowse->GetObjectIDAsUInt(), pUPnPBrowse->virtualFolderLayout());
		qry.select(sql);
    ASSERT(!qry.eof());
    nContainerType = (OBJECT_TYPE)qry.result()->asInt("TYPE");
  }

	
  // get child count
  string sChildCount = "0";
  if(nContainerType < CONTAINER_MAX) {
		sql = qry.build(SQL_COUNT_CHILD_OBJECTS, pUPnPBrowse->GetObjectIDAsUInt(), pUPnPBrowse->virtualFolderLayout());
		qry.select(sql);
    sChildCount = qry.result()->asString("COUNT");
  }

  
  string sParentId;
  
  // root folder
  if(pUPnPBrowse->GetObjectIDAsUInt() == 0)
  {                                  
    sParentId = "-1";
    
    /* build container  */
    xmlTextWriterStartElement(pWriter, BAD_CAST "container"); 
       
      /* id */  
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST pUPnPBrowse->objectId().c_str()); 
      /* searchable  */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "searchable", BAD_CAST "0"); 
      /* parentID  */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST sParentId.c_str()); 
      /* restricted */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");     
      /* childCount */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "childCount", BAD_CAST sChildCount.c_str());   
       
      /* title */
      //xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");     
      xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
      xmlTextWriterWriteString(pWriter, BAD_CAST "root"); 
      xmlTextWriterEndElement(pWriter);
     
      /* class */
      //xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
      xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");     
      xmlTextWriterWriteString(pWriter, BAD_CAST "object.container");
      xmlTextWriterEndElement(pWriter); 
     
    /* end container */
    xmlTextWriterEndElement(pWriter);    
  }
  
  // sub folders
  else
  {
		sql = qry.build(SQL_GET_OBJECT_DETAILS, pUPnPBrowse->GetObjectIDAsUInt(), pUPnPBrowse->virtualFolderLayout());
		qry.select(sql);
		

    char szParentId[11];
    unsigned int nParentId = qry.result()->asUInt("PARENT_ID");
    
    if(nParentId > 0) {
      sprintf(szParentId, "%010X", nParentId);
      sParentId = szParentId;
    }
    else {
      sParentId = "0";
    } 
    
    BuildDescription(pWriter, qry.result(), pUPnPBrowse, sParentId);
  }

}


void CContentDirectory::BrowseDirectChildren(xmlTextWriterPtr pWriter, 
                          unsigned int* p_pnTotalMatches,
                          unsigned int* p_pnNumberReturned,
                          CUPnPBrowse*  pUPnPBrowse)
{ 
  std::stringstream sSql;
	SQLQuery qry;
  //OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
 
	//cout << "BrowseDirectChildren VIRTUAL LAYOUT: " << pUPnPBrowse->virtualFolderLayout() << ":" << endl;

  
  // get total matches
	string sql = qry.build(SQL_COUNT_CHILD_OBJECTS, pUPnPBrowse->GetObjectIDAsUInt(), pUPnPBrowse->virtualFolderLayout());
	qry.select(sql);
 	*p_pnTotalMatches = 0;
  if(!qry.eof()) {
    *p_pnTotalMatches = qry.result()->asInt("COUNT");
  }
  sSql.str("");
  sSql.clear();  

  //cout << "DONE get total matches " << *p_pnTotalMatches << endl; fflush(stdout);  


	sql = qry.build(SQL_GET_CHILD_OBJECTS, pUPnPBrowse->GetObjectIDAsUInt(), pUPnPBrowse->virtualFolderLayout());
	sql +=  pUPnPBrowse->m_sortCriteriaSQL;
    //"  o.TYPE, o.FILE_NAME ";
  
  
  if((pUPnPBrowse->m_nRequestedCount > 0) || (pUPnPBrowse->m_nStartingIndex > 0))  {
    
    sSql << " limit " << pUPnPBrowse->m_nStartingIndex << ", ";    
    if(pUPnPBrowse->m_nRequestedCount == 0)
      sSql << "-1";
    else
      sSql << pUPnPBrowse->m_nRequestedCount;

		sql += sSql.str();		
  }
  
  unsigned int tmpInt = *p_pnNumberReturned;
  

  qry.select(sql);
  while(!qry.eof()) {
	  
    BuildDescription(pWriter, qry.result(), pUPnPBrowse, pUPnPBrowse->objectId());

		qry.next();
    tmpInt++;
  }        
  
   //pDb->ClearResult();                    
  *p_pnNumberReturned = tmpInt;
}

void CContentDirectory::BuildDescription(xmlTextWriterPtr pWriter,
                                         CSQLResult* pSQLResult,
                                         CUPnPBrowseSearchBase* pUPnPBrowse,
                                         std::string p_sParentId)
{
  OBJECT_TYPE nObjType = (OBJECT_TYPE)pSQLResult->asInt("TYPE");
  
  // container
  if(nObjType < CONTAINER_MAX) {
    
    if((nObjType == CONTAINER_PLAYLIST_CONTAINER) && pUPnPBrowse->DeviceSettings()->playlistStyle () != CDeviceSettings::container) {
      BuildItemDescription(pWriter, pSQLResult, pUPnPBrowse, nObjType, p_sParentId);
    }
    else {
      BuildContainerDescription(pWriter, pSQLResult, pUPnPBrowse, p_sParentId, nObjType);
    }
      
  }
  // item
  else if(nObjType >= ITEM) {
    BuildItemDescription(pWriter, pSQLResult, pUPnPBrowse, nObjType, p_sParentId);
  }
}


void CContentDirectory::BuildContainerDescription(xmlTextWriterPtr pWriter, 
                                                  CSQLResult* pSQLResult, 
                                                  CUPnPBrowseSearchBase* pUPnPBrowse, 
                                                  std::string p_sParentId,
                                                  OBJECT_TYPE p_nContainerType)
{
  
  // get child count
  string sChildCount = "0";
  //CContentDatabase* pDb = new CContentDatabase();
  stringstream sSql;
	SQLQuery qry;
	
  string sDevice = pUPnPBrowse->virtualFolderLayout();
  //cout << "BuildContainerDescription DEVICE: " << sDevice << "*" << endl;
  
  /*sSql = string("select count(*) as COUNT from MAP_OBJECTS ") +
    "where PARENT_ID = " + pSQLResult->asString("OBJECT_ID") + " and " + sDevice;*/

	/*sSql <<
      "select count(*) as COUNT " <<
      "from OBJECTS o, MAP_OBJECTS m " <<
      "where " <<
			"m.PARENT_ID = " << pSQLResult->asString("OBJECT_ID") << " and " << 
			"o.OBJECT_ID = m.OBJECT_ID and " <<
			"o.HIDDEN = 0 and " <<
			"m." << sDevice << " and o." << sDevice;*/

	string sql = qry.build(SQL_COUNT_CHILD_OBJECTS, pSQLResult->asString("OBJECT_ID"), sDevice);
	qry.select(sql);
	if(!qry.eof())
		sChildCount = qry.result()->asString("COUNT");
  
  // container
  xmlTextWriterStartElement(pWriter, BAD_CAST "container");   

    // id
    char szObjId[11];    
    unsigned int nObjId = pSQLResult->asUInt("OBJECT_ID");
    sprintf(szObjId, "%010X", nObjId);
  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST szObjId); 
    // searchable
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "searchable", BAD_CAST "true"); 
    // parentID
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST p_sParentId.c_str()); 
    // restricted
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "true");     
    // childCount
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "childCount", BAD_CAST sChildCount.c_str());   
     
    // title
    string sTitle = pSQLResult->asString("TITLE");
    int nLen = pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength;
    if(nLen > 0 && pUPnPBrowse->DeviceSettings()->DisplaySettings()->bShowChildCountInTitle) {
      nLen -= (sChildCount.length() + 3); // "_(n)"
    }
    sTitle = TrimFileName(sTitle, nLen);
    if(pUPnPBrowse->DeviceSettings()->DisplaySettings()->bShowChildCountInTitle) {
      sTitle = sTitle + " (" + sChildCount + ")";
    }
  
	
    //xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");     
    xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
      xmlTextWriterWriteString(pWriter, BAD_CAST sTitle.c_str());    
    xmlTextWriterEndElement(pWriter);
   
    // class
    //xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");       
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");
    xmlTextWriterWriteString(pWriter, BAD_CAST CFileDetails::Shared()->GetContainerTypeAsStr(p_nContainerType).c_str());
    xmlTextWriterEndElement(pWriter); 
     
    // writeStatus (optional)
    /*if(pUPnPBrowse->IncludeProperty("upnp:writeStatus")) {    
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
      xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN"); 
      xmlTextWriterEndElement(pWriter);
    }*/

		if(p_nContainerType == CONTAINER_ALBUM_MUSIC_ALBUM) {

      if(pUPnPBrowse->IncludeProperty("upnp:artist") && !pSQLResult->isNull("AV_ARTIST")) {
        xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:artist");    
          xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("AV_ARTIST").c_str());
      	xmlTextWriterEndElement(pWriter); 
      }

      if(pUPnPBrowse->IncludeProperty("upnp:genre") && !pSQLResult->isNull("AV_GENRE")) {
        xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:genre");
          xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("AV_GENRE").c_str());
      	xmlTextWriterEndElement(pWriter);
      }

		} //  type == CONTAINER_ALBUM_MUSIC_ALBUM


    writeAlbumArtUrl(pWriter, pUPnPBrowse, pSQLResult);
  

    if(p_nContainerType == CONTAINER_PLAYLIST_CONTAINER) {
      // res
      xmlTextWriterStartElement(pWriter, BAD_CAST "res");
      
      string sTmp;
      string ext = ExtractFileExt(pSQLResult->asString("FILE_NAME"));
        
      //sTmp = "http-get:*:" + pSQLResult->asString("MIME_TYPE") + ":*";
			#warning todo MIME TYPE
			sTmp = "http-get:*:todo:*";
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());
      
        
      sTmp = "http://" + m_sHTTPServerURL + "/MediaServer/Playlists/" + szObjId + 
             "." + ext;
      //xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());   
   
      xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.c_str());
      xmlTextWriterEndElement(pWriter); 
    }
	
    
  // end container
  xmlTextWriterEndElement(pWriter); 
}


void CContentDirectory::BuildItemDescription(xmlTextWriterPtr pWriter, 
                                             CSQLResult* pSQLResult, 
                                             CUPnPBrowseSearchBase* pUPnPBrowse, 
                                             OBJECT_TYPE p_nObjectType, 
                                             std::string p_sParentId)
{                                            
  // item
  xmlTextWriterStartElement(pWriter, BAD_CAST "item");

    // id
    char szObjId[11];
    sprintf(szObjId, "%010X", pSQLResult->asUInt("OBJECT_ID"));  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST szObjId); 
    // parentID
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST p_sParentId.c_str()); 
    // restricted 
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "true");
    // ref id
    if(pSQLResult->asUInt("REF_ID") != 0) {
      char refId[11];
      sprintf(refId, "%010X", pSQLResult->asUInt("REF_ID"));
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "refID", BAD_CAST refId); 
    }
  
    // date
    if(pUPnPBrowse->IncludeProperty("dc:date") && !pSQLResult->isNull("DATE")) {
      //CSharedLog::Shared()->Log(L_DBG, "Writing date to stream", __FILE__, __LINE__);
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("DATE").c_str());
      xmlTextWriterEndElement(pWriter);
    }
    
    // writeStatus (optional)
    /*if(pUPnPBrowse->IncludeProperty("upnp:writeStatus")) {
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
      xmlTextWriterEndElement(pWriter);
    }*/    
    
    switch(p_nObjectType)
    {
      case ITEM_AUDIO_ITEM:
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:
        BuildAudioItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      case ITEM_AUDIO_ITEM_AUDIO_BROADCAST:
        BuildAudioBroadcastItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
			case ITEM_VIDEO_ITEM_VIDEO_BROADCAST:
        BuildVideoBroadcastItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      case ITEM_IMAGE_ITEM:
      case ITEM_IMAGE_ITEM_PHOTO:
        BuildImageItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      case ITEM_VIDEO_ITEM:
      case ITEM_VIDEO_ITEM_MOVIE:
        BuildVideoItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      case CONTAINER_PLAYLIST_CONTAINER:
        BuildPlaylistItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      
      default:
        break;
    }           
  
  /* end item */
  xmlTextWriterEndElement(pWriter);  
}
  

void CContentDirectory::BuildAudioItemDescription(xmlTextWriterPtr pWriter,
                                                  CSQLResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{                 
  string sExt = ExtractFileExt(pSQLResult->asString("FILE_NAME"));
                                          
  // title  
  xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");

    string sFileName;
		if(!pSQLResult->isNull("TITLE")) {
		  sFileName = pSQLResult->asString("TITLE");

			// trim filename				
			if(pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength > 0) {
				sFileName = TrimFileName(sFileName, pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength);
			}				
		}

    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);
	
  // class  
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");    
    xmlTextWriterWriteString(pWriter, BAD_CAST pUPnPBrowse->DeviceSettings()->ObjectTypeAsStr(sExt).c_str());    
  xmlTextWriterEndElement(pWriter);                                                    

	if(pUPnPBrowse->IncludeProperty("upnp:artist") && !pSQLResult->isNull("AV_ARTIST")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:artist");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("AV_ARTIST").c_str());
  	xmlTextWriterEndElement(pWriter); 
  }
	
	if(pUPnPBrowse->IncludeProperty("upnp:album") && !pSQLResult->isNull("AV_ALBUM")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:album");
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("AV_ALBUM").c_str());
    xmlTextWriterEndElement(pWriter);
  }

  if(pUPnPBrowse->IncludeProperty("upnp:genre") && !pSQLResult->isNull("AV_GENRE")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:genre");
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("AV_GENRE").c_str());
  	xmlTextWriterEndElement(pWriter);
  }
  
  if(pUPnPBrowse->IncludeProperty("upnp:originalTrackNumber") && !pSQLResult->isNull("A_TRACK_NUMBER")) {	
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:originalTrackNumber");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("A_TRACK_NUMBER").c_str());
	  xmlTextWriterEndElement(pWriter);
  }

  // albumArt
  writeAlbumArtUrl(pWriter, pUPnPBrowse, pSQLResult);

	
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  bool bTranscode  = pUPnPBrowse->DeviceSettings()->DoTranscode(sExt);
  string sMimeType = pUPnPBrowse->DeviceSettings()->MimeType(sExt);
  string targetExt = pUPnPBrowse->DeviceSettings()->Extension(sExt);

	// res@protocolInfo
  string profile;
	if(pUPnPBrowse->DeviceSettings()->dlnaVersion() != CMediaServerSettings::dlna_none) {  

    int channels = 0;
    int bitrate = 0;
    if(!bTranscode) {
      channels = pSQLResult->asInt("A_CHANNELS");
      bitrate = pSQLResult->asInt("A_BITRATE");
    }
    
    DLNA::getAudioProfile(targetExt, channels, bitrate, profile, sMimeType);
  }

  string sTmp = BuildProtocolInfo(bTranscode, sMimeType, profile, pUPnPBrowse);
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());
	
																											
  // res@duration
  if(pUPnPBrowse->IncludeProperty("res@duration") && !pSQLResult->isNull("AV_DURATION")) {
    string dur = FormatHelper::msToUpnpDuration(pSQLResult->asInt("AV_DURATION"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "duration", BAD_CAST dur.c_str());
  }
	
	// res@nrAudioChannels 
  if(pUPnPBrowse->IncludeProperty("res@nrAudioChannels") && !pSQLResult->isNull("A_CHANNELS")) {		
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "nrAudioChannels", BAD_CAST pSQLResult->asString("A_CHANNELS").c_str());
  }

  // res@sampleFrequency (Hz)
  if(pUPnPBrowse->IncludeProperty("res@sampleFrequency")) {    
    if(!bTranscode && !pSQLResult->isNull("A_SAMPLERATE")) {		  
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "sampleFrequency", BAD_CAST pSQLResult->asString("A_SAMPLERATE").c_str());
    }    
    else if(bTranscode && pUPnPBrowse->DeviceSettings()->TargetAudioSampleRate(sExt) > 0) {      
      xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST "sampleFrequency", "%d", pUPnPBrowse->DeviceSettings()->TargetAudioSampleRate(sExt));
    }
  }

	// res@bitrate (bytes! per second)
  if(pUPnPBrowse->IncludeProperty("res@bitrate")) {    
    if(!bTranscode && !pSQLResult->isNull("A_BITRATE")) {
      xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST "bitrate", "%d", (pSQLResult->asInt("A_BITRATE") / 8));
    }
    else if(bTranscode && pUPnPBrowse->DeviceSettings()->TargetAudioBitRate(sExt) > 0) {      
      xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST "bitrate", "%d", (pUPnPBrowse->DeviceSettings()->TargetAudioBitRate(sExt) / 8));
    }
  }

  // res@bitsPerSample 
  if(!bTranscode && pUPnPBrowse->IncludeProperty("res@bitsPerSample") && !pSQLResult->isNull("A_BITS_PER_SAMPLE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "bitsPerSample", BAD_CAST pSQLResult->asString("A_BITS_PER_SAMPLE").c_str());
  }
                                                    
  // res@size
  if(!bTranscode && pUPnPBrowse->IncludeProperty("res@size") && !pSQLResult->isNull("SIZE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "size", BAD_CAST pSQLResult->asString("SIZE").c_str());
  }                                                    

  sExt = pUPnPBrowse->DeviceSettings()->Extension(sExt);

  sTmp = "http://" + m_sHTTPServerURL + "/AudioItems/" + buildObjectAlias(p_sObjectID, pSQLResult) + "." + sExt;    
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.c_str());
  xmlTextWriterEndElement(pWriter);  
}



void CContentDirectory::BuildAudioBroadcastItemDescription(xmlTextWriterPtr pWriter,
                                                  CSQLResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string /*p_sObjectID*/)
{
  // title
	xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
    // trim filename
    string title = TrimFileName(pSQLResult->asString("TITLE"), pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength);    
    //sFileName = TruncateFileExt(title);
    xmlTextWriterWriteString(pWriter, BAD_CAST title.c_str());    
	xmlTextWriterEndElement(pWriter);

  // class
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.audioItem.audioBroadcast");
  xmlTextWriterEndElement(pWriter);      

  // genre (item.audioItem)
  if(pUPnPBrowse->IncludeProperty("upnp:genre") && !pSQLResult->isNull("AV_GENRE")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:genre");
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("AV_GENRE").c_str());
  	xmlTextWriterEndElement(pWriter);
  }

  // dc:description
  if(pUPnPBrowse->IncludeProperty("dc:description")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "dc:description");
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("TITLE").c_str());
  	xmlTextWriterEndElement(pWriter);
  }
  
  // album art
  writeAlbumArtUrl(pWriter, pUPnPBrowse, pSQLResult);

  
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  string protocolInfo = "http-get:*:audio/mpeg:*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST protocolInfo.c_str());
  
  xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("PATH").c_str());
  xmlTextWriterEndElement(pWriter);  
}



void CContentDirectory::BuildImageItemDescription(xmlTextWriterPtr pWriter,
                                                  CSQLResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{
  string sExt = ExtractFileExt(pSQLResult->asString("FILE_NAME"));
  bool bTranscode = pUPnPBrowse->DeviceSettings()->DoTranscode(sExt);
																										
  // title
  xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
    // trim filename
		string sFileName = pSQLResult->asString("TITLE");
		if(pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength > 0) {
			sFileName = TrimFileName(sFileName, pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength);
		}
    //sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);

  // class  
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");
    xmlTextWriterWriteString(pWriter, BAD_CAST pUPnPBrowse->DeviceSettings()->ObjectTypeAsStr(sExt).c_str());
  xmlTextWriterEndElement(pWriter);

  /* storageMedium */
  /*if(pUPnPBrowse->IncludeProperty("upnp:storageMedium")) {  
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "storageMedium", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);
  }*/

/* longDescription upnp No 
   rating upnp No
   description dc No
   publisher dc No  
   date dc No
   rights dc No */  

    // date
    if(pUPnPBrowse->IncludeProperty("dc:date"))
    {
    	if(!pSQLResult->isNull("DATE"))
    	{
    		xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
        	xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("DATE").c_str());
        	xmlTextWriterEndElement(pWriter);
    	}
    }
    
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
	// res@protocolInfo
	string profile;
  string sMimeType = pUPnPBrowse->DeviceSettings()->MimeType(sExt);
  string targetExt = pUPnPBrowse->DeviceSettings()->Extension(sExt);
  
	if(pUPnPBrowse->DeviceSettings()->dlnaVersion() != CMediaServerSettings::dlna_none) {  
    DLNA::getImageProfile(targetExt, pSQLResult->asInt("IV_WIDTH"), pSQLResult->asInt("IV_HEIGHT"), profile, sMimeType);
	}	

  string sTmp = BuildProtocolInfo(bTranscode, sMimeType, profile, pUPnPBrowse);
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());

  
  // res@resolution
	if(pUPnPBrowse->IncludeProperty("res@resolution")) {
    #warning todo rescaling

		if(!pSQLResult->isNull("IV_WIDTH") && !pSQLResult->isNull("IV_HEIGHT")) {
			sTmp = pSQLResult->asString("IV_WIDTH") + "x" + pSQLResult->asString("IV_HEIGHT");
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST sTmp.c_str());
		}
		else if(pUPnPBrowse->DeviceSettings()->ShowEmptyResolution()) {
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST "0x0");
		}
	}

	// res@size
  if(!bTranscode && pUPnPBrowse->IncludeProperty("res@size") && !pSQLResult->isNull("SIZE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "size", BAD_CAST pSQLResult->asString("SIZE").c_str());
  }
  
	sExt = pUPnPBrowse->DeviceSettings()->Extension(sExt);
																									
  sTmp = "http://" + m_sHTTPServerURL + "/ImageItems/" + buildObjectAlias(p_sObjectID, pSQLResult) + "." + sExt;
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.c_str());
  xmlTextWriterEndElement(pWriter);  
    
}

void CContentDirectory::BuildVideoItemDescription(xmlTextWriterPtr pWriter,
                                                  CSQLResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{                                                     
  string sExt = ExtractFileExt(pSQLResult->asString("FILE_NAME"));
    
  bool bTranscode = pUPnPBrowse->DeviceSettings()->DoTranscode(sExt, pSQLResult->asString("AUDIO_CODEC"), pSQLResult->asString("VIDEO_CODEC"));

  // title
  xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
    // trim filename
    string sFileName = pSQLResult->asString("TITLE");
		if(pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength > 0) {
			sFileName = TrimFileName(sFileName, pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength);
		}
                                                    
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);

  // class
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");    
    xmlTextWriterWriteString(pWriter, BAD_CAST pUPnPBrowse->DeviceSettings()->ObjectTypeAsStr(sExt).c_str());
  xmlTextWriterEndElement(pWriter);      

	// albumArt
  writeAlbumArtUrl(pWriter, pUPnPBrowse, pSQLResult);
  
  // subtitle
  if(pSQLResult->asInt("V_HAS_SUBTITLES_FILE") == 1) {
    //<sec:CaptionInfoEx sec:type="srt">http://subtile.url.srt</sec:CaptionInfoEx>    
    xmlTextWriterStartElement(pWriter, BAD_CAST "sec:CaptionInfoEx");    
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "sec:typ", BAD_CAST "srt");
      string sub = "http://" + m_sHTTPServerURL + "/VideoItems/" + p_sObjectID + ".srt";    
      xmlTextWriterWriteString(pWriter, BAD_CAST sub.c_str());
    xmlTextWriterEndElement(pWriter);
    m_hasSubtitles = true;
  }
  
  /*
	if(pSQLResult->asUInt("ALBUM_ART_ID") > 0 || CPluginMgr::hasMetadataPlugin("ffmpegthumbnailer")) {
		xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:albumArtURI");
		  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "xmlns:dlna", BAD_CAST "urn:schemas-dlna-org:metadata-1-0/");
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "dlna:profileID", BAD_CAST "JPEG_SM");
    
			string url = "http://" + m_sHTTPServerURL + "/ImageItems/";
      if(pSQLResult->asUInt("ALBUM_ART_ID") > 0) {
        char szArtId[11];
			  sprintf(szArtId, "%010X", pSQLResult->asUInt("ALBUM_ART_ID"));
        url += szArtId;
      }
      else {
        url += p_sObjectID;
      }
      url += ".jpg?vfolder=none";
      
			xmlTextWriterWriteString(pWriter, BAD_CAST url.c_str());
	  xmlTextWriterEndElement(pWriter);
  }*/

	
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");    

  string sMimeType = pUPnPBrowse->DeviceSettings()->MimeType(sExt, pSQLResult->asString("AUDIO_CODEC"), pSQLResult->asString("VIDEO_CODEC"));
  string targetExt = pUPnPBrowse->DeviceSettings()->Extension(sExt, pSQLResult->asString("AUDIO_CODEC"), pSQLResult->asString("VIDEO_CODEC"));

  // res@protocolInfo
  string profile;
	if(pUPnPBrowse->DeviceSettings()->dlnaVersion() != CMediaServerSettings::dlna_none) {
#warning TODO VIDEO DLNA PROFILE
    //CPluginMgr::dlnaPlugin()->getVideoProfile(targetExt, channels, bitrate, &profile, &sMimeType);
  }
  
  string sTmp = BuildProtocolInfo(bTranscode, sMimeType, profile, pUPnPBrowse);
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());
  
                                                    
  // res@duration
  if(pUPnPBrowse->IncludeProperty("res@duration") && !pSQLResult->isNull("AV_DURATION")) {
    string dur = FormatHelper::msToUpnpDuration(pSQLResult->asInt("AV_DURATION"));
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "duration", BAD_CAST dur.c_str());
  }
      
	// res@resolution 
	if(pUPnPBrowse->IncludeProperty("res@resolution") && !pSQLResult->isNull("IV_WIDTH") && !pSQLResult->isNull("IV_HEIGHT")) {	
		if(!pSQLResult->isNull("IV_WIDTH") && !pSQLResult->isNull("IV_HEIGHT")) {
			sTmp = pSQLResult->asString("IV_WIDTH") + "x" + pSQLResult->asString("IV_HEIGHT");
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST sTmp.c_str());
		}
		else if(pUPnPBrowse->DeviceSettings()->ShowEmptyResolution()) {
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST "0x0");
		}
	}

	// res@bitrate
  if(pUPnPBrowse->IncludeProperty("res@bitrate")) {
    
    if(bTranscode) {
      int nBitRate =
        pUPnPBrowse->DeviceSettings()->FileSettings(sExt)->pTranscodingSettings->VideoBitRate();
      if(nBitRate > 0) {
        stringstream sBitRate;
        sBitRate << nBitRate;
        xmlTextWriterWriteAttribute(pWriter, BAD_CAST "bitrate", BAD_CAST sBitRate.str().c_str());
      }      
    }
    else if(!pSQLResult->isNull("V_BITRATE")) {
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "bitrate", BAD_CAST pSQLResult->asString("V_BITRATE").c_str());
    }
  }
      
  // res@size
  if(!bTranscode && pUPnPBrowse->IncludeProperty("res@size") && !pSQLResult->isNull("SIZE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "size", BAD_CAST pSQLResult->asString("SIZE").c_str());
  }
	
  sExt = pUPnPBrowse->DeviceSettings()->Extension(sExt, pSQLResult->asString("AUDIO_CODEC"), pSQLResult->asString("VIDEO_CODEC"));
                                                    
  sTmp = "http://" + m_sHTTPServerURL + "/VideoItems/" + buildObjectAlias(p_sObjectID, pSQLResult) + "." + sExt;  
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.c_str());
  xmlTextWriterEndElement(pWriter);  
}

void CContentDirectory::BuildVideoBroadcastItemDescription(xmlTextWriterPtr pWriter,
                                                  CSQLResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string /*p_sObjectID*/)
{ 
/* // title
	xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
    // trim filename
    string sFileName = TrimFileName(pSQLResult->asString("FILE_NAME"), pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength, true);    
    sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);

  // class
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.audioItem.audioBroadcast");
  xmlTextWriterEndElement(pWriter);      
  
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");  
  xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("PATH").c_str());
  xmlTextWriterEndElement(pWriter);   */
																											
																											
																											// title
	xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
    // trim filename
    string sFileName = TrimFileName(pSQLResult->asString("FILE_NAME"), pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength);
    sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);
  
  // class
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.videoItem.videoBroadcast");
  xmlTextWriterEndElement(pWriter);
  
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("PATH").c_str());
  xmlTextWriterEndElement(pWriter);  
}

void CContentDirectory::BuildPlaylistItemDescription(xmlTextWriterPtr pWriter,
                                                  CSQLResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{   
  // title	
  xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
		// trim filename
		string sFileName = TrimFileName(pSQLResult->asString("TITLE"), pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength);
		//sFileName = TruncateFileExt(sFileName);
		xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);  
	
	// class
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.playlistItem");
  xmlTextWriterEndElement(pWriter);      

  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
 

  string ext = ExtractFileExt(pSQLResult->asString("FILE_NAME"));

  switch(pUPnPBrowse->DeviceSettings()->playlistStyle()) {
    case CDeviceSettings::container:
    case CDeviceSettings::file:
      break;
    case CDeviceSettings::pls:
      ext = "pls";
      break;
    case CDeviceSettings::m3u:
      ext = "m3u";
      break;
    case CDeviceSettings::wpl:
      ext = "wpl";
      break;
    case CDeviceSettings::xspf:
      ext = "xspf";
      break;
  }

  string mimeType = pUPnPBrowse->DeviceSettings()->MimeType(ext);
  
  std::stringstream sTmp;
	sTmp << "http-get:*:" << mimeType << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
    
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/Playlists/" << p_sObjectID << "." << ext;
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
}

void CContentDirectory::HandleUPnPGetSearchCapabilities(CUPnPAction* /*pAction*/, std::string* p_psResult)
{
  *p_psResult =  
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSearchCapabilitiesResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <SearchCaps>dc:title,upnp:class,upnp:artist,upnp:genre,upnp:album</SearchCaps>"
    "    </u:GetSearchCapabilitiesResponse>"
    "  </s:Body>"
    "</s:Envelope>";

  //<SearchCaps>dc:title,dc:creator,upnp:artist,upnp:genre,upnp:album,dc:date,upnp:originalTrackNumber,upnp:class,@id,@refID,upnp:albumArtURI</SearchCaps>    
} 

void CContentDirectory::HandleUPnPGetSortCapabilities(CUPnPAction* /*pAction*/, std::string* p_psResult)
{
  *p_psResult = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSortCapabilitiesResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <SortCaps>upnp:class,dc:title,upnp:artist,upnp:genre,upnp:album,upnp:originalTrackNumber</SortCaps>"
    "    </u:GetSortCapabilitiesResponse>"
    "  </s:Body>"
    "</s:Envelope>";
	
  // <SortCaps>dc:title,dc:creator,upnp:artist,upnp:genre,upnp:album,dc:date,upnp:originalTrackNumber,Philips:shuffle</SortCaps>
}

void CContentDirectory::HandleUPnPGetSortExtensionCapabilities(CUPnPAction* /*pAction*/, std::string* p_psResult)
{
  *p_psResult = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSortExtensionsCapabilitiesResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <SortExtensionCaps>+, -</SortExtensionCaps>"
    "    </u:GetSortExtensionsCapabilitiesResponse>"
    "  </s:Body>"
    "</s:Envelope>";
}

void CContentDirectory::HandleUPnPGetSystemUpdateID(CUPnPAction* /*pAction*/, std::string* p_psResult)
{
  stringstream result;
  result <<
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSystemUpdateIDResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <Id>" << CContentDatabase::systemUpdateId() << "</Id>"
    "    </u:GetSystemUpdateIDResponse>"
    "  </s:Body>"
    "</s:Envelope>";

  *p_psResult = result.str();
}


void CContentDirectory::HandleUPnPSearch(CUPnPSearch* pSearch, std::string* p_psResult)
{
  unsigned int	nTotalMatches = 0;
  unsigned int	nNumberReturned = 0;
  CSQLQuery*		qry = CDatabase::query();
	CSQLResult*   pRow = NULL;
	
  // get total matches     
	//pSearch->prepareSQL();
	qry->select(pSearch->getQuery(true));
  if(!qry->eof()) {
    nTotalMatches = qry->result()->asInt("COUNT");
  }

  // get items     	
	qry->select(pSearch->getQuery());

  // build result
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;	
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
  
  // root
  xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Envelope", NULL);    
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "s", 
    BAD_CAST "encodingStyle", 
    BAD_CAST  "http://schemas.xmlsoap.org/soap/envelope/", 
    BAD_CAST "http://schemas.xmlsoap.org/soap/encoding/");
   
    // body
    xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Body", NULL);    
  
      // search response
      xmlTextWriterStartElementNS(writer, BAD_CAST "u",        
        BAD_CAST "SearchResponse", 
        BAD_CAST "urn:schemas-upnp-org:service:ContentDirectory:1");
          
      // result
      xmlTextWriterStartElement(writer, BAD_CAST "Result");
          
      // build result xml
      xmlTextWriterPtr resWriter;
      xmlBufferPtr resBuf;
      resBuf    = xmlBufferCreate();   
      resWriter = xmlNewTextWriterMemory(resBuf, 0);    
      xmlTextWriterStartDocument(resWriter, NULL, "UTF-8", NULL);
  
      xmlTextWriterStartElementNS(resWriter, NULL, BAD_CAST "DIDL-Lite", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite/");
      xmlTextWriterWriteAttribute(resWriter, BAD_CAST "xmlns:dc", BAD_CAST "http://purl.org/dc/elements/1.1/");
      xmlTextWriterWriteAttribute(resWriter, BAD_CAST "xmlns:upnp", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");          
  
      while(!qry->eof()) {        
        pRow = qry->result();        
        BuildDescription(resWriter, pRow, pSearch, "0");
        nNumberReturned++;
        
        qry->next();
      }

      // finalize result xml
      xmlTextWriterEndElement(resWriter);
      xmlTextWriterEndDocument(resWriter);
      xmlFreeTextWriter(resWriter);

      string sResOutput = (const char*)resBuf->content;

      xmlBufferFree(resBuf);        
      sResOutput = sResOutput.substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> "));
      xmlTextWriterWriteString(writer, BAD_CAST sResOutput.c_str());
      
      // end result
      xmlTextWriterEndElement(writer);
        
      // number returned      
      xmlTextWriterStartElement(writer, BAD_CAST "NumberReturned");      
      xmlTextWriterWriteFormatString(writer, "%u", nNumberReturned);
      xmlTextWriterEndElement(writer);  
  
      // total matches
      xmlTextWriterStartElement(writer, BAD_CAST "TotalMatches");      
      xmlTextWriterWriteFormatString(writer, "%u", nTotalMatches);
      xmlTextWriterEndElement(writer);
      
      // update id
      xmlTextWriterStartElement(writer, BAD_CAST "UpdateID");
      xmlTextWriterWriteFormatString(writer, "%u", CContentDatabase::systemUpdateId());
      xmlTextWriterEndElement(writer);
  
      // end search response
      xmlTextWriterEndElement(writer);
      
    // end body
    xmlTextWriterEndElement(writer);
   
	// end root
	xmlTextWriterEndElement(writer);
  xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	string output;
	output = (const char*)buf->content;	
  CSharedLog::Log(L_DBG, __FILE__, __LINE__, output);
  
	xmlBufferFree(buf);
	delete qry;
	
  *p_psResult = output;
}

#warning FIXME

void CContentDirectory::HandeUPnPDestroyObject(CUPnPAction* pAction, std::string* p_psResult)
{
  /*
  if(CSharedConfig::Shared()->globalSettings->TrashDir().length() == 0 ||
    CDatabase::connectionParams().readonly == true) {
    return;
  }

  //cout << "DESTROY OBJECT: " << pAction->GetObjectIDAsInt() << endl;

  stringstream sql;  
  sql << "select TYPE, PATH, FILE_NAME from OBJECTS where OBJECT_ID = " << pAction->GetObjectIDAsUInt() << " and DEVICE is NULL";
	
	CSQLQuery* qry = CDatabase::query();
	qry->select(sql.str());
	sql.str("");
    
  string objects;
	if(qry->eof()) {
		delete qry;
	  return;
	}

	CSQLResult* result = qry->result(); 
  OBJECT_TYPE type = (OBJECT_TYPE)result->asInt("TYPE");

    
  #ifndef WIN32
  time_t now;
  char nowtime[26];
  time(&now);  
  ctime_r(&now, nowtime);
	nowtime[24] = '\0';
	string sNowtime = nowtime;
	#else		
  char timeStr[9];    
  _strtime(timeStr);	
	string sNowtime = timeStr;	
	#endif 
    
  // create target dir
  string targetDir = CSharedConfig::Shared()->globalSettings->TrashDir();
  if(type < ITEM) { // container
    targetDir += result->asString("FILE_NAME") + "_" + sNowtime + "/";
  }
  else {
    targetDir += sNowtime + "/";
  }
  
  if(!Directory::create(targetDir)) {
    cout << "contentdir: error creating trash folder : " << targetDir << endl;
		delete qry;
    return;
  }
  if(type >= ITEM) { // item
    targetDir += result->asString("FILE_NAME");
  }
    
  // move    
  //cout << "mv " << db.GetResult()->asString("PATH") << " " << targetDir << endl;  
  
  int ret = rename(string(result->asString("PATH") + result->asString("FILE_NAME")).c_str(), 
									 targetDir.c_str());
  if(ret != 0) {
    cout << "contentdir: error moving to trash folder" << endl;
		delete qry;
    return;
  }

  
  // delete from db
	//CContentDatabase::Shared()->deleteObject(pAction->GetObjectIDAsUInt());
	delete qry;
		
#warning todo: error code
  *p_psResult = "";
  */
}



/* stolen from libdlna :: copyright (C) 2007-2008 Benjamin Zores */


/*
# Play speed
#    1 normal
#    0 invalid
DLNA_ORG_PS = 'DLNA.ORG_PS'
DLNA_ORG_PS_VAL = '1'
*/

enum dlna_org_playSpeed {
  ps_invalid  = 0,
  ps_normal   = 1
};

/*
# Conversion Indicator
#    1 transcoded
#    0 not transcoded
DLNA_ORG_CI = 'DLNA.ORG_CI'
DLNA_ORG_CI_VAL = '0'
*/

enum dlna_org_conversionIndicator {
  ci_none = 0,
  ci_transcoded = 1
};


/*
# Operations
#    00 not time seek range, not range
#    01 range supported
#    10 time seek range supported
#    11 both supported
DLNA_ORG_OP = 'DLNA.ORG_OP'
DLNA_ORG_OP_VAL = '01'
*/

enum dlna_org_operations {
  op_none   = 0x00,
  op_range  = 0x01,
  op_time   = 0x10,
  op_both   = 0x11
};


/*
# Flags
#    senderPaced                      80000000  31
#    lsopTimeBasedSeekSupported       40000000  30
#    lsopByteBasedSeekSupported       20000000  29
#    playcontainerSupported           10000000  28
#    s0IncreasingSupported            08000000  27
#    sNIncreasingSupported            04000000  26
#    rtspPauseSupported               02000000  25
#    streamingTransferModeSupported   01000000  24
#    interactiveTransferModeSupported 00800000  23
#    backgroundTransferModeSupported  00400000  22
#    connectionStallingSupported      00200000  21
#    dlnaVersion15Supported           00100000  20
DLNA_ORG_FLAGS = 'DLNA.ORG_FLAGS'
DLNA_ORG_FLAGS_VAL = '01500000000000000000000000000000'
*/

enum dlna_org_flags {
  flag_senderPaced                      = (1 << 31),
  flag_lsopTimeBasedSeekSupported       = (1 << 30),
  flag_lsopByteBasedSeekSupported       = (1 << 29),
  flag_playcontainerSupported           = (1 << 28),
  flag_s0IncreasingSupported            = (1 << 27),
  flag_sNIncreasingSupported            = (1 << 26),
  flag_rtspPauseSupported               = (1 << 25),
  flag_streamingTransferModeSupported   = (1 << 24),
  flag_interactiveTransferModeSupported = (1 << 23),
  flag_backgroundTransferModeSupported  = (1 << 22),
  flag_connectionStallingSupported      = (1 << 21),
  flag_dlnaVersion15Supported           = (1 << 20)
};


std::string CContentDirectory::buildDlnaInfo(bool transcode, std::string dlnaProfile) // static
{
  string result = "";
  
  // play speed
  dlna_org_playSpeed ps = ps_normal;

  // conversion indicator
  dlna_org_conversionIndicator ci = ci_none;
  if(transcode)
    ci = ci_transcoded;

  // operations
  dlna_org_operations op = op_range;
  if(transcode)
    op = op_none;

  // flags
  int flags = 0;
  flags = 
    flag_streamingTransferModeSupported |
    flag_backgroundTransferModeSupported |
    flag_connectionStallingSupported |  flag_dlnaVersion15Supported;
  if(!transcode)
    flags |= flag_lsopByteBasedSeekSupported;
    

	char dlna_info[448];
	if(!dlnaProfile.empty()) {
		sprintf(dlna_info, "%s=%s;%s=%.2x;%s=%d;%s=%d;%s=%.8x%.24x",
				  "DLNA.ORG_PN", dlnaProfile.c_str(), "DLNA.ORG_OP", op, 
          "DLNA.ORG_PS", ps, "DLNA.ORG_CI", ci, 
          "DLNA.ORG_FLAGS", flags, 0);
	}
	else {
		sprintf(dlna_info, "%s=%.2x;%s=%d;%s=%d;%s=%.8x%.24x",
				  "DLNA.ORG_OP", op, "DLNA.ORG_PS", ps,
          "DLNA.ORG_CI", ci, "DLNA.ORG_FLAGS", flags, 0);
	}

  result = dlna_info;
  return result;
}


std::string CContentDirectory::BuildProtocolInfo(bool p_bTranscode,
                                  std::string p_sMimeType,
                                  std::string p_sProfileId,
                                  CUPnPBrowseSearchBase*  pUPnPBrowse)
{
  string sTmp;
  if(pUPnPBrowse->DeviceSettings()->dlnaVersion() == CMediaServerSettings::dlna_none) {
		sTmp = "http-get:*:" + p_sMimeType + ":*";
		return sTmp;
	}
	
	sTmp = "http-get:*:" + p_sMimeType + ":";
  sTmp += CContentDirectory::buildDlnaInfo(p_bTranscode, p_sProfileId);
  return sTmp;
}




void CContentDirectory::writeAlbumArtUrl(xmlTextWriterPtr pWriter,
                                                CUPnPAction* pAction,
                                                CSQLResult* pSQLResult)
{
  OBJECT_TYPE objectType = (OBJECT_TYPE)pSQLResult->asInt("TYPE");
  bool video = (objectType >= ITEM_VIDEO_ITEM && objectType < ITEM_VIDEO_ITEM_MAX);
  bool audio = (objectType >= ITEM_AUDIO_ITEM && objectType < ITEM_AUDIO_ITEM_MAX);
  bool image = (objectType >= ITEM_IMAGE_ITEM && objectType < ITEM_IMAGE_ITEM_MAX);
  bool container = (objectType >= CONTAINER_STORAGE_FOLDER && objectType < CONTAINER_MAX);

  SQLQuery qry;
  object_id_t albumArtId = 0;
  std::string albumArtExt;
  int albumArtWidth = 0;
  int albumArtHeight = 0;
  bool appendSize = false;
  
  if(audio) {
    // audio files with an album art have either the id for an image file
    // or their own object id if the file contains an image.
    // if the id is 0 there is no album art available
    if(pSQLResult->isNull("ALBUM_ART_ID") || pSQLResult->asUInt("ALBUM_ART_ID") == 0)
      return;

    // object contains the image
    if(pSQLResult->asUInt("ALBUM_ART_ID") == pSQLResult->asUInt("OBJECT_ID") || 
       pSQLResult->asUInt("ALBUM_ART_ID") == pSQLResult->asUInt("REF_ID") ||
       pSQLResult->asUInt("ALBUM_ART_ID") == pSQLResult->asUInt("VREF_ID")) {

      albumArtId = pSQLResult->asUInt("ALBUM_ART_ID");
      albumArtExt = pSQLResult->asString("ALBUM_ART_EXT");
      albumArtWidth = pSQLResult->asInt("ALBUM_ART_WIDTH");
      albumArtHeight = pSQLResult->asInt("ALBUM_ART_HEIGHT");
      if(albumArtWidth == 0 || albumArtHeight == 0) {
        albumArtWidth = 300;
        albumArtHeight = 300;
      }
      appendSize = true;
    }
    // image file
    else {

      if(pSQLResult->isNull("ALBUM_ART_ID") || pSQLResult->asUInt("ALBUM_ART_ID") == 0)
        return;

      albumArtId = pSQLResult->asUInt("ALBUM_ART_ID");
      albumArtExt = pSQLResult->asString("ALBUM_ART_EXT");
      albumArtWidth = pSQLResult->asInt("ALBUM_ART_WIDTH");
      albumArtHeight = pSQLResult->asInt("ALBUM_ART_HEIGHT");
    }
    
  } // audio


  if(image) {

    // if we got an image and no magickWand transcoder there is no album art
    if(!CPluginMgr::hasTranscoderPlugin("magickWand") && 
       (pSQLResult->isNull("ALBUM_ART_ID") || pSQLResult->asUInt("ALBUM_ART_ID") == 0))
    return;

    // else we use the object itself
    albumArtId = pSQLResult->asUInt("OBJECT_ID");
    albumArtExt = ExtractFileExt(pSQLResult->asString("FILE_NAME"));
    albumArtWidth = 300;
    albumArtHeight = 300;
  } // image


  if(video) {

    // no album art set and no ffmpegthumbniler means no album art
    if(!CPluginMgr::hasMetadataPlugin("ffmpegthumbnailer") &&
       (pSQLResult->isNull("ALBUM_ART_ID") || pSQLResult->asUInt("ALBUM_ART_ID") == 0))
      return;

    // create image from the video using ffmpegthumbnailer
    if((pSQLResult->isNull("ALBUM_ART_ID") || pSQLResult->asUInt("ALBUM_ART_ID") == 0)) {
      albumArtId = pSQLResult->asUInt("OBJECT_ID");
      albumArtExt = "jpg";
      albumArtWidth = 300;
      albumArtHeight = 300;    
      
    }
    // image file
    else {

      albumArtId = pSQLResult->asUInt("ALBUM_ART_ID");
      albumArtExt = pSQLResult->asString("ALBUM_ART_EXT");
      albumArtWidth = pSQLResult->asInt("ALBUM_ART_WIDTH");
      albumArtHeight = pSQLResult->asInt("ALBUM_ART_HEIGHT");
    }
    
  } // video


  if(container) {

    if(pSQLResult->isNull("ALBUM_ART_ID") || pSQLResult->asUInt("ALBUM_ART_ID") == 0)
      return;

    albumArtId = pSQLResult->asUInt("ALBUM_ART_ID");
    albumArtExt = pSQLResult->asString("ALBUM_ART_EXT");
    albumArtWidth = pSQLResult->asInt("ALBUM_ART_WIDTH");
    albumArtHeight = pSQLResult->asInt("ALBUM_ART_HEIGHT");
  } // container
  

  std::string mimeType;
  std::string profile;
  DLNA::getImageProfile(albumArtExt, albumArtWidth, albumArtHeight, profile, mimeType);

  if(mimeType.empty()) {
    mimeType = pAction->DeviceSettings()->MimeType(albumArtExt);
  }


  char szArtId[11];
  sprintf(szArtId, "%010X", albumArtId);
  stringstream url;
  url << "http://" << m_sHTTPServerURL << "/ImageItems/" << string(szArtId) << "." << albumArtExt << "?vfolder=none";
  if(appendSize)
    url << "&width=" << albumArtWidth << "&height=" << albumArtHeight;

  // write the xml node
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:albumArtURI");

    if(!profile.empty()) {
      //xmlTextWriterWriteAttribute(pWriter, BAD_CAST "xmlns:dlna", BAD_CAST "urn:schemas-dlna-org:metadata-1-0/");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "dlna:profileID", BAD_CAST profile.c_str());
    }
    
	  xmlTextWriterWriteString(pWriter, BAD_CAST url.str().c_str());

  xmlTextWriterEndElement(pWriter);
}


inline bool convertAlias(std::string& alias)
{
  string tmp = alias;
  tmp = StringReplace(tmp, " & ", " and ");
  tmp = StringReplace(tmp, " ", "%20");
  
  RegEx rxAlias("^[A-Z|a-z|%20|-]+$");
  if(rxAlias.search(tmp)) {
    alias = tmp;
    return true;
  }
  return false;
}

std::string CContentDirectory::buildObjectAlias(std::string objectId, 
                                                CSQLResult* pSQLResult)
{
  /*
  if(!rewriteUrl()) {  
    return objectId;
  }
*/
  
  /*cout << "buildObjectAlias for object id: " << objectId << endl;
  cout << "title: " << pSQLResult->asString("TITLE") << endl;  */

  string alias;
  bool valid = false;

/*
  switch((OBJECT_TYPE)pSQLResult->asInt("TYPE")) {

    case ITEM_AUDIO_ITEM:
    case ITEM_AUDIO_ITEM_MUSIC_TRACK:
      // ARTIST ALBUM TRACK_NO ...
      //valid = convertAlias(alias);
      if(!valid) {
        alias = pSQLResult->asString("TITLE");
        valid = convertAlias(alias);
      }
      break;
    
    default:
      alias = pSQLResult->asString("TITLE");
      valid = convertAlias(alias);
      break;
  }
*/

  //cout << "alias: " << alias << " " << (valid ? "valid" : "invalid") << endl;

  return (valid ? objectId + "/" + alias : objectId);
}

