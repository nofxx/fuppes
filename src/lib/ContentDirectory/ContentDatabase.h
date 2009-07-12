/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ContentDatabase.h
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

#include "DatabaseConnection.h"

class RebuildThread: public fuppes::Thread
{
	private:
		void run();
};
	
class CContentDatabase: public IFileAlterationMonitor
{
  public:
    static CContentDatabase* Shared();  
  
    CContentDatabase(bool p_bShared = false);
    ~CContentDatabase();
  
    std::string GetLibVersion();

    bool Init(bool* p_bIsNewDB);
  

		bool Execute(std::string p_sStatement);
    unsigned int Insert(std::string p_sStatement);

    bool Select(std::string p_sStatement);
  

    bool Eof();
    CSQLResult* GetResult();
    void Next();
  
    std::list<CSQLResult*> m_ResultList;
    std::list<CSQLResult*>::iterator m_ResultListIterator;
    unsigned int  m_nRowsReturned;		
		
    void RebuildDB();
    void UpdateDB();
    void AddNew();
    void RemoveMissing();
    bool IsRebuilding();// { return m_bIsRebuilding; };
	
    unsigned int GetObjId();
  
		fuppesThreadMutex m_Mutex;
  
    CFileAlterationMonitor* fileAlterationMonitor() { return Shared()->m_pFileAlterationMonitor; }
    //void FamEvent(FAM_EVENT_TYPE eventType, std::string path, std::string name, std::string oldPath = "", std::string oldName = "");
    void FamEvent(CFileAlterationEvent* event);
    
    void deleteObject(unsigned int objectId); 
    void deleteContainer(std::string path);    
    
  private:    
    void BuildDB();
    
		void Lock();
    void Unlock();
    void ClearResult();
		
	  //fuppesThread  m_RebuildThread;
		RebuildThread*	 m_RebuildThread;

    //bool m_bIsRebuilding;
		bool m_bShared;
    static CContentDatabase* m_Instance;
    int m_nLockCount;
  
    CFileAlterationMonitor* m_pFileAlterationMonitor;
  
    std::string   m_sDbFileName;
    bool          m_bInTransaction;
    bool Open();
    void Close();
};

#endif // _CONTENTDATABASE_H
