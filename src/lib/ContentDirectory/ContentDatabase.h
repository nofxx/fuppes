/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ContentDatabase.h
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

#ifndef _CONTENTDATABASE_H
#define _CONTENTDATABASE_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include <map>
#include <list>
#include "../Common/Common.h"
#include "../Common/Thread.h"
#include "FileAlterationMonitor.h"
#include "FileAlterationHandler.h"
#include "UpdateThread.h"

#include "DatabaseConnection.h"

class CContentDatabase;

class RebuildThread: public fuppes::Thread
{
	public:

    enum RebuildType {
      rebuild       = 1,
      addNew        = 2,
      removeMissing = 4
    };
    
		RebuildThread() : fuppes::Thread("db rebuild thread") {
      m_rebuildType = rebuild;
		}

    ~RebuildThread() {
      close();
    }
    
    void setRebuildType(int type) {
      m_rebuildType = type;
    }
    
	private:
		void run();

    void DbScanDir(CContentDatabase* db, SQLQuery* qry, std::string p_sDirectory, long long int p_nParentId);
    unsigned int InsertFile(CContentDatabase* pDb, SQLQuery* qry, unsigned int p_nParentId, std::string p_sFileName, bool hidden = false);
    
    int m_rebuildType;
};


class ScanDirectoryThread: public fuppes::Thread
{
  public:
    ScanDirectoryThread(std::string path) :
      fuppes::Thread("ScanDirectoryThread" + path) {
      m_path = path;
	  }
  
  private:
    void run();
    void scanDir(SQLQuery* qry, std::string path, unsigned int parentId);

    std::string m_path;
};

class CContentDatabase
{
  friend class RebuildThread;
  
  public:
    static CContentDatabase* Shared();  
    ~CContentDatabase();

    bool Init(bool* p_bIsNewDB);

    void RebuildDB();
    void UpdateDB();
    void AddNew();
    void RemoveMissing();
    bool IsRebuilding();
	
    static unsigned int GetObjId();
   
    CFileAlterationMonitor* fileAlterationMonitor() { return Shared()->m_pFileAlterationMonitor; }
    
    void deleteObject(unsigned int objectId); 
    void deleteContainer(std::string path);    



    static unsigned int insertFile(std::string fileName, object_id_t parentId = 0, SQLQuery* qry = NULL, bool lock = true);
    static unsigned int insertDirectory(std::string path, std::string title, object_id_t parentId, SQLQuery* qry = NULL, bool lock = true);

    static void scanDirectory(std::string path);

    static int systemUpdateId();
    static void incSystemUpdateId();

    
  private:    
    CContentDatabase();
    void BuildDB(int rebuildType);
    
		RebuildThread*	 m_rebuildThread;
    fuppes::UpdateThread*    m_updateThread;

    static CContentDatabase* m_Instance;
  
    CFileAlterationMonitor* m_pFileAlterationMonitor;
    FileAlterationHandler*  m_fileAlterationHandler;

		unsigned int	m_objectId;
    unsigned int  m_systemUpdateId;

    std::list<ScanDirectoryThread*>     m_scanDirThreadList;
    fuppes::Mutex                       m_insertMutex;
};

#endif // _CONTENTDATABASE_H
