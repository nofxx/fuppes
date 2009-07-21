/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ContentDirectory.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "VirtualContainerMgr.h"
#include "libdlna/dlna.h"

#include "ContentDatabase.h"

#include <iostream>
#include <sstream>
#include <libxml/xmlwriter.h>

using namespace std;

CContentDirectory::CContentDirectory(std::string p_sHTTPServerURL):
CUPnPService(UPNP_SERVICE_CONTENT_DIRECTORY, p_sHTTPServerURL)
{

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
  
      unsigned int nNumberReturned = 0;
      unsigned int nTotalMatches   = 0;
         
      switch(pUPnPBrowse->browseFlag()) {
        
        case UPNP_BROWSE_FLAG_METADATA:          
          BrowseMetadata(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);          
          break;
        case UPNP_BROWSE_FLAG_DIRECT_CHILDREN:                    
          BrowseDirectChildren(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);          
          break;
        default:
          break;
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
      xmlTextWriterWriteString(writer, BAD_CAST "0");
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
  *p_psResult = (const char*)buf->content;
	//output << (const char*)buf->content;  
	xmlBufferFree(buf);  
  
  /**p_psResult = sResult;  
  CSharedLog::Shared()->Log(L_DEBUG, sResult, __FILE__, __LINE__);*/
}

void CContentDirectory::BrowseMetadata(xmlTextWriterPtr pWriter, 
                        unsigned int* p_pnTotalMatches,
                        unsigned int* p_pnNumberReturned,
                        CUPnPBrowse*  pUPnPBrowse)
{
  // total matches and
  // number returned are always 1
  // on metadata browse
  *p_pnTotalMatches   = 1;
  *p_pnNumberReturned = 1;

  stringstream sSql;

	CSQLQuery* qry = CDatabase::query();

  bool bVirtualContainer = CVirtualContainerMgr::isVirtualContainer(pUPnPBrowse->GetObjectIDAsUInt(),
                                                           pUPnPBrowse->DeviceSettings()->VirtualFolderDevice(), qry);
  
  string sDevice = "DEVICE is NULL ";
  if(bVirtualContainer)
    sDevice = "DEVICE = '" + pUPnPBrowse->DeviceSettings()->VirtualFolderDevice() + "' ";
                        
  // get container type
  OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
  if(pUPnPBrowse->GetObjectIDAsUInt() > 0) {
    
    sSql << "select TYPE from OBJECTS " <<
      "where OBJECT_ID = " << pUPnPBrowse->GetObjectIDAsUInt() << " and " << sDevice;

		qry->select(sSql.str());        
    nContainerType = (OBJECT_TYPE)qry->result()->asInt("TYPE");

    //pDb->ClearResult(); 
    sSql.str("");
    sSql.clear();
  }

  // get child count
  bool bNeedCount = false;
  if(nContainerType < ITEM) {
    sSql <<
      "select count(*) as COUNT " <<
      "from OBJECTS o, MAP_OBJECTS m " <<
      "where " <<
			"m.PARENT_ID = " << pUPnPBrowse->GetObjectIDAsUInt() << " and " << 
			"o.OBJECT_ID = m.OBJECT_ID and " <<
			"o.HIDDEN = 0 and " <<
			"m." << sDevice << " and o." << sDevice;
    
    bNeedCount = true;
  }

  string sChildCount = "0";
  if(bNeedCount) {

		qry->select(sSql.str());        
    sChildCount = qry->result()->asString("COUNT");

    //pDb->ClearResult();    
    sSql.str("");
    sSql.clear();    
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
    sSql <<
      "select " <<
      "  m.OBJECT_ID, m.PARENT_ID, " <<
      "  o.TITLE, o.PATH, o.FILE_NAME, o.TYPE, " <<
      "  d.* " <<
      "from " <<
      "  MAP_OBJECTS m, OBJECTS o " <<
      "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
      "where " <<
      "  m.OBJECT_ID = o.OBJECT_ID and " <<
      "  o.OBJECT_ID = " << pUPnPBrowse->GetObjectIDAsUInt() << " and " <<
			"  o." << sDevice << " and " <<
      "  m." << sDevice;

		qry->select(sSql.str());
    CSQLResult* pRow = qry->result();
		sSql.str("");
    sSql.clear();    
    

    //sTitle = pRow->asString("FILE_NAME");


    char szParentId[11];
    unsigned int nParentId = pRow->asUInt("PARENT_ID");
    
    if(nParentId > 0) {
      sprintf(szParentId, "%010X", nParentId);
      sParentId = szParentId;
    }
    else {
      sParentId = "0";
    } 
    
    BuildDescription(pWriter, pRow, pUPnPBrowse, sParentId);
  }
  
	delete qry;
}


void CContentDirectory::BrowseDirectChildren(xmlTextWriterPtr pWriter, 
                          unsigned int* p_pnTotalMatches,
                          unsigned int* p_pnNumberReturned,
                          CUPnPBrowse*  pUPnPBrowse)
{ 
  std::stringstream sSql;
	CSQLQuery* qry = CDatabase::query();
  //OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
 
                            
  bool bVirtualContainer = CVirtualContainerMgr::hasVirtualChildren(pUPnPBrowse->GetObjectIDAsUInt(), pUPnPBrowse->DeviceSettings()->VirtualFolderDevice(), qry);
 
  string sDevice = "DEVICE is NULL ";
  if(bVirtualContainer)
    sDevice = "DEVICE = '" + pUPnPBrowse->DeviceSettings()->VirtualFolderDevice() + "' ";
                            
  // get total matches
  //cout << "get total matches" << endl; fflush(stdout);
  /*sSql << "select count(*) as COUNT from MAP_OBJECTS where PARENT_ID = " <<
          pUPnPBrowse->GetObjectIDAsUInt() << " and " << sDevice; */
	sSql <<
      "select count(*) as COUNT " <<
      "from OBJECTS o, MAP_OBJECTS m " <<
      "where " <<
			"m.PARENT_ID = " << pUPnPBrowse->GetObjectIDAsUInt() << " and " << 
			"o.OBJECT_ID = m.OBJECT_ID and " <<
			"o.HIDDEN = 0 and " <<
			"m." << sDevice << " and o." << sDevice;
    
  
	*p_pnTotalMatches = 0;

  qry->select(sSql.str()); 
  if(!qry->eof()) {
    //cout << "COUNT: " << pDb->GetResult()->asString("COUNT") << endl; fflush(stdout);
    *p_pnTotalMatches = qry->result()->asInt("COUNT");
  }
  sSql.str("");
  sSql.clear();  
  //cout << "DONE get total matches " << *p_pnTotalMatches << endl; fflush(stdout);  
  
  // get description
  sSql << 
    "select " <<
    "  o.OBJECT_ID, o.TYPE, o.PATH, o.FILE_NAME, " <<
    "  d.IV_HEIGHT, d.IV_WIDTH, d.DATE, " <<
    "  o.TITLE, d.AV_DURATION, d.A_ALBUM, d.A_ARTIST, d.A_GENRE, " <<
  	"  d.A_TRACK_NO, d.AV_BITRATE, d.A_SAMPLERATE, d.A_CHANNELS, " <<
	  "  d.AV_DURATION, d.SIZE, d.A_CODEC, d.V_CODEC, d.DLNA_PROFILE, " <<
		"  d.ALBUM_ART_ID, d.ALBUM_ART_EXT " <<
    "from " <<
    "  MAP_OBJECTS m, OBJECTS o " <<
    "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  o.OBJECT_ID = m.OBJECT_ID and " <<
    "  m.PARENT_ID = " << pUPnPBrowse->GetObjectIDAsUInt() << " and " << 
		"  o." << sDevice << " and " <<
    "  m." << sDevice << " and " <<
		"  o.HIDDEN = 0 " <<
    "order by " <<
		pUPnPBrowse->m_sortCriteriaSQL;
    //"  o.TYPE, o.FILE_NAME ";
  
  
  if((pUPnPBrowse->m_nRequestedCount > 0) || (pUPnPBrowse->m_nStartingIndex > 0))  {
    
    sSql << " limit " << pUPnPBrowse->m_nStartingIndex << ", ";    
    if(pUPnPBrowse->m_nRequestedCount == 0)
      sSql << "-1";
    else
      sSql << pUPnPBrowse->m_nRequestedCount;
  }
  
  unsigned int tmpInt = *p_pnNumberReturned;
  CSQLResult* pRow = NULL;
                          
  qry->select(sSql.str());
  while(!qry->eof()) {
		pRow = qry->result();
    
    BuildDescription(pWriter, pRow, pUPnPBrowse, pUPnPBrowse->objectId());

		qry->next();
    tmpInt++;
  }        
  
   //pDb->ClearResult();                    
  *p_pnNumberReturned = tmpInt;
	delete qry;
}

void CContentDirectory::BuildDescription(xmlTextWriterPtr pWriter,
                                         CSQLResult* pSQLResult,
                                         CUPnPBrowseSearchBase* pUPnPBrowse,
                                         std::string p_sParentId)
{
  OBJECT_TYPE nObjType = (OBJECT_TYPE)pSQLResult->asInt("TYPE");
  
  // container
  if(nObjType < ITEM) {
    
    if((nObjType == CONTAINER_PLAYLIST_CONTAINER) && !pUPnPBrowse->DeviceSettings()->ShowPlaylistAsContainer()) {
      BuildItemDescription(pWriter, pSQLResult, pUPnPBrowse, nObjType, p_sParentId);
    }
    else {
      BuildContainerDescription(pWriter, pSQLResult, pUPnPBrowse, p_sParentId, nObjType);
    }
      
  }
  // item
  else {
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
	CSQLQuery* qry = CDatabase::query();
	
  string sDevice = "DEVICE is NULL ";
  if(CVirtualContainerMgr::hasVirtualChildren(pSQLResult->asUInt("OBJECT_ID"), pUPnPBrowse->DeviceSettings()->VirtualFolderDevice(), qry))
    sDevice = "DEVICE = '" + pUPnPBrowse->DeviceSettings()->VirtualFolderDevice() + "' ";                                                    

  /*sSql = string("select count(*) as COUNT from MAP_OBJECTS ") +
    "where PARENT_ID = " + pSQLResult->asString("OBJECT_ID") + " and " + sDevice;*/

	sSql <<
      "select count(*) as COUNT " <<
      "from OBJECTS o, MAP_OBJECTS m " <<
      "where " <<
			"m.PARENT_ID = " << pSQLResult->asString("OBJECT_ID") << " and " << 
			"o.OBJECT_ID = m.OBJECT_ID and " <<
			"o.HIDDEN = 0 and " <<
			"m." << sDevice << " and o." << sDevice;

	//cout << sSql.str() << endl;	

	qry->select(sSql.str());
	if(!qry->eof())
		sChildCount = qry->result()->asString("COUNT");
  delete qry;
  
  // container
  xmlTextWriterStartElement(pWriter, BAD_CAST "container");   

    // id
    char szObjId[11];    
    unsigned int nObjId = pSQLResult->asUInt("OBJECT_ID");
    sprintf(szObjId, "%010X", nObjId);
  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST szObjId); 
    // searchable
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "searchable", BAD_CAST "0"); 
    // parentID
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST p_sParentId.c_str()); 
    // restricted
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");     
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
   
    #warning todo: create playlist for other folder types
    if(p_nContainerType == CONTAINER_PLAYLIST_CONTAINER) {
      // res
      xmlTextWriterStartElement(pWriter, BAD_CAST "res");
      
      string sTmp;
      //sTmp = "http-get:*:" + pSQLResult->asString("MIME_TYPE") + ":*";
			#warning todo
			sTmp = "http-get:*:todo:*";
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());
      
        
      sTmp = "http://" + m_sHTTPServerURL + "/MediaServer/Playlists/" + szObjId + 
             "." + ExtractFileExt(pSQLResult->asString("FILE_NAME"));
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
  /* item */
  xmlTextWriterStartElement(pWriter, BAD_CAST "item");

    /* id */  
    char szObjId[11];         
    unsigned int nObjId = pSQLResult->asUInt("OBJECT_ID");
    sprintf(szObjId, "%010X", nObjId);   
    //cout << "ITEM ID: " << nObjId << endl;    
  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST szObjId); 
    /* parentID  */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST p_sParentId.c_str()); 
    /* restricted */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");    
  
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
			case ITEM_VIDEO_ITEM_VIDEO_BROADCAST:
        BuildAudioVideoBroadcastItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
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
  
	if(pUPnPBrowse->IncludeProperty("upnp:artist") && !pSQLResult->isNull("A_ARTIST")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:artist");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("A_ARTIST").c_str());
  	xmlTextWriterEndElement(pWriter); 
  }
	
	if(pUPnPBrowse->IncludeProperty("upnp:album") && !pSQLResult->isNull("A_ALBUM")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:album");
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("A_ALBUM").c_str());
    xmlTextWriterEndElement(pWriter);
  }

  if(pUPnPBrowse->IncludeProperty("upnp:genre") && !pSQLResult->isNull("A_GENRE")) {
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:genre");
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("A_GENRE").c_str());
  	xmlTextWriterEndElement(pWriter);
  }
  
  if(pUPnPBrowse->IncludeProperty("upnp:originalTrackNumber") && !pSQLResult->isNull("A_TRACK_NO")) {	
    xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:originalTrackNumber");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->asString("A_TRACK_NO").c_str());
	  xmlTextWriterEndElement(pWriter);
  }
																												
	if(!pSQLResult->isNull("ALBUM_ART_ID") && pSQLResult->asUInt("ALBUM_ART_ID") > 0) {
		xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:albumArtURI");
		  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "xmlns:dlna", BAD_CAST "urn:schemas-dlna-org:metadata-1-0/");
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "dlna:profileID", BAD_CAST "JPEG_TN");

			char szArtId[11];
  		sprintf(szArtId, "%010X", pSQLResult->asUInt("ALBUM_ART_ID"));
			string url = "http://" + m_sHTTPServerURL + "/MediaServer/ImageItems/" + string(szArtId) + ".jpg?width=160&height=160";
			xmlTextWriterWriteString(pWriter, BAD_CAST url.c_str());
	  xmlTextWriterEndElement(pWriter);
  }
	
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  bool bTranscode  = pUPnPBrowse->DeviceSettings()->DoTranscode(sExt);
  string sMimeType = pUPnPBrowse->DeviceSettings()->MimeType(sExt);


	// protocol info
  string sDLNA = pSQLResult->asString("DLNA_PROFILE");
  if(sDLNA.empty())
    sDLNA = pUPnPBrowse->DeviceSettings()->DLNA(sExt);
  
  string sTmp = BuildProtocolInfo(bTranscode, sMimeType, sDLNA, pUPnPBrowse);
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());
	
																											
  // duration
  if(pUPnPBrowse->IncludeProperty("res@duration") && !pSQLResult->isNull("AV_DURATION")) {	
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "duration", BAD_CAST pSQLResult->asString("AV_DURATION").c_str());
  }
	
	// nrAudioChannels 
  if(pUPnPBrowse->IncludeProperty("res@nrAudioChannels") && !pSQLResult->isNull("A_CHANNELS")) {		
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "nrAudioChannels", BAD_CAST pSQLResult->asString("A_CHANNELS").c_str());
  }

  // sampleFrequency
  if(pUPnPBrowse->IncludeProperty("res@sampleFrequency")) {    
    if(!bTranscode && !pSQLResult->isNull("A_SAMPLERATE")) {		  
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "sampleFrequency", BAD_CAST pSQLResult->asString("A_SAMPLERATE").c_str());
    }    
    else if(bTranscode && pUPnPBrowse->DeviceSettings()->TargetAudioSampleRate(sExt) > 0) {      
      xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST "sampleFrequency", "%d", pUPnPBrowse->DeviceSettings()->TargetAudioSampleRate(sExt));
    }
  }

	// bitrate
  if(pUPnPBrowse->IncludeProperty("res@bitrate")) {    
    if(!bTranscode && !pSQLResult->isNull("AV_BITRATE")) {
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "bitrate", BAD_CAST pSQLResult->asString("AV_BITRATE").c_str());
    }    
    else if(bTranscode && pUPnPBrowse->DeviceSettings()->TargetAudioBitRate(sExt) > 0) {      
      xmlTextWriterWriteFormatAttribute(pWriter, BAD_CAST "bitrate", "%d", pUPnPBrowse->DeviceSettings()->TargetAudioBitRate(sExt));
    }
  }
                                                    
  // size
  if(!bTranscode && pUPnPBrowse->IncludeProperty("res@size") && !pSQLResult->isNull("SIZE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "size", BAD_CAST pSQLResult->asString("SIZE").c_str());
  }                                                    

  sExt = pUPnPBrowse->DeviceSettings()->Extension(sExt);
                                                    
  sTmp = "http://" + m_sHTTPServerURL + "/MediaServer/AudioItems/" + p_sObjectID + "." + sExt;  
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.c_str());
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
  
	// protocol info
  string dlna;
	
	if(pUPnPBrowse->DeviceSettings()->DLNAEnabled()) {
		// check image format and convert/rescale if necessary
	}	
	
	if(!bTranscode) {
		dlna = pSQLResult->asString("DLNA_PROFILE");
	}
	else {
		// CPluginMgr::DlnaPlugin()->getImageProfile();
	}

  string sTmp = BuildProtocolInfo(false, pUPnPBrowse->DeviceSettings()->MimeType(sExt), dlna, pUPnPBrowse);
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());

  // resolution
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

	// size
  if(!bTranscode && pUPnPBrowse->IncludeProperty("res@size") && !pSQLResult->isNull("SIZE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "size", BAD_CAST pSQLResult->asString("SIZE").c_str());
  }
  
	sExt = pUPnPBrowse->DeviceSettings()->Extension(sExt);
																									
  sTmp = "http://" + m_sHTTPServerURL + "/MediaServer/ImageItems/" + p_sObjectID + "." + sExt;
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.c_str());
  xmlTextWriterEndElement(pWriter);  
    
}

void CContentDirectory::BuildVideoItemDescription(xmlTextWriterPtr pWriter,
                                               CSQLResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{                                                     
  string sExt = ExtractFileExt(pSQLResult->asString("FILE_NAME"));
    
  bool bTranscode = pUPnPBrowse->DeviceSettings()->DoTranscode(sExt, pSQLResult->asString("A_CODEC"), pSQLResult->asString("V_CODEC"));

  // title
  xmlTextWriterStartElement(pWriter, BAD_CAST "dc:title");
    // trim filename
    string sFileName;
 
		/*string duration;
	  if(!pSQLResult->isNull("AV_DURATION")) {
			duration = " " + pSQLResult->asString("AV_DURATION");
		}*/
	
    sFileName = pSQLResult->asString("TITLE");
		if(pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength > 0) {
			sFileName = TrimFileName(sFileName, pUPnPBrowse->DeviceSettings()->DisplaySettings()->nMaxFileNameLength);
		}
    //sFileName = TruncateFileExt(sFileName);
		//sFileName = sFileName + duration;
                                                    
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);

  // class
  xmlTextWriterStartElement(pWriter, BAD_CAST "upnp:class");    
    xmlTextWriterWriteString(pWriter, BAD_CAST pUPnPBrowse->DeviceSettings()->ObjectTypeAsStr(sExt).c_str());
  xmlTextWriterEndElement(pWriter);      
  
  // res
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");    
    
  string sMimeType = pUPnPBrowse->DeviceSettings()->MimeType(sExt, pSQLResult->asString("A_CODEC"), pSQLResult->asString("V_CODEC"));
  
  

  // protocol info
  string sDLNA = pSQLResult->asString("DLNA_PROFILE");
  if(sDLNA.empty())
    sDLNA = pUPnPBrowse->DeviceSettings()->DLNA(sExt);
  
  string sTmp = BuildProtocolInfo(bTranscode, sMimeType, sDLNA, pUPnPBrowse);  
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.c_str());
                                                    
  // duration
  if(pUPnPBrowse->IncludeProperty("res@duration") && !pSQLResult->isNull("AV_DURATION")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "duration", BAD_CAST pSQLResult->asString("AV_DURATION").c_str());
  }
      
	// resolution 
	if(pUPnPBrowse->IncludeProperty("res@resolution") && !pSQLResult->isNull("IV_WIDTH") && !pSQLResult->isNull("IV_HEIGHT")) {	
		if(!pSQLResult->isNull("IV_WIDTH") && !pSQLResult->isNull("IV_HEIGHT")) {
			sTmp = pSQLResult->asString("IV_WIDTH") + "x" + pSQLResult->asString("IV_HEIGHT");
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST sTmp.c_str());
		}
		else if(pUPnPBrowse->DeviceSettings()->ShowEmptyResolution()) {
			xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST "0x0");
		}
	}

	// bitrate
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
    else if(!pSQLResult->isNull("AV_BITRATE")) {
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "bitrate", BAD_CAST pSQLResult->asString("AV_BITRATE").c_str());
    }
  }
      
  // size
  if(!bTranscode && pUPnPBrowse->IncludeProperty("res@size") && !pSQLResult->isNull("SIZE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "size", BAD_CAST pSQLResult->asString("SIZE").c_str());
  }
  
  sExt = pUPnPBrowse->DeviceSettings()->Extension(sExt, pSQLResult->asString("A_CODEC"), pSQLResult->asString("V_CODEC"));
                                                    
  sTmp = "http://" + m_sHTTPServerURL + "/MediaServer/VideoItems/" + p_sObjectID + "." + sExt;  
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.c_str());
  xmlTextWriterEndElement(pWriter);  
}

void CContentDirectory::BuildAudioVideoBroadcastItemDescription(xmlTextWriterPtr pWriter,
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
  
  std::stringstream sTmp;
  //sTmp << "http-get:*:" << pSQLResult->asString("MIME_TYPE") << ":*";
	#warning todo
	sTmp << "http-get:*:todo:*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
    
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/Playlists/" << p_sObjectID << "." << ExtractFileExt(pSQLResult->asString("PATH"));      
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
    "      <SearchCaps>dc:title,upnp:class,upnp:artist</SearchCaps>"
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
    "      <SortCaps>upnp:class,dc:title,upnp:artist,upnp:originalTrackNumber</SortCaps>"
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
  *p_psResult =  
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSystemUpdateIDResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <Id>1</Id>"
    "    </u:GetSystemUpdateIDResponse>"
    "  </s:Body>"
    "</s:Envelope>";            
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
      xmlTextWriterWriteString(writer, BAD_CAST "0");
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

void CContentDirectory::HandeUPnPDestroyObject(CUPnPAction* pAction, std::string* p_psResult)
{
  if(CSharedConfig::Shared()->ConfigFile()->TrashDir().length() == 0) {
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
  string targetDir = CSharedConfig::Shared()->ConfigFile()->TrashDir();
  if(type < ITEM) { // container
    targetDir += result->asString("FILE_NAME") + "_" + sNowtime + "/";
  }
  else {
    targetDir += sNowtime + "/";
  }
  
  if(!CreateDirectory(targetDir)) {
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
	CContentDatabase db;
  db.deleteObject(pAction->GetObjectIDAsUInt());
	delete qry;
		
#warning todo: error code
  *p_psResult = "";
}


std::string CContentDirectory::BuildProtocolInfo(bool p_bTranscode,
                                  std::string p_sMimeType,
                                  std::string p_sProfileId,
                                  CUPnPBrowseSearchBase*  pUPnPBrowse)
{
  string sTmp;
  if(!pUPnPBrowse->DeviceSettings()->DLNAEnabled()) {
		sTmp = "http-get:*:" + p_sMimeType + ":*";
		return sTmp;
	}
	
	sTmp = "http-get:*:" + p_sMimeType + ":";

	dlna_org_conversion_t ci = DLNA_ORG_CONVERSION_NONE;
	if(p_bTranscode)
		ci = DLNA_ORG_CONVERSION_TRANSCODED;
	
	dlna_org_operation_t op = DLNA_ORG_OPERATION_NONE;
	if(!p_bTranscode)
		op = DLNA_ORG_OPERATION_RANGE;

	int flags;
	flags = 
		DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
		DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
		DLNA_ORG_FLAG_CONNECTION_STALL |
		DLNA_ORG_FLAG_DLNA_V15;
	
	if(!p_bTranscode) {
		flags |= DLNA_ORG_FLAG_BYTE_BASED_SEEK;
	}
	
	char dlna_info[448];
	if(!p_sProfileId.empty()) {
		sprintf(dlna_info, "%s=%d;%s=%d;%s=%.2x;%s=%s;%s=%.8x%.24x",
				 "DLNA.ORG_PS", DLNA_ORG_PLAY_SPEED_NORMAL, "DLNA.ORG_CI", ci,
				 "DLNA.ORG_OP", op, "DLNA.ORG_PN", p_sProfileId.c_str(),
				 "DLNA.ORG_FLAGS", flags, 0);
	}
	else {
		sprintf(dlna_info, "%s=%d;%s=%d;%s=%.2x;%s=%.8x%.24x",
				 "DLNA.ORG_PS", DLNA_ORG_PLAY_SPEED_NORMAL, "DLNA.ORG_CI", ci,
				 "DLNA.ORG_OP", op, "DLNA.ORG_FLAGS", flags, 0);
	}

  sTmp += dlna_info;

  return sTmp;
}
