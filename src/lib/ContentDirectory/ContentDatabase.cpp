/***************************************************************************
 *            ContentDatabase.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

std::string CSelectResult::GetValue(std::string p_sFieldName)
{
  return m_FieldValues[p_sFieldName];
}

bool CSelectResult::IsNull(std::string p_sFieldName)
{
  std::string sValue = GetValue(p_sFieldName);
  if((sValue.length() == 0) || (sValue.compare("NULL") == 0))
    return true;
  else
    return false;
}

unsigned int CSelectResult::GetValueAsUInt(std::string p_sFieldName)
{
  unsigned int nResult = 0;
  if(!IsNull(p_sFieldName)) {
    nResult = strtoul(GetValue(p_sFieldName).c_str(), NULL, 0);
  }
  return nResult;
}

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
	m_pDbHandle 			= NULL;
	m_nRowsReturned 	= 0;  
  m_RebuildThread 	= (fuppesThread)NULL;	
  m_bShared       	= p_bShared;	
  m_bInTransaction 	= false;
  m_nLockCount 			= -1;
  
	if(m_bShared) {            
		m_sDbFileName 	  = CSharedConfig::Shared()->GetDbFileName();	
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
    if(m_pFileAlterationMonitor) {
      delete m_pFileAlterationMonitor;
    }
  }
	
	if(m_bShared && (m_RebuildThread != (fuppesThread)NULL)) {
	  fuppesThreadClose(m_RebuildThread);
		m_RebuildThread = (fuppesThread)NULL;
	}

  ClearResult();  
  Close();
}

std::string CContentDatabase::GetLibVersion()
{
  return sqlite3_libversion();
}

bool CContentDatabase::Init(bool* p_bIsNewDB)
{   
  if(!m_bShared) {
		return false;
	}
  
  bool bIsNewDb = !FileExists(m_sDbFileName);
  *p_bIsNewDB = bIsNewDb;  
  
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
				"  REF_ID INTEGER DEFAULT NULL "
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
        "  A_CODEC, "
        "  V_CODEC, "
        "  SIZE INTEGER DEFAULT 0, "
				"  DLNA_PROFILE TEXT DEFAULT NULL"
				");"))   
			return false;    
    
    if(!Execute("CREATE TABLE MAP_OBJECTS ( "
        "  ID INTEGER PRIMARY KEY AUTOINCREMENT, "
        "  OBJECT_ID INTEGER NOT NULL, "
        "  PARENT_ID INTEGER NOT NULL, "
        "  DEVICE TEXT "
        ");"))
      return false;               
    

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
  }
	
  Execute("pragma temp_store = MEMORY");
  Execute("pragma synchronous = OFF;");  
  
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

void CContentDatabase::ClearResult()
{
  // clear old results
  for(m_ResultListIterator = m_ResultList.begin(); m_ResultListIterator != m_ResultList.end();)
  {
    if(m_ResultList.empty())
      break;
    
    CSelectResult* pResult = *m_ResultListIterator;
    std::list<CSelectResult*>::iterator tmpIt = m_ResultListIterator;          
    ++tmpIt;
    m_ResultList.erase(m_ResultListIterator);
    m_ResultListIterator = tmpIt;
    delete pResult;
  } 
  
  m_ResultList.clear();
  m_nRowsReturned = 0;
}

bool CContentDatabase::Open()
{  
  if(!m_bShared) {	
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
	
	return true;
}

void CContentDatabase::Close()
{
  if(m_bShared) {
    sqlite3_close(m_pDbHandle);
    m_pDbHandle = NULL;
	}
}

void CContentDatabase::BeginTransaction()
{
  if(m_bInTransaction) {
    return;
  }
  
  Lock();  
  char* szErr = 0;
  
  if(sqlite3_exec(m_pDbHandle, "BEGIN TRANSACTION;", NULL, NULL, &szErr) != SQLITE_OK) {
    fprintf(stderr, "CContentDatabase::BeginTransaction() :: SQL error: %s\n", szErr);
  }
  else {
    m_bInTransaction = true;
  }

  Unlock();
}

void CContentDatabase::Commit()
{
  if(!m_bInTransaction) {
    return;
  }
  
  Lock();
  char* szErr = 0;  
  
  if(sqlite3_exec(m_pDbHandle, "COMMIT;", NULL, NULL, &szErr) != SQLITE_OK) {
    fprintf(stderr, "CContentDatabase::Commit() :: SQL error: %s\n", szErr);
  }
  else {
    m_bInTransaction = false;
  }
  
  Unlock();  
}

unsigned int CContentDatabase::Insert(std::string p_sStatement)
{  
  bool bTransaction = false;
  
  if(!m_bInTransaction) {
    bTransaction = true;
    BeginTransaction();
  }

  Lock();
  
  char* szErr  = NULL;
  bool  bRetry = true;
  int   nResult;
    
  while(bRetry) {  
    
    nResult = sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), NULL, NULL, &szErr);  
    switch(nResult) {
      case SQLITE_BUSY:
        bRetry = true;
        fuppesSleep(50);
        break;
      
      case SQLITE_OK:
        bRetry  = false;
        nResult = sqlite3_last_insert_rowid(m_pDbHandle);
        break;
      
      default:
        bRetry = false;
        CSharedLog::Log(L_NORM, __FILE__, __LINE__, "CContentDatabase::Insert - insert :: SQL error: %s\nStatement: %s", szErr, p_sStatement.c_str());
        sqlite3_free(szErr);
        nResult = 0;
        break;
    }
    
  }
   
  Unlock();
  
  if(bTransaction) {
    Commit();
  }
  
  return nResult;  
}

bool CContentDatabase::Execute(std::string p_sStatement)
{
  Lock();
  //Open();
	bool bResult = false;
  char* szErr  = NULL;
	
  int nStat = sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), NULL, NULL, &szErr);  
  if(nStat != SQLITE_OK) {
    fprintf(stderr, "CContentDatabase::Execute :: SQL error: %s\n", szErr);  
    sqlite3_free(szErr);  
    bResult = false;
  }
  else {
    bResult = true;
  }
	
	//Close();
	Unlock();
	return bResult;
}

bool CContentDatabase::Select(std::string p_sStatement)
{  
  Lock();
  //Open();  
  ClearResult();    
  bool bResult = true;
  
  char* szErr = NULL;
  char** szResult;
  int nRows = 0;
  int nCols = 0;
  
  int nResult = SQLITE_OK;  
  int nTry = 0;
  
  CSharedLog::Log(L_DBG, __FILE__, __LINE__, "SELECT %s", p_sStatement.c_str());
  
  do {
    nResult = sqlite3_get_table(m_pDbHandle, p_sStatement.c_str(), &szResult, &nRows, &nCols, &szErr);   
    if(nTry > 0) {      
      //CSharedLog::Shared()->Log(L_EXTENDED_WARN, "SQLITE_BUSY", __FILE__, __LINE__);
      fuppesSleep(100);
    }
    nTry++;
  }while(nResult == SQLITE_BUSY);
    
  if(nResult != SQLITE_OK) {    
    CSharedLog::Log(L_DBG, __FILE__, __LINE__, "SQL error: %s, Statement: %s\n", szErr, p_sStatement.c_str());
    sqlite3_free(szErr);
    bResult = false;
  }
  else {

   CSelectResult* pResult;
       
    for(int i = 1; i < nRows + 1; i++) {
      pResult = new CSelectResult();
            
      for(int j = 0; j < nCols; j++) {        
        pResult->m_FieldValues[szResult[j]] =  szResult[(i * nCols) + j] ? szResult[(i * nCols) + j] : "NULL";
      }
      
      m_ResultList.push_back(pResult);
      m_nRowsReturned++;
    }
    m_ResultListIterator = m_ResultList.begin();       
       
    sqlite3_free_table(szResult);
  }
	Unlock();
	
  return bResult;
}

bool CContentDatabase::Eof()
{
  return (m_ResultListIterator == m_ResultList.end());
}
    
CSelectResult* CContentDatabase::GetResult()
{
  return *m_ResultListIterator;
}

void CContentDatabase::Next()
{
  if(m_ResultListIterator != m_ResultList.end()) {
    m_ResultListIterator++;
  } 
}



fuppesThreadCallback BuildLoop(void *arg);
void DbScanDir(std::string p_sDirectory, long long int p_nParentId);
void BuildPlaylists();
void ParsePlaylist(CSelectResult* pResult);
void ParseM3UPlaylist(CSelectResult* pResult);
void ParsePLSPlaylist(CSelectResult* pResult);


unsigned int InsertFile(CContentDatabase* pDb, unsigned int p_nParentId, std::string p_sFileName);
unsigned int InsertURL(unsigned int p_nParentId, std::string p_sURL);

unsigned int GetObjectIDFromFileName(CContentDatabase* pDb, std::string p_sFileName);

void CContentDatabase::BuildDB()
{     
  if(CContentDatabase::Shared()->IsRebuilding())
	  return;
	
	if(!m_bShared)
	  return;
		
	if(m_RebuildThread != (fuppesThread)NULL) {
	  fuppesThreadClose(m_RebuildThread);
		m_RebuildThread = (fuppesThread)NULL;
	}
		
	fuppesThreadStart(m_RebuildThread, BuildLoop);
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
    
  Select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
  if(!Eof()) {  
    g_nObjId = GetResult()->GetValueAsUInt("VALUE");
  }
  ClearResult();
  
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
  
  Select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
  if(!Eof()) {  
    g_nObjId = GetResult()->GetValueAsUInt("VALUE");
  } 
  ClearResult();
  
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
  // append trailing slash if neccessary
  if(p_sDirectory.substr(p_sDirectory.length()-1).compare(upnpPathDelim) != 0) {
    p_sDirectory += upnpPathDelim;
  }    
     
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
          
          if(g_bAddNew) {
            nObjId = GetObjectIDFromFileName(pDb, sTmp);
          }
          
          if(nObjId == 0) {
            nObjId = pDb->GetObjId();          
            pDb->BeginTransaction();          
                
            sSql << "insert into objects ( " <<
              "  OBJECT_ID, TYPE, " <<
              "  PATH, FILE_NAME, TITLE) " <<
              "values ( " << 
                 nObjId << ", " <<  
                 CONTAINER_STORAGE_FOLDER << 
                 ", '" << SQLEscape(sTmp) << "', '" <<
                 sTmpFileName << "', '" <<
                 sTmpFileName <<
              "');";

            pDb->Insert(sSql.str());          
            
            sSql.str("");
            sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
                    "values (" << nObjId << ", " << p_nParentId << ")";          
            pDb->Insert(sSql.str());
            
            pDb->Commit();
          }
            
          // recursively scan subdirectories
          DbScanDir(pDb, sTmp, nObjId);          
        }
        else if(IsFile(sTmp) && CFileDetails::Shared()->IsSupportedFileExtension(sExt))
        {
          InsertFile(pDb, p_nParentId, sTmp);
        }         
       
      }
    }  /* while */  
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif         
}

unsigned int InsertAudioFile(CContentDatabase* pDb, std::string p_sFileName, std::string* p_sTitle)
{
	SAudioItem TrackInfo; 
	if(!CFileDetails::Shared()->GetMusicTrackDetails(p_sFileName, &TrackInfo))
	  return 0;
		
  string sDlna = CFileDetails::Shared()->GuessDLNAProfileId(p_sFileName);
  
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(A_ARTIST, A_ALBUM, A_TRACK_NO, A_GENRE, AV_DURATION, DATE, " <<
    "A_CHANNELS, AV_BITRATE, A_SAMPLERATE, SIZE, DLNA_PROFILE) " <<
		"values (" <<
		//"'" << SQLEscape(TrackInfo.mAudioItem.sTitle) << "', " <<
		"'" << SQLEscape(TrackInfo.sArtist) << "', " <<
		"'" << SQLEscape(TrackInfo.sAlbum) << "', " <<
		TrackInfo.nOriginalTrackNumber << ", " <<
		"'" << SQLEscape(TrackInfo.sGenre) << "', " <<
		"'" << TrackInfo.sDuration << "', " <<
		"'" << TrackInfo.sDate << "', " <<
		TrackInfo.nNrAudioChannels << ", " <<
		TrackInfo.nBitrate << ", " <<
		TrackInfo.nSampleRate << ", " <<
    TrackInfo.nSize << ", " <<
    "'" << sDlna << "')";
		
	//cout << sSql.str() << endl;
  *p_sTitle = TrackInfo.sTitle;
	
  return pDb->Insert(sSql.str());
}

unsigned int InsertImageFile(CContentDatabase* pDb, std::string p_sFileName)
{
  SImageItem ImageItem;
	if(!CFileDetails::Shared()->GetImageDetails(p_sFileName, &ImageItem))
	  return 0;
	
  string sDlna = CFileDetails::Shared()->GuessDLNAProfileId(p_sFileName);
  
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(IV_WIDTH, IV_HEIGHT, DLNA_PROFILE) " <<
		"values (" <<
		ImageItem.nWidth << ", " <<
		ImageItem.nHeight << ", " <<
    "'" << sDlna << "')";
	
	//cout << sSql.str() << endl;
	
  return pDb->Insert(sSql.str());  
} 

unsigned int InsertVideoFile(CContentDatabase* pDb, std::string p_sFileName)
{
  SVideoItem VideoItem;
	if(!CFileDetails::Shared()->GetVideoDetails(p_sFileName, &VideoItem))
	  return 0;  
  
  string sDlna = CFileDetails::Shared()->GuessDLNAProfileId(p_sFileName);
    
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

unsigned int InsertFile(CContentDatabase* pDb, unsigned int p_nParentId, std::string p_sFileName)
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
    
  // we insert file details first to get the detail ID
  unsigned int nDetailId = 0;
  string sTitle;
  switch(nObjectType)
  {
    case ITEM_AUDIO_ITEM:
    case ITEM_AUDIO_ITEM_MUSIC_TRACK:     
      nDetailId = InsertAudioFile(pDb, p_sFileName, &sTitle); 
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
  string sTmpFileName =  p_sFileName;
  
  // format file name
  int nPathLen = ExtractFilePath(sTmpFileName).length();
  sTmpFileName = sTmpFileName.substr(nPathLen, sTmpFileName.length() - nPathLen);  
  sTmpFileName = ToUTF8(sTmpFileName);
  sTmpFileName = SQLEscape(sTmpFileName);  
  
  if(!sTitle.empty()) {
    sTitle = SQLEscape(sTitle);
  }
  else {
    sTitle = sTmpFileName;
  }
  
  
  nObjId = pDb->GetObjId();
  
  stringstream sSql;
  sSql << "insert into objects (" <<
    "  OBJECT_ID, DETAIL_ID, TYPE, " <<
    "  PATH, FILE_NAME, TITLE) values (" <<
       nObjId << ", " <<
       nDetailId << ", " <<   
       nObjectType << ", " <<
       "'" << SQLEscape(p_sFileName) << "', " << 
       "'" << sTmpFileName << "', " << 
       "'" << sTitle << "')";
               
  pDb->Insert(sSql.str());  
  sSql.str("");
  
  sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
            "values (" << nObjId << ", " << p_nParentId << ")";  
  pDb->Insert(sSql.str());

	return nObjId;         
}

unsigned int InsertURL(unsigned int p_nParentId, std::string p_sURL)
{
  /*stringstream sSql;
  sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME, MD5, MIME_TYPE, DETAILS) values ";
  //sSql << "(" << ITEM_VIDEO_ITEM_VIDEO_BROADCAST << ", ";
  sSql << "(" << ITEM_AUDIO_ITEM_MUSIC_TRACK << ", ";
  //sSql << "(" <<  << ", ";
  
  sSql << p_nParentId << ", ";
  sSql << "'" << SQLEscape(p_sURL) << "', ";
  sSql << "'" << p_sURL << "', ";
  //sSql << "'" << MD5Sum(sTmp.str()) << "', ";
  sSql << "'" << "todo" << "', ";
  sSql << "'" << "audio/mpeg" << "', ";
  sSql << "'" << "details - todo" << "');";
  
  #warning FIXME: mime type
  
  CContentDatabase* pDB = new CContentDatabase();          
  unsigned int nRowId = pDB->Insert(sSql.str());
  delete pDB;
  return nRowId;*/
}

void BuildPlaylists()
{
  CContentDatabase* pDb = new CContentDatabase();
	
	stringstream sGetPlaylists;
    sGetPlaylists << 
    "select     " << 
    "  *        " <<
    "from       " <<
    "  OBJECTS  " <<
    "where      " <<
    "  TYPE = " << CONTAINER_PLAYLIST_CONTAINER; 
  
  if(!pDb->Select(sGetPlaylists.str())) {
	  delete pDb;
    return;
  }
  
  CSelectResult* pResult = NULL;
  while(!pDb->Eof()) {
    pResult = pDb->GetResult();    
    ParsePlaylist(pResult);    
    pDb->Next();
  }

  delete pDb;
}
    
unsigned int GetObjectIDFromFileName(CContentDatabase* pDb, std::string p_sFileName)
{
  unsigned int nResult = 0;
  stringstream sSQL;
  sSQL << "select OBJECT_ID from OBJECTS where PATH = '" << SQLEscape(p_sFileName) << "' and DEVICE is NULL";
  
  pDb->Select(sSQL.str());
  if(!pDb->Eof())
    nResult = atoi(pDb->GetResult()->GetValue("OBJECT_ID").c_str());   
  
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
    

void ParsePlaylist(CSelectResult* pResult)
{
  CPlaylistParser Parser;
  if(!Parser.LoadPlaylist(pResult->GetValue("PATH"))) {
    return;
  }
    
  unsigned int nPlaylistID = pResult->GetValueAsUInt("OBJECT_ID");
  unsigned int nObjectID   = 0;  
   
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
      nObjectID = InsertURL(nPlaylistID, Parser.Entry()->sFileName);
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

void ParsePLSPlaylist(CSelectResult* pResult)
{
  std::string sContent = ReadFile(pResult->GetValue("PATH"));
  RegEx rxNumber("NumberOfEntries=(\\d+)", PCRE_CASELESS);
  if(!rxNumber.Search(sContent.c_str()))
    return;
  
  CContentDatabase* pDb = new CContentDatabase();

  //cout << "parse pls" << endl;
  
  int nEntryCount = atoi(rxNumber.Match(1));  
  for(int i = 0; i < nEntryCount; i++)
  {
    stringstream sExpr;
    sExpr << "File" << i + 1 << "=(.+)\\n";    
    RegEx rxFile(sExpr.str().c_str(), PCRE_CASELESS);
    if(!rxFile.Search(sContent.c_str()))
      continue;
    
    string sFileName = rxFile.Match(1);
    //cout << "FILE: " << rxFile.Match(1) << endl;
    if(sFileName.length() == 0)
      continue;
    
    //cout << sFileName << endl;
        
    // relative or absolute file name 
    bool bIsLocalFile = true;
    unsigned int nObjectID = 0;
    unsigned int nPlaylistID = pResult->GetValueAsUInt("OBJECT_ID");
    
    if(sFileName.substr(0, 1).compare(upnpPathDelim) == 0)
    {
      //cout << "absolute" << endl;
      bIsLocalFile = true;
    }
    else
    {
      //cout << "relative or url" << endl;
      RegEx rxUrl("\\w+://", PCRE_CASELESS);
      if(rxUrl.Search(sFileName.c_str()))
      {
        //cout << "URL" << endl;
        bIsLocalFile = false;        
        nObjectID = InsertURL(nPlaylistID, sFileName);        
      }
      else
      {
        bIsLocalFile = true;
        //cout << "relative" << endl;
        
        sFileName = ExtractFilePath(pResult->GetValue("PATH")) + sFileName;
        //cout << sFileName << endl;        
      }
    
    }
    
    if(bIsLocalFile && FileExists(sFileName))
    {
      nObjectID = GetObjectIDFromFileName(pDb, sFileName);      
      if(nObjectID == 0) {
        //cout << "file does not exist in db" << endl;        
        InsertFile(pDb, nPlaylistID, sFileName);       
      }         
      else {
        //cout << "file exists: " << nObjectID << endl;
        MapPlaylistItem(nPlaylistID, nObjectID);
      }
    } /* if(bIsLocalFile && FileExists(sFileName)) */        

    
  } /* for(int i = 0; i < nEntryCount; i++) */
  
  delete pDb;
}


fuppesThreadCallback BuildLoop(void* arg)
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
  CContentDatabase* pDb = new CContentDatabase();
  
  if(g_bFullRebuild) {
    pDb->Execute("delete from OBJECTS");
    pDb->Execute("delete from OBJECT_DETAILS");
    pDb->Execute("delete from MAP_OBJECTS");
  }

	/*pDb->Execute("drop index IDX_OBJECTS_OBJECT_ID");
	pDb->Execute("drop index IDX_MAP_OBJECTS_OBJECT_ID");
	pDb->Execute("drop index IDX_MAP_OBJECTS_PARENT_ID");
	pDb->Execute("drop index IDX_OBJECTS_DETAIL_ID");
	pDb->Execute("drop index IDX_OBJECT_DETAILS_ID");*/
	
  pDb->Execute("vacuum");  
  
  int i;
  unsigned int nObjId;
  stringstream sSql;
  string sFileName;
  bool bInsert = true;
  
  CSharedLog::Print("read shared directories");

  for(i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++)
  {
    if(DirectoryExists(CSharedConfig::Shared()->GetSharedDir(i)))
    {  
      sSql.str("");
      
      ExtractFolderFromPath(CSharedConfig::Shared()->GetSharedDir(i), &sFileName);

      bInsert = true;
      if(g_bAddNew) {
        if((nObjId = GetObjectIDFromFileName(pDb, CSharedConfig::Shared()->GetSharedDir(i))) > 0) {
          bInsert = false;
        }
      }
      
      
      if(bInsert) {      
        nObjId = pDb->GetObjId();      
      
        sSql << 
          "insert into OBJECTS (OBJECT_ID, TYPE, PATH, FILE_NAME, TITLE) values " <<
          "(" << nObjId << 
          ", " << CONTAINER_STORAGE_FOLDER << 
          ", '" << SQLEscape(CSharedConfig::Shared()->GetSharedDir(i)) << "'" <<
          ", '" << SQLEscape(sFileName) << "'" <<
          ", '" << SQLEscape(sFileName) << "');";
        
        pDb->Insert(sSql.str());
    
        sSql.str("");
        sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
          "values (" << nObjId << ", 0)";
      
        pDb->Insert(sSql.str());      
      }
      
      DbScanDir(pDb, CSharedConfig::Shared()->GetSharedDir(i), nObjId);      
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
    
  delete pDb;
  
  
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
  fuppesThreadExit();
}
    
void CContentDatabase::FamEvent(FAM_EVENT_TYPE p_nEventType, std::string p_sPath)
{
}
