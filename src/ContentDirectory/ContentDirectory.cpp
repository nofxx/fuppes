/***************************************************************************
 *            ContentDirectory.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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

#include "ContentDirectory.h" 
#include "ContentDirectoryDescription.cpp"
#include "../UPnPActions/UPnPBrowse.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "FileDetails.h"
 
#include <iostream>
#include <sstream>
#include <libxml/xmlwriter.h>

using namespace std;

const string LOGNAME = "ContentDir";


/* constructor */
CContentDirectory::CContentDirectory(std::string p_sHTTPServerURL):
CUPnPService(UPNP_DEVICE_TYPE_CONTENT_DIRECTORY, p_sHTTPServerURL)
{
  /* Init database */  
  bool bIsNewDB = false;   
  if(!CContentDatabase::Shared()->Init(&bIsNewDB))
  {
    if(bIsNewDB)
    {
      stringstream sLog;
      sLog << "unable to create database file '" << CSharedConfig::Shared()->GetConfigDir() << "fuppes.db" << "'." << endl <<
              "make sure you have write permissions on that directory" << endl;      
      CSharedLog::Shared()->Log(L_ERROR, sLog.str(), __FILE__, __LINE__);
    }    
    return;
  } 
  
  if(bIsNewDB)
    CContentDatabase::Shared()->BuildDB();
}

/* destructor */
CContentDirectory::~CContentDirectory()
{
}

std::string CContentDirectory::GetServiceDescription()
{
  return sContentDirectoryDescription;
}


/*===============================================================================
 UPNP ACTION HANDLING
===============================================================================*/
 
/* HandleUPnPAction */
void CContentDirectory::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
  string sContent = "";
  
  switch(pUPnPAction->GetActionType())
  {
    // Browse
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_BROWSE:      
      sContent = DbHandleUPnPBrowse((CUPnPBrowse*)pUPnPAction);      
      break;
		// Search
		case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_SEARCH:
		  sContent = HandleUPnPSearch((CUPnPSearch*)pUPnPAction);
			break;
    // GetSearchCapabilities
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SEARCH_CAPABILITIES:
      sContent = HandleUPnPGetSearchCapabilities(pUPnPAction);
      break;      
    // GetSortCapabilities
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SORT_CAPABILITIES:
      sContent = HandleUPnPGetSortCapabilities(pUPnPAction);
      break;
    // GetSystemUpdateID  
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SYSTEM_UPDATE_ID:
      sContent = HandleUPnPGetSystemUpdateID(pUPnPAction);
      break;
  }
  
  if(!sContent.empty())
  {    
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml; charset=\"utf-8\"");
    pMessageOut->SetContent(sContent);
  }
  else
  {
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
std::string CContentDirectory::DbHandleUPnPBrowse(CUPnPBrowse* pUPnPBrowse)
{  
  //cout << "BROWSE ID: " << pUPnPBrowse->m_sObjectID << " (" << pUPnPBrowse->GetObjectIDAsInt() << ")" << endl << endl;
  
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
  
  /* root */
  xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Envelope", NULL);    
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "s", 
    BAD_CAST "encodingStyle", 
    BAD_CAST  "http://schemas.xmlsoap.org/soap/envelope/", 
    BAD_CAST "http://schemas.xmlsoap.org/soap/encoding/");
   
    /* body */
    xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Body", NULL);    
  
      /* browse response */
      xmlTextWriterStartElementNS(writer, BAD_CAST "u",        
        BAD_CAST "BrowseResponse", 
        BAD_CAST "urn:schemas-upnp-org:service:ContentDirectory:1");
          
      /* result */
      xmlTextWriterStartElement(writer, BAD_CAST "Result");
          
      /* build result xml */
      xmlTextWriterPtr resWriter;
      xmlBufferPtr resBuf;
      resBuf    = xmlBufferCreate();   
      resWriter = xmlNewTextWriterMemory(resBuf, 0);    
      xmlTextWriterStartDocument(resWriter, NULL, "UTF-8", NULL);
  
      xmlTextWriterStartElementNS(resWriter, NULL, BAD_CAST "DIDL-Lite", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite");
          
      unsigned int nNumberReturned = 0;
      unsigned int nTotalMatches   = 0;
         
      /* switch browse flag */
      switch(pUPnPBrowse->m_nBrowseFlag)
      {
        case UPNP_BROWSE_FLAG_METADATA:
          CSharedLog::Shared()->DebugLog(LOGNAME, "CContentDirectory::DbHandleUPnPBrowse - BrowseMetadata");  
          BrowseMetadata(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);
          break;
        case UPNP_BROWSE_FLAG_DIRECT_CHILDREN:
          CSharedLog::Shared()->DebugLog(LOGNAME, "CContentDirectory::DbHandleUPnPBrowse - BrowseDirectChildren");
          BrowseDirectChildren(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);
          break;        
      }   
  
      /*cout << "start idx: " << pUPnPBrowse->m_nStartingIndex << endl;
      cout << "request: " << pUPnPBrowse->m_nRequestedCount << endl;
      cout << "return: " << nNumberReturned << endl;*/
              
      /* finalize result xml */
      xmlTextWriterEndElement(resWriter);
      xmlTextWriterEndDocument(resWriter);
      xmlFreeTextWriter(resWriter);

      std::stringstream sResOutput;
      sResOutput << (const char*)resBuf->content;

      xmlBufferFree(resBuf);        
      string sTmpRes = sResOutput.str().substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> "));
      xmlTextWriterWriteString(writer, BAD_CAST sTmpRes.c_str());
      
      /* end result */
      xmlTextWriterEndElement(writer);
        
      /* number returned */
      xmlTextWriterStartElement(writer, BAD_CAST "NumberReturned");
      sTmp << nNumberReturned;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
      xmlTextWriterEndElement(writer);
      
      /* total matches */
      xmlTextWriterStartElement(writer, BAD_CAST "TotalMatches");
      sTmp << nTotalMatches;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
      xmlTextWriterEndElement(writer);
      
      /* update id */
      xmlTextWriterStartElement(writer, BAD_CAST "UpdateID");
      xmlTextWriterWriteString(writer, BAD_CAST "0");
      xmlTextWriterEndElement(writer);
  
      /* end browse response */
      xmlTextWriterEndElement(writer);
      
    /* end body */
    xmlTextWriterEndElement(writer);
   
	/* end root */
	xmlTextWriterEndElement(writer);
  xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	std::stringstream output;
	output << (const char*)buf->content;	
  CSharedLog::Shared()->DebugLog(LOGNAME, output.str());
  
	xmlBufferFree(buf);
  
  //cout << output.str() << endl;
  
	return output.str();  
}

void CContentDirectory::BrowseMetadata(xmlTextWriterPtr pWriter, 
                        unsigned int* p_pnTotalMatches,
                        unsigned int* p_pnNumberReturned,
                        CUPnPBrowse*  pUPnPBrowse)
{
  *p_pnTotalMatches   = 1;
  *p_pnNumberReturned = 1;
  
  stringstream sSql;
  
  /* get container type */
  OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
  if(pUPnPBrowse->GetObjectIDAsInt() > 0)
  {
    sSql << "select TYPE from OBJECTS where ID = " << pUPnPBrowse->GetObjectIDAsInt() << ";";
    
    CContentDatabase::Shared()->Lock();
    CContentDatabase::Shared()->Select(sSql.str());        
    nContainerType = (OBJECT_TYPE)atoi(CContentDatabase::Shared()->GetResult()->GetValue("TYPE").c_str());
    CContentDatabase::Shared()->ClearResult();
    CContentDatabase::Shared()->Unlock();  
    sSql.str("");
  }
  
  /* get child count */
  bool bNeedCount = false;  
  if(nContainerType == CONTAINER_STORAGE_FOLDER) {
    sSql << "select count(*) as COUNT from OBJECTS where " <<
            "PARENT_ID = " << pUPnPBrowse->GetObjectIDAsInt() << ";";
    bNeedCount = true;
  }
  else if(nContainerType == CONTAINER_PLAYLIST_CONTAINER) {
    sSql << "select count(*) as COUNT from PLAYLIST_ITEMS where " <<
            "PLAYLIST_ID = " << pUPnPBrowse->GetObjectIDAsInt() << ";";          
    bNeedCount = true;
  }
  
  string sChildCount = "0";
  if(bNeedCount) {
    CContentDatabase::Shared()->Lock();
    CContentDatabase::Shared()->Select(sSql.str());        
    sChildCount = CContentDatabase::Shared()->GetResult()->GetValue("COUNT").c_str();
    CContentDatabase::Shared()->ClearResult();
    CContentDatabase::Shared()->Unlock();
  }
  sSql.str("");

  string sParentId;
  string sTitle;
  
  // root folder
  if(pUPnPBrowse->GetObjectIDAsInt() == 0)
  {
    sParentId = "-1";
    sTitle    = "root";   
    
    /* build container  */
    xmlTextWriterStartElement(pWriter, BAD_CAST "container"); 
       
      /* id */  
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST pUPnPBrowse->m_sObjectID.c_str()); 
      /* searchable  */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "searchable", BAD_CAST "0"); 
      /* parentID  */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST sParentId.c_str()); 
      /* restricted */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");     
      /* childCount */
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "childCount", BAD_CAST sChildCount.c_str());   
       
      /* title */
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");     
      xmlTextWriterWriteString(pWriter, BAD_CAST sTitle.c_str()); 
      xmlTextWriterEndElement(pWriter);
     
      /* class */
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
      xmlTextWriterWriteString(pWriter, BAD_CAST "object.container");  //"object.container.musicContainer"
      xmlTextWriterEndElement(pWriter); 
     
    /* end container */
    xmlTextWriterEndElement(pWriter);    
  }
  
  // sub folders
  else
  {
    sSql << "select ID, PARENT_ID, PATH, FILE_NAME, TYPE, MIME_TYPE from OBJECTS where ID = " << pUPnPBrowse->GetObjectIDAsInt();
    CContentDatabase::Shared()->Lock();
    
    CContentDatabase::Shared()->Select(sSql.str());                
    CSelectResult* pRow = CContentDatabase::Shared()->GetResult(); 
       
    sTitle = pRow->GetValue("FILE_NAME").c_str();
    
    char szParentId[11];
    unsigned int nParentId = atoi(pRow->GetValue("PARENT_ID").c_str());
    if(nParentId > 0)
    {
      sprintf(szParentId, "%010X", nParentId);
      sParentId = szParentId;
    }
    else
    {
      sParentId = "0";
    }   
   

    sSql.str("");      
    
    BuildDescription(pWriter, pRow, pUPnPBrowse, sParentId);

    CContentDatabase::Shared()->ClearResult();
    CContentDatabase::Shared()->Unlock();
  }
}


void CContentDirectory::BrowseDirectChildren(xmlTextWriterPtr pWriter, 
                          unsigned int* p_pnTotalMatches,
                          unsigned int* p_pnNumberReturned,
                          CUPnPBrowse*  pUPnPBrowse)
{ 
  std::stringstream sSql;
  OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
  
  /* get container type */
  if(pUPnPBrowse->GetObjectIDAsInt() > 0)
  {    
    sSql << "select TYPE from OBJECTS where ID = " << pUPnPBrowse->GetObjectIDAsInt() << ";";
  
    CContentDatabase::Shared()->Lock();
    CContentDatabase::Shared()->Select(sSql.str()); 
    if(!CContentDatabase::Shared()->Eof()) {
      nContainerType = (OBJECT_TYPE)atoi(CContentDatabase::Shared()->GetResult()->GetValue("TYPE").c_str());
    }
    else {
      nContainerType = OBJECT_TYPE_UNKNOWN;
    }
    CContentDatabase::Shared()->ClearResult();
    CContentDatabase::Shared()->Unlock();  
    sSql.str("");
  }
    
  /* get total matches */
  if(nContainerType == CONTAINER_STORAGE_FOLDER) {
    sSql << "select count(*) as COUNT from OBJECTS where PARENT_ID = " <<
            pUPnPBrowse->GetObjectIDAsInt();
  }
  else if(nContainerType == CONTAINER_PLAYLIST_CONTAINER) {
    sSql << "select count(*) as COUNT from PLAYLIST_ITEMS where PLAYLIST_ID = " <<
            pUPnPBrowse->GetObjectIDAsInt();
  }

  
  CContentDatabase::Shared()->Lock();
  CContentDatabase::Shared()->Select(sSql.str()); 
  if(!CContentDatabase::Shared()->Eof())
    *p_pnTotalMatches = atoi(CContentDatabase::Shared()->GetResult()->GetValue("COUNT").c_str());
  else
    *p_pnTotalMatches = 0;
  //string sChildCount = CContentDatabase::Shared()->GetResult()->GetValue("COUNT");
  CContentDatabase::Shared()->ClearResult();
  CContentDatabase::Shared()->Unlock();
  sSql.str("");
  
  /* get description */
  if(nContainerType == CONTAINER_STORAGE_FOLDER)
  {
    sSql << "select ID, TYPE, PATH, FILE_NAME, MIME_TYPE, DETAILS " <<
            "from OBJECTS where PARENT_ID = " << pUPnPBrowse->GetObjectIDAsInt() << " " <<
            "order by TYPE, FILE_NAME ";
  }
  else if(nContainerType == CONTAINER_PLAYLIST_CONTAINER)
  {
      sSql << "select " <<
              "  o.ID, o.TYPE, o.PATH, o.FILE_NAME, o.MIME_TYPE, o.DETAILS " <<
              "from " <<
              "  OBJECTS o, PLAYLIST_ITEMS p " <<
              "where " <<
              "  o.ID = p.OBJECT_ID and " <<
              "  p.PLAYLIST_ID = " << pUPnPBrowse->GetObjectIDAsInt() << " ";
  }

  
  if((pUPnPBrowse->m_nRequestedCount > 0) || (pUPnPBrowse->m_nStartingIndex > 0))
  {
    sSql << " limit " << pUPnPBrowse->m_nStartingIndex << ", ";
    if(pUPnPBrowse->m_nRequestedCount == 0)
      sSql << "-1";
    else
      sSql << pUPnPBrowse->m_nRequestedCount;
  }

    
  
  
  unsigned int tmpInt = *p_pnNumberReturned;
    
  CContentDatabase::Shared()->Lock();
  CContentDatabase::Shared()->Select(sSql.str());
  while(!CContentDatabase::Shared()->Eof())
  {
    CSelectResult* pRow = CContentDatabase::Shared()->GetResult();              
    
    BuildDescription(pWriter, pRow, pUPnPBrowse, pUPnPBrowse->m_sObjectID);
        
    CContentDatabase::Shared()->Next();
    tmpInt++;
  }        
  
  CContentDatabase::Shared()->ClearResult();
  CContentDatabase::Shared()->Unlock();                          
  *p_pnNumberReturned = tmpInt;
}

void CContentDirectory::BuildDescription(xmlTextWriterPtr pWriter,
                                         CSelectResult* pSQLResult,
                                         CUPnPBrowse* pUPnPBrowse,
                                         std::string p_sParentId)
{
  OBJECT_TYPE nObjType = (OBJECT_TYPE)atoi(pSQLResult->GetValue("TYPE").c_str());
  
  switch(nObjType)
  {
    // CONTAINER_STORAGE_FOLDER
    case CONTAINER_STORAGE_FOLDER :
      BuildContainerDescription(pWriter, pSQLResult, pUPnPBrowse, p_sParentId, CONTAINER_STORAGE_FOLDER);         
      break;
      
    // CONTAINER_PLAYLIST_CONTAINER
    case CONTAINER_PLAYLIST_CONTAINER :
      if(CSharedConfig::Shared()->GetDisplaySettings().bShowPlaylistsAsContainers)
        BuildContainerDescription(pWriter, pSQLResult, pUPnPBrowse, p_sParentId, CONTAINER_PLAYLIST_CONTAINER);
      else
        BuildItemDescription(pWriter, pSQLResult, pUPnPBrowse, CONTAINER_PLAYLIST_CONTAINER, p_sParentId);           
      break;      
    
    // ITEM_IMAGE_ITEM_PHOTO
    // ITEM_AUDIO_ITEM_MUSIC_TRACK    
    // ITEM_AUDIO_ITEM_AUDIO_BROADCAST
    // ITEM_VIDEO_ITEM_MOVIE    
    // ITEM_VIDEO_ITEM_VIDEO_BROADCAST
    case ITEM_IMAGE_ITEM_PHOTO :
    case ITEM_AUDIO_ITEM_MUSIC_TRACK :
    case ITEM_AUDIO_ITEM_AUDIO_BROADCAST :
    case ITEM_VIDEO_ITEM_MOVIE :  
    case ITEM_VIDEO_ITEM_VIDEO_BROADCAST :
      BuildItemDescription(pWriter, pSQLResult, pUPnPBrowse, nObjType, p_sParentId);
      break;
      
    default :
      throw EException("unhandled object type", __FILE__, __LINE__);
  }
}


void CContentDirectory::BuildContainerDescription(xmlTextWriterPtr pWriter, 
                                                  CSelectResult* pSQLResult, 
                                                  CUPnPBrowse* pUPnPBrowse, 
                                                  std::string p_sParentId,
                                                  OBJECT_TYPE p_nContainerType)
{
  
  /* get child count */
  string sChildCount = "0";
  CContentDatabase* pDb = new CContentDatabase();
  stringstream sSql;
  
  if(p_nContainerType == CONTAINER_STORAGE_FOLDER)
    sSql << "select count(*) as COUNT from OBJECTS where PARENT_ID = " << pSQLResult->GetValue("ID") << ";";
  else if(p_nContainerType == CONTAINER_PLAYLIST_CONTAINER)
    sSql << "select count(*) as COUNT from PLAYLIST_ITEMS " <<
            "where PLAYLIST_ID = " << pSQLResult->GetValue("ID") << ";";
  
  pDb->Select(sSql.str());
  if(!pDb->Eof())
  {    
    sChildCount = pDb->GetResult()->GetValue("COUNT");    
  }
  
  pDb->ClearResult();
  delete pDb;
  
  
  /* container  */  
  xmlTextWriterStartElement(pWriter, BAD_CAST "container");  
  
  /* 
container Properties
Property Name NS R/O Remarks
@childCount DIDL-Lite O NO
upnp:createClass upnp O
upnp:searchClass upnp O
@searchable DIDL-Lite O
@neverPlayable DIDL-Lite O */
  
  /* 
playlistContainer:container Properties
Property Name NS R/O Remarks
upnp:artist upnp O
upnp:genre upnp O
upnp:longDescription upnp O
upnp:producer upnp O
upnp:storageMedium upnp O
dc:description dc O
dc:contributor dc O
dc:date dc O
dc:language dc O
dc:rights dc O */
  
     
    /* id */  
    char szObjId[11];         
    unsigned int nObjId = atoi(pSQLResult->GetValue("ID").c_str());
    sprintf(szObjId, "%010X", nObjId);   
    //cout << "CONTAINER ID: " << nObjId << endl;
    
  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST szObjId); 
    /* searchable  */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "searchable", BAD_CAST "0"); 
    /* parentID  */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST p_sParentId.c_str()); 
    /* restricted */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");     
    /* childCount */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "childCount", BAD_CAST sChildCount.c_str());   
     
    /* title */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");     
    xmlTextWriterWriteString(pWriter, BAD_CAST TrimFileName(pSQLResult->GetValue("FILE_NAME"), CSharedConfig::Shared()->GetMaxFileNameLength()).c_str());    
    xmlTextWriterEndElement(pWriter);
   
    /* class */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
    
    if(p_nContainerType == CONTAINER_STORAGE_FOLDER)
      xmlTextWriterWriteString(pWriter, BAD_CAST "object.container");
    else if(p_nContainerType == CONTAINER_PLAYLIST_CONTAINER)
      xmlTextWriterWriteString(pWriter, BAD_CAST "object.container.playlistContainer");  
  
    xmlTextWriterEndElement(pWriter); 
     
    /* writeStatus */
    if(pUPnPBrowse->m_sFilter.find("upnp:writeStatus") != std::string::npos)
    {    
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
      xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN"); 
      xmlTextWriterEndElement(pWriter);
    }
   
    if(p_nContainerType == CONTAINER_PLAYLIST_CONTAINER)
    {
      /* res */
      xmlTextWriterStartElement(pWriter, BAD_CAST "res");
      
      std::stringstream sTmp;
      sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
      sTmp.str("");
        
      sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/Playlists/" << szObjId << "." << ExtractFileExt(pSQLResult->GetValue("PATH"));    
      //xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
   
//cout << sTmp.str() << endl;      
   
      xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
      xmlTextWriterEndElement(pWriter); 
    }      
    
    
  /* end container */
  xmlTextWriterEndElement(pWriter); 
}


void CContentDirectory::BuildItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, CUPnPBrowse* pUPnPBrowse, OBJECT_TYPE p_nObjectType, std::string p_sParentId)
{
  /* item */
  xmlTextWriterStartElement(pWriter, BAD_CAST "item");

    /* id */  
    char szObjId[11];         
    unsigned int nObjId = atoi(pSQLResult->GetValue("ID").c_str());
    sprintf(szObjId, "%010X", nObjId);   
    //cout << "ITEM ID: " << nObjId << endl;    
  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST szObjId); 
    /* parentID  */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST p_sParentId.c_str()); 
    /* restricted */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");    
  
    /* date */
    if(pUPnPBrowse->m_sFilter.find("dc:date") != std::string::npos)
    {
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST "2005-10-15");
      xmlTextWriterEndElement(pWriter);
    }
    
    /* writeStatus */
    if(pUPnPBrowse->m_sFilter.find("upnp:writeStatus") != std::string::npos)
    {
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
      xmlTextWriterEndElement(pWriter);
    }
    
    /* title */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    
    /* trim filename */
    string sFileName = TrimFileName(pSQLResult->GetValue("FILE_NAME"), CSharedConfig::Shared()->GetMaxFileNameLength(), true);    
    sFileName = TruncateFileExt(sFileName);
    
    // to utf 8
    /*unsigned char* szBuf = new unsigned char[4096];
    int nSize   = 4096;
    int nLength = pSQLResult->GetValue("FILE_NAME").length();
    
    memcpy(szBuf, pSQLResult->GetValue("FILE_NAME").c_str(), pSQLResult->GetValue("FILE_NAME").length());
    szBuf[pSQLResult->GetValue("FILE_NAME").length()] = '\0';
    
    cout << szBuf << endl; //pSQLResult->GetValue("FILE_NAME") << endl;
    cout << xmlCheckUTF8(szBuf) << endl;
    if(xmlCheckUTF8(szBuf))
      cout << "true" << endl;
    else
      cout << "false" << endl;
    
    if(!xmlCheckUTF8(szBuf))
    {
      isolat1ToUTF8(szBuf, &nSize, (const unsigned char*)pSQLResult->GetValue("FILE_NAME").c_str(), &nLength);
      szBuf[nSize] = '\0';
    }
    cout << szBuf << endl;
    xmlTextWriterWriteString(pWriter, BAD_CAST szBuf);*/    
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
    xmlTextWriterEndElement(pWriter);
    
    switch(p_nObjectType)
    {
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:
        BuildAudioItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      case ITEM_AUDIO_ITEM_AUDIO_BROADCAST:
        BuildAudioItemAudioBroadcastDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;      
      case ITEM_IMAGE_ITEM_PHOTO:
        BuildImageItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      case ITEM_VIDEO_ITEM_MOVIE:
        BuildVideoItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
      case ITEM_VIDEO_ITEM_VIDEO_BROADCAST:
        BuildVideoItemVideoBroadcastDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;      
      case CONTAINER_PLAYLIST_CONTAINER:
        BuildPlaylistItemDescription(pWriter, pSQLResult, pUPnPBrowse, szObjId);
        break;
    }           
  
  /* end item */
  xmlTextWriterEndElement(pWriter);
}
  

void CContentDirectory::BuildAudioItemDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowse*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{
  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.audioItem.musicTrack");
  xmlTextWriterEndElement(pWriter);

  /* creator */
  if(pUPnPBrowse->m_sFilter.find("dc:creator") != std::string::npos)
  {
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "creator", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "-Unknown-");
    xmlTextWriterEndElement(pWriter);
  }

  /* storageMedium */
  if(pUPnPBrowse->m_sFilter.find("upnp:storageMedium") != std::string::npos)
  {
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "storageMedium", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);    
  }
  
  
  xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->GetValue("DETAILS").c_str());   
  
  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  string sMimeType = pSQLResult->GetValue("MIME_TYPE");
  string sExt      = ExtractFileExt(pSQLResult->GetValue("PATH"));
  if(CFileDetails::Shared()->IsTranscodingExtension(sExt)) {
    sMimeType = CFileDetails::Shared()->GetMimeType(pSQLResult->GetValue("PATH"), true);
    sExt      = CFileDetails::Shared()->GetTargetExtension(sExt);
  }
  
  std::stringstream sTmp;
  sTmp << "http-get:*:" << sMimeType << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
  
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/AudioItems/" << p_sObjectID << "." << sExt;  
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter); 
  
}

void CContentDirectory::BuildAudioItemAudioBroadcastDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowse*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{
  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.audioItem.audioBroadcast");
  xmlTextWriterEndElement(pWriter);      
  
  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  std::stringstream sTmp;
  /*sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");*/
  
  sTmp << pSQLResult->GetValue("PATH");
  //"http://" << m_sHTTPServerURL << "/MediaServer/VideoItems/" << p_sObjectID << "." << ExtractFileExt(pSQLResult->GetValue("FILE_NAME"));    
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);   
}

void CContentDirectory::BuildImageItemDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowse*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{
  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.imageItem");
  xmlTextWriterEndElement(pWriter);

  /* storageMedium */
  if(pUPnPBrowse->m_sFilter.find("upnp:storageMedium") != std::string::npos) {  
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "storageMedium", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);
  }

/* longDescription upnp No 
   rating upnp No
   description dc No
   publisher dc No  
   rights dc No */  

  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  std::stringstream sTmp;
  sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
  
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/ImageItems/" << p_sObjectID << "." << ExtractFileExt(pSQLResult->GetValue("PATH"));
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
    
}

void CContentDirectory::BuildVideoItemDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowse*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{   
  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.videoItem.movie");
  xmlTextWriterEndElement(pWriter);      
  
  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  std::stringstream sTmp;
  sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");   

  /* set transcoding target extension 
     as video transcoding is not yet supported
     we just rename files e.g. vdr to vob */
  string sExt = ExtractFileExt(pSQLResult->GetValue("PATH"));
  if(CFileDetails::Shared()->IsTranscodingExtension(sExt)) {
    sExt = CFileDetails::Shared()->GetTargetExtension(sExt);
  }     
  
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/VideoItems/" << p_sObjectID << "." << sExt;  
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
}

void CContentDirectory::BuildVideoItemVideoBroadcastDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowse*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{   
  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.videoItem.videoBroadcast");
  xmlTextWriterEndElement(pWriter);      
  
  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  std::stringstream sTmp;
  /*sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");*/
  
  sTmp << pSQLResult->GetValue("PATH");
  //"http://" << m_sHTTPServerURL << "/MediaServer/VideoItems/" << p_sObjectID << "." << ExtractFileExt(pSQLResult->GetValue("FILE_NAME"));  
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
}

void CContentDirectory::BuildPlaylistItemDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowse*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{   
  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.playlistItem");
  xmlTextWriterEndElement(pWriter);      
  
  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  std::stringstream sTmp;
  sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
    
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/Playlists/" << p_sObjectID << "." << ExtractFileExt(pSQLResult->GetValue("PATH"));      
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
}

std::string CContentDirectory::HandleUPnPGetSearchCapabilities(CUPnPAction* pAction)
{
  return 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSearchCapabilitiesResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <SearchCaps></SearchCaps>"
    "    </u:GetSearchCapabilitiesResponse>"
    "  </s:Body>"
    "</s:Envelope>";

  //<SearchCaps>dc:title,dc:creator,upnp:artist,upnp:genre,upnp:album,dc:date,upnp:originalTrackNumber,upnp:class,@id,@refID,upnp:albumArtURI</SearchCaps>    
} 

std::string CContentDirectory::HandleUPnPGetSortCapabilities(CUPnPAction* pAction)
{
  return 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSortCapabilitiesResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <SortCaps></SortCaps>"
    "    </u:GetSortCapabilitiesResponse>"
    "  </s:Body>"
    "</s:Envelope>";

  // <SortCaps>dc:title,dc:creator,upnp:artist,upnp:genre,upnp:album,dc:date,upnp:originalTrackNumber,Philips:shuffle</SortCaps>
}

std::string CContentDirectory::HandleUPnPGetSystemUpdateID(CUPnPAction* pAction)
{
  return 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <u:GetSystemUpdateIDResponse xmlns:u=\"urn:schemas-upnp-org:service:ContentDirectory:1\">"
    "      <Id>1</Id>"
    "    </u:GetSystemUpdateIDResponse>"
    "  </s:Body>"
    "</s:Envelope>";            
}


std::string CContentDirectory::HandleUPnPSearch(CUPnPSearch* pSearch)
{
  cout << __FILE__ << __LINE__ << " " << pSearch->BuildSQL() << endl;

  return "";
}

