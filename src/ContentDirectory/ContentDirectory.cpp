/***************************************************************************
 *            ContentDirectory.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/
 
#include "ContentDirectory.h" 
#include "ContentDirectoryDescription.cpp" 
#include "UPnPItem.h"
#include "AudioItem.h"
#include "ImageItem.h"
#include "VideoItem.h"
#include "../UPnPActions/UPnPBrowse.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Common.h"
#include "../RegEx.h"
#include "FileDetails.h"
#include "UPnPObjectFactory.h"
 
#include <iostream>
#include <sstream>
#include <libxml/xmlwriter.h>
#include <cstdio>
#ifndef WIN32
#include <dirent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h> 
using namespace std;
 
/*===============================================================================
 CONSTANTS
===============================================================================*/

const string LOGNAME = "ContentDir";
 
/*===============================================================================
 CLASS CContentDirectory
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

/* constructor */
CContentDirectory::CContentDirectory(std::string p_sHTTPServerURL):
CUPnPService(UPNP_DEVICE_TYPE_CONTENT_DIRECTORY, p_sHTTPServerURL)
{
  /* Init database */   
  bool bIsNewDB = false; 
  CContentDatabase::Shared()->Init(&bIsNewDB);
  
  if(bIsNewDB)
    BuildDB();
}

/* destructor */
CContentDirectory::~CContentDirectory()
{
}

std::string CContentDirectory::GetServiceDescription()
{
  return sContentDirectoryDescription;
}

void CContentDirectory::BuildDB()
{
  CSharedLog::Shared()->Log(LOGNAME, "creating content database. this may take a while.");
  CContentDatabase::Shared()->Insert("delete from objects");
  
  for(unsigned int i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++)
  {
    if(DirectoryExists(CSharedConfig::Shared()->GetSharedDir(i)))
    {  
      if(CSharedConfig::Shared()->GetDisplaySettings().bShowDirNamesInFirstLevel)
      {      
        string sFileName;
        ExtractFolderFromPath(CSharedConfig::Shared()->GetSharedDir(i), &sFileName);          
        
        stringstream sSql;
        sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME) values ";
        sSql << "(" << CONTAINER_STORAGE_FOLDER << ", ";
        sSql << 0 << ", ";
        sSql << "'" << CSharedConfig::Shared()->GetSharedDir(i) << "', ";
        sSql << "'" << sFileName << "');";
        
        CContentDatabase::Shared()->Lock();
        long long int nRowId = CContentDatabase::Shared()->Insert(sSql.str());
        CContentDatabase::Shared()->Unlock();
        DbScanDir(CSharedConfig::Shared()->GetSharedDir(i), nRowId);
      }
      else
      {
        DbScanDir(CSharedConfig::Shared()->GetSharedDir(i), 0);        
      }
      
    }
    else
    {
      stringstream sLog;
      sLog << "shared directory: \"" << CSharedConfig::Shared()->GetSharedDir(i) << "\" not found";
      CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    }
  } // for
  
  CSharedLog::Shared()->Log(LOGNAME, "content database created");   
}

/*===============================================================================
 UPNP ACTION HANDLING
===============================================================================*/
 
/* HandleUPnPAction */
bool CContentDirectory::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
  BOOL_CHK_RET_POINTER(pUPnPAction);
  BOOL_CHK_RET_POINTER(pMessageOut);
  
  string sContent = "";
  
  switch(pUPnPAction->GetActionType())
  {
    /* Handle UPnP browse */
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_BROWSE:
      //cout << pUPnPAction->m_sMessage<< endl;       
      sContent = DbHandleUPnPBrowse((CUPnPBrowse*)pUPnPAction);      
      //cout << sContent << endl; 
      break;
      
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SEARCH_CAPABILITIES:
      sContent = HandleUPnPGetSearchCapabilities(pUPnPAction);
      break;
      
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SORT_CAPABILITIES:
      sContent = HandleUPnPGetSortCapabilities(pUPnPAction);
      break;
      
    case UPNP_ACTION_TYPE_CONTENT_DIRECTORY_GET_SYSTEM_UPDATE_ID:
      sContent = HandleUPnPGetSystemUpdateID(pUPnPAction);
      break;
  }
  
  if(!sContent.empty())
  {
    /* Set a message for the incoming action */
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
    //cout << pMessageOut->GetContent() << endl;
  }
  return true;
}

/*===============================================================================
 GET
===============================================================================*/

/* GetItemFromObjectID */
CUPnPObject* CContentDirectory::GetItemFromObjectID(std::string p_sObjectID)
{
  CUPnPObjectFactory* pFact = new CUPnPObjectFactory(m_sHTTPServerURL);    
  CUPnPObject* pResult = pFact->CreateObjectFromId(p_sObjectID);  
  delete pFact;
  return pResult;  
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

void CContentDirectory::DbScanDir(std::string p_sDirectory, long long int p_nParentId)
{
  #ifdef WIN32  
  /* Add slash, if neccessary */
  char szTemp[MAX_PATH];
  if(p_sDirectory.substr(p_sDirectory.length()-1).compare(upnpPathDelim) != 0)
  {
    strcpy(szTemp, p_sDirectory.c_str());
    strcat(szTemp, upnpPathDelim);
  }
  
  /* Add search criteria */
  strcat(szTemp, "*");
  
  /* Find first file */
  WIN32_FIND_DATA data;
  HANDLE hFile = FindFirstFile(szTemp, &data);
  if(NULL == hFile)
    return;

  /* Loop trough all subdirectories and files */
  while(TRUE == FindNextFile(hFile, &data))
  {
    if(((string(".").compare(data.cFileName) != 0) && 
      (string("..").compare(data.cFileName) != 0)))
    {        
      
      /* Save current filename */
      strcpy(szTemp, p_sDirectory.c_str());
      strcat(szTemp, upnpPathDelim);
      strcat(szTemp, data.cFileName);
      
      stringstream sTmp;
      sTmp << szTemp;
      
      string sTmpFileName = data.cFileName;
  #else
      
  DIR*    pDir;
  dirent* pDirEnt;
  stringstream sTmp;
   
  /* append upnpPathDelim if necessary */  
  if(p_sDirectory.substr(p_sDirectory.length()-1).compare(upnpPathDelim) != 0)
  {
    sTmp << p_sDirectory << upnpPathDelim;
    p_sDirectory = sTmp.str();
    sTmp.str("");
  }
  
  if((pDir = opendir(p_sDirectory.c_str())) != NULL)
  {
    sTmp << "read directory: " << p_sDirectory;    
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sTmp.str());
    sTmp.str("");
    
    while((pDirEnt = readdir(pDir)))
    {
      if(((string(".").compare(pDirEnt->d_name) != 0) && 
         (string("..").compare(pDirEnt->d_name) != 0)))
      {        
        sTmp << p_sDirectory << pDirEnt->d_name;        
        string sTmpFileName = pDirEnt->d_name;        
  #endif  
        string sExt = ExtractFileExt(sTmp.str());
        
        /* directory */
        if(IsDirectory(sTmp.str()))
        {          
          stringstream sSql;
          sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME) values ";
          sSql << "(" << CONTAINER_STORAGE_FOLDER << ", ";
          sSql << p_nParentId << ", ";
          sSql << "'" << SQLEscape(sTmp.str()) << "', ";
          sSql << "'" << SQLEscape(sTmpFileName) << "');";
        
          CContentDatabase::Shared()->Lock();
          long long int nRowId = CContentDatabase::Shared()->Insert(sSql.str());
          CContentDatabase::Shared()->Unlock();
          if(nRowId == -1)
            cout << "ERROR: " << sSql.str() << endl;
          DbScanDir(sTmp.str(), nRowId);          
        }
        else if(IsFile(sTmp.str()) && CSharedConfig::Shared()->IsSupportedFileExtension(sExt))
        {
          OBJECT_TYPE nObjectType = CFileDetails::Shared()->GetObjectType(sTmp.str());
          /*cout << "Parent: " << p_nParentId << endl;
          cout << "FileName: " << sTmpFileName << endl;
          cout << "Path: " << sTmp.str() << endl; 
          cout << "Type: " << nObjectType << endl;*/
          
          /* todo: build file description          
          switch(nObjectType)
          {
            case ITEM_AUDIO_ITEM_MUSIC_TRACK:
              cout << "MusicTrack" << endl;
              SMusicTrack TrackInfo = CFileDetails::Shared()->GetMusicTrackDetails(sTmp.str());
              break;
          }*/      
          if(nObjectType == OBJECT_TYPE_UNKNOWN)
            break;          
          
          stringstream sSql;
          sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME, MD5, MIME_TYPE, DETAILS) values ";
          sSql << "(" << nObjectType << ", ";
          sSql << p_nParentId << ", ";
          sSql << "'" << SQLEscape(sTmp.str()) << "', ";
          sSql << "'" << SQLEscape(sTmpFileName) << "', ";
          //sSql << "'" << MD5Sum(sTmp.str()) << "', ";
          sSql << "'" << "todo" << "', ";
          sSql << "'" << CFileDetails::Shared()->GetMimeType(sTmp.str()) << "', ";
          sSql << "'" << "details - todo" << "');";
          
          //cout << sSql.str() << endl;
          
          CContentDatabase::Shared()->Lock();
          long long int nRowId = CContentDatabase::Shared()->Insert(sSql.str());          
          CContentDatabase::Shared()->Unlock();
          if(nRowId == -1)
            cout << "ERROR: " << sSql.str() << endl;
          //DbScanDir(sTmp.str(), nRowId);          
        }   
        
        sTmp.str("");
      }
    }  /* while */  
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif         
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
          BrowseMetadata(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);
          break;
        case UPNP_BROWSE_FLAG_DIRECT_CHILDREN:
          BrowseDirectChildren(resWriter, &nTotalMatches, &nNumberReturned, pUPnPBrowse);
          break;        
      }   
  
      cout << "start idx: " << pUPnPBrowse->m_nStartingIndex << endl;
      cout << "request: " << pUPnPBrowse->m_nRequestedCount << endl;
      cout << "return: " << nNumberReturned << endl;

        
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
  
  /* get child count */  
  stringstream sSql;
  sSql << "select count(*) as COUNT from OBJECTS where PARENT_ID = " << pUPnPBrowse->GetObjectIDAsInt();
  CContentDatabase::Shared()->Lock();
  CContentDatabase::Shared()->Select(sSql.str());        
  string sChildCount = CContentDatabase::Shared()->GetResult()->GetValue("COUNT").c_str();
  CContentDatabase::Shared()->Unlock();
  sSql.str("");  

  string sParentId;
  string sTitle;
  
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
  else
  {
    sSql << "select ID, PARENT_ID, FILE_NAME, TYPE, MIME_TYPE from OBJECTS where ID = " << pUPnPBrowse->GetObjectIDAsInt();
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
    
    CContentDatabase::Shared()->Unlock();
    sSql.str("");      
    
    BuildDescription(pWriter, pRow, sParentId, sChildCount);
  }

}


void CContentDirectory::BrowseDirectChildren(xmlTextWriterPtr pWriter, 
                          unsigned int* p_pnTotalMatches,
                          unsigned int* p_pnNumberReturned,
                          CUPnPBrowse*  pUPnPBrowse)
{   
  /* get total matches */         
  std::stringstream sSql;
  sSql << "select count(*) as COUNT from OBJECTS where PARENT_ID = ";
  sSql << pUPnPBrowse->GetObjectIDAsInt(); // << " order by FILE_NAME ";        
  CContentDatabase::Shared()->Lock();
  CContentDatabase::Shared()->Select(sSql.str());        
  *p_pnTotalMatches  = atoi(CContentDatabase::Shared()->GetResult()->GetValue("COUNT").c_str());
  //string sChildCount = CContentDatabase::Shared()->GetResult()->GetValue("COUNT");
  CContentDatabase::Shared()->Unlock();
  sSql.str("");
  
  /* get description */
  sSql << "select o.ID, o.TYPE, o.PATH, o.FILE_NAME, o.MIME_TYPE, o.DETAILS, (select count(*) ";
  sSql << "from OBJECTS p where p.PARENT_ID = o.ID) as COUNT from OBJECTS o where o.PARENT_ID = " << pUPnPBrowse->GetObjectIDAsInt() << " ";
  sSql << "order by o.FILE_NAME ";
  if(pUPnPBrowse->m_nRequestedCount > 0)
    sSql << " limit " << pUPnPBrowse->m_nStartingIndex << ", " << pUPnPBrowse->m_nRequestedCount;        
    
  unsigned int tmpInt = *p_pnNumberReturned;
    
  CContentDatabase::Shared()->Lock();
  CContentDatabase::Shared()->Select(sSql.str());
  while(!CContentDatabase::Shared()->Eof())
  {
    CSelectResult* pRow = CContentDatabase::Shared()->GetResult();          
    
    BuildDescription(pWriter, pRow, pUPnPBrowse->m_sObjectID, pRow->GetValue("COUNT"));
    
    CContentDatabase::Shared()->Next();                    

    tmpInt++;
    //cout << tmpInt << endl;
  }        
  CContentDatabase::Shared()->Unlock();                          
  *p_pnNumberReturned = tmpInt;
}

void CContentDirectory::BuildDescription(xmlTextWriterPtr pWriter,
                                         CSelectResult* pSQLResult,
                                         std::string p_sParentId,
                                         std::string p_sChildCount)
{
  if(pSQLResult->GetValue("TYPE").compare("10") == 0)
  {
    //cout << "CONTAINER_STORAGE_FOLDER" << endl;
    cout << "CHILD COUNT: " << p_sChildCount << endl;
    BuildContainerDescription(pWriter, pSQLResult, p_sParentId, p_sChildCount);
  }
  else if(pSQLResult->GetValue("TYPE").compare("100") == 0)
  {
    //cout << "ITEM_IMAGE_ITEM_PHOTO" << endl;
    BuildItemDescription(pWriter, pSQLResult, ITEM_IMAGE_ITEM_PHOTO, p_sParentId);
  }
  else if(pSQLResult->GetValue("TYPE").compare("200") == 0)
  {
    //cout << "ITEM_AUDIO_ITEM_MUSIC_TRACK" << endl;
    BuildItemDescription(pWriter, pSQLResult, ITEM_AUDIO_ITEM_MUSIC_TRACK, p_sParentId);
  }
  else if(pSQLResult->GetValue("TYPE").compare("300") == 0)
  {
    //cout << "ITEM_VIDEO_ITEM_MOVIE" << endl;
    BuildItemDescription(pWriter, pSQLResult, ITEM_VIDEO_ITEM_MOVIE, p_sParentId);
  }              
}


void CContentDirectory::BuildContainerDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sParentId, std::string p_sChildCount)
{
  /* container  */
  xmlTextWriterStartElement(pWriter, BAD_CAST "container"); 
     
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
    xmlTextWriterWriteAttribute(pWriter, BAD_CAST "childCount", BAD_CAST p_sChildCount.c_str());   
     
    /* title */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");     
    xmlTextWriterWriteString(pWriter, BAD_CAST pSQLResult->GetValue("FILE_NAME").c_str()); 
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


void CContentDirectory::BuildItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, OBJECT_TYPE p_nObjectType, std::string p_sParentId)
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
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "date", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "2005-10-15");
    xmlTextWriterEndElement(pWriter);
    
    /* writeStatus */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "upnp", BAD_CAST "writeStatus", BAD_CAST "urn:schemas-upnp-org:metadata-1-0/upnp/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST "UNKNOWN");
    xmlTextWriterEndElement(pWriter);
    
    /* title */
    xmlTextWriterStartElementNS(pWriter, BAD_CAST "dc", BAD_CAST "title", BAD_CAST "http://purl.org/dc/elements/1.1/");    
    xmlTextWriterWriteString(pWriter, BAD_CAST BAD_CAST pSQLResult->GetValue("FILE_NAME").c_str());
    xmlTextWriterEndElement(pWriter);
    
    switch(p_nObjectType)
    {
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:
        BuildAudioItemDescription(pWriter, pSQLResult, szObjId);
        break;
      case ITEM_IMAGE_ITEM_PHOTO:
        BuildImageItemDescription(pWriter, pSQLResult, szObjId);
        break;
      case ITEM_VIDEO_ITEM_MOVIE:
        BuildVideoItemDescription(pWriter, pSQLResult, szObjId);
        break;
    }           
  
  /* end item */
  xmlTextWriterEndElement(pWriter);
}
  

void CContentDirectory::BuildAudioItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sObjectID)
{
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
  
  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  std::stringstream sTmp;
  sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
  
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/AudioItems/" << p_sObjectID;
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter); 
  
}

void CContentDirectory::BuildImageItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sObjectID)
{    

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
  
/* rights
dc
No */  

  /* res */
  xmlTextWriterStartElement(pWriter, BAD_CAST "res");
  
  std::stringstream sTmp;
  sTmp << "http-get:*:" << pSQLResult->GetValue("MIME_TYPE") << ":*";
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "protocolInfo", BAD_CAST sTmp.str().c_str());
  sTmp.str("");
  
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/ImageItems/" << p_sObjectID;
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
  xmlTextWriterWriteString(pWriter, BAD_CAST sTmp.str().c_str());
  xmlTextWriterEndElement(pWriter);  
    
}

void CContentDirectory::BuildVideoItemDescription(xmlTextWriterPtr pWriter, CSelectResult* pSQLResult, std::string p_sObjectID)
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
  
  sTmp << "http://" << m_sHTTPServerURL << "/MediaServer/VideoItems/" << p_sObjectID;
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "importUri", BAD_CAST sTmp.str().c_str());
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

/* <\PRIVATE> */
