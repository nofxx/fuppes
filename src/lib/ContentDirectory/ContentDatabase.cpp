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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "ContentDatabase.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "FileDetails.h"
#include "../Common/RegEx.h"
#include "../Common/Common.h"
#include "iTunesImporter.h"

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
 
int SelectCallback(void *pDatabase, int argc, char **argv, char **azColName)
{
  /*for(int i = 0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }*/
    
  // build new result set
  CSelectResult* pResult = new CSelectResult();
  for(int i = 0; i < argc; i++) {
    string sFieldName = azColName[i];
    pResult->m_FieldValues[sFieldName] = argv[i] ? argv[i] : "NULL";        
  }  
  ((CContentDatabase*)pDatabase)->m_ResultList.push_back(pResult);
  ((CContentDatabase*)pDatabase)->m_nRowsReturned++;
		  
  // select first entry
  ((CContentDatabase*)pDatabase)->m_ResultListIterator = ((CContentDatabase*)pDatabase)->m_ResultList.begin();

  return 0;
}


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
	if (m_Instance == 0)
		m_Instance = new CContentDatabase(true);
	return m_Instance;
}

CContentDatabase::CContentDatabase(bool p_bShared)
{ 
  stringstream sDbFile;
  sDbFile << CSharedConfig::Shared()->GetConfigDir() << "fuppes.db";  
  m_sDbFileName = sDbFile.str();
  
  m_nRowsReturned = 0;
  //m_bIsRebuilding = false;
  m_RebuildThread = (fuppesThread)NULL;	
  m_bShared       = p_bShared;		
	m_pFileSystemMonitor = NULL;
  m_bInTransaction = false;
  
	if(m_bShared) {            
	  g_bIsRebuilding = false;
    fuppesThreadInitMutex(&m_Mutex);
    #ifdef HAVE_INOTIFY
    m_pFileSystemMonitor = new CInotifyMonitor(this);
    #endif
  }
    
  if(FileExists(m_sDbFileName))
    Open();
}
 
CContentDatabase::~CContentDatabase()
{
	/*if(m_Instance == this) {                 
    fuppesThreadDestroyMutex(&m_Mutex);
  }*/
	
	if(m_bShared && (m_RebuildThread != (fuppesThread)NULL)) {
	  fuppesThreadClose(m_RebuildThread);
		m_RebuildThread = (fuppesThread)NULL;
	}
	
	if(m_pFileSystemMonitor != NULL)
	  delete m_pFileSystemMonitor;

  ClearResult();
  Close();
}

std::string CContentDatabase::GetLibVersion()
{
  return sqlite3_libversion();
}

bool CContentDatabase::Init(bool* p_bIsNewDB)
{
  #warning TODO: set OBJECTS.ID to 30 after db creation
  
  bool bIsNewDb = !FileExists(m_sDbFileName);
  *p_bIsNewDB = bIsNewDb;
  int nRes = sqlite3_open(m_sDbFileName.c_str(), &m_pDbHandle);   
  if(nRes)
  {
    fprintf(stderr, "Can't create/open database: %s\n", sqlite3_errmsg(m_pDbHandle));
    sqlite3_close(m_pDbHandle);
    return false;
  }
  
  if(bIsNewDb)
  {
    if(!Execute("CREATE TABLE OBJECTS ( "
				"  ID INTEGER PRIMARY KEY AUTOINCREMENT, "
				"  OBJECT_ID INTEGER NOT NULL, "
        "  DETAIL_ID INTEGER DEFAULT NULL, "
        //"  PARENT_ID INTEGER NOT NULL, "
				"  TYPE INTEGER NOT NULL, "
        "  DEVICE TEXT DEFAULT NULL, "  
				"  PATH TEXT NOT NULL, "
				"  FILE_NAME TEXT DEFAULT NULL, "
				"  TITLE TEXT DEFAULT NULL, "
				"  MD5 TEXT DEFAULT NULL, "
				"  MIME_TYPE TEXT DEFAULT NULL "				
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
        "  SIZE INTEGER DEFAULT 0 "          
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
  }

  return true;
}

void CContentDatabase::Lock()
{
  fuppesThreadLockMutex(&CContentDatabase::Shared()->m_Mutex);
}

void CContentDatabase::Unlock()
{
  fuppesThreadUnlockMutex(&CContentDatabase::Shared()->m_Mutex);
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
  //cout << "OPEN" << endl; fflush(stdout);
  if(sqlite3_open(m_sDbFileName.c_str(), &m_pDbHandle)) {
    fprintf(stderr, "Can't create/open database: %s\n", sqlite3_errmsg(m_pDbHandle));
    sqlite3_close(m_pDbHandle);
    return false;
  }
  //cout << "OPENED" << endl; fflush(stdout);
	return true;
}

void CContentDatabase::Close()
{
  //cout << "CLOSE" << endl; fflush(stdout);
  sqlite3_close(m_pDbHandle);
  //cout << "CLOSED" << endl; fflush(stdout);
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
  //cout << "INSERT :: " << p_sStatement << endl; fflush(stdout);

  Lock();
  //Open();

  char* szErr = 0;
  
  /*int nTrans = sqlite3_exec(m_pDbHandle, "BEGIN TRANSACTION;", NULL, NULL, &szErr);
  if(nTrans != SQLITE_OK)
    fprintf(stderr, "CContentDatabase::Insert - start transaction :: SQL error: %s\n", szErr);  */
    
  int nResult = sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), NULL, NULL, &szErr);  
  if(nResult != SQLITE_OK) {
    fprintf(stderr, "CContentDatabase::Insert - insert :: SQL error: %s\n", szErr);    
    nResult = 0;
  }
  else {
    nResult = sqlite3_last_insert_rowid(m_pDbHandle);
  }
	
  /*nTrans = sqlite3_exec(m_pDbHandle, "COMMIT;", NULL, NULL, &szErr);
  if(nTrans != SQLITE_OK)
    fprintf(stderr, "CContentDatabase::Insert - commit :: SQL error: %s\n", szErr);       */ 

  //Close();
	Unlock();
	
  //cout << "INSERT DONE" << endl; fflush(stdout);
	
  return nResult;  
}

bool CContentDatabase::Execute(std::string p_sStatement)
{
  Lock();
  //Open();
	bool bResult = false;
  char* szErr = 0;
	
  int nStat = sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), NULL, NULL, &szErr);  
  if(nStat != SQLITE_OK) {
    fprintf(stderr, "CContentDatabase::Execute :: SQL error: %s\n", szErr);    
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
  //cout << "SELECT :: " << p_sStatement << endl; fflush(stdout);
   
  Lock();
  //Open();  
  ClearResult();    
  bool bResult = true;
  
  char* szErr = 0;
  
  int nResult = SQLITE_OK;  
  int nTry = 0;
  do    
  {
    nResult = sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), SelectCallback, this, &szErr);
    if(nTry > 0) {
      CSharedLog::Shared()->Log(L_EXTENDED_WARN, "SQLITE_BUSY", __FILE__, __LINE__);
      fuppesSleep(100);
    }
    nTry++;
  }while(nResult == SQLITE_BUSY);
    
  if(nResult != SQLITE_OK) {
    cout << "RESULT: " << nResult << endl;    
    fprintf(stderr, "CContentDatabase::Select :: SQL error: %s, Statement: %s\n", szErr, p_sStatement.c_str());
    sqlite3_close(m_pDbHandle);    
    bResult = false;
  }
  
  //Close();
	Unlock();
	
  //cout << "SELECT DONE" << endl << endl; fflush(stdout);	
	
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
  m_ResultListIterator++;
}



fuppesThreadCallback BuildLoop(void *arg);
void DbScanDir(std::string p_sDirectory, long long int p_nParentId);
void BuildPlaylists();
void ParsePlaylist(CSelectResult* pResult);
void ParseM3UPlaylist(CSelectResult* pResult);
void ParsePLSPlaylist(CSelectResult* pResult);


unsigned int InsertFile(CContentDatabase* pDb, unsigned int p_nParentId, std::string p_sFileName);
unsigned int InsertURL(unsigned int p_nParentId, std::string p_sURL);

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

bool CContentDatabase::IsRebuilding()
{
  return g_bIsRebuilding;
}

unsigned int g_nObjId = 0;

unsigned int CContentDatabase::GetObjId()
{
  return ++g_nObjId;
}

void DbScanDir(CContentDatabase* pDb, std::string p_sDirectory, long long int p_nParentId)
{
  #ifdef WIN32  
  // append trailing backslash if neccessary
  char szTemp[MAX_PATH];
  if(p_sDirectory.substr(p_sDirectory.length()-1).compare(upnpPathDelim) != 0) {
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
   
  // append trailing slash if neccessary
  if(p_sDirectory.substr(p_sDirectory.length()-1).compare(upnpPathDelim) != 0) {
    sTmp << p_sDirectory << upnpPathDelim;
    p_sDirectory = sTmp.str();
    sTmp.str("");
  }
  
  if((pDir = opendir(p_sDirectory.c_str())) != NULL)
  {    
    CSharedLog::Shared()->Log(L_EXTENDED, "read directory: " + p_sDirectory, __FILE__, __LINE__, false);
    
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
          sTmpFileName = ToUTF8(sTmpFileName);
          sTmpFileName = SQLEscape(sTmpFileName);
          
          stringstream sSql;
          /*sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME) values ";
          sSql << "(" << CONTAINER_STORAGE_FOLDER << ", ";
          sSql << p_nParentId << ", ";
          sSql << "'" << SQLEscape(sTmp.str()) << "', ";
          sSql << "'" << sTmpFileName << "');";*/
          
          unsigned int nObjId = pDb->GetObjId();
          
          pDb->BeginTransaction();
          
          sSql << "insert into objects ( " <<
            "  OBJECT_ID, TYPE, " <<
            "  PATH, FILE_NAME, TITLE) " <<
            "values ( " << 
               nObjId << ", " <<  
               CONTAINER_STORAGE_FOLDER << 
               ", '" << SQLEscape(sTmp.str()) << "', '" <<
               sTmpFileName << "', '" <<
               sTmpFileName <<
            "');";

          pDb->Insert(sSql.str());          
          
          sSql.str("");
          sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
                  "values (" << nObjId << ", " << p_nParentId << ")";          
          pDb->Insert(sSql.str());
          
          pDb->Commit();
          
          // recursively scan subdirectories
          DbScanDir(pDb, sTmp.str(), nObjId);          
        }
        else if(IsFile(sTmp.str()) && CFileDetails::Shared()->IsSupportedFileExtension(sExt))
        {
          InsertFile(pDb, p_nParentId, sTmp.str());
        }   
        
        sTmp.str("");
      }
    }  /* while */  
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif         
}

unsigned int InsertAudioFile(CContentDatabase* pDb, std::string p_sFileName)
{
	struct SMusicTrack TrackInfo; 
	if(!CFileDetails::Shared()->GetMusicTrackDetails(p_sFileName, &TrackInfo))
	  return 0;
		
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(A_ARTIST, A_ALBUM, A_TRACK_NO, A_GENRE, AV_DURATION, DATE, A_CHANNELS, AV_BITRATE, A_SAMPLERATE) " <<
		"values (" <<
		//"'" << SQLEscape(TrackInfo.mAudioItem.sTitle) << "', " <<
		"'" << SQLEscape(TrackInfo.sArtist) << "', " <<
		"'" << SQLEscape(TrackInfo.sAlbum) << "', " <<
		TrackInfo.nOriginalTrackNumber << ", " <<
		"'" << SQLEscape(TrackInfo.mAudioItem.sGenre) << "', " <<
		"'" << TrackInfo.mAudioItem.sDuration << "', " <<
		"'" << TrackInfo.sDate << "', " <<
		TrackInfo.mAudioItem.nNrAudioChannels << ", " <<
		TrackInfo.mAudioItem.nBitrate << ", " <<
		TrackInfo.mAudioItem.nSampleRate << ")";
		
	//cout << sSql.str() << endl;
	
  return pDb->Insert(sSql.str());
}

unsigned int InsertImageFile(CContentDatabase* pDb, std::string p_sFileName)
{
  struct SImageItem ImageItem;
	if(!CFileDetails::Shared()->GetImageDetails(p_sFileName, &ImageItem))
	  return 0;
		
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(IV_WIDTH, IV_HEIGHT) " <<
		"values (" <<
		ImageItem.nWidth << ", " <<
		ImageItem.nHeight << ")";
	
	//cout << sSql.str() << endl;
	
  return pDb->Insert(sSql.str());  
} 

unsigned int InsertVideoFile(CContentDatabase* pDb, std::string p_sFileName)
{
  struct SVideoItem VideoItem;
	if(!CFileDetails::Shared()->GetVideoDetails(p_sFileName, &VideoItem))
	  return 0;
		
	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(IV_WIDTH, IV_HEIGHT, AV_DURATION, SIZE, AV_BITRATE) " <<
		"values (" <<
		VideoItem.nWidth << ", " <<
		VideoItem.nHeight << "," <<
		"'" << VideoItem.sDuration << "', " <<
		VideoItem.nSize << ", " <<
		VideoItem.nBitrate << ");";
	
	//cout << sSql.str() << endl;

  return pDb->Insert(sSql.str());
} 

unsigned int InsertFile(CContentDatabase* pDb, unsigned int p_nParentId, std::string p_sFileName)
{
  OBJECT_TYPE nObjectType = CFileDetails::Shared()->GetObjectType(p_sFileName);  
  if(nObjectType == OBJECT_TYPE_UNKNOWN)
    return false;
  
  pDb->BeginTransaction();
      
  // we insert file details first to get the details ID
  unsigned int nDetailId = 0;
  switch(nObjectType)
  {
    case ITEM_AUDIO_ITEM_MUSIC_TRACK:     
		  nDetailId = InsertAudioFile(pDb, p_sFileName); 
      break;
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
  
  // vdr -> vob
  if(ExtractFileExt(sTmpFileName) == "vdr") {
    sTmpFileName = TruncateFileExt(sTmpFileName) + ".vob";
  }
  
  // format file name
  int nPathLen = ExtractFilePath(sTmpFileName).length();
  sTmpFileName = sTmpFileName.substr(nPathLen, sTmpFileName.length() - nPathLen);  
  sTmpFileName = ToUTF8(sTmpFileName);
  sTmpFileName = SQLEscape(sTmpFileName);  
  
  
  unsigned int nObjId = pDb->GetObjId();
  
  stringstream sSql;
  sSql << "insert into objects (" <<
    "  OBJECT_ID, DETAIL_ID, TYPE, " <<
    "  PATH, FILE_NAME, " <<
    "  TITLE, MIME_TYPE) values (" <<
       nObjId << ", " <<
       nDetailId << ", " <<   
       nObjectType << ", " <<
       "'" << SQLEscape(p_sFileName) << "', " << 
       "'" << sTmpFileName << "', " << 
       "'" << sTmpFileName << "', " << 
       "'" << CFileDetails::Shared()->GetMimeType(p_sFileName, false) << "');";
               
  pDb->Insert(sSql.str());  
  sSql.str("");
  
  sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
            "values (" << nObjId << ", " << p_nParentId << ")";  
  pDb->Insert(sSql.str());  
  
  pDb->Commit();  

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
  
  pDb->ClearResult();
  delete pDb;
}

void ParsePlaylist(CSelectResult* pResult)
{
  //cout << "PLAYLIST: " << pResult->GetValue("PATH") << endl;
  string sExt = ToLower(ExtractFileExt(pResult->GetValue("PATH")));
  if(sExt.compare("m3u") == 0)
    ParseM3UPlaylist(pResult);
  else if(sExt.compare("pls") == 0)
    ParsePLSPlaylist(pResult);
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

unsigned int GetObjectIDFromFileName(std::string p_sFileName)
{
  CContentDatabase* pDB = new CContentDatabase();  
  
  unsigned int nResult = 0;
  stringstream sSQL;
  sSQL << "select OBJECT_ID from OBJECTS where PATH = \"" << p_sFileName << "\" and DEVICE is NULL";
  
  pDB->Select(sSQL.str());
  if(!pDB->Eof())
    nResult = atoi(pDB->GetResult()->GetValue("ID").c_str());   
  
  pDB->ClearResult();  
  delete pDB;
  
  return nResult;
}

bool MapPlaylistItem(unsigned int p_nPlaylistID, unsigned int p_nItemID, int p_nPosition)
{
  CContentDatabase* pDB = new CContentDatabase();
  
  stringstream sSql;  
  sSql <<
    "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) values " <<
    "( " << p_nItemID <<
    ", " << p_nPlaylistID << ")";                         
         
  //cout << sSql.str() << endl;
  
  pDB->Insert(sSql.str());
  
  delete pDB;
	return true;
}

bool IsRelativeFileName(std::string p_sValue)
{
  /* url */
  RegEx rxUrl("\\w+://", PCRE_CASELESS);
  if(rxUrl.Search(p_sValue.c_str()))
    return false;
  
  /* absolute filename on unix systems? */
  if(p_sValue.substr(0, 1).compare(upnpPathDelim) == 0)
    return false;
  
  /* absolute filename on windows */
  RegEx rxWin("^\\w:\\\\", PCRE_CASELESS);
  if(rxWin.Search(p_sValue.c_str()))
    return false;   
    
  return true;
}

bool IsLocalFile(std::string p_sValue)
{
  if(p_sValue.length() == 0)
    return false;
  
  /* absolute filename on unix systems? */
  if(p_sValue.substr(0, 1).compare(upnpPathDelim) == 0)
    return true;
  
  /* absolute filename on windows */
  RegEx rxWin("^\\w:\\\\", PCRE_CASELESS);
  if(rxWin.Search(p_sValue.c_str()))
    return true;  
  
  /* relative filename */
  if(IsRelativeFileName(p_sValue))
    return true;  
  
  return false;
}

void ParseM3UPlaylist(CSelectResult* pResult)
{
  std::string  sContent = ReadFile(pResult->GetValue("PATH"));
  std::string  sFileName;
  unsigned int nPlaylistID = pResult->GetValueAsUInt("OBJECT_ID");
  unsigned int nObjectID   = 0;
  bool bIsLocalFile;
  int nPlsPosi = 0;
  
  //cout << "parse m3u" << endl;
  
  CContentDatabase* pDb = new CContentDatabase();
  
  RegEx rxLines("(.*)\n", PCRE_CASELESS);
  RegEx rxFile("^[#EXTM3U|#EXTINF]", PCRE_CASELESS);  
  if(rxLines.Search(sContent.c_str()))
  {
    do
    {
      if(rxFile.Search(rxLines.Match(1)))
        continue;      
      
      sFileName    = rxLines.Match(1);      
      bIsLocalFile = IsLocalFile(sFileName);
      
      //cout << sFileName << endl;
      
      if(bIsLocalFile && FileExists(sFileName))
      {
        if(IsRelativeFileName(sFileName))
          sFileName = ExtractFilePath(pResult->GetValue("PATH")) + sFileName;
        
        nObjectID = GetObjectIDFromFileName(sFileName);      
        
        if(nObjectID == 0) {
          //cout << "file does not exist in db" << endl;        
          nObjectID = InsertFile(pDb, nPlaylistID, sFileName);       
        }            
        else {
          nPlsPosi++;
          MapPlaylistItem(nPlaylistID, nObjectID, nPlsPosi);
        }
        
      } /* if(bIsLocalFile && FileExists(sFileName)) */
      else if(!bIsLocalFile)
      {
        nObjectID = InsertURL(nPlaylistID, sFileName);
      }       
      
    }while(rxLines.SearchAgain());
  }
  
  delete pDb;
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
      nObjectID = GetObjectIDFromFileName(sFileName);      
      if(nObjectID == 0) {
        //cout << "file does not exist in db" << endl;        
        InsertFile(pDb, nPlaylistID, sFileName);       
      }         
      else {
        //cout << "file exists: " << nObjectID << endl;
        MapPlaylistItem(nPlaylistID, nObjectID, i + 1);
      }
    } /* if(bIsLocalFile && FileExists(sFileName)) */        

    
  } /* for(int i = 0; i < nEntryCount; i++) */
  
  delete pDb;
}


fuppesThreadCallback BuildLoop(void* arg)
{
  CSharedLog::Shared()->Log(L_NORMAL, "[ContentDatabase] create database. this may take a while.", __FILE__, __LINE__, false);
  
  CContentDatabase::Shared()->Execute("delete from OBJECTS");
  CContentDatabase::Shared()->Execute("delete from OBJECT_DETAILS");
  CContentDatabase::Shared()->Execute("delete from MAP_OBJECTS");
    
  CContentDatabase* pDb = new CContentDatabase();
  int i;
  
  CSharedLog::Shared()->Log(L_NORMAL, "read shared directories", __FILE__, __LINE__, false);
  for(i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++)
  {
    if(DirectoryExists(CSharedConfig::Shared()->GetSharedDir(i)))
    {  
      string sFileName;
      ExtractFolderFromPath(CSharedConfig::Shared()->GetSharedDir(i), &sFileName);          
        
      pDb->BeginTransaction();
      
      unsigned int nObjId = pDb->GetObjId();
      
      stringstream sSql;
      sSql << 
        "insert into OBJECTS (OBJECT_ID, TYPE, PATH, FILE_NAME) values " <<
        "(" << nObjId << 
        ", " << CONTAINER_STORAGE_FOLDER << 
        ", '" << SQLEscape(CSharedConfig::Shared()->GetSharedDir(i)) << "'" <<
        ", '" << SQLEscape(sFileName) << "');";
        
		  pDb->Insert(sSql.str());
    
      sSql.str("");
      sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
        "values (" << nObjId << ", 0)";
      
      pDb->Insert(sSql.str());      
      
      pDb->Commit();
      
      DbScanDir(pDb, CSharedConfig::Shared()->GetSharedDir(i), nObjId);      
    }
    else {      
      CSharedLog::Shared()->Log(L_WARNING, "shared directory: \"" + CSharedConfig::Shared()->GetSharedDir(i) + "\" not found", __FILE__, __LINE__, false);
    }
  } // for
  CSharedLog::Shared()->Log(L_NORMAL, "[DONE] read shared directories", __FILE__, __LINE__, false);
    
  
  CSharedLog::Shared()->Log(L_NORMAL, "parse playlists", __FILE__, __LINE__, false);
  BuildPlaylists();
  CSharedLog::Shared()->Log(L_NORMAL, "[DONE] parse playlists", __FILE__, __LINE__, false);
    
  delete pDb;
  
  
  // import iTunes db
  CSharedLog::Shared()->Log(L_NORMAL, "parse iTunes databases", __FILE__, __LINE__, false);
  CiTunesImporter* pITunes = new CiTunesImporter();
  for(i = 0; i < CSharedConfig::Shared()->SharedITunesCount(); i++) {
    pITunes->Import(CSharedConfig::Shared()->GetSharedITunes(i));
  }
  delete pITunes;
  CSharedLog::Shared()->Log(L_NORMAL, "[DONE] parse iTunes databases", __FILE__, __LINE__, false);  
  
  CSharedLog::Shared()->Log(L_NORMAL, "[ContentDatabase] database created", __FILE__, __LINE__, false);
  
  g_bIsRebuilding = false;
  fuppesThreadExit();
}
