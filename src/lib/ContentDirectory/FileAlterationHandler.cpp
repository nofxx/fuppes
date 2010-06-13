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
#include "FileDetails.h"
#include "VirtualContainerMgr.h"
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

  if(event->type() != FAM_MODIFY) {  
    m_lastEventTime = DateTime::now();
  }
  
  //cout << CFileAlterationEvent::toString(event) << endl;

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

  // modify
  else if(event->type() == FAM_MODIFY && !event->isDir()) {

    Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "fam modify file: %s", event->path().c_str());
    modifyFile(event);
    
  } // modify
    
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

  // remove all virtual files that are build from the directories content
  VirtualContainerMgr::deleteDirectory(dir);

  // remove the dir
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

  // get the new parent
  DbObject* parent = DbObject::createFromFileName(event->path());
  if(!parent) {
    cout << "fam error: directory: " << event->path() << " not found" << endl;
    delete obj;
    return;
  }
  
  string newPath = Directory::appendTrailingSlash(event->path() + event->dir());
  obj->setPath(newPath);
  obj->setTitle(event->dir());
  obj->setParentId(parent->objectId());
  obj->save();
  
  delete obj;
  delete parent;
                                               
  // increment systemUpdateId
  CContentDatabase::incSystemUpdateId();
}



void FileAlterationHandler::createFile(CFileAlterationEvent* event)
{
  CContentDatabase::insertFile(event->path() + event->file(), 0, NULL, true);

  // virtual folder structure is updated by UpdateThread after reading the files metadata
}

void FileAlterationHandler::deleteFile(CFileAlterationEvent* event)
{
  DbObject* file = DbObject::createFromFileName(event->path() + event->file());
  if(!file) {
    cout << "fam error: file: " << (event->path() + event->file()) << " not found" << endl;
    return;
  }


  // TODO remove thumbnail for video file
  // CSharedConfig::Shared()->globalSettings->GetTempDir() << file->objectId() << ".jpg"

// check for album art
  

  // delete the file from the virtual folder structure
  VirtualContainerMgr::deleteFile(file);
  
  file->remove();
  delete file;
}

void FileAlterationHandler::moveFile(CFileAlterationEvent* event)
{
  // if the old object was an unsupported file type ...
  OBJECT_TYPE objectType = CFileDetails::Shared()->GetObjectType(event->oldFile());  
  // ... we create a new file
  if(objectType == OBJECT_TYPE_UNKNOWN) {
    createFile(event);
    return;
  } 

  
  // get the object from the old path
  string oldPath = event->oldPath() + event->oldFile();
  DbObject* file = DbObject::createFromFileName(oldPath);
  if(!file) {
    cout << "fam error: file: " << oldPath << " not found" << endl;
    return;
  }

  // get the new parent
  DbObject* parent = DbObject::createFromFileName(event->path());
  if(!parent) {
    cout << "fam error: dir: " << event->path() << " not found" << endl;
    delete file;
    return;
  }


cout << "old parent id: " << file->parentId() << " new pid: " << parent->objectId() << endl;
  

// check for album art
  
  file->setParentId(parent->objectId());
  file->setPath(event->path());
  file->setFileName(event->file());

  string title = FormatHelper::fileNameToTitle(event->oldFile());
  if(file->title() == title) {
    file->setTitle(TruncateFileExt(event->file()));
  }
  file->save();  
  delete file;
  delete parent;

  // increment systemUpdateId
  CContentDatabase::incSystemUpdateId();
}



void FileAlterationHandler::modifyFile(CFileAlterationEvent* event)
{
  // get the object
  DbObject* file = DbObject::createFromFileName(event->path() + event->file());
  if(!file) {
    cout << "fam error: file: " << event->path() + event->file() << " not found" << endl;
    return;
  }

  // check the modification time
  time_t modified = File::lastModified(event->path() + event->file());

 /* 
  cout << "MODIFIED  : " << modified << endl;
  cout << "DBMODIFIED: " << file->lastModified() << endl;
  cout << "DBUPDATED : " << file->lastUpdated() << endl;
  */
  

  if(modified > file->lastUpdated()) {
    file->setLastModified(modified);
    file->save();
  }
  delete file;

  // UpdateThread will check the file for changes and update the virtual folder structure if necessary

  // increment systemUpdateId
  //CContentDatabase::incSystemUpdateId();
}
