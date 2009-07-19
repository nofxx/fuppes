/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ContentDatabase.cpp
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
 
#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "ContentDatabase.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "FileDetails.h"
#include "../Common/RegEx.h"
#include "../Common/Common.h"
#include "iTunesImporter.h"
#include "PlaylistParser.h"
#include "../Plugins/Plugin.h"

#include <sstream>
#include <string>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstdio>
#ifndef WIN32
#include <dirent.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>

using namespace std;

static bool g_bIsRebuilding;
static bool g_bFullRebuild;
static bool g_bAddNew;
static bool g_bRemoveMissing;

unsigned int g_nObjId = 0;

CContentDatabase* CContentDatabase::m_Instance = 0;

CContentDatabase* CContentDatabase::Shared()
{
	if(m_Instance == 0) {
		m_Instance = new CContentDatabase(true);
  }
	return m_Instance;
}

CContentDatabase::CContentDatabase(bool p_bShared)
{ 
	//m_RebuildThread 	= (fuppesThread)NULL;	
	m_RebuildThread		= NULL;
  m_bShared       	= p_bShared;	
  m_bInTransaction 	= false;
  m_nLockCount 			= -1;
  
	if(m_bShared) {
		g_bIsRebuilding   = false;
    g_bFullRebuild    = true;
    g_bAddNew         = false;
    g_bRemoveMissing  = false;    
    m_nLockCount 		  = 0;
    
    m_pFileAlterationMonitor = CFileAlterationMgr::Shared()->CreateMonitor(this);
    fuppesThreadInitMutex(&m_Mutex);
  }
    
  if(!m_bShared) {
    if(!Open())
      cout << "FAILED OPENING DB FILE" << endl;
  }
}
 
CContentDatabase::~CContentDatabase()
{                            
	if(m_bShared) {                  
    fuppesThreadDestroyMutex(&m_Mutex);
    delete m_pFileAlterationMonitor;    
  }
	
	if(m_bShared && (m_RebuildThread != NULL)) { //(fuppesThread)NULL)) {
	  //fuppesThreadClose(m_RebuildThread);
		//m_RebuildThread = (fuppesThread)NULL;
		delete m_RebuildThread;
		m_RebuildThread = NULL;
	}

  Close();
}

std::string CContentDatabase::GetLibVersion()
{
  return ""; //sqlite3_libversion();
}

bool CContentDatabase::Init(bool* p_bIsNewDB)
{   
  if(!m_bShared) {
		return false;
	}
  
  bool bIsNewDb = !FileExists(CSharedConfig::Shared()->GetDbFileName());
  *p_bIsNewDB = bIsNewDb;  
  
  
  //cout << "init db " << m_sDbFileName << endl;
  
  Open();
	
  if(bIsNewDb) {

    if(!Execute("CREATE TABLE OBJECTS ( "
				"  ID INTEGER PRIMARY KEY AUTOINCREMENT, "
				"  OBJECT_ID INTEGER NOT NULL, "
        "  DETAIL_ID INTEGER DEFAULT NULL, "
				"  TYPE INTEGER NOT NULL, "
        "  DEVICE TEXT DEFAULT NULL, "  
				"  PATH TEXT NOT NULL, "
				"  FILE_NAME TEXT DEFAULT NULL, "
				"  TITLE TEXT DEFAULT NULL, "
				"  MD5 TEXT DEFAULT NULL, "
				"  MIME_TYPE TEXT DEFAULT NULL, "
				"  REF_ID INTEGER DEFAULT NULL, " 
				"  HIDDEN INTEGER DEFAULT 0 "
				//"  UPDATE_ID INTEGER DEFAULT 0"
				");"))   
			return false;
    
    if(!Execute("CREATE TABLE OBJECT_DETAILS ( "
        "  ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  AV_BITRATE INTEGER, "
				"  AV_DURATION TEXT, "
				"  A_ALBUM TEXT, "
				"  A_ARTIST TEXT, "
				"  A_CHANNELS INTEGER, "
				"  A_DESCRIPTION TEXT, "
				"  A_GENRE TEXT, "
				"  A_SAMPLERATE INTEGER, "
				"  A_TRACK_NO INTEGER, "
				"  DATE TEXT, "
				"  IV_HEIGHT INTEGER, "
				"  IV_WIDTH INTEGER, "
        "  A_CODEC TEXT, "
        "  V_CODEC TEXT, "
				"  ALBUM_ART_ID INTEGER, "
				"  ALBUM_ART_EXT TEXT, "
        "  SIZE INTEGER DEFAULT 0, "
				"  DLNA_PROFILE TEXT DEFAULT NULL,"
				"  DLNA_MIME_TYPE TEXT DEFAULT NULL"
				");"))   
			return false;
    
    if(!Execute("CREATE TABLE MAP_OBJECTS ( "
        "  OBJECT_ID INTEGER NOT NULL, "
        "  PARENT_ID INTEGER NOT NULL, "
        "  DEVICE TEXT "
        ");"))
      return false;

		/*if(!Execute("CREATE TABLE MAP_OBJECT_DETAILS ( "
        "  OBJECT_ID INTEGER NOT NULL, "
        "  DETAIL_ID INTEGER NOT NULL, "
        "  DEVICE TEXT "
        ");"))
      return false;		*/
		
    if(!Execute("CREATE INDEX IDX_OBJECTS_OBJECT_ID ON OBJECTS(OBJECT_ID);"))
      return false;
    
    if(!Execute("CREATE INDEX IDX_MAP_OBJECTS_OBJECT_ID ON MAP_OBJECTS(OBJECT_ID);"))
      return false;
    
    if(!Execute("CREATE INDEX IDX_MAP_OBJECTS_PARENT_ID ON MAP_OBJECTS(PARENT_ID);"))
      return false;	
        
    if(!Execute("CREATE INDEX IDX_OBJECTS_DETAIL_ID ON OBJECTS(DETAIL_ID);"))
      return false;

    if(!Execute("CREATE INDEX IDX_OBJECT_DETAILS_ID ON OBJECT_DETAILS(ID);"))
      return false;
		
    /*if(!Execute("CREATE INDEX IDX_MAP_OBJECT_DETAILS_OBJECT_ID ON MAP_OBJECT_DETAILS(OBJECT_ID);"))
      return false;
    
    if(!Execute("CREATE INDEX IDX_MAP_OBJECT_DETAILS_DETAILS_ID ON MAP_OBJECT_DETAILS(DETAIL_ID);"))
      return false;*/
  }

#ifdef OLD_DB
	// setup file alteration monitor
	if(m_pFileAlterationMonitor->isActive()) {
		
		Select("select PATH from OBJECTS where TYPE = 1 and DEVICE is NULL");
		while(!Eof()) {
			m_pFileAlterationMonitor->addWatch(GetResult()->asString("PATH"));
			Next();
		}
	}
#endif
  return true;
}

void CContentDatabase::Lock()
{
  if(m_bShared) {
    fuppesThreadLockMutex(&m_Mutex);
    //m_nLockCount++;
    //cout << "CDB LOCK: " << m_nLockCount << endl; fflush(stdout);    
  }
  else
    CContentDatabase::Shared()->Lock();
}

void CContentDatabase::Unlock()
{
  if(m_bShared) {
    //cout << "CDB UNLOCK: " << m_nLockCount << endl; fflush(stdout);
    //m_nLockCount--;    
    fuppesThreadUnlockMutex(&m_Mutex);    
  }
  else
    CContentDatabase::Shared()->Unlock();
}


bool CContentDatabase::Open()
{  
	CConnectionParams params;
	//params.type = "sqlite3";
	params.filename = CSharedConfig::Shared()->GetDbFileName();
	/*params.type = "mysql";
	params.hostname = "localhost";
	params.username = "fuppes";
	params.password = "fuppes";
	params.dbname = "fuppes";*/
	
	if(!CDatabase::open(params))
		return false;
	
  /*if(!m_bShared) {	
		m_pDbHandle = CContentDatabase::Shared()->m_pDbHandle;		
		return (m_pDbHandle != NULL);
	}	

  if(m_pDbHandle != NULL) {
    return true;
  }
  
  if(sqlite3_open(m_sDbFileName.c_str(), &m_pDbHandle) != SQLITE_OK) {
    fprintf(stderr, "Can't create/open database: %s\n", sqlite3_errmsg(m_pDbHandle));
    sqlite3_close(m_pDbHandle);
    return false;
  }
  //JM: Tell sqlite3 to retry queries for up to 1 second if the database is locked.
  sqlite3_busy_timeout(m_pDbHandle, 1000);
	*/

	CSQLQuery* qry = CDatabase::query();
	qry->select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
  if(!qry->eof()) {  
    g_nObjId = qry->result()->asUInt("VALUE");
  }
	delete qry;
  
	return true;
}

void CContentDatabase::Close()
{
  if(m_bShared) {
		CDatabase::close();
	}
}


void CContentDatabase::ClearResult()
{
  // clear old results
  for(m_ResultListIterator = m_ResultList.begin(); m_ResultListIterator != m_ResultList.end();)
  {
    if(m_ResultList.empty())
      break;
    
    CSQLResult* pResult = *m_ResultListIterator;
    std::list<CSQLResult*>::iterator tmpIt = m_ResultListIterator;          
    ++tmpIt;
    m_ResultList.erase(m_ResultListIterator);
    m_ResultListIterator = tmpIt;
    delete pResult;
  } 
  
  m_ResultList.clear();
  m_nRowsReturned = 0;
}

unsigned int CContentDatabase::Insert(std::string p_sStatement)
{
	CSQLQuery* qry = CDatabase::query();
	fuppes_off_t result = qry->insert(p_sStatement);
	delete qry;
	return result;
}

bool CContentDatabase::Execute(std::string p_sStatement)
{
	CSQLQuery* qry = CDatabase::query();
	bool result = qry->exec(p_sStatement);
	delete qry;
	return result;
}

bool CContentDatabase::Select(std::string p_sStatement)
{  
  ClearResult();    

  CSQLResult* pResult;
  CSQLQuery* qry = CDatabase::query();     
	
	
	if(!qry->select(p_sStatement)) {
		delete qry;
		return false;
	}
	
	while(!qry->eof()) {	
    pResult = qry->result()->clone();      
    m_ResultList.push_back(pResult);
		qry->next();
	}
	m_ResultListIterator = m_ResultList.begin();       
  
	delete qry;
  return true;
}

bool CContentDatabase::Eof()
{
  return (m_ResultListIterator == m_ResultList.end());
}
    
CSQLResult* CContentDatabase::GetResult()
{
  return *m_ResultListIterator;
}

void CContentDatabase::Next()
{
  if(m_ResultListIterator != m_ResultList.end()) {
    m_ResultListIterator++;
  } 
}




//fuppesThreadCallback BuildLoop(void *arg);
void DbScanDir(std::string p_sDirectory, long long int p_nParentId);
void BuildPlaylists();
void ParsePlaylist(CSQLResult* pResult);
void ParseM3UPlaylist(CSQLResult* pResult);
void ParsePLSPlaylist(CSQLResult* pResult);

std::string findAlbumArtFile(std::string dir);

unsigned int InsertFile(CContentDatabase* pDb, unsigned int p_nParentId, std::string p_sFileName, bool hidden = false);

unsigned int GetObjectIDFromFileName(CContentDatabase* pDb, std::string p_sFileName);

void CContentDatabase::BuildDB()
{     
  if(CContentDatabase::Shared()->IsRebuilding())
	  return;
	
	if(!m_bShared)
	  return;
		
	/*if(m_RebuildThread != (fuppesThread)NULL) {
	  fuppesThreadClose(m_RebuildThread);
		m_RebuildThread = (fuppesThread)NULL;
	}*/
	if(m_RebuildThread) {
		delete m_RebuildThread;
		m_RebuildThread = NULL;
	}	
		
	m_RebuildThread = new RebuildThread();
	m_RebuildThread->start();
	//fuppesThreadStart(m_RebuildThread, BuildLoop);
  g_bIsRebuilding = true;  
}

void CContentDatabase::RebuildDB()
{     
  if(CContentDatabase::Shared()->IsRebuilding())
	  return;
	
	if(!m_bShared)
	  return;
	
  g_bFullRebuild = true;
  g_bAddNew = false;
  g_bRemoveMissing = false;
  
  BuildDB();
}

void CContentDatabase::UpdateDB()
{
  if(CContentDatabase::Shared()->IsRebuilding())
    return;
  
  if(!m_bShared)
    return;
  
  g_bFullRebuild = false;
  g_bAddNew = true;
  g_bRemoveMissing = true;  
    
	CSQLQuery* qry = CDatabase::query();
	qry->select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
  if(!qry->eof()) {  
    g_nObjId = qry->result()->asUInt("VALUE");
  }
	delete qry;

  BuildDB();
}

void CContentDatabase::AddNew()
{
  if(CContentDatabase::Shared()->IsRebuilding())
    return;
  
  if(!m_bShared)
    return;
  
  g_bFullRebuild = false;
  g_bAddNew = true;
  g_bRemoveMissing = false;  

	CSQLQuery* qry = CDatabase::query();
	qry->select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
  if(!qry->eof()) {  
    g_nObjId = qry->result()->asUInt("VALUE");
  }
	delete qry;
	
  BuildDB();
}

void CContentDatabase::RemoveMissing()
{
  if(CContentDatabase::Shared()->IsRebuilding())
    return;
  
  if(!m_bShared)
    return;
  
  g_bFullRebuild = false;
  g_bAddNew = false;
  g_bRemoveMissing = true;  
  
  BuildDB();
}


bool CContentDatabase::IsRebuilding()
{
  return g_bIsRebuilding;
}

unsigned int CContentDatabase::GetObjId()
{
  return ++g_nObjId;
}

void DbScanDir(CContentDatabase* pDb, std::string p_sDirectory, long long int p_nParentId)
{
	p_sDirectory = appendTrailingSlash(p_sDirectory);

	if(!DirectoryExists(p_sDirectory))
		return;
	
  #ifdef WIN32
  char szTemp[MAX_PATH];
  strcpy(szTemp, p_sDirectory.c_str());
  
  // Add search criteria
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
      //strcat(szTemp, upnpPathDelim);
      strcat(szTemp, data.cFileName);
      
      string sTmp;
      sTmp = szTemp;
      
      string sTmpFileName = data.cFileName;
  #else
      
  DIR*    pDir;
  dirent* pDirEnt;
  string  sTmp;
  
  if((pDir = opendir(p_sDirectory.c_str())) != NULL)
  {    
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "read directory: %s",  p_sDirectory.c_str());
    while((pDirEnt = readdir(pDir)) != NULL)
    {
      if(((string(".").compare(pDirEnt->d_name) != 0) && 
         (string("..").compare(pDirEnt->d_name) != 0)))
      {        
        sTmp = p_sDirectory + pDirEnt->d_name;        
        string sTmpFileName = pDirEnt->d_name;        
  #endif  
                
        string sExt = ExtractFileExt(sTmp);
        unsigned int nObjId = 0;
        
        /* directory */
        if(IsDirectory(sTmp))
        {				
          sTmpFileName = ToUTF8(sTmpFileName);          
          sTmpFileName = SQLEscape(sTmpFileName);        
          
          stringstream sSql;
          
          appendTrailingSlash(&sTmp);
          
          if(g_bAddNew) {
            nObjId = GetObjectIDFromFileName(pDb, sTmp);
          }
          
          if(nObjId == 0) {

#ifdef OLD_DB
//            pDb->BeginTransaction();          
#endif
						
            nObjId = pDb->GetObjId();
						OBJECT_TYPE folderType = CONTAINER_STORAGE_FOLDER;
	
						// check for album art
						string albumArt = findAlbumArtFile(sTmp);
						if(albumArt.length() > 0) {
							
							unsigned int artId = GetObjectIDFromFileName(pDb, albumArt);
#warning device compatibility!?
							folderType = CONTAINER_ALBUM_MUSIC_ALBUM;
							if(artId == 0) {
								InsertFile(pDb, nObjId, albumArt, true);
							}
							else {
#warning todo hide
							}
						}
	
						// insert folder
            sSql << "insert into OBJECTS ( " <<
              "  OBJECT_ID, TYPE, " <<
              "  PATH, TITLE) " <<
              "values ( " << 
                 nObjId << ", " <<  
                 folderType << 
                 ", '" << SQLEscape(sTmp) << "', '" <<
                 //sTmpFileName << "', '" <<
                 sTmpFileName <<
              "');";

            pDb->Insert(sSql.str());          
            
            sSql.str("");
            sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
                    "values (" << nObjId << ", " << p_nParentId << ")";          
            pDb->Insert(sSql.str());
            
#ifdef OLD_DB
//            pDb->Commit();
#endif
            
            pDb->fileAlterationMonitor()->addWatch(sTmp);
          }
            
          // recursively scan subdirectories
          DbScanDir(pDb, sTmp, nObjId);          
        }
        else if(IsFile(sTmp) && CFileDetails::Shared()->IsSupportedFileExtension(sExt))
        {
					unsigned int objId = GetObjectIDFromFileName(pDb, sTmp);
					if(objId == 0) {
	          InsertFile(pDb, p_nParentId, sTmp);
					}
        }         
       
      }
    }  /* while */  
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif         
}

std::string findAlbumArtFile(std::string dir)
{
  string file;
	dir = appendTrailingSlash(dir);
	string result;

	#ifdef WIN32
	WIN32_FIND_DATA data;
  HANDLE hFile = FindFirstFile(string(dir + "*").c_str(), &data);
  if(NULL == hFile)
    return "";

  while(FindNextFile(hFile, &data)) {		
    if(((string(".").compare(data.cFileName) != 0) && 
      (string("..").compare(data.cFileName) != 0))) {        
      
			file = data.cFileName;
  #else
      
  DIR*    pDir;
  dirent* pDirEnt;
  
  if((pDir = opendir(dir.c_str())) != NULL) { 
    while((pDirEnt = readdir(pDir)) != NULL) {
      if(((string(".").compare(pDirEnt->d_name) != 0) && 
         (string("..").compare(pDirEnt->d_name) != 0))) {

        file = pDirEnt->d_name;
  #endif
					 
				if(CSharedConfig::isAlbumArtFile(file)) {
					result = dir + file;
					break;
				}
				
			} // if
				
		} // while  FindNext | readdir
			
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif 
	
	return result;
}

unsigned int findAlbumArt(std::string dir, std::string* ext, CSQLQuery* qry)
{
	string file = findAlbumArtFile(dir);
	if(file.length() == 0) {
		return 0;
	}
	
	string path = ExtractFilePath(file);
	file = file.substr(path.length(), file.length());

	qry->select("select * from OBJECTS "
							"where PATH = '" + SQLEscape(path) + "' and "
							"FILE_NAME = '" + SQLEscape(file) + "' and "
							"DEVICE is NULL");
	if(qry->eof()) {
		return 0;
	}

	*ext = ExtractFileExt(file);
	return qry->result()->asUInt("OBJECT_ID");
}

/**
 * if "file" is set only selected "file" is updated else 
 * all audio files in the "path" will be set to "artId, ext" using "qry"
 */

void setAlbumArtImage(std::string path, std::string file, unsigned int artId, std::string ext, CSQLQuery* qry)
{	
	stringstream sql;
	sql << "select * from OBJECTS " <<
		"where PATH = '" + SQLEscape(path) + "' ";
	if(file.empty()) {
		sql << " and TYPE in (" <<
			ITEM_AUDIO_ITEM << ", " << ITEM_AUDIO_ITEM_MUSIC_TRACK << ")";  
	}
	else {
		sql << " and FILE_NAME = '" << SQLEscape(file) + "' ";
	}
	sql <<  " and DEVICE is NULL";

	//cout << sql.str() << endl;
	
	unsigned int detailId;
	bool updateDetails;
	qry->select(sql.str());
	while(!qry->eof()) {

		sql.str("");
		sql.clear();
		
		detailId = 0;
		updateDetails = false;
		if(!qry->result()->isNull("DETAIL_ID")) {
			detailId = qry->result()->asUInt("DETAIL_ID");
			updateDetails = true;
		}		
	
		if(updateDetails) {
			sql << "update OBJECT_DETAILS set " <<
				"ALBUM_ART_ID = " << artId <<	", " <<
				"ALBUM_ART_EXT = '" << SQLEscape(ext) << "' " <<
				"where ID = " << detailId;
			qry->exec(sql.str());
			//cout << sql.str() << endl;
		}
		else {
			sql << "insert into OBJECT_DETAILS " <<
				"(ALBUM_ART_ID, ALBUM_ART_EXT) " <<
				"values " <<
				"(" << artId << ", " <<
				"'" << SQLEscape(ext) << "')";
			detailId = qry->insert(sql.str());
			//cout << sql.str() << endl;
			
			sql.str("");
			sql << "update OBJECTS set " <<
				"DETAIL_ID = " << detailId << " " <<
				"where OBJECT_ID = " << qry->result()->asString("OBJECT_ID");
			qry->exec(sql.str());
		}
		
		qry->next();
	}
}

unsigned int InsertAudioFile(CContentDatabase* pDb, unsigned int objectId, std::string p_sFileName, std::string* p_sTitle)
{
	metadata_t metadata;
	init_metadata(&metadata);
	
	if(!CFileDetails::Shared()->getMusicTrackDetails(p_sFileName, &metadata)) {
		free_metadata(&metadata);
	  return 0;
	}

  string sDlna; // = CFileDetails::Shared()->GuessDLNAProfileId(p_sFileName);
  fuppes_off_t fileSize = getFileSize(p_sFileName);
	
	unsigned int imgId = 0;
	string imgMimeType;
	if(metadata.has_image == 1) {
		imgId = objectId;
		imgMimeType = metadata.image_mime_type;
	}
	
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(A_ARTIST, A_ALBUM, A_TRACK_NO, A_GENRE, AV_DURATION, DATE, " <<
    "A_CHANNELS, AV_BITRATE, A_SAMPLERATE, " <<
		"ALBUM_ART_ID, ALBUM_ART_EXT, SIZE, DLNA_PROFILE) " <<
		"values (" <<
		"'" << SQLEscape(metadata.artist) << "', " <<
		"'" << SQLEscape(metadata.album) << "', " <<
		metadata.track_no << ", " <<
		"'" << SQLEscape(metadata.genre) << "', " <<
		"'" << metadata.duration << "', " <<
		"'" << "" << "', " <<
		metadata.channels << ", " <<
		metadata.bitrate << ", " <<
		metadata.samplerate << ", " <<
		imgId << ", " <<
		"'" << imgMimeType << "', " <<
    fileSize << ", " <<
    "'" << sDlna << "')";
		
  *p_sTitle = metadata.title;
	
	free_metadata(&metadata);	
  return pDb->Insert(sSql.str());
}

unsigned int InsertImageFile(CContentDatabase* pDb, std::string fileName)
{
  SImageItem ImageItem;
	if(!CFileDetails::Shared()->GetImageDetails(fileName, &ImageItem))
	  return 0;
	
  string dlna;
	string mimeType;
	string ext = ExtractFileExt(fileName);
	if(CPluginMgr::dlnaPlugin()) {
		CPluginMgr::dlnaPlugin()->getImageProfile(ext, ImageItem.nWidth, ImageItem.nHeight, &dlna, &mimeType);
	}
	
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(SIZE, IV_WIDTH, IV_HEIGHT, DATE, " <<
		"DLNA_PROFILE, DLNA_MIME_TYPE) " <<
		"values (" <<
		getFileSize(fileName) << ", " <<
		ImageItem.nWidth << ", " <<
		ImageItem.nHeight << ", " <<
   (ImageItem.sDate.empty() ? "NULL" : "'" + ImageItem.sDate + "'") << ", " <<
    "'" << dlna << "', " <<
		"'" << mimeType << "')";
	
  return pDb->Insert(sSql.str());  
} 

unsigned int InsertVideoFile(CContentDatabase* pDb, std::string p_sFileName)
{
  SVideoItem VideoItem;
	if(!CFileDetails::Shared()->GetVideoDetails(p_sFileName, &VideoItem))
	  return 0;  
  
  string sDlna; // = CFileDetails::Shared()->GuessDLNAProfileId(p_sFileName);
	VideoItem.nSize = getFileSize(p_sFileName);
	 
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(IV_WIDTH, IV_HEIGHT, AV_DURATION, SIZE, " <<
    "AV_BITRATE, A_CODEC, V_CODEC, DLNA_PROFILE) " <<
		"values (" <<
		VideoItem.nWidth << ", " <<
		VideoItem.nHeight << "," <<
		"'" << VideoItem.sDuration << "', " <<
		VideoItem.nSize << ", " <<
		VideoItem.nBitrate << ", " <<
    "'" << VideoItem.sACodec << "', " <<
    "'" << VideoItem.sVCodec << "', " <<
    "'" << sDlna << "');";
  
  return pDb->Insert(sSql.str());
} 

unsigned int InsertFile(CContentDatabase* pDb, unsigned int p_nParentId, std::string p_sFileName, bool hidden /* = false*/)
{
  unsigned int nObjId = 0;

  if(g_bAddNew) {
    nObjId = GetObjectIDFromFileName(pDb, p_sFileName);
    if(nObjId > 0) {
      return nObjId;
    }
  } 
  
  OBJECT_TYPE nObjectType = CFileDetails::Shared()->GetObjectType(p_sFileName);  
  if(nObjectType == OBJECT_TYPE_UNKNOWN)
    return false;
   
	nObjId = pDb->GetObjId();
	
  // we insert file details first to get the detail ID
  unsigned int nDetailId = 0;
  string title;
  switch(nObjectType)
  {
    case ITEM_AUDIO_ITEM:
    case ITEM_AUDIO_ITEM_MUSIC_TRACK:     
      nDetailId = InsertAudioFile(pDb, nObjId, p_sFileName, &title); 
      break;
    case ITEM_IMAGE_ITEM:
    case ITEM_IMAGE_ITEM_PHOTO:
      nDetailId = InsertImageFile(pDb, p_sFileName);
      break;
    case ITEM_VIDEO_ITEM:
    case ITEM_VIDEO_ITEM_MOVIE:
      nDetailId = InsertVideoFile(pDb, p_sFileName);
      break;

    default:
      break;
  }  

  // insert object  
  string path = ExtractFilePath(p_sFileName);
	string fileName = p_sFileName.substr(path.length(), p_sFileName.length() - path.length());
  if(title.empty()) {
    title = ToUTF8(fileName);
#warning make configurable
		title = StringReplace(title, "_", " ");
  }
  
  stringstream sSql;
  sSql << "insert into OBJECTS (" <<
    "  OBJECT_ID, DETAIL_ID, TYPE, " <<
    "  PATH, FILE_NAME, TITLE, HIDDEN) values (" <<
       nObjId << ", " <<
       nDetailId << ", " <<   
       nObjectType << ", " <<
       "'" << SQLEscape(path) << "', " <<
       "'" << SQLEscape(fileName) << "', " <<
       "'" << SQLEscape(title) << "', " <<
			 (hidden ? 1 : 0) << ")";

  pDb->Insert(sSql.str());  
  sSql.str("");
  
  sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
            "values (" << nObjId << ", " << p_nParentId << ")";  
  pDb->Insert(sSql.str());

	
	CSQLQuery* qry = CDatabase::query();
	string ext;

	switch(nObjectType) {
		
    case ITEM_AUDIO_ITEM:
    case ITEM_AUDIO_ITEM_MUSIC_TRACK: {
				unsigned int artId = findAlbumArt(path, &ext, qry);
				if(artId > 0) {
					setAlbumArtImage(path, fileName, artId, ext, qry);
				}
			}
			break;
    case ITEM_IMAGE_ITEM:
    case ITEM_IMAGE_ITEM_PHOTO: {
				if(CSharedConfig::isAlbumArtFile(fileName)) {
					//cout << "set image " << fileName << " as album art for all audio files in " << path << endl;
					ext = ExtractFileExt(p_sFileName);
		  		setAlbumArtImage(path, "", nObjId, ext, qry);
				}
			}
      break;
    default:
      break;
  }
	
	delete qry;
	return nObjId;         
}

unsigned int InsertURL(std::string p_sURL,
											 std::string p_sTitle = "",
											 std::string p_sMimeType = "")
{
	OBJECT_TYPE nObjectType = OBJECT_TYPE_UNKNOWN;
	#warning FIXME: object type
	nObjectType = ITEM_AUDIO_ITEM_AUDIO_BROADCAST;
	
	CContentDatabase* pDb = new CContentDatabase();          								 
	unsigned int nObjId = pDb->GetObjId();

  stringstream sSql;
  sSql << 
	"insert into OBJECTS (TYPE, OBJECT_ID, PATH, FILE_NAME, TITLE, MIME_TYPE) values " <<
  "(" << nObjectType << ", " <<
  nObjId << ", " <<
  "'" << SQLEscape(p_sURL) << "', " <<
  "'" << SQLEscape(p_sURL) << "', " <<
  "'" << SQLEscape(p_sTitle) << "', " <<  
  "'" << SQLEscape(p_sMimeType) << "');";

  pDb->Insert(sSql.str());
  delete pDb;
  return nObjId;
}

void BuildPlaylists()
{
	stringstream sGetPlaylists;
    sGetPlaylists << 
    "select     " << 
    "  *        " <<
    "from       " <<
    "  OBJECTS  " <<
    "where      " <<
    "  TYPE = " << CONTAINER_PLAYLIST_CONTAINER; 

	CSQLQuery* qry = CDatabase::query();
  qry->select(sGetPlaylists.str());
  while(!qry->eof()) {
    ParsePlaylist(qry->result());
    qry->next();
  }
  delete qry;
}
    
unsigned int GetObjectIDFromFileName(CContentDatabase* pDb, std::string p_sFileName)
{
  unsigned int nResult = 0;
  stringstream sSQL;
	
	string path = ExtractFilePath(p_sFileName);
	string fileName;
	if(path.length() < p_sFileName.length()) {
		fileName = p_sFileName.substr(path.length(), p_sFileName.length());
	}
	
  sSQL << 
		"select OBJECT_ID "
		"from OBJECTS "
		"where PATH = '" << SQLEscape(path) << "' ";
	
	if(fileName.empty())
		sSQL << " and FILE_NAME is NULL ";
	else
		sSQL << " and FILE_NAME = '" + SQLEscape(fileName) + "' ";
	
	sSQL <<
		"and DEVICE is NULL";
  
  pDb->Select(sSQL.str());
  if(!pDb->Eof())
    nResult = pDb->GetResult()->asUInt("OBJECT_ID");
  
  return nResult;
}

bool MapPlaylistItem(unsigned int p_nPlaylistID, unsigned int p_nItemID)
{
  CContentDatabase* pDB = new CContentDatabase();
  
  stringstream sSql;  
  sSql <<
    "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) values " <<
    "( " << p_nItemID <<
    ", " << p_nPlaylistID << ")";
  
  pDB->Insert(sSql.str());
  
  delete pDB;
	return true;
}
    

void ParsePlaylist(CSQLResult* pResult)
{
  CPlaylistParser Parser;
  if(!Parser.LoadPlaylist(pResult->asString("PATH") + pResult->asString("FILE_NAME"))) {
    return;
  }

  unsigned int nPlaylistID = pResult->asUInt("OBJECT_ID");
  unsigned int nObjectID   = 0;  

	//cout << "playlist id: " << nPlaylistID << endl;
		
  CContentDatabase* pDb = new CContentDatabase();
  
  while(!Parser.Eof()) {
    if(Parser.Entry()->bIsLocalFile && FileExists(Parser.Entry()->sFileName)) {       
            
      nObjectID = GetObjectIDFromFileName(pDb, Parser.Entry()->sFileName);
      
      if(nObjectID == 0) {        
        nObjectID = InsertFile(pDb, nPlaylistID, Parser.Entry()->sFileName);
      }            
      else {
        MapPlaylistItem(nPlaylistID, nObjectID);
      }
    }
    else if(!Parser.Entry()->bIsLocalFile) {
      nObjectID = InsertURL(Parser.Entry()->sFileName, Parser.Entry()->sTitle, Parser.Entry()->sMimeType);
			MapPlaylistItem(nPlaylistID, nObjectID);
    }    
    
    Parser.Next();
  }
  
  delete pDb;
}

std::string ReadFile(std::string p_sFileName)
{
  fstream fsPlaylist;
  char*   szBuf;
  long    nSize;  
  string  sResult;
  
  fsPlaylist.open(p_sFileName.c_str(), ios::in);
  if(fsPlaylist.fail())
    return ""; 
   
  fsPlaylist.seekg(0, ios::end); 
  nSize = streamoff(fsPlaylist.tellg()); 
  fsPlaylist.seekg(0, ios::beg);
  szBuf = new char[nSize + 1];  
  fsPlaylist.read(szBuf, nSize); 
  szBuf[nSize] = '\0';
  fsPlaylist.close();
    
  sResult = szBuf;
  delete[] szBuf;  
   
  return sResult;
}

//fuppesThreadCallback BuildLoop(void* arg)
void RebuildThread::run()
{  
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

  CSharedLog::Print("[ContentDatabase] create database at %s", sNowtime.c_str());

	CSQLQuery* qry = CDatabase::query();
  stringstream sSql;
		
  if(g_bFullRebuild) {
    qry->exec("delete from OBJECTS");
    qry->exec("delete from OBJECT_DETAILS");
    qry->exec("delete from MAP_OBJECTS");
  }

	/*pDb->Execute("drop index IDX_OBJECTS_OBJECT_ID");
	pDb->Execute("drop index IDX_MAP_OBJECTS_OBJECT_ID");
	pDb->Execute("drop index IDX_MAP_OBJECTS_PARENT_ID");
	pDb->Execute("drop index IDX_OBJECTS_DETAIL_ID");
	pDb->Execute("drop index IDX_OBJECT_DETAILS_ID");*/

	if(!g_bFullRebuild && g_bRemoveMissing) {
	
		CSharedLog::Print("remove missing");		
		//CContentDatabase* pDel = new CContentDatabase();
		CSQLQuery* del = CDatabase::query();
		
		qry->select("select * from OBJECTS");
		while(!qry->eof()) {
			CSQLResult* result = qry->result();

			if(result->asUInt("TYPE") < ITEM) {
				if(DirectoryExists(result->asString("PATH"))) {
					qry->next();
					continue;
				}
			}
			else {
				if(FileExists(result->asString("PATH") + result->asString("FILE_NAME"))) {
					qry->next();
					continue;
				}
			}
				
			sSql << "delete from OBJECT_DETAILS where ID = " << result->asString("OBJECT_ID");
			del->exec(sSql.str());
			sSql.str("");
			
			sSql << "delete from MAP_OBJECTS where OBJECT_ID = " << result->asString("OBJECT_ID");
			del->exec(sSql.str());
			sSql.str("");
				
			sSql << "delete from OBJECTS where OBJECT_ID = " << result->asString("OBJECT_ID");
			del->exec(sSql.str());
			sSql.str("");
			
			qry->next();
		}

		delete del;
		CSharedLog::Print("[DONE] remove missing");		
	}
		
  qry->exec("vacuum");
  
  int i;
  unsigned int nObjId;
  string sFileName;
  bool bInsert = true;
  
  CSharedLog::Print("read shared directories");

	CContentDatabase* db = new CContentDatabase();
	
  for(i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++)
  {
    if(DirectoryExists(CSharedConfig::Shared()->GetSharedDir(i)))
    { 	
			db->fileAlterationMonitor()->addWatch(CSharedConfig::Shared()->GetSharedDir(i));
      
      ExtractFolderFromPath(CSharedConfig::Shared()->GetSharedDir(i), &sFileName);
      bInsert = true;
      if(g_bAddNew) {
        if((nObjId = GetObjectIDFromFileName(db, CSharedConfig::Shared()->GetSharedDir(i))) > 0) {
          bInsert = false;
        }
      }

      sSql.str("");
      if(bInsert) {      
        nObjId = db->GetObjId();      
      
				sFileName = ToUTF8(sFileName);
				
        sSql << 
          "insert into OBJECTS (OBJECT_ID, TYPE, PATH, TITLE) values " <<
          "(" << nObjId << 
          ", " << CONTAINER_STORAGE_FOLDER << 
          ", '" << SQLEscape(CSharedConfig::Shared()->GetSharedDir(i)) << "'" <<
          ", '" << SQLEscape(sFileName) << "');";
        
        qry->insert(sSql.str());
    
        sSql.str("");
        sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
          "values (" << nObjId << ", 0)";
      
        qry->insert(sSql.str());
      }
      
      DbScanDir(db, CSharedConfig::Shared()->GetSharedDir(i), nObjId);
    }
    else {      
      CSharedLog::Log(L_EXT, __FILE__, __LINE__,
        "shared directory: \" %s \" not found", CSharedConfig::Shared()->GetSharedDir(i).c_str());
    }
  } // for
  CSharedLog::Print("[DONE] read shared directories");
 
	/*if( !pDb->Execute("CREATE INDEX IDX_OBJECTS_OBJECT_ID ON OBJECTS(OBJECT_ID);") )
		CSharedLog::Shared()->Log(L_NORMAL, "Create index failed", __FILE__, __LINE__, false);
	if( !pDb->Execute("CREATE INDEX IDX_MAP_OBJECTS_OBJECT_ID ON MAP_OBJECTS(OBJECT_ID);") )
		CSharedLog::Shared()->Log(L_NORMAL, "Create index failed", __FILE__, __LINE__, false);
	if( !pDb->Execute("CREATE INDEX IDX_MAP_OBJECTS_PARENT_ID ON MAP_OBJECTS(PARENT_ID);") )
		CSharedLog::Shared()->Log(L_NORMAL, "Create index failed", __FILE__, __LINE__, false);
	if( !pDb->Execute("CREATE INDEX IDX_OBJECTS_DETAIL_ID ON OBJECTS(DETAIL_ID);") )
		CSharedLog::Shared()->Log(L_NORMAL, "Create index failed", __FILE__, __LINE__, false);
	if( !pDb->Execute("CREATE INDEX IDX_OBJECT_DETAILS_ID ON OBJECT_DETAILS(ID);") )
		CSharedLog::Shared()->Log(L_NORMAL, "Create index failed", __FILE__, __LINE__, false);*/
	
  
  CSharedLog::Print("parse playlists");
  BuildPlaylists();
  CSharedLog::Print("[DONE] parse playlists");
    
  delete db;
	delete qry;
  
  
  // import iTunes db
  CSharedLog::Print("parse iTunes databases");
  CiTunesImporter* pITunes = new CiTunesImporter();
  for(i = 0; i < CSharedConfig::Shared()->SharedITunesCount(); i++) {
    pITunes->Import(CSharedConfig::Shared()->GetSharedITunes(i));
  }
  delete pITunes;
  CSharedLog::Print("[DONE] parse iTunes databases");
  	
	#ifndef WIN32
  time(&now);
  ctime_r(&now, nowtime);
	nowtime[24] = '\0';
	sNowtime = nowtime;
	#else	  
  _strtime(timeStr);	
	sNowtime = timeStr;	
	#endif
  CSharedLog::Print("[ContentDatabase] database created at %s", sNowtime.c_str());

  g_bIsRebuilding = false;
  //fuppesThreadExit();
}


void CContentDatabase::FamEvent(CFileAlterationEvent* event)
{
  if(g_bIsRebuilding)
    return;
  
  //cout << "[ContentDatabase] fam event: ";
  
  switch(event->type()) {
    case FAM_CREATE:
      //cout << "CREATE";
      break;
    case FAM_DELETE:
      //cout << "DELETE";
      break;
    case FAM_MOVE:
      //cout << "MOVE - " << event->oldFullPath();
      break;
    case FAM_MODIFY:
      //cout << "MODIFY";
      break;
    default:
      //cout << "UNKNOWN";
      break;
  }
  
  //cout << (event->isDir() ? " DIR " : " FILE ") << endl;
  //cout << event->fullPath() << endl << endl;
  
  stringstream sSql;
	unsigned int objId;
	unsigned int parentId;  
  
  // newly created or moved in directory
	if((event->type() == FAM_CREATE || event->type() == (FAM_CREATE | FAM_MOVE)) && event->isDir()) {

		objId = GetObjId();
		parentId = GetObjectIDFromFileName(this, event->path());
		string path = appendTrailingSlash(event->fullPath());

    sSql << 
          "insert into OBJECTS (OBJECT_ID, TYPE, PATH, TITLE) values " <<
          "(" << objId << 
          ", " << CONTAINER_STORAGE_FOLDER << 
          ", '" << SQLEscape(path) << "'" <<
          ", '" << SQLEscape(event->file()) << "');";        
    Insert(sSql.str());
    
		//cout << "SQL INSERT NEW DIR: " << sSql.str() << endl;
		
    sSql.str("");
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
          "values (" << objId << ", " << parentId << ")";      
    Insert(sSql.str());
    
		// moved in from outside the watches dirs
    if(event->type() == (FAM_CREATE | FAM_MOVE)) {
      //cout << "scan moved in" << endl;
      DbScanDir(this, appendTrailingSlash(event->fullPath()), objId);
    }
	} // new or moved directory
  
	// directory deleted
  else if(event->type() == FAM_DELETE && event->isDir()) {    
    deleteContainer(event->fullPath());    
  } // directory deleted  
  
	// directory moved/renamed
  else if(event->type() == FAM_MOVE && event->isDir()) {        
     
    // update moved folder
    objId = GetObjectIDFromFileName(this, appendTrailingSlash(event->oldFullPath()));
		
		//cout << "OBJID: " << objId << " " << appendTrailingSlash(event->oldFullPath()) << endl;
		
		// update path
    sSql << 
      "update OBJECTS set " <<
      " PATH = '" << SQLEscape(appendTrailingSlash(event->fullPath())) << "', "
      " TITLE = '" << SQLEscape(event->file()) << "' " <<
      "where OBJECT_ID = " << objId;
      
    //cout << sSql.str() << endl;
    Execute(sSql.str());
    sSql.str("");
		
		// move fam watch
		m_pFileAlterationMonitor->moveWatch(event->oldFullPath(), event->fullPath());
		
    // update mapping
    parentId = GetObjectIDFromFileName(this, event->path());
    
    sSql << 
      "update MAP_OBJECTS set " <<
      " PARENT_ID = " << parentId << " "
      "where OBJECT_ID = " << objId << " and DEVICE is NULL";
      
    //cout << sSql.str() << endl;
    Execute(sSql.str());
    sSql.str("");

    // update child object's path
    sSql << "select ID, PATH from OBJECTS where PATH like '" << SQLEscape(appendTrailingSlash(event->oldFullPath())) << "%' and DEVICE is NULL";

		CSQLQuery* qry = CDatabase::query();
		qry->select(sSql.str());
		sSql.str("");

    string newPath;  
    CContentDatabase db; 

		while(!qry->eof()) {
			CSQLResult* result = qry->result();

      newPath = StringReplace(result->asString("PATH"), appendTrailingSlash(event->oldFullPath()), appendTrailingSlash(event->fullPath()));

#warning move fam watch
			//m_pFileAlterationMonitor->moveWatch();
			
      #warning sql prepare
      sSql << 
        "update OBJECTS set " <<
        " PATH = '" << SQLEscape(newPath) << "' " <<
        "where ID = " << result->asString("ID");
      
      //cout << sSql.str() << endl;
      db.Execute(sSql.str());
      sSql.str("");

			qry->next();
		}   
		delete qry;    
  } // directory moved/renamed  
 
  
  
  // file created
  else if(event->type() == FAM_CREATE && !event->isDir()) { 
    //cout << "FAM_FILE_NEW: " << path << " name: " << name << endl;
        
    parentId = GetObjectIDFromFileName(this, event->path());
    InsertFile(this, parentId, event->fullPath());    
  } // file created

	// file deleted
  else if(event->type() == FAM_DELETE && !event->isDir()) { 
    //cout << "FAM_FILE_DEL: " << path << " name: " << name << endl;
    
    objId = GetObjectIDFromFileName(this, event->fullPath());
    deleteObject(objId);    
  } // file deleted 
  
	// file moved
  else if(event->type() == FAM_MOVE && !event->isDir()) { 
    //cout << "FAM_FILE_MOVE: " << path << " name: " << name << " old: " << oldPath << " - " << oldName << endl;
        
    objId = GetObjectIDFromFileName(this, event->oldFullPath());
    parentId = GetObjectIDFromFileName(this, event->path());
    
    // update mapping
    sSql << 
        "update MAP_OBJECTS set " <<
        "  PARENT_ID = " << parentId << " " <<        
        "where OBJECT_ID = " << objId << " and DEVICE is NULL";
    Execute(sSql.str());
    sSql.str("");
    
    // update object
    sSql << "update OBJECTS set " <<
        "PATH = '" << SQLEscape(event->path()) << "', " <<
        "FILE_NAME = '" << SQLEscape(event->file()) << "', " <<
        "TITLE = '" << SQLEscape(event->file()) << "' " <<
        "where ID = " << objId;
    //cout << sSql.str() << endl;
    Execute(sSql.str());
    sSql.str("");

  } // file moved
  
  // file modified
  else if(event->type() == FAM_MOVE && !event->isDir()) {
    //cout << "file modified" << endl;
  }
  
}

void CContentDatabase::deleteObject(unsigned int objectId)
{
  stringstream sql;
	CSQLQuery* qry = CDatabase::query();
	
  // get type
  sql << "select TYPE, PATH from OBJECTS where OBJECT_ID = " << objectId << " and DEVICE is NULL";
	qry->select(sql.str());
	sql.str("");
    
  string objects;
	if(qry->eof()) {
		delete qry;
	  return;
	}

	CSQLResult* result = qry->result();

  OBJECT_TYPE type = (OBJECT_TYPE)result->asUInt("TYPE");
  if(type < ITEM) { // is a container
		fileAlterationMonitor()->removeWatch(result->asString("PATH"));
    deleteContainer(result->asString("PATH"));    
  }
  else {  
    //cout << "contentdb: delete object : " << result->asString("PATH") << endl;  

		// delete details    
    sql << "select DETAIL_ID from OBJECTS where OBJECT_ID = " << objectId;
		qry->select(sql.str());
		sql.str("");

		if(!qry->eof() && !qry->result()->isNull("DETAIL_ID")) {      
      sql << "delete from OBJECT_DETAILS where ID = " << qry->result()->asString("DETAIL_ID");
      qry->exec(sql.str());
      sql.str("");
		}
    
    // delete mapping
    sql << "delete from MAP_OBJECTS where OBJECT_ID = " << objectId;
		qry->exec(sql.str());
    sql.str("");
    
    // delete object
    sql << "delete from OBJECTS where OBJECT_ID = " << objectId;
		qry->exec(sql.str());
    sql.str("");
  }

	delete qry;
}

void CContentDatabase::deleteContainer(std::string path)
{
  stringstream sSql;
	CSQLQuery* qry = CDatabase::query();
	
  // delete object details
  sSql << 
    "select DETAIL_ID from OBJECTS where PATH like '" <<
    SQLEscape(appendTrailingSlash(path)) << "%' and DEVICE is NULL";

	qry->select(sSql.str());
	sSql.str("");
	
  string details;
	while(!qry->eof()) {
		
    if(!qry->result()->isNull("DETAIL_ID")) {
      details += qry->result()->asString("DETAIL_ID") + ", ";
    }
    qry->next();
	}
  
  if(details.length() > 0) {
    details = details.substr(0, details.length() -2);
    
    sSql << "delete from OBJECT_DETAILS where ID in (" << details << ")";
    //cout << sSql.str() << endl;
    qry->exec(sSql.str());
    sSql.str("");
  }
  
  // delete mappings
  sSql << "select OBJECT_ID from OBJECTS where PATH like '" << SQLEscape(appendTrailingSlash(path)) << "%' and DEVICE is NULL";		
	qry->select(sSql.str());
	sSql.str("");
  
  string objects;
	while(!qry->eof()) {
    objects += qry->result()->asString("OBJECT_ID") + ", ";
    qry->next();
	}
  
  if(objects.length() > 0) {
    objects = objects.substr(0, objects.length() -2);
    qry->exec("delete from MAP_OBJECTS where OBJECT_ID in (" + objects + ")");
  }
  
#warning todo: remove fam watches
  
  // delete objects
  sSql << "delete from OBJECTS where PATH like '" << SQLEscape(appendTrailingSlash(path)) << "%'";
  qry->exec(sSql.str());
  sSql.str("");

	delete qry;
}
