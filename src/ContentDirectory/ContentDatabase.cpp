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
 
static int SelectCallback(void *pDatabase, int argc, char **argv, char **azColName)
{
  /*for(int i = 0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }*/
    
  /* build new result set */  
  ((CContentDatabase*)pDatabase)->m_nRowsReturned++;
  
  CSelectResult* pResult = new CSelectResult();
  for(int i = 0; i < argc; i++)
  {
    string sFieldName = azColName[i];
    pResult->m_FieldValues[sFieldName] = argv[i] ? argv[i] : "NULL";        
  }  
  ((CContentDatabase*)pDatabase)->m_ResultList.push_back(pResult);
    
  /* select first entry */
  ((CContentDatabase*)pDatabase)->m_ResultListIterator = ((CContentDatabase*)pDatabase)->m_ResultList.begin();

  return 0;
}


unsigned int InsertFile(unsigned int p_nParentId, std::string p_sFileName);
unsigned int InsertURL(unsigned int p_nParentId, std::string p_sURL);

CContentDatabase* CContentDatabase::m_Instance = 0;

CContentDatabase* CContentDatabase::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CContentDatabase();
	return m_Instance;
}

CContentDatabase::CContentDatabase()
{ 
  stringstream sDbFile;
  sDbFile << CSharedConfig::Shared()->GetConfigDir() << "fuppes.db";  
  m_sDbFileName = sDbFile.str();
  
  m_nRowsReturned = 0;
  m_bIsRebuilding = false;
  
  fuppesThreadInitMutex(&m_Mutex);
}
 
CContentDatabase::~CContentDatabase()
{
  fuppesThreadDestroyMutex(&m_Mutex);
  ClearResult();
  sqlite3_close(m_pDbHandle);
}

std::string CContentDatabase::GetLibVersion()
{
  return sqlite3_libversion();
}

std::string CSelectResult::GetValue(std::string p_sFieldName)
{
  return m_FieldValues[p_sFieldName];
}

bool CContentDatabase::Init(bool* p_bIsNewDB)
{
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
    string sTableObjects = 
      "create table OBJECTS ("
      "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  PARENT_ID INTEGER NOT NULL DEFAULT 0,"
      "  TYPE INTEGER NOT NULL,"
      "  PATH TEXT NOT NULL,"
      "  FILE_NAME TEXT DEFAULT NULL,"
      "  MD5 TEXT DEFAULT NULL,"
      "  MIME_TYPE TEXT DEFAULT NULL,"
      "  DETAILS TEXT DEFAULT NULL"
      ");";        
    
    if(Insert(sTableObjects) < 0)
      return false;
    if(Insert("CREATE INDEX IDX_FILE_NAME ON OBJECTS (FILE_NAME);") < 0)
      return false;
    if(Insert("CREATE INDEX IDX_PARENT_ID ON OBJECTS (PARENT_ID);") < 0)
      return false;
    
		if(Insert("CREATE TABLE AUDIO_ITEMS (ID INTEGER PRIMARY KEY, DATE TEXT, TRACK_NO INTEGER, DESCRIPTION TEXT, DURATION TEXT, GENRE TEXT, ALBUM TEXT, ARTIST TEXT, TITLE TEXT);") < 0)
		  return false;
		
		if(Insert("CREATE TABLE IMAGE_ITEMS (ID INTEGER PRIMARY KEY, RESOLUTION TEXT);") < 0)
		  return false;
			
    if(Insert("CREATE TABLE VIDEO_ITEMS (ID INTEGER PRIMARY KEY, RESOLUTION TEXT);") < 0)
		  return false;
    
    string sTablePlaylistItems =
      "create table PLAYLIST_ITEMS ("
      "  ID INTEGER PRIMARY KEY AUTOINCREMENT, "
      "  PLAYLIST_ID INTEGER NOT NULL, " // id des playlist objektes OBJECTS.ID
      "  OBJECT_ID INTEGER NOT NULL, " // id des zugehoerigen eintrags aus OBJECTS.ID
      "  POSITION INTEGER NOT NULL " // position in der liste */
      ");";
    
    if(Insert(sTablePlaylistItems) < 0)
      return false;    
  }
  else
  {
    //Insert("delete from OBJECTS;");
  }  
  
  sqlite3_close(m_pDbHandle);
  return true;
}

void CContentDatabase::Lock()
{
  fuppesThreadLockMutex(&m_Mutex);
}

void CContentDatabase::Unlock()
{
  fuppesThreadUnlockMutex(&m_Mutex);
}

void CContentDatabase::ClearResult()
{
  /* clear old results */ 
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
  if(sqlite3_open(m_sDbFileName.c_str(), &m_pDbHandle))
  {
    fprintf(stderr, "Can't create/open database: %s\n", sqlite3_errmsg(m_pDbHandle));
    sqlite3_close(m_pDbHandle);
    return false;
  }
  return true;
}

void CContentDatabase::Close()
{
  sqlite3_close(m_pDbHandle);
}

unsigned int CContentDatabase::Insert(std::string p_sStatement)
{
  Open();
  
  char* szErr = 0;
  
  int nTrans = sqlite3_exec(m_pDbHandle, "BEGIN TRANSACTION;", NULL, NULL, &szErr);
  if(nTrans != SQLITE_OK)
    fprintf(stderr, "CContentDatabase::Insert - start transaction :: SQL error: %s\n", szErr);    
    
  int nResult = sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), NULL, NULL, &szErr);  
  if(nResult != SQLITE_OK)
  {
    fprintf(stderr, "CContentDatabase::Insert - insert :: SQL error: %s\n", szErr);    
    nResult = -1;
  }
  else  
  {
    nResult = sqlite3_last_insert_rowid(m_pDbHandle);
  }
  nTrans = sqlite3_exec(m_pDbHandle, "COMMIT;", NULL, NULL, &szErr);
  if(nTrans != SQLITE_OK)
    fprintf(stderr, "CContentDatabase::Insert - commit :: SQL error: %s\n", szErr);        
    //cout << "error commit transaction" << endl;
  
  Close();
  return nResult;  
}

bool CContentDatabase::Select(std::string p_sStatement)
{  
  Open();  
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
    
  if(nResult != SQLITE_OK)
  {
    cout << "RESULT: " << nResult << endl;    
    fprintf(stderr, "CContentDatabase::Select :: SQL error: %s, Statement: %s\n", szErr, p_sStatement.c_str());
    sqlite3_close(m_pDbHandle);    
    bResult = false;
  }
  
  Close();
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





void CContentDatabase::BuildDB()
{
  m_bIsRebuilding = true;
  
  CSharedLog::Shared()->Log(L_NORMAL, "[ContentDatabase] creating database. this may take a while.", __FILE__, __LINE__, false);
  
  CContentDatabase::Shared()->Insert("delete from objects");
  CContentDatabase::Shared()->Insert("delete from playlist_items");
	CContentDatabase::Shared()->Insert("delete from audio_items");
	CContentDatabase::Shared()->Insert("delete from video_items");
	CContentDatabase::Shared()->Insert("delete from image_items");
  
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
    else {      
      CSharedLog::Shared()->Log(L_WARNING, "shared directory: \"" + CSharedConfig::Shared()->GetSharedDir(i) + "\" not found", __FILE__, __LINE__, false);
    }
  } // for
  
  //cout << "parsing playlists" << endl;
  BuildPlaylists();
  //cout << "done parsing playlists" << endl;  
    
  CSharedLog::Shared()->Log(L_NORMAL, "[ContentDatabase] database created", __FILE__, __LINE__, false);
  m_bIsRebuilding = false;
}

void CContentDatabase::DbScanDir(std::string p_sDirectory, long long int p_nParentId)
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
          sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME) values ";
          sSql << "(" << CONTAINER_STORAGE_FOLDER << ", ";
          sSql << p_nParentId << ", ";
          sSql << "'" << SQLEscape(sTmp.str()) << "', ";
          sSql << "'" << sTmpFileName << "');";
            
          
          CContentDatabase::Shared()->Lock();
          long long int nRowId = CContentDatabase::Shared()->Insert(sSql.str());
          CContentDatabase::Shared()->Unlock();
          if(nRowId == -1)
            cout << "ERROR: " << sSql.str() << endl;
          
          // recursively scan subdirectories
          DbScanDir(sTmp.str(), nRowId);          
        }
        else if(IsFile(sTmp.str()) && CSharedConfig::Shared()->IsSupportedFileExtension(sExt))
        {
          InsertFile(p_nParentId, sTmp.str());
        }   
        
        sTmp.str("");
      }
    }  /* while */  
  #ifndef WIN32
    closedir(pDir);
  } /* if opendir */
  #endif         
}

unsigned int InsertAudioFile(unsigned int p_nObjectId, std::string p_sFileName)
{
	struct SMusicTrack TrackInfo; 
	if(!CFileDetails::Shared()->GetMusicTrackDetails(p_sFileName, &TrackInfo))
	  return 0;
		
	stringstream sSql;
	sSql << 
	  "insert into AUDIO_ITEMS " <<
		"(ID, TITLE, ARTIST, ALBUM, TRACK_NO, GENRE, DURATION, DATE) " <<
		"values (" <<
		p_nObjectId << ", " <<
		"'" << SQLEscape(TrackInfo.mAudioItem.sTitle) << "', " <<
		"'" << SQLEscape(TrackInfo.sArtist) << "', " <<
		"'" << SQLEscape(TrackInfo.sAlbum) << "', " <<
		TrackInfo.nOriginalTrackNumber << ", " <<
		"'" << SQLEscape(TrackInfo.mAudioItem.sGenre) << "', " <<
		"'" << TrackInfo.mAudioItem.sDuration << "', " <<
		"'" << TrackInfo.sDate << "')";
		
	cout << sSql.str() << endl;
		
	CContentDatabase* pDB = new CContentDatabase();          
  unsigned int nRowId = pDB->Insert(sSql.str());
  delete pDB;

	return nRowId;
}

unsigned int InsertImageFile(unsigned int p_nObjectId, std::string p_sFileName)
{
  struct SImageItem ImageItem;
	if(!CFileDetails::Shared()->GetImageDetails(p_sFileName, &ImageItem))
	  return 0;
} 

unsigned int InsertFile(unsigned int p_nParentId, std::string p_sFileName)
{
  OBJECT_TYPE nObjectType = CFileDetails::Shared()->GetObjectType(p_sFileName);         

  
  if(nObjectType == OBJECT_TYPE_UNKNOWN)
    return false;          
  
  string sTmpFileName =  p_sFileName;
  // vdr -> vob
  if(ExtractFileExt(sTmpFileName) == "vdr") {
    sTmpFileName = TruncateFileExt(sTmpFileName) + ".vob";
  }
  
  int nPathLen = ExtractFilePath(sTmpFileName).length();
  sTmpFileName = sTmpFileName.substr(nPathLen, sTmpFileName.length() - nPathLen);
  
  string sDetails = "";

  
  sTmpFileName = ToUTF8(sTmpFileName);
  sTmpFileName = SQLEscape(sTmpFileName);  
  
  
  stringstream sSql;
  sSql << "insert into objects (TYPE, PARENT_ID, PATH, FILE_NAME, MD5, MIME_TYPE) values ";
  sSql << "(" << nObjectType << ", ";
  sSql << p_nParentId << ", ";
  sSql << "'" << SQLEscape(p_sFileName) << "', ";
  sSql << "'" << sTmpFileName << "', ";
  sSql << "'" << "todo" << "', ";   //sSql << "'" << MD5Sum(p_sFileName) << "', ";  
  sSql << "'" << CFileDetails::Shared()->GetMimeType(p_sFileName, false) << "');";
  
  //cout << sSql.str() << endl;         
  
  CContentDatabase* pDB = new CContentDatabase();          
  unsigned int nRowId = pDB->Insert(sSql.str());
  delete pDB;
  
  if(nRowId == -1) {
    return 0;
    cout << "ERROR: " << sSql.str() << endl;
  }
	
	// build file description          
  switch(nObjectType)
  {
    case ITEM_AUDIO_ITEM_MUSIC_TRACK:     
		  InsertAudioFile(nRowId, p_sFileName); 
      break;
		case ITEM_IMAGE_ITEM_PHOTO:
		  InsertImageFile(nRowId, p_sFileName);
			break;
  }
	
	
	return nRowId;         
}

unsigned int InsertURL(unsigned int p_nParentId, std::string p_sURL)
{
  stringstream sSql;
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
  return nRowId;
}

void CContentDatabase::BuildPlaylists()
{
  this->Lock();
  
  stringstream sGetPlaylists;
    sGetPlaylists << 
    "select     " << 
    "  *        " <<
    "from       " <<
    "  OBJECTS  " <<
    "where      " <<
    "  TYPE = 5 ";  
  
  if(!this->Select(sGetPlaylists.str()))
  {
    this->Unlock();
    return;
  }
  
  CSelectResult* pResult = NULL;
  while(!this->Eof())
  {
    pResult = GetResult();    
    ParsePlaylist(pResult);    
    this->Next();
  }  
  
  this->ClearResult();
  this->Unlock();
}

void CContentDatabase::ParsePlaylist(CSelectResult* pResult)
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
  sSQL << "select ID from OBJECTS where PATH = \"" << p_sFileName << "\"";
  
  pDB->Select(sSQL.str());
  if(!pDB->Eof())
    nResult = atoi(pDB->GetResult()->GetValue("ID").c_str());   
  
  pDB->ClearResult();  
  delete pDB;
  
  return nResult;
}

bool InsertPlaylistItem(unsigned int p_nPlaylistID, unsigned int p_nItemID, int p_nPosition)
{
  CContentDatabase* pDB = new CContentDatabase();
  
  stringstream sSQL;  
  sSQL <<
    "insert into PLAYLIST_ITEMS " << 
    "  (PLAYLIST_ID, OBJECT_ID, POSITION) " <<
    "values " <<
    "  (" << p_nPlaylistID << ", " <<
    "   " << p_nItemID << ", " <<
    "   " << p_nPosition << ");"; 
 
  pDB->Insert(sSQL.str());
  
  delete pDB;
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

void CContentDatabase::ParseM3UPlaylist(CSelectResult* pResult)
{
  std::string  sContent = ReadFile(pResult->GetValue("PATH"));
  std::string  sFileName;
  unsigned int nPlaylistID = atoi(pResult->GetValue("ID").c_str());
  unsigned int nObjectID   = 0;
  bool bIsLocalFile;
  int nPlsPosi = 0;
  
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
      
      if(bIsLocalFile && FileExists(sFileName))
      {
        if(IsRelativeFileName(sFileName))
          sFileName = ExtractFilePath(pResult->GetValue("PATH")) + sFileName;
        
        nObjectID = GetObjectIDFromFileName(sFileName);      
        if(nObjectID == 0)
        {
          //cout << "file does not exist in db" << endl;        
          nObjectID = InsertFile(nPlaylistID, sFileName);       
        }            
      } /* if(bIsLocalFile && FileExists(sFileName)) */
      else if(!bIsLocalFile)
      {
        nObjectID = InsertURL(nPlaylistID, sFileName);
      }     

      if(nObjectID > 0)
      {
        nPlsPosi++;
        InsertPlaylistItem(nPlaylistID, nObjectID, nPlsPosi);
      }      
      
    }while(rxLines.SearchAgain());
  }
}

void CContentDatabase::ParsePLSPlaylist(CSelectResult* pResult)
{
  std::string sContent = ReadFile(pResult->GetValue("PATH"));
  RegEx rxNumber("NumberOfEntries=(\\d+)", PCRE_CASELESS);
  if(!rxNumber.Search(sContent.c_str()))
    return;
  
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
        
    // relative or absolute file name 
    bool bIsLocalFile = true;
    unsigned int nObjectID = 0;
    unsigned int nPlaylistID = atoi(pResult->GetValue("ID").c_str());
    
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
      if(nObjectID == 0)
      {
        //cout << "file does not exist in db" << endl;        
        nObjectID = InsertFile(nPlaylistID, sFileName);       
      }            
    } /* if(bIsLocalFile && FileExists(sFileName)) */    
    
    
    if(nObjectID > 0)
    {       
        //cout << "file exists: " << nObjectID << endl;
      InsertPlaylistItem(nPlaylistID, nObjectID, i + 1);
    }
    
  } /* for(int i = 0; i < nEntryCount; i++) */
}
