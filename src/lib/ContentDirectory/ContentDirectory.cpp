/***************************************************************************
 *            ContentDirectory.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <fuppes@ulrich-voelkel.de>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
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
#include "VirtualContainerMgr.h"
 
#include <iostream>
#include <sstream>
#include <libxml/xmlwriter.h>

using namespace std;

const string LOGNAME = "ContentDir";

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
  string sContent = "";
  
  //cout << "CDIR HANDLE ACTION " << endl; fflush(stdout);
  
  switch((UPNP_CONTENT_DIRECTORY_ACTIONS)pUPnPAction->GetActionType())
  {
    // Browse
    case UPNP_BROWSE:
      sContent = DbHandleUPnPBrowse((CUPnPBrowse*)pUPnPAction);
      break;
		// Search
		case UPNP_SEARCH:
		  sContent = HandleUPnPSearch((CUPnPSearch*)pUPnPAction);
			break;
    // GetSearchCapabilities
    case UPNP_GET_SEARCH_CAPABILITIES:
      sContent = HandleUPnPGetSearchCapabilities(pUPnPAction);
      break;      
    // GetSortCapabilities
    case UPNP_GET_SORT_CAPABILITIES:
      sContent = HandleUPnPGetSortCapabilities(pUPnPAction);
      break;
    // GetSystemUpdateID  
    case UPNP_GET_SYSTEM_UPDATE_ID:
      sContent = HandleUPnPGetSystemUpdateID(pUPnPAction);
      break;
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
  
  //cout << "CDIR ACTION HANDLED" << endl; fflush(stdout);
}

  

/* HandleUPnPBrowse */
std::string CContentDirectory::DbHandleUPnPBrowse(CUPnPBrowse* pUPnPBrowse)
{ 
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
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
  
      xmlTextWriterStartElementNS(resWriter, NULL, BAD_CAST "DIDL-Lite", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite");
          
      unsigned int nNumberReturned = 0;
      unsigned int nTotalMatches   = 0;
         
      // switch browse flag
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
        default:
          break;
      }
              
      // finalize result xml
      xmlTextWriterEndElement(resWriter);
      xmlTextWriterEndDocument(resWriter);
      xmlFreeTextWriter(resWriter);

      std::stringstream sResOutput;
      sResOutput << (const char*)resBuf->content;

      xmlBufferFree(resBuf);        
      string sTmpRes = sResOutput.str().substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> "));
      xmlTextWriterWriteString(writer, BAD_CAST sTmpRes.c_str());
      
      // end result
      xmlTextWriterEndElement(writer);
        
      // number returned
      xmlTextWriterStartElement(writer, BAD_CAST "NumberReturned");
      sTmp << nNumberReturned;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
      xmlTextWriterEndElement(writer);
      
      // total matches
      xmlTextWriterStartElement(writer, BAD_CAST "TotalMatches");
      sTmp << nTotalMatches;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
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
	
	std::stringstream output;
	output << (const char*)buf->content;  
	xmlBufferFree(buf);  
  
  CSharedLog::Shared()->Log(L_DEBUG, output.str(), __FILE__, __LINE__);
	return output.str();  
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
  CContentDatabase* pDb = new CContentDatabase();

  pUPnPBrowse->m_bVirtualContainer = 
        CVirtualContainerMgr::Shared()->IsVirtualContainer(pUPnPBrowse->GetObjectIDAsInt(),
                                                           pUPnPBrowse->GetDeviceSettings()->m_sVirtualFolderDevice);
  
  string sDevice = "DEVICE is NULL ";
  if(pUPnPBrowse->m_bVirtualContainer)
    sDevice = "DEVICE = '" + pUPnPBrowse->GetDeviceSettings()->m_sVirtualFolderDevice + "' ";
                        
  // get container type
  OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
  if(pUPnPBrowse->GetObjectIDAsInt() > 0) {
    
    sSql << "select TYPE from OBJECTS " <<
      "where OBJECT_ID = " << pUPnPBrowse->GetObjectIDAsInt() << " and " << sDevice;

    pDb->Select(sSql.str());        
    nContainerType = (OBJECT_TYPE)atoi(pDb->GetResult()->GetValue("TYPE").c_str());
    //pDb->ClearResult(); 
    sSql.str("");
  }

  // get child count
  bool bNeedCount = false;
  if(nContainerType < ITEM) {
    sSql <<
      "select count(*) as COUNT " <<
      "from MAP_OBJECTS " <<
      "where PARENT_ID = " << pUPnPBrowse->GetObjectIDAsInt() << " and " << sDevice;
    
    bNeedCount = true;
  }

  string sChildCount = "0";
  if(bNeedCount) {
    pDb->Select(sSql.str());        
    sChildCount = pDb->GetResult()->GetValue("COUNT").c_str();
    //pDb->ClearResult();    
    sSql.str("");
  }
  
  string sParentId;
  
  // root folder
  if(pUPnPBrowse->GetObjectIDAsInt() == 0)
  {                                  
    sParentId = "-1";
    
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
      xmlTextWriterWriteString(pWriter, BAD_CAST "root"); 
      xmlTextWriterEndElement(pWriter);
     
      /* class */
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");     
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
      "  o.PATH, o.FILE_NAME, o.TYPE, o.MIME_TYPE, " <<
      "  d.* " <<
      "from " <<
      "  OBJECTS o, MAP_OBJECTS m " <<
      "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
      "where " <<
      "  m.OBJECT_ID = o.OBJECT_ID and " <<
      "  o.OBJECT_ID = " << pUPnPBrowse->GetObjectIDAsInt() << " and " <<
      "  m." << sDevice;

    pDb->Select(sSql.str());
    sSql.str("");
    
    CSelectResult* pRow = pDb->GetResult();        
    //sTitle = pRow->GetValue("FILE_NAME");


    char szParentId[11];
    unsigned int nParentId = pRow->GetValueAsUInt("PARENT_ID");
    
    if(nParentId > 0) {
      sprintf(szParentId, "%010X", nParentId);
      sParentId = szParentId;
    }
    else {
      sParentId = "0";
    } 
    
    BuildDescription(pWriter, pRow, pUPnPBrowse, sParentId);
  }
  
  delete pDb;
}


void CContentDirectory::BrowseDirectChildren(xmlTextWriterPtr pWriter, 
                          unsigned int* p_pnTotalMatches,
                          unsigned int* p_pnNumberReturned,
                          CUPnPBrowse*  pUPnPBrowse)
{ 
  std::stringstream sSql;
  CContentDatabase* pDb = new CContentDatabase();
  //OBJECT_TYPE nContainerType = CONTAINER_STORAGE_FOLDER;
 
                            
  pUPnPBrowse->m_bVirtualContainer = CVirtualContainerMgr::Shared()->HasVirtualChildren(pUPnPBrowse->GetObjectIDAsInt(), pUPnPBrowse->GetDeviceSettings()->m_sVirtualFolderDevice);	
 
  string sDevice = "DEVICE is NULL ";
  if(pUPnPBrowse->m_bVirtualContainer)
    sDevice = "DEVICE = '" + pUPnPBrowse->GetDeviceSettings()->m_sVirtualFolderDevice + "' ";
                            
  // get total matches
  //cout << "get total matches" << endl; fflush(stdout);
  sSql << "select count(*) as COUNT from MAP_OBJECTS where PARENT_ID = " <<
          pUPnPBrowse->GetObjectIDAsInt() << " and " << sDevice;                          
  
  pDb->Select(sSql.str()); 
  if(!pDb->Eof()) {
    //cout << "COUNT: " << pDb->GetResult()->GetValue("COUNT") << endl; fflush(stdout);
    *p_pnTotalMatches = atoi(pDb->GetResult()->GetValue("COUNT").c_str());
  }
  else
    *p_pnTotalMatches = 0;

  //pDb->ClearResult();
  
  sSql.str("");
  //cout << "DONE get total matches " << *p_pnTotalMatches << endl; fflush(stdout);  
  
  // get description
  sSql << 
    "select " <<
    "  o.OBJECT_ID, o.TYPE, o.PATH, o.FILE_NAME, o.MIME_TYPE, " <<
    "  d.IV_HEIGHT, d.IV_WIDTH, " <<
    "  o.TITLE, d.AV_DURATION, d.A_ALBUM, d.A_ARTIST, d.A_GENRE, " <<
  	"  d.A_TRACK_NO, d.AV_BITRATE, d.A_SAMPLERATE, d.A_CHANNELS, " <<
	  "  d.AV_DURATION, d.SIZE " <<
    "from " <<
    "  OBJECTS o, MAP_OBJECTS m" <<
    "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  o.OBJECT_ID = m.OBJECT_ID and " <<
    "  m.PARENT_ID = " << pUPnPBrowse->GetObjectIDAsInt() << " and " << 
    "  m." << sDevice <<
    "order by " <<
    "  o.TYPE, o.FILE_NAME ";
  
  
  if((pUPnPBrowse->m_nRequestedCount > 0) || (pUPnPBrowse->m_nStartingIndex > 0))  {
    
    sSql << " limit " << pUPnPBrowse->m_nStartingIndex << ", ";
    if(pUPnPBrowse->m_nRequestedCount == 0)
      sSql << "-1";
    else
      sSql << pUPnPBrowse->m_nRequestedCount;
  }
  
  unsigned int tmpInt = *p_pnNumberReturned;
  CSelectResult* pRow = NULL;
                            
  pDb->Select(sSql.str());
  while(!pDb->Eof()) {
    
    pRow = pDb->GetResult();    
    BuildDescription(pWriter, pRow, pUPnPBrowse, pUPnPBrowse->m_sObjectID);
        
    pDb->Next();
    tmpInt++;
  }        
  
  //pDb->ClearResult();                    
  *p_pnNumberReturned = tmpInt;
  
  delete pDb;
}

void CContentDirectory::BuildDescription(xmlTextWriterPtr pWriter,
                                         CSelectResult* pSQLResult,
                                         CUPnPBrowseSearchBase* pUPnPBrowse,
                                         std::string p_sParentId)
{
  OBJECT_TYPE nObjType = (OBJECT_TYPE)atoi(pSQLResult->GetValue("TYPE").c_str());
  
  // container
  if(nObjType < ITEM) {
    
    if((nObjType == CONTAINER_PLAYLIST_CONTAINER) && !pUPnPBrowse->GetDeviceSettings()->m_bShowPlaylistAsContainer) {
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
                                                  CSelectResult* pSQLResult, 
                                                  CUPnPBrowseSearchBase* pUPnPBrowse, 
                                                  std::string p_sParentId,
                                                  OBJECT_TYPE p_nContainerType)
{
  
  // get child count
  string sChildCount = "0";
  CContentDatabase* pDb = new CContentDatabase();
  stringstream sSql;
  
  string sDevice = "DEVICE is NULL ";
  if(CVirtualContainerMgr::Shared()->HasVirtualChildren(pSQLResult->GetValueAsUInt("OBJECT_ID"), pUPnPBrowse->GetDeviceSettings()->m_sVirtualFolderDevice))
    sDevice = "DEVICE = '" + pUPnPBrowse->GetDeviceSettings()->m_sVirtualFolderDevice + "' ";                                                    

  sSql << "select count(*) as COUNT from MAP_OBJECTS " <<
    "where PARENT_ID = " << pSQLResult->GetValue("OBJECT_ID") << " and " << sDevice;
  
  pDb->Select(sSql.str());
  sChildCount = pDb->GetResult()->GetValue("COUNT");
  delete pDb;
  
  
  // container
  xmlTextWriterStartElement(pWriter, BAD_CAST "container");   

    // id
    char szObjId[11];    
    unsigned int nObjId = pSQLResult->GetValueAsUInt("OBJECT_ID");
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
    string sTitle = pSQLResult->GetValue("FILE_NAME");
    int nLen = pUPnPBrowse->GetDeviceSettings()->m_nMaxFileNameLength;
    if(nLen > 0 && pUPnPBrowse->GetDeviceSettings()->m_DisplaySettings.bShowChildCountInTitle) {
      nLen -= (sChildCount.length() + 3); // "_(n)"
    }
    sTitle = TrimFileName(sTitle, nLen);
    if(pUPnPBrowse->GetDeviceSettings()->m_DisplaySettings.bShowChildCountInTitle) {
      sTitle = sTitle + " (" + sChildCount + ")";
    }
                                                      
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");     
      xmlTextWriterWriteString(pWriter, BAD_CAST sTitle.c_str());    
    xmlTextWriterEndElement(pWriter);
   
    // class
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");       
    xmlTextWriterWriteString(pWriter, BAD_CAST CFileDetails::Shared()->GetObjectTypeAsString(p_nContainerType).c_str());  
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
      
      std::stringstream sTmp;
      sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
      sTmp.str("");
        
      sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/Playlists/" << szObjId << "." << ExtractFileExt(pSQLResult->GetValue("PATH"));    
      //xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());   
   
      xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
      xmlTextWriterEndElement(pWriter); 
    }      
    
    
  // end container
  xmlTextWriterEndElement(pWriter); 
}


void CContentDirectory::BuildItemDescription(xmlTextWriterPtr pWriter, 
                                             CSelectResult* pSQLResult, 
                                             CUPnPBrowseSearchBase* pUPnPBrowse, 
                                             OBJECT_TYPE p_nObjectType, 
                                             std::string p_sParentId)
{                                            
  /* item */
  xmlTextWriterStartElement(pWriter, BAD_CAST "item");

    /* id */  
    char szObjId[11];         
    unsigned int nObjId = pSQLResult->GetValueAsUInt("OBJECT_ID");
    sprintf(szObjId, "%010X", nObjId);   
    //cout << "ITEM ID: " << nObjId << endl;    
  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "id", BAD_CAST szObjId); 
    /* parentID  */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "parentID", BAD_CAST p_sParentId.c_str()); 
    /* restricted */
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "restricted", BAD_CAST "0");    
  
    // date
    /*if(pUPnPBrowse->IncludeProperty("dc:date")) {
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST "2005-10-15");
      xmlTextWriterEndElement(pWriter);
    }*/
    
    // writeStatus (optional)
    /*if(pUPnPBrowse->IncludeProperty("upnp:writeStatus")) {
      xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
      xmlTextWriterEndElement(pWriter);
    }*/
    
    // title
    /*xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");
      // trim filename
      string sFileName = TrimFileName(pSQLResult->GetValue("FILE_NAME"), CSharedConfig::Shared()->GetMaxFileNameLength(), true);    
      sFileName = TruncateFileExt(sFileName);
      xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
    xmlTextWriterEndElement(pWriter);*/
    
    
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
      
      default:
        break;
    }           
  
  /* end item */
  xmlTextWriterEndElement(pWriter);
  
  //cout << "BUILD ITEM DESCRIPT DONE" << endl;
  fflush(stdout);
}
  

void CContentDirectory::BuildAudioItemDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{                                              
  // title
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");
    // trim filename
    string sFileName = pSQLResult->GetValue("FILE_NAME");
		if(!pSQLResult->IsNull("TITLE"))
		  sFileName = pSQLResult->GetValue("TITLE");
			
	  sFileName = TrimFileName(sFileName, pUPnPBrowse->GetDeviceSettings()->m_nMaxFileNameLength, true);    
    sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);
	
  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.audioItem.musicTrack");
  xmlTextWriterEndElement(pWriter);

  // creator
  /*if(pUPnPBrowse->IncludeProperty("dc:creator")) {
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "creator", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "-Unknown-");
    xmlTextWriterEndElement(pWriter);
  }*/

  // storageMedium
  /*if(pUPnPBrowse->IncludeProperty("upnp:storageMedium")) {
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "storageMedium", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);    
  }*/
  
	if(pUPnPBrowse->IncludeProperty("upnp:artist") && !pSQLResult->IsNull("A_ARTIST")) {
  	xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "artist", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->GetValue("A_ARTIST").c_str());
  	xmlTextWriterEndElement(pWriter); 
  }
	
	if(pUPnPBrowse->IncludeProperty("upnp:album") && !pSQLResult->IsNull("A_ALBUM")) {
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "album", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->GetValue("A_ALBUM").c_str());
    xmlTextWriterEndElement(pWriter);
  }

  if(pUPnPBrowse->IncludeProperty("upnp:genre") && !pSQLResult->IsNull("A_GENRE")) {
  	xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "genre", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->GetValue("A_GENRE").c_str());
  	xmlTextWriterEndElement(pWriter);
  }
  
  if(pUPnPBrowse->IncludeProperty("upnp:originalTrackNumber") && !pSQLResult->IsNull("A_TRACK_NO")) {	
	  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "originalTrackNumber", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
      xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->GetValue("A_TRACK_NO").c_str());
	  xmlTextWriterEndElement(pWriter);
  }
	
  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  string sMimeType = pSQLResult->GetValue("MIME_TYPE");
  string sExt      = ExtractFileExt(pSQLResult->GetValue("PATH"));
  if(CFileDetails::Shared()->IsTranscodingExtension(sExt)) {
    sMimeType = CFileDetails::Shared()->GetMimeType(pSQLResult->GetValue("PATH"), true);
    sExt      = CFileDetails::Shared()->GetTargetExtension(sExt);
  }
  
  // protocol info
  std::stringstream sTmp;
  sTmp << "http-get:*:" << sMimeType << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");

  // duration
  if(pUPnPBrowse->IncludeProperty("res@duration") && !pSQLResult->IsNull("AV_DURATION")) {	
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "duration", BAD_CAST pSQLResult->GetValue("AV_DURATION").c_str());
  }
	
	// nrAudioChannels 
  if(pUPnPBrowse->IncludeProperty("res@nrAudioChannels") && !pSQLResult->IsNull("A_CHANNELS")) {		
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "nrAudioChannels", BAD_CAST pSQLResult->GetValue("A_CHANNELS").c_str());
  }

  // sampleFrequency
  if(pUPnPBrowse->IncludeProperty("res@sampleFrequency") && !pSQLResult->IsNull("A_SAMPLERATE")) {		  
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "sampleFrequency", BAD_CAST pSQLResult->GetValue("A_SAMPLERATE").c_str());
  }

	// bitrate
  if(pUPnPBrowse->IncludeProperty("res@bitrate") && !pSQLResult->IsNull("AV_BITRATE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "bitrate", BAD_CAST pSQLResult->GetValue("AV_BITRATE").c_str());
  }

  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/AudioItems/" << p_sObjectID << "." << sExt;  
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
}

void CContentDirectory::BuildAudioItemAudioBroadcastDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{
  // title
	xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");
    // trim filename
    string sFileName = TrimFileName(pSQLResult->GetValue("FILE_NAME"), pUPnPBrowse->GetDeviceSettings()->m_nMaxFileNameLength, true);    
    sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);

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
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{
  // title
	xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");
    // trim filename
    string sFileName = TrimFileName(pSQLResult->GetValue("FILE_NAME"), pUPnPBrowse->GetDeviceSettings()->m_nMaxFileNameLength, true);    
    sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);

  /* class */
  xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "class", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
  xmlTextWriterWriteString(pWriter, BAD_CAST "object.item.imageItem.photo");
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
   rights dc No */  

  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  // protocol info
  std::stringstream sTmp;
  sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
  
  // resolution
	if(pUPnPBrowse->IncludeProperty("res@resolution") &&
     (pSQLResult->GetValue("IV_WIDTH").compare("NULL") != 0) && 
     (pSQLResult->GetValue("IV_HEIGHT").compare("NULL") != 0)) {
    sTmp << pSQLResult->GetValue("IV_WIDTH") << "x" << pSQLResult->GetValue("IV_HEIGHT");
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST sTmp.str().c_str());
    sTmp.str("");
	}
  
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/ImageItems/" << p_sObjectID << "." << ExtractFileExt(pSQLResult->GetValue("PATH"));
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
    
}

void CContentDirectory::BuildVideoItemDescription(xmlTextWriterPtr pWriter,
                                                  CSelectResult* pSQLResult,
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{   
  // title
	xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");
    // trim filename
    string sFileName = TrimFileName(pSQLResult->GetValue("FILE_NAME"), pUPnPBrowse->GetDeviceSettings()->m_nMaxFileNameLength, true);    
    sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);

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


  // duration
  if(pUPnPBrowse->IncludeProperty("res@duration") && !pSQLResult->IsNull("AV_DURATION")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "duration", BAD_CAST pSQLResult->GetValue("AV_DURATION").c_str());
  }
      
	// resolution 
	if(pUPnPBrowse->IncludeProperty("res@resolution") && !pSQLResult->IsNull("IV_WIDTH") && !pSQLResult->IsNull("IV_HEIGHT")) {
    sTmp << pSQLResult->GetValue("IV_WIDTH") << "x" << pSQLResult->GetValue("IV_HEIGHT");
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "resolution", BAD_CAST sTmp.str().c_str());
    sTmp.str("");
	}

	// bitrate
  if(pUPnPBrowse->IncludeProperty("res@bitrate") && !pSQLResult->IsNull("AV_BITRATE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "bitrate", BAD_CAST pSQLResult->GetValue("AV_BITRATE").c_str());
  }
      
  // size
  if(pUPnPBrowse->IncludeProperty("res@size") && !pSQLResult->IsNull("SIZE")) {
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "size", BAD_CAST pSQLResult->GetValue("SIZE").c_str());
  }

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
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{ 
  // title
	xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");
    // trim filename
    string sFileName = TrimFileName(pSQLResult->GetValue("FILE_NAME"), pUPnPBrowse->GetDeviceSettings()->m_nMaxFileNameLength, true);    
    sFileName = TruncateFileExt(sFileName);
    xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);
  
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
                                                  CUPnPBrowseSearchBase*  pUPnPBrowse,
                                                  std::string p_sObjectID)
{   
  // title
	xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");
		// trim filename
		string sFileName = TrimFileName(pSQLResult->GetValue("FILE_NAME"), pUPnPBrowse->GetDeviceSettings()->m_nMaxFileNameLength, true);    
		sFileName = TruncateFileExt(sFileName);
		xmlTextWriterWriteString(pWriter, BAD_CAST sFileName.c_str());    
	xmlTextWriterEndElement(pWriter);  
	
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
    "      <SearchCaps>dc:title,upnp:class,upnp:artist</SearchCaps>"
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
	CContentDatabase* pDb  = new CContentDatabase();
	CSelectResult*    pRow = NULL;
  stringstream      sSql;
  int nTotalMatches = 0;
  
  // get total matches     
  sSql << pSearch->BuildSQL(true);
  //cout << "SEARCH GET TOTAL MATCHES: " << endl << sSql.str() << endl << endl;
  pDb->Select(sSql.str());
  if(!pDb->Eof()) {
    nTotalMatches = atoi(pDb->GetResult()->GetValue("COUNT").c_str());
  }
  sSql.str("");                                                                              
     
  // get items     
	int nNumberReturned = 0;
  sSql << pSearch->BuildSQL(false);
  //cout << "SEARCH GET ITEMS: " << endl << sSql.str() << endl << endl;
	pDb->Select(sSql.str());
	
	
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
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
  
      xmlTextWriterStartElementNS(resWriter, NULL, BAD_CAST "DIDL-Lite", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/DIDL-Lite");
          
  
      while(!pDb->Eof()) {
        pRow = pDb->GetResult();
        //cout << "result " << nNumberReturned++ << ": " << pRow->GetValue("TITLE") << endl;
		
        BuildDescription(resWriter, pRow, pSearch, "0");
		
        pDb->Next();
      }
			

              
      // finalize result xml
      xmlTextWriterEndElement(resWriter);
      xmlTextWriterEndDocument(resWriter);
      xmlFreeTextWriter(resWriter);

      std::stringstream sResOutput;
      sResOutput << (const char*)resBuf->content;

      xmlBufferFree(resBuf);        
      string sTmpRes = sResOutput.str().substr(strlen("<?xml version=\"1.0\" encoding=\"UTF-8\"?> "));
      xmlTextWriterWriteString(writer, BAD_CAST sTmpRes.c_str());
      
      // end result
      xmlTextWriterEndElement(writer);
        
      // number returned
      xmlTextWriterStartElement(writer, BAD_CAST "NumberReturned");
      sTmp << nNumberReturned;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
      xmlTextWriterEndElement(writer);
      
      // total matches
      xmlTextWriterStartElement(writer, BAD_CAST "TotalMatches");
      sTmp << nTotalMatches;
      xmlTextWriterWriteString(writer, BAD_CAST sTmp.str().c_str());
      sTmp.str("");
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
	
	std::stringstream output;
	output << (const char*)buf->content;	
  CSharedLog::Shared()->DebugLog(LOGNAME, output.str());
  
	xmlBufferFree(buf);
  
	delete pDb;
	
  //cout << output.str() << endl;
  
	return output.str();
}

