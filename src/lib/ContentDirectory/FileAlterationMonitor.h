/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            FileAlterationMonitor.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007-2009 Ulrich Völkel <fuppes@ulrich-voelkel.de>
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

#ifndef _FILEALTERATIONMONITOR_H
#define _FILEALTERATIONMONITOR_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../Common/Common.h"
#include "../Common/Thread.h"

#ifdef HAVE_INOTIFY
#include "inotify-cxx-0.7.2/inotify-cxx.h"
#endif

#include <string>
#include <map>

typedef enum {

  FAM_UNKNOWN   = 0,
  FAM_CREATE    = 1,
  FAM_DELETE    = 2,
  FAM_MOVE      = 4,
  FAM_MODIFY    = 8
    
}FAM_EVENT_TYPE;

/*
 dir events:
    FAM_CREATE              = new dir created
    FAM_CREATE | FAM_MOVE   = dir moved in from unwatched dir
    FAM_MOVE                = dir moved inside watched dirs
    FAM_DELETE              = dir deleted / moved outside watched dirs
 
 file events:
    FAM_CREATE              = new file created / moved in from unwatched dir
    FAM_MOVE                = file moved inside watched dirs
    FAM_DELETE              = file deleted / moved outside watched dirs
    FAM_MODIFY              = file modified 
*/


class CFileAlterationMonitor;
#ifdef HAVE_INOTIFY
class CInotifyMonitor;
#endif
#ifdef WIN32
class CWindowsFileMonitor;
#endif

class CFileAlterationEvent
{
  friend class CFileAlterationMonitor;
	#ifdef HAVE_INOTIFY
	friend class CInotifyMonitor;
	#endif
	#ifdef WIN32
	friend class CWindowsFileMonitor;
	#endif
	
  /*#ifdef HAVE_INOTIFY
  friend fuppesThreadCallback WatchLoop(void* arg);
  #endif*/
  
  public:
    CFileAlterationEvent() {
      m_type = FAM_UNKNOWN;
      m_isDir = false;
    }
    
    int             type() { return m_type; }
    bool            isDir() { return m_isDir; }

    std::string     path() { return m_path; }
    std::string     file() { return m_file; }
    std::string     fullPath() { return m_path + m_file; }
    
    // for moved events
    std::string     oldPath() { return m_oldPath; }
    std::string     oldFile() { return m_oldFile; }
    std::string     oldFullPath() { return m_oldPath + m_oldFile; }
    
  private:
    int               m_type;
    bool              m_isDir;
    
    std::string       m_path;
    std::string       m_file;
    std::string       m_oldPath;
    std::string       m_oldFile;
};

class IFileAlterationMonitor
{
  public:
    //virtual void FamEvent(FAM_EVENT_TYPE eventType, std::string path, std::string name, std::string oldPath = "", std::string oldName = "") = 0;
     virtual void FamEvent(CFileAlterationEvent* event) = 0;
};

class CFileAlterationMonitor: protected fuppes::Thread
{
  public:
		virtual ~CFileAlterationMonitor() {
			fuppesThreadDestroyMutex(&mutex);
		}
		
    virtual bool  addWatch(std::string path) = 0;
    virtual void  removeWatch(std::string path) = 0;
    virtual void  moveWatch(std::string fromPath, std::string toPath) = 0;
    
    bool isActive() { return m_active; }
    
    /*void famEvent(FAM_EVENT_TYPE eventType, std::string path, std::string name, std::string oldPath = "", std::string oldName = "")	{
			if(!m_pEventHandler) {
				return;
			}
			
      appendTrailingSlash(&path);
      appendTrailingSlash(&oldPath);      
      
			fuppesThreadLockMutex(&mutex);
			m_pEventHandler->FamEvent(eventType, path, name, oldPath, oldName);
			fuppesThreadUnlockMutex(&mutex);
		}*/
    
    void FamEvent(CFileAlterationEvent* event) {
      fuppesThreadLockMutex(&mutex);
			m_pEventHandler->FamEvent(event);
			fuppesThreadUnlockMutex(&mutex);
    }
			
  protected:
    CFileAlterationMonitor(IFileAlterationMonitor* pEventHandler) { 
			fuppesThreadInitMutex(&mutex);
			m_pEventHandler = pEventHandler; 
		}
			
    IFileAlterationMonitor* m_pEventHandler;

    bool m_active;
		fuppesThreadMutex	mutex;
};

class CFileAlterationMgr
{
  public:
    static CFileAlterationMgr* Shared();
  
    CFileAlterationMonitor* CreateMonitor(IFileAlterationMonitor* pEventHandler);
  
  private:
    static CFileAlterationMgr* m_Instance;
};

class CDummyMonitor: public CFileAlterationMonitor
{
  public:
    CDummyMonitor(IFileAlterationMonitor* pEventHandler)
      :CFileAlterationMonitor(pEventHandler) { m_active = false; }

    virtual ~CDummyMonitor() {}
    virtual bool  addWatch(std::string /*path*/) { return true; }
    virtual void  removeWatch(std::string /*path*/) { }
    virtual void  moveWatch(std::string /*fromPath*/, std::string /*toPath*/) { }
		
	private:
		void run() {}
};

#ifdef HAVE_INOTIFY
class CInotifyMonitor: public CFileAlterationMonitor
{
  public:
    CInotifyMonitor(IFileAlterationMonitor* pEventHandler);
    virtual ~CInotifyMonitor();
  
    bool  addWatch(std::string path);
    void  removeWatch(std::string path);
    void  moveWatch(std::string fromPath, std::string toPath);
    
  private:
    Inotify*                                m_pInotify;
		void run();
    // path, watch
    std::map<std::string, InotifyWatch*>    m_watches;
};
#endif


#ifdef WIN32
class CWindowsFileMonitor: public CFileAlterationMonitor
{
  public:
    CWindowsFileMonitor(IFileAlterationMonitor* pEventHandler);
    virtual ~CWindowsFileMonitor();
    
    bool  addWatch(std::string path);
    void  removeWatch(std::string path);
    void  moveWatch(std::string fromPath, std::string toPath);
		
	private:
		void run();
    std::map<std::string, HANDLE>    m_watches;
};


#endif


#endif // _FILEALTERATIONMONITOR_H
