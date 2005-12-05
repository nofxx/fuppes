/***************************************************************************
 *            ContentDirectory.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
  /* Init members */
  m_pBaseFolder = new CStorageFolder(m_sHTTPServerURL);
  m_ObjectList["0"] = m_pBaseFolder;

  /* Init object list */
  //BuildObjectList();
  
  m_pDatabase = new CContentDatabase();
  //cout << "Insert: " << m_pDatabase->Insert("insert into objects (TYPE, PATH, MD5) values (7, '/mnt/test', '1235dasd648536');"); 
  
  for(unsigned int i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++)
  {
    if(DirectoryExists(CSharedConfig::Shared()->GetSharedDir(i)))
    {  
      if(CSharedConfig::Shared()->GetDisplaySettings().bShowDirNamesInFirstLevel)
      {      
        stringstream sSql;
        sSql << "insert into objects (TYPE, PARENT_ID, PATH) values ";
        sSql << "(" << CONTAINER_STORAGE_FOLDER << ", ";
        sSql << 0 << ", ";
        sSql << "'" << CSharedConfig::Shared()->GetSharedDir(i) << "');";
        
        long long int nRowId = m_pDatabase->Insert(sSql.str());
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
  }  
  
  
}

/* destructor */
CContentDirectory::~CContentDirectory()
{
  delete m_pDatabase;
  
  /* T.S.TODO: Delete all objects that were created with 'new'.
               'm_Objectlist' has the pointers to these objects */
  
  SAFE_DELETE(m_pBaseFolder);
}

/*===============================================================================
 UPNP ACTION HANDLING
===============================================================================*/
 
/* HandleUPnPAction */
bool CContentDirectory::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
  BOOL_CHK_RET_POINTER(pUPnPAction);
  BOOL_CHK_RET_POINTER(pMessageOut);
  
  /* Handle UPnP browse */
  string sContent = HandleUPnPBrowse((CUPnPBrowse*)pUPnPAction);  

  /* Set a message for the incoming action */
  pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml; charset=\"utf-8\""); // HTTP_CONTENT_TYPE_TEXT_XML
  pMessageOut->SetContent(sContent);

  return true;
}

/*===============================================================================
 GET
===============================================================================*/

/* GetFileNameFromObjectID */
std::string CContentDirectory::GetFileNameFromObjectID(std::string p_sObjectID)
{ 
  /* Find object id in list */
  m_ListIterator = m_ObjectList.find(p_sObjectID.c_str());

  if(m_ListIterator != m_ObjectList.end())
  {
    /* return filename */
    return ((CUPnPObject*)m_ObjectList[p_sObjectID.c_str()])->GetFileName();
  }
  else
  {
    /* object not found, return empty string */
    ASSERT(0);
    return "";
  }
}

/* GetItemFromObjectID */
CUPnPObject* CContentDirectory::GetItemFromObjectID(std::string p_sObjectID)
{
  /* Find object id in list */
  m_ListIterator = m_ObjectList.find(p_sObjectID.c_str());

  if(m_ListIterator != m_ObjectList.end())
  {
    /* return object */
    return ((CUPnPObject*)m_ObjectList[p_sObjectID.c_str()]);
  }
  else
  {
    /* object not found, return NULL */
    ASSERT(0);
    return NULL;
  }
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

/* HandleUPnPBrowse */
std::string CContentDirectory::HandleUPnPBrowse(CUPnPBrowse* pUPnPBrowse)
{
  STRING_CHK_RET_POINTER(pUPnPBrowse);

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
      
        unsigned int nNumberReturned = 0;
        unsigned int nTotalMatches   = 0;
        m_ListIterator = m_ObjectList.find(pUPnPBrowse->m_sObjectID);
        if(m_ListIterator != m_ObjectList.end())
        {
          xmlTextWriterWriteString(writer, 
            BAD_CAST ((CStorageFolder*)m_ObjectList[pUPnPBrowse->m_sObjectID])->GetContentAsString(pUPnPBrowse, &nNumberReturned, &nTotalMatches).c_str());        
        }
      
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
	return output.str();
}

/* BuildObjectList */
void CContentDirectory::BuildObjectList()
{
  unsigned int nCount = 1;

  CSharedLog::Shared()->Log(LOGNAME, "building content list");
  
  for(unsigned int i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++)
  {
    if(DirectoryExists(CSharedConfig::Shared()->GetSharedDir(i)))
    {  
      if(CSharedConfig::Shared()->GetDisplaySettings().bShowDirNamesInFirstLevel)
      {      
        CStorageFolder* pTmpFolder = new CStorageFolder(m_sHTTPServerURL);
  
        char szObjId[11];                            
        sprintf(szObjId, "%010X", nCount);          
        pTmpFolder->SetObjectID(szObjId);    
  
        pTmpFolder->SetParent(m_pBaseFolder);
        pTmpFolder->SetFileName(CSharedConfig::Shared()->GetSharedDir(i));
  
        #ifdef WIN32
        RegEx rxDirName("\\\\([^\\\\|\\.]*)$", PCRE_CASELESS);
        #else
        RegEx rxDirName("/([^/|\\.]*)$", PCRE_CASELESS);
        #endif
        std::string sSharedDir = CSharedConfig::Shared()->GetSharedDir(i);
        const char* pszDir = sSharedDir.c_str();
        if(rxDirName.Search(pszDir))
          pTmpFolder->SetName(rxDirName.Match(1));
        else
          pTmpFolder->SetName(CSharedConfig::Shared()->GetSharedDir(i));
  
        /* Add folder to list and parent folder */
        m_ObjectList[szObjId] = pTmpFolder;
        m_pBaseFolder->AddUPnPObject(pTmpFolder);
  
        /* increment counter */
        nCount++;      
      
        ScanDirectory(CSharedConfig::Shared()->GetSharedDir(i), &nCount, pTmpFolder);     
      }
      else
      {
        ScanDirectory(CSharedConfig::Shared()->GetSharedDir(i), &nCount, m_pBaseFolder);     
      }
      
    }
    else
    {
      stringstream sLog;
      sLog << "shared directory: \"" << CSharedConfig::Shared()->GetSharedDir(i) << "\" not found";
      CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    }
  }
  
  CSharedLog::Shared()->Log(LOGNAME, "done building content list");
}

/* ScanDirectory */
void CContentDirectory::ScanDirectory(std::string p_sDirectory, unsigned int* p_pnCount, CStorageFolder* pParentFolder)
{ 
  VOID_CHK_RET_POINTER(p_pnCount);
  VOID_CHK_RET_POINTER(pParentFolder);

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
        /* Audio item */
        if(IsFile(sTmp.str()) && CSharedConfig::Shared()->IsSupportedFileExtension(sExt))
        {
          CAudioItem* pTmpItem = new CAudioItem(m_sHTTPServerURL);
          
          char szObjId[10];
          sprintf(szObjId, "%010X", *p_pnCount);
            
          pTmpItem->SetObjectID(szObjId);
          pTmpItem->SetParent(pParentFolder);     
          pTmpItem->SetFileName(sTmp.str());
        
          if(pTmpItem->SetupTranscoding())
          {
            /* set object name */
            stringstream sName;
            sName << TruncateFileExt(sTmpFileName);
            if(pTmpItem->GetDoTranscode() && CSharedConfig::Shared()->GetDisplaySettings().bShowTranscodingTypeInItemNames)
            {
              switch(pTmpItem->GetDecoderType())
              {
                case AUDIO_DECODER_UNKNOWN:
                  break;
                case AUDIO_DECODER_VORBIS:
                  sName << " (ogg)";
                  break;
                case AUDIO_DECODER_MUSEPACK:
                  sName << " (mpc)";
                  break;
                case AUDIO_DECODER_FLAC:
                  sName << " (flac)";
                case AUDIO_DECODER_NONE:
                  break;                
              }                     
            } 
            pTmpItem->SetName(sName.str());            
            
            /* Add audio item to list and parent folder */
            m_ObjectList[szObjId] = pTmpItem;
            pParentFolder->AddUPnPObject(pTmpItem);
            
            /* increment counter */
            int nTmp = *p_pnCount;
            nTmp++;
            *p_pnCount = nTmp;
  
            /* log */
            sTmp.str("");
            sTmp << "added audioItem: \"" << sTmpFileName << "\"";
            CSharedLog::Shared()->ExtendedLog(LOGNAME, sTmp.str());            
          }
          else
          {
            delete pTmpItem;
          }

        }
        
        /* ImageItem */
        if(IsFile(sTmp.str()) && (sExt.compare("jpg") == 0))
        {
          CImageItem* pTmpItem = new CImageItem(m_sHTTPServerURL);
          
          pTmpItem->m_nImageType = IMAGE_TYPE_JPEG;
          
          char szObjId[10];
          sprintf(szObjId, "%010X", *p_pnCount);
            
          pTmpItem->SetObjectID(szObjId);
          pTmpItem->SetParent(pParentFolder);     
          pTmpItem->SetFileName(sTmp.str());        
       
          /* set object name */
          stringstream sName;
          sName << TruncateFileExt(sTmpFileName);
          pTmpItem->SetName(sName.str());
          
          /* Add audio item to list and parent folder */
          m_ObjectList[szObjId] = pTmpItem;
          pParentFolder->AddUPnPObject(pTmpItem);
          
          /* increment counter */
          int nTmp = *p_pnCount;
          nTmp++;
          *p_pnCount = nTmp;

          /* log */
          sTmp.str("");
          sTmp << "added imageItem: \"" << sTmpFileName << "\"";
          CSharedLog::Shared()->ExtendedLog(LOGNAME, sTmp.str());
        }    

        /* VideoItem */
        if(IsFile(sTmp.str()) && ((sExt.compare("mpeg") == 0) || (sExt.compare("mpg") == 0) || (sExt.compare("avi") == 0)))
        {
          CVideoItem* pTmpItem = new CVideoItem(m_sHTTPServerURL);
          
          if ((sExt.compare("mpeg") == 0) || (sExt.compare("mpg") == 0))
            pTmpItem->m_nVideoType = VIDEO_TYPE_MPEG;
          else if (sExt.compare("avi") == 0)
            pTmpItem->m_nVideoType = VIDEO_TYPE_AVI;
          
          char szObjId[10];
          sprintf(szObjId, "%010X", *p_pnCount);
            
          pTmpItem->SetObjectID(szObjId);
          pTmpItem->SetParent(pParentFolder);     
          pTmpItem->SetFileName(sTmp.str());        
       
          /* set object name */
          stringstream sName;
          sName << TruncateFileExt(sTmpFileName);
          pTmpItem->SetName(sName.str());
          
          /* Add audio item to list and parent folder */
          m_ObjectList[szObjId] = pTmpItem;
          pParentFolder->AddUPnPObject(pTmpItem);
          
          /* increment counter */
          int nTmp = *p_pnCount;
          nTmp++;
          *p_pnCount = nTmp;

          /* log */
          sTmp.str("");
          sTmp << "added videoItem: \"" << sTmpFileName << "\"";
          CSharedLog::Shared()->ExtendedLog(LOGNAME, sTmp.str());
        }    
        
        /* StorageFolder */
        else if(IsDirectory(sTmp.str()))
        {            
          /* create folder object */
          CStorageFolder* pTmpFolder = new CStorageFolder(m_sHTTPServerURL);
          
          char szObjId[10];                            
          sprintf(szObjId, "%010X", *p_pnCount);            
          
          pTmpFolder->SetObjectID(szObjId);            
          pTmpFolder->SetParent(pParentFolder);
          pTmpFolder->SetName(sTmpFileName);
          pTmpFolder->SetFileName(sTmp.str());
          
          /* Add folder to list and parent folder */
          m_ObjectList[szObjId] = pTmpFolder;
          pParentFolder->AddUPnPObject(pTmpFolder);
          
          /* increment counter */
          int nTmp = *p_pnCount;
          nTmp++;
          *p_pnCount = nTmp;
          
          /* scan subdirectories */
          ScanDirectory(sTmp.str(), p_pnCount, pTmpFolder);          
          
          /* log */
          sTmp.str("");
          sTmp << "added StorageFolder: \"" << sTmpFileName << "\"";
          CSharedLog::Shared()->ExtendedLog(LOGNAME, sTmp.str()); 
        }
        sTmp.str("");
      }
    }    
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif  
}

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
          sSql << "insert into objects (TYPE, PARENT_ID, PATH) values ";
          sSql << "(" << CONTAINER_STORAGE_FOLDER << ", ";
          sSql << p_nParentId << ", ";
          sSql << "'" << sTmp.str() << "');";
          
          long long int nRowId = m_pDatabase->Insert(sSql.str());
          DbScanDir(sTmp.str(), nRowId);          
        }
        else if(IsFile(sTmp.str()) && CSharedConfig::Shared()->IsSupportedFileExtension(sExt))
        {
          cout << "Parent: " << p_nParentId << endl;
          cout << "FileName: " << sTmpFileName << endl;
          cout << "Path: " << sTmp.str() << endl;

          OBJECT_TYPE nObjectType = CFileDetails::Shared()->GetObjectType(sTmp.str());
          switch(nObjectType)
          {
            case ITEM_AUDIO_ITEM_MUSIC_TRACK:
              cout << "MusicTrack" << endl;
              SMusicTrack TrackInfo = CFileDetails::Shared()->GetMusicTrackDetails(sTmp.str());
              break;
          }
          
          stringstream sSql;
          sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME, MD5) values ";
          sSql << "(" << nObjectType << ", ";
          sSql << p_nParentId << ", ";
          sSql << "'" << sTmp.str() << "', ";
          sSql << "'" << sTmpFileName << "', ";
          sSql << "'" << "todo" << "');";
          
          long long int nRowId = m_pDatabase->Insert(sSql.str());
          DbScanDir(sTmp.str(), nRowId);    
          
        }   
        
        sTmp.str("");
      }
    }  /* while */  
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif         
}
        
/* <\PRIVATE> */
