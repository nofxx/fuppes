/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ContentDatabase.cpp
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
 
#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

// increment this value if the database structure has changed
#define DB_VERSION 2

#include "ContentDatabase.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "FileDetails.h"
#include "../Common/RegEx.h"
#include "../Common/Common.h"
#include "../Common/Directory.h"
#include "../Common/File.h"
#include "iTunesImporter.h"
#include "PlaylistParser.h"
#include "HotPlug.h"

#include "DatabaseObject.h"
#include "VirtualContainerMgr.h"

#include <sstream>
#include <string>
#include <fstream>
#include <cstdio>
#ifndef WIN32
#include <dirent.h>
#endif
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <iostream>

using namespace std;
using namespace fuppes;

//static bool g_bIsRebuilding;
/*static bool g_bFullRebuild;
static bool g_bAddNew;
static bool g_bRemoveMissing;
*/


CContentDatabase* CContentDatabase::m_Instance = 0;

CContentDatabase* CContentDatabase::Shared()
{
	if(m_Instance == 0) {
		m_Instance = new CContentDatabase();
  }
	return m_Instance;
}

CContentDatabase::CContentDatabase()
{ 
	m_rebuildThread		= NULL;
  m_objectId				= 0;
  m_systemUpdateId  = 0;
  m_fileAlterationHandler = new FileAlterationHandler();
  m_pFileAlterationMonitor = CFileAlterationMgr::CreateMonitor(m_fileAlterationHandler);
  m_fileAlterationHandler->setMonitor(m_pFileAlterationMonitor);
  m_updateThread = new UpdateThread(m_fileAlterationHandler);

  HotPlugMgr::init();
}
 
CContentDatabase::~CContentDatabase()
{ 
  HotPlugMgr::uninit();
  
  delete m_updateThread;
  delete m_pFileAlterationMonitor;
  delete m_fileAlterationHandler;
  
	if(m_rebuildThread != NULL) {
		delete m_rebuildThread;
		m_rebuildThread = NULL;
	}
}

bool CContentDatabase::Init(bool* p_bIsNewDB)
{
  *p_bIsNewDB = false;
  
 	SQLQuery qry;
  string sql = qry.build(SQL_TABLES_EXIST, 0);
  qry.select(sql);
  if(qry.eof()) {
    *p_bIsNewDB = true;
    
    // create tables
    sql = qry.build(SQL_CREATE_TABLE_DB_INFO, 0);
    qry.exec(sql);
    
    sql = qry.build(SQL_CREATE_TABLE_OBJECTS, 0);
    qry.exec(sql);
    
    sql = qry.build(SQL_CREATE_TABLE_OBJECT_DETAILS, 0);    
    qry.exec(sql);

    // create indices
    StringList indices = String::split(qry.connection()->getStatement(SQL_CREATE_INDICES), ";");
    for(unsigned int i = 0; i < indices.size(); i++) {
      qry.exec(indices.at(i));
    }
    
    // set db version
    sql = qry.build(SQL_SET_DB_INFO, DB_VERSION);
    qry.exec(sql);
  }
  
  


  // this is mysql only code
  /*
  qry.select("SHOW VARIABLES LIKE  'char%'");
  while(!qry.eof()) {
    cout << qry.result()->asString("Variable_name") << " :: " << qry.result()->asString("Value") << endl;
    qry.next();
  }
  */

  
	qry.select("select VERSION from FUPPES_DB_INFO");
	if(qry.eof()) {
		CSharedLog::Shared()->UserError("no database version information found. remove the fuppes.db and restart fuppes");
		return false;
	}

	if(qry.result()->asInt("VERSION") != DB_VERSION) {
		CSharedLog::Shared()->UserError("database version mismatch. remove the fuppes.db and restart fuppes");
		return false;
	}


	

  if(CDatabase::connectionParams().readonly == false) {

    // get max object id
    qry.select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
    if(!qry.eof()) {  
		  Shared()->m_objectId = qry.result()->asUInt("VALUE");
    }

    // setup file alteration monitor
    if(m_pFileAlterationMonitor->isActive()) {
		  qry.select("select PATH from OBJECTS where TYPE >= 1 and TYPE < 100 and DEVICE is NULL");
		  while(!qry.eof()) {
			  m_pFileAlterationMonitor->addWatch(qry.result()->asString("PATH"));
			  qry.next();
		  }
	  }

    // start update thread
    if(!*p_bIsNewDB)
      m_updateThread->start();
  }
 

  return true;
}




int CContentDatabase::systemUpdateId() // static
{
  return m_Instance->m_systemUpdateId;
}

void CContentDatabase::incSystemUpdateId() // static
{
  m_Instance->m_systemUpdateId++;
  // todo: execute "Service Reset Procedure" if m_systemUpdateId reaches it's max value
}


void BuildPlaylists();
void ParsePlaylist(CSQLResult* pResult);
void ParseM3UPlaylist(CSQLResult* pResult);
void ParsePLSPlaylist(CSQLResult* pResult);

std::string findAlbumArtFile(std::string dir);

//unsigned int InsertFile(CContentDatabase* pDb, SQLQuery* qry, unsigned int p_nParentId, std::string p_sFileName, bool hidden = false);

unsigned int GetObjectIDFromFileName(SQLQuery* qry, std::string p_sFileName);

void CContentDatabase::BuildDB(int rebuildType)
{     
  if(CContentDatabase::Shared()->IsRebuilding())
	  return;

	if(m_rebuildThread) {
		delete m_rebuildThread;
		m_rebuildThread = NULL;
	}	
		
	m_rebuildThread = new RebuildThread();
  m_rebuildThread->setRebuildType(rebuildType);
	m_rebuildThread->start();
}

void CContentDatabase::RebuildDB()
{     
  if(CContentDatabase::Shared()->IsRebuilding() || CDatabase::connectionParams().readonly)
	  return;

  BuildDB(RebuildThread::rebuild);
}

void CContentDatabase::UpdateDB()
{
  if(CContentDatabase::Shared()->IsRebuilding() || CDatabase::connectionParams().readonly)
	  return;
    
	CSQLQuery* qry = CDatabase::query();
	qry->select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
  if(!qry->eof()) {  
    Shared()->m_objectId = qry->result()->asUInt("VALUE");
  }
	delete qry;
  
  BuildDB(RebuildThread::addNew | RebuildThread::removeMissing);
}

void CContentDatabase::AddNew()
{
  if(CContentDatabase::Shared()->IsRebuilding() || CDatabase::connectionParams().readonly)
	  return;

	CSQLQuery* qry = CDatabase::query();
	qry->select("select max(OBJECT_ID) as VALUE from OBJECTS where DEVICE is NULL");
  if(!qry->eof()) {  
    Shared()->m_objectId = qry->result()->asUInt("VALUE");
  }
	delete qry;
	
  BuildDB(RebuildThread::addNew);
}

void CContentDatabase::RemoveMissing()
{
  if(CContentDatabase::Shared()->IsRebuilding() || CDatabase::connectionParams().readonly)
	  return;
  
  BuildDB(RebuildThread::removeMissing);
}


bool CContentDatabase::IsRebuilding()
{
  return (m_rebuildThread && m_rebuildThread->running());
}

unsigned int CContentDatabase::GetObjId() // static
{
  return ++m_Instance->m_objectId;
}





unsigned int CContentDatabase::insertFile(std::string fileName, object_id_t parentId /*= 0*/, SQLQuery* qry /*= NULL*/, bool lock /*= false*/) // static
{
  if(lock) {
    MutexLocker locker(&m_Instance->m_insertMutex);
  }
  
  DbObject* file = DbObject::createFromFileName(fileName, qry);
  if(file) {
    object_id_t result = file->objectId();
    delete file;
    return result;
  }
  

  
  // get the object type
  OBJECT_TYPE objectType = CFileDetails::Shared()->GetObjectType(fileName);  
  if(objectType == OBJECT_TYPE_UNKNOWN) {
    std::cout << "unknown object type: " << fileName << std::endl;
    return 0;
  }

  bool visible = !CSharedConfig::isAlbumArtFile(fileName);
  
  // split path and filename
  string path = ExtractFilePath(fileName);
	fileName = fileName.substr(path.length(), fileName.length() - path.length());

  
  // get the parent object
  if(parentId == 0) {
    DbObject* parent = DbObject::createFromFileName(path, qry);
    if(!parent) {
      cout << "fam error: directory: " << path << " not found" << endl;
      return 0;
    }
    parentId = parent->objectId();
    delete parent;
  }
  
  // format title
  string title = fuppes::FormatHelper::fileNameToTitle(fileName);

  // create the object
  DbObject obj;
  obj.setParentId(parentId);
  obj.setType(objectType);
  obj.setPath(path);
  obj.setFileName(fileName);
  obj.setTitle(title);  
  obj.setVisible(visible);
  obj.save(qry);

  
  return obj.objectId();
}

unsigned int CContentDatabase::insertDirectory(std::string path, std::string title, object_id_t parentId, SQLQuery* qry /*= NULL*/, bool lock /*= false*/) // static
{
  if(lock) {
    MutexLocker locker(&m_Instance->m_insertMutex);
  }

  DbObject* dir = DbObject::createFromFileName(path, qry);
  if(!dir) {
    dir = new DbObject();
    dir->setParentId(parentId);
    dir->setType(CONTAINER_STORAGE_FOLDER);
    dir->setPath(path);
    dir->setTitle(title);
    dir->save(qry);

    m_Instance->m_pFileAlterationMonitor->addWatch(path);    
  }

  object_id_t result = dir->objectId();
  delete dir;
  return result;
}


void CContentDatabase::scanDirectory(std::string path) // static
{
  ScanDirectoryThread* thread = new ScanDirectoryThread(path);
  thread->start();


  std::list<ScanDirectoryThread*>::iterator iter;
  for(iter = m_Instance->m_scanDirThreadList.begin();
      iter != m_Instance->m_scanDirThreadList.end();
      ) {

    if((*iter)->finished()) {
      iter = m_Instance->m_scanDirThreadList.erase(iter);
      delete *iter;
    }
    else {
      iter++;        
    }
  }
  
  m_Instance->m_scanDirThreadList.push_back(thread);
}




void ScanDirectoryThread::run()
{
  DbObject* parent = DbObject::createFromFileName(m_path);

  SQLQuery qry;
  scanDir(&qry, m_path, parent->objectId());
  delete parent;
}

void ScanDirectoryThread::scanDir(SQLQuery* qry, std::string path, unsigned int parentId)
{
  Directory dir(path);
  dir.open();
  DirEntryList entries = dir.dirEntryList();
  dir.close();


  DirEntry entry;
  unsigned int objectId = 0;
  for(unsigned int i = 0; i < entries.size(); i++) {
    entry = entries.at(i);

    if(entry.type() == DirEntry::Directory) {
      //CContentDatabase::insertFile(fileName, qry);
      objectId = CContentDatabase::insertDirectory(entry.absolutePath(), entry.name(), parentId, qry, true);
      scanDir(qry, entry.absolutePath(), objectId);
    }
    else if(entry.type() == DirEntry::File) {
      CContentDatabase::insertFile(entry.absolutePath(), parentId, qry, true);
    }
    
  }
  
}




void RebuildThread::DbScanDir(CContentDatabase* db, SQLQuery* qry, std::string p_sDirectory, long long int p_nParentId)
{
	p_sDirectory = appendTrailingSlash(p_sDirectory);

	if(!Directory::exists(p_sDirectory))
		return;

  Log::log(Log::contentdb, Log::extended, __FILE__, __LINE__, "read dir \"%s\"", p_sDirectory.c_str());


  DbObject obj;
  
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
        if(Directory::exists(sTmp) && !Directory::hidden(sTmp)) {
					
          sTmpFileName = ToUTF8(sTmpFileName);          
          sTmpFileName = SQLEscape(sTmpFileName);        
          
          stringstream sSql;
          
          appendTrailingSlash(&sTmp);
          
          if(m_rebuildType & RebuildThread::addNew) {
            nObjId = GetObjectIDFromFileName(qry, sTmp);
          }
          
          if(nObjId == 0) {

            nObjId = db->GetObjId();
						OBJECT_TYPE folderType = CONTAINER_STORAGE_FOLDER;
	
						// check for album art
						string albumArt = findAlbumArtFile(sTmp);
						if(albumArt.length() > 0) {
							
							unsigned int artId = GetObjectIDFromFileName(qry, albumArt);
#warning device compatibility!?
							folderType = CONTAINER_ALBUM_MUSIC_ALBUM;
							if(artId == 0) {
								InsertFile(db, qry, nObjId, albumArt, true);
							}
						}

            
            obj.reset();
            obj.setObjectId(nObjId);
            obj.setParentId(p_nParentId);
            obj.setType(folderType);
            obj.setPath(sTmp);
            obj.setTitle(sTmpFileName);
            obj.save(qry);

            
            db->fileAlterationMonitor()->addWatch(sTmp);
          }
            
          // recursively scan subdirectories
          DbScanDir(db, qry, sTmp, nObjId);          
        }
        else if(File::exists(sTmp) && CFileDetails::Shared()->IsSupportedFileExtension(sExt)) {
          
          unsigned int objId = 0;
          if(m_rebuildType & RebuildThread::addNew)
            objId = GetObjectIDFromFileName(qry, sTmp);

					if(objId == 0) {
	          InsertFile(db, qry, p_nParentId, sTmp);
            msleep(1);
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

unsigned int findAlbumArt(std::string dir, std::string* ext, SQLQuery* qry)
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



unsigned int RebuildThread::InsertFile(CContentDatabase* pDb, SQLQuery* qry, unsigned int p_nParentId, std::string p_sFileName, bool hidden /* = false*/)
{
  unsigned int nObjId = 0;
 
  if(m_rebuildType & RebuildThread::addNew) {
    nObjId = GetObjectIDFromFileName(qry, p_sFileName);
    if(nObjId > 0) {
      return nObjId;
    }
  } 

  return CContentDatabase::insertFile(p_sFileName);
}

unsigned int InsertURL(std::string p_sURL,
											 std::string p_sTitle = "",
											 std::string p_sMimeType = "")
{
	#warning FIXME: object type
	OBJECT_TYPE nObjectType = ITEM_AUDIO_ITEM_AUDIO_BROADCAST;	
	unsigned int nObjId = CContentDatabase::Shared()->GetObjId();
  SQLQuery qry;

  stringstream sSql;
  sSql << 
	"insert into OBJECTS (TYPE, OBJECT_ID, PATH, FILE_NAME, TITLE, MIME_TYPE) values " <<
  "(" << nObjectType << ", " <<
  nObjId << ", " <<
  "'" << SQLEscape(p_sURL) << "', " <<
  "'" << SQLEscape(p_sURL) << "', " <<
  "'" << SQLEscape(p_sTitle) << "', " <<  
  "'" << SQLEscape(p_sMimeType) << "');";

  qry.exec(sSql.str());
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

	SQLQuery qry;
  qry.select(sGetPlaylists.str());
  while(!qry.eof()) {
    ParsePlaylist(qry.result());
    qry.next();
  }
}
    
unsigned int GetObjectIDFromFileName(SQLQuery* qry, std::string p_sFileName)
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
		"where "
    "  REF_ID = 0 and "
    "  PATH = '" << SQLEscape(path) << "' ";
	
	if(fileName.empty())
		sSQL << " and FILE_NAME is NULL ";
	else
		sSQL << " and FILE_NAME = '" + SQLEscape(fileName) + "' ";
	
	sSQL <<
		"and DEVICE is NULL";
  
  qry->select(sSQL.str());
  if(!qry->eof())
    nResult = qry->result()->asUInt("OBJECT_ID");
  
  return nResult;
}



void ParsePlaylist(CSQLResult* pResult)
{
  BasePlaylistParser* Parser = BasePlaylistParser::Load(pResult->asString("PATH") + pResult->asString("FILE_NAME"));
  if(Parser == NULL) {
    return;
  }

  unsigned int nPlaylistID = pResult->asUInt("OBJECT_ID");
  unsigned int nObjectID   = 0;  

	//cout << "playlist id: " << nPlaylistID << endl;
		
  CContentDatabase* pDb = CContentDatabase::Shared(); //new CContentDatabase();
  SQLQuery qry;
  DbObject* existing;
  
  while(!Parser->Eof()) {
    if(Parser->Entry()->bIsLocalFile && File::exists(Parser->Entry()->sFileName)) {       

      // nObjectID = GetObjectIDFromFileName(&qry, Parser->Entry()->sFileName);
      existing = DbObject::createFromFileName(Parser->Entry()->sFileName, &qry);

      //if(nObjectID == 0) {        
      if(existing == NULL) {
        //nObjectID = InsertFile(pDb, &qry, nPlaylistID, Parser->Entry()->sFileName);
      }            
      else {
        //MapPlaylistItem(nPlaylistID, nObjectID);

        DbObject* object = new DbObject(existing);
        object->setObjectId(pDb->GetObjId());
        object->setParentId(nPlaylistID);        
        object->setRefId(existing->objectId());
        object->save(&qry);
        delete object;
        delete existing;
      }
    }
    else if(!Parser->Entry()->bIsLocalFile) {
      nObjectID = InsertURL(Parser->Entry()->sFileName, Parser->Entry()->sTitle, Parser->Entry()->sMimeType);
			//MapPlaylistItem(nPlaylistID, nObjectID);
    }    
    
    Parser->Next();
  }
  
  delete Parser;
  //delete pDb;
}

//fuppesThreadCallback BuildLoop(void* arg)
void RebuildThread::run()
{
  if(CDatabase::connectionParams().readonly == true) {
    CSharedLog::Print("[ContentDatabase] readonly database rebuild disabled");
    return;
  }


  // stop update thread
  CContentDatabase::Shared()->m_updateThread->stop();
  
  
  DateTime start = DateTime::now();
  CSharedLog::Print("[ContentDatabase] create database at %s", start.toString().c_str());
  

	SQLQuery qry;
  stringstream sSql;
		
  if(m_rebuildType & RebuildThread::rebuild) {
    qry.exec("delete from OBJECTS");
    qry.exec("delete from OBJECT_DETAILS");
    //qry.exec("delete from MAP_OBJECTS");
  }

	/*pDb->Execute("drop index IDX_OBJECTS_OBJECT_ID");
	pDb->Execute("drop index IDX_MAP_OBJECTS_OBJECT_ID");
	pDb->Execute("drop index IDX_MAP_OBJECTS_PARENT_ID");
	pDb->Execute("drop index IDX_OBJECTS_DETAIL_ID");
	pDb->Execute("drop index IDX_OBJECT_DETAILS_ID");*/

  if(m_rebuildType & RebuildThread::removeMissing) {
	
		CSharedLog::Print("remove missing");		
		//CContentDatabase* pDel = new CContentDatabase();
		CSQLQuery* del = CDatabase::query();
		
		qry.select("select * from OBJECTS");
		while(!qry.eof()) {
			CSQLResult* result = qry.result();

			if(result->asUInt("TYPE") < CONTAINER_MAX) {
				if(Directory::exists(result->asString("PATH"))) {
					qry.next();
					continue;
				}
			}
			else {
				if(File::exists(result->asString("PATH") + result->asString("FILE_NAME"))) {
					qry.next();
					continue;
				}
			}
      DbObject* file = new DbObject(result);
      VirtualContainerMgr::deleteFile(file);
      delete file;
      
			sSql << "delete from OBJECT_DETAILS where ID = " << result->asString("OBJECT_ID");
			del->exec(sSql.str());
			sSql.str("");
			
			/*sSql << "delete from MAP_OBJECTS where OBJECT_ID = " << result->asString("OBJECT_ID");
			del->exec(sSql.str());
			sSql.str("");*/
				
			sSql << "delete from OBJECTS where OBJECT_ID = " << result->asString("OBJECT_ID");
			del->exec(sSql.str());
			sSql.str("");
	
			qry.next();
		}

		delete del;
		CSharedLog::Print("[DONE] remove missing");		
	}
		
  qry.connection()->vacuum();
  
  int i;
  unsigned int nObjId = 0;
  string sFileName;
  bool bInsert = true;
  DbObject obj;
  
  CSharedLog::Print("read shared directories");

	CContentDatabase* db = CContentDatabase::Shared();
  SharedObjects* so = CSharedConfig::Shared()->sharedObjects();
	
  for(i = 0; i < so->SharedDirCount(); i++) {
    string tempSharedDir = so->GetSharedDir(i);
		
    if(Directory::exists(tempSharedDir)) { 	
			
			db->fileAlterationMonitor()->addWatch(tempSharedDir);
      
      ExtractFolderFromPath(tempSharedDir, &sFileName);
      bInsert = true;
      if(m_rebuildType & RebuildThread::addNew) {
        if((nObjId = GetObjectIDFromFileName(&qry, tempSharedDir)) > 0) {
          bInsert = false;
        }
      }

      sSql.str("");
      if(bInsert) {      
        nObjId = db->GetObjId();
				sFileName = ToUTF8(sFileName);

        
        obj.reset();
        obj.setObjectId(nObjId);
        obj.setParentId(0);
        obj.setType(CONTAINER_STORAGE_FOLDER);
        obj.setPath(tempSharedDir);
        obj.setTitle(sFileName);
        obj.save(&qry);
				
        /*sSql << 
          "insert into OBJECTS (OBJECT_ID, TYPE, PATH, TITLE) values " <<
          "(" << nObjId << 
          ", " << CONTAINER_STORAGE_FOLDER << 
          ", '" << SQLEscape(CSharedConfig::Shared()->GetSharedDir(i)) << "'" <<
          ", '" << SQLEscape(sFileName) << "');";
        
        qry->insert(sSql.str());
    
        sSql.str("");
        sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID) " <<
          "values (" << nObjId << ", 0)";
      
        qry->insert(sSql.str());*/
      }
      
      DbScanDir(db, &qry, tempSharedDir, nObjId);
    }
    else {      
      CSharedLog::Log(L_EXT, __FILE__, __LINE__,
        "shared directory: \" %s \" not found", tempSharedDir.c_str());
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
    
  //delete db;
	//delete qry;
  
  
  // import iTunes db
  CSharedLog::Print("parse iTunes databases");
  CiTunesImporter* pITunes = new CiTunesImporter();
  for(i = 0; i < so->SharedITunesCount(); i++) {
    pITunes->Import(so->GetSharedITunes(i));
  }
  delete pITunes;
  CSharedLog::Print("[DONE] parse iTunes databases");



  // create virtual folder layout
  if(m_rebuildType & RebuildThread::rebuild) {
    CVirtualContainerMgr::Shared()->RebuildContainerList(true, false);
  }

  
  DateTime end = DateTime::now();
  CSharedLog::Print("[ContentDatabase] database created at %s", end.toString().c_str());


  // start update thread
  CContentDatabase::Shared()->m_updateThread->start();
  
  //g_bIsRebuilding = false;
  //fuppesThreadExit();
}


bool CContentDatabase::exportData(std::string fileName, std::string path, bool remove) // static
{
  CConnectionParams  params;
  params.filename = fileName;

  CDatabasePlugin* sqlitePlugin = CPluginMgr::databasePlugin("sqlite3");
  if(!sqlitePlugin)
    return false;

  CDatabaseConnection* connection = sqlitePlugin->createConnection();
  if(!connection)
    return false;

  if(!connection->connect(params)) {
    delete connection;
    return false;      
  }


  stringstream sql;
  // get from local db
  SQLQuery get;
  // write to new connection
  SQLQuery set(connection);


  sql << set.build(SQL_TABLES_EXIST, 0);
  set.select(sql.str());
  sql.str("");
  if(!set.eof()) { 
    set.exec("drop table FUPPES_DB_INFO");
    set.exec("drop table OBJECTS");
    set.exec("drop table OBJECT_DETAILS");
  }
  
  // create tables
  sql << set.build(SQL_CREATE_TABLE_DB_INFO, 0);
  set.exec(sql.str());
  sql.str("");

  sql << set.build(SQL_CREATE_TABLE_OBJECTS, 0);
  set.exec(sql.str());
  sql.str("");
  
  sql << set.build(SQL_CREATE_TABLE_OBJECT_DETAILS, 0);
  set.exec(sql.str());
  sql.str("");
  
  sql << set.build(SQL_SET_DB_INFO, DB_VERSION);
  set.exec(sql.str());
  sql.str("");

    
  
  // get all objects in path
  path = appendTrailingSlash(path);
  sql.str("");
  sql << "select * from OBJECTS where " <<
    "PATH like '" << SQLEscape(path) << "%' and " <<
    "DEVICE is NULL";

  DbObject* in;
  DbObject* out;
  ObjectDetails details;
  
  object_id_t oid = 0;
  
  get.select(sql.str());
  cout << "START EXPORT" << endl;
  while(!get.eof()) {
    oid++;
    in = new DbObject(get.result());
    out = new DbObject(in);

    cout << "export OBJECT: " << in->title() << "*" << endl;

    // export details
    if(in->detailId() > 0) {
      details.reset();
      details = *in->details();
      details.save(&set);
      
      out->setDetailId(details.id());
    }
    
    // export object
    string newPath = in->path();
    newPath = "./" + StringReplace(newPath, path, "");
    
    out->setPath(newPath);
    out->setObjectId(in->objectId());
    out->save(&set);
    delete out;


    // delete object from local db
    
    
    delete in;
    
    get.next();
  }
  cout << "EXPORT FINISHED" << endl;

  delete connection;
  return true;
}

bool CContentDatabase::importData(std::string fileName, std::string path) // static
{
  CConnectionParams  params;
  params.filename = fileName;

  CDatabasePlugin* sqlitePlugin = CPluginMgr::databasePlugin("sqlite3");
  if(!sqlitePlugin)
    return false;

  CDatabaseConnection* connection = sqlitePlugin->createConnection();
  if(!connection)
    return false;

  if(!connection->connect(params)) {
    delete connection;
    return false;      
  }


  stringstream sql;
  // get from new connection
  SQLQuery get(connection);
    // write to local db
  SQLQuery set;
  
  return false;
}

