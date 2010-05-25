/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            FileAlterationHandler.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "FileAlterationHandler.h"

#include "../Log.h"
#include "DatabaseObject.h"
#include "ContentDatabase.h"
using namespace fuppes;

#include <iostream>
using namespace std;

FileAlterationHandler::FileAlterationHandler()
{
  m_fileAlterationMonitor = NULL;
  m_lastEventTime = DateTime::now();
}

void FileAlterationHandler::setMonitor(CFileAlterationMonitor* fileAlterationMonitor)
{
  m_fileAlterationMonitor = fileAlterationMonitor;
}

void FileAlterationHandler::famEvent(CFileAlterationEvent* event)
{
  fuppes::MutexLocker locker(&m_mutex);

  
  m_lastEventTime = DateTime::now();

  cout << CFileAlterationEvent::toString(event) << endl;

  // create
  if(event->type() == FAM_CREATE) {

    if(event->isDir()) {
      Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "fam create dir: %s", event->path().c_str());
      createDirectory(event);
    }
    else {
      Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "fam create file: %s", event->path().c_str());
      createFile(event);
    }
    
  } // create

  // delete
  else if(event->type() == FAM_DELETE) {

    if(event->isDir()) {
      Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "fam delete dir: %s", event->path().c_str());
      deleteDirectory(event);
    }
    else {
      Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "fam delete file: %s", event->path().c_str());
      deleteFile(event);
    }
    
  } // delete

  // move
  else if(event->type() == FAM_MOVE) {

    if(event->isDir()) {
      Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "fam move dir: %s", event->path().c_str());
      moveDirectory(event);
    }
    else {
      Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "fam move file: %s", event->path().c_str());
      moveFile(event);
    }
    
  } // move

  
  /*
 stringstream sSql;
	unsigned int objId;
	unsigned int parentId;  
  SQLQuery qry;
  
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
      DbScanDir(this, &qry, appendTrailingSlash(event->fullPath()), objId);
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

		//CSQLQuery* qry = CDatabase::query();
		qry.select(sSql.str());
		sSql.str("");

    string newPath;  
    //CContentDatabase db; 

		while(!qry.eof()) {
			CSQLResult* result = qry.result();

      newPath = StringReplace(result->asString("PATH"), appendTrailingSlash(event->oldFullPath()), appendTrailingSlash(event->fullPath()));

#warning move fam watch
			//m_pFileAlterationMonitor->moveWatch();
			
      #warning sql prepare
      sSql << 
        "update OBJECTS set " <<
        " PATH = '" << SQLEscape(newPath) << "' " <<
        "where ID = " << result->asString("ID");
      
      //cout << sSql.str() << endl;
      Execute(sSql.str());
      sSql.str("");

			qry.next();
		}   

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

*/
  
  
}


void FileAlterationHandler::createDirectory(CFileAlterationEvent* event)
{
  // get the parent
  DbObject* parent = DbObject::createFromFileName(event->path());
  if(!parent) {
    cout << "fam error: directory: " << event->path() << " not found" << endl;
    return;
  }

  // insert the directory
  string path = Directory::appendTrailingSlash(event->path() + event->dir());
  CContentDatabase::insertDirectory(path, event->dir(), parent->objectId(), NULL, true);
  delete parent;

  // scan the directory
  CContentDatabase::scanDirectory(path);

  // increment systemUpdateId
  CContentDatabase::incSystemUpdateId();
}

void FileAlterationHandler::deleteDirectory(CFileAlterationEvent* event)
{  
  // get the object
  string path = Directory::appendTrailingSlash(event->path() + event->dir());
  DbObject* dir = DbObject::createFromFileName(path);
  if(!dir) {
    cout << "fam error: directory: " << path << " not found" << endl;
    return;
  }

  dir->remove();
  delete dir;

  // increment systemUpdateId
  CContentDatabase::incSystemUpdateId();
}

void FileAlterationHandler::moveDirectory(CFileAlterationEvent* event)
{
  // get the object from the old path
  string oldPath = Directory::appendTrailingSlash(event->oldPath() + event->oldDir()); 
  DbObject* obj = DbObject::createFromFileName(oldPath);
  if(!obj) {
    cout << "fam error: directory: " << oldPath << " not found" << endl;
    return;
  }

  string newPath = Directory::appendTrailingSlash(event->path() + event->dir());
  obj->setPath(newPath);
  obj->setTitle(event->dir());
  obj->save();
  delete obj;
                                               
  // increment systemUpdateId
  CContentDatabase::incSystemUpdateId();
}



void FileAlterationHandler::createFile(CFileAlterationEvent* event)
{
  CContentDatabase::insertFile(event->path() + event->file(), 0, NULL, true);
}

void FileAlterationHandler::deleteFile(CFileAlterationEvent* event)
{
  DbObject* file = DbObject::createFromFileName(event->path() + event->file());
  if(!file) {
    cout << "fam error: file: " << (event->path() + event->file()) << " not found" << endl;
    return;
  }
  
  file->remove();
  delete file;
}

void FileAlterationHandler::moveFile(CFileAlterationEvent* event)
{
  // get the object from the old path
  string oldPath = event->oldPath() + event->oldFile();
  DbObject* file = DbObject::createFromFileName(oldPath);
  if(!file) {
    cout << "fam error: file: " << oldPath << " not found" << endl;
    return;
  }

  string newPath = event->path();
  file->setPath(newPath);
  file->setFileName(event->file());

  string title = FormatHelper::fileNameToTitle(event->oldFile());
  if(file->title() == title) {
    file->setTitle(TruncateFileExt(event->file()));
  }
  file->save();
  delete file;

  // increment systemUpdateId
  CContentDatabase::incSystemUpdateId();
}

