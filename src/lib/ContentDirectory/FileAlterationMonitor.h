/***************************************************************************
 *            FileAlterationMonitor.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007-2008 Ulrich Völkel <fuppes@ulrich-voelkel.de>
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

#ifdef HAVE_INOTIFY
#include "inotify-cxx-0.7.2/inotify-cxx.h"
#endif

#include <string>
#include <list>

typedef enum {
  FAM_FILE_NEW,
  FAM_FILE_DEL,
  FAM_FILE_MOD,   // modified (e.g. metadata change)
  FAM_FILE_MOVE,  // rename or move inside watched dirs
  
  FAM_DIR_NEW,    // newly created or moved into watched dirs
  FAM_DIR_DEL,    // watched dir deleted
  FAM_DIR_MOVE    // rename or move inside watched dirs

}FAM_EVENT_TYPE;

class IFileAlterationMonitor
{
  public:
    virtual void FamEvent(FAM_EVENT_TYPE eventType, std::string path, std::string name, std::string oldPath = "", std::string oldName = "") = 0;
};

class CFileAlterationMonitor
{
  public:
		virtual ~CFileAlterationMonitor() {
			fuppesThreadDestroyMutex(&mutex);
		}
		
    virtual bool addWatch(std::string path) = 0;
    bool isActive() { return m_active; }
    
    void famEvent(FAM_EVENT_TYPE eventType, std::string path, std::string name, std::string oldPath = "", std::string oldName = "")	{
			if(!m_pEventHandler) {
				return;
			}
			
      appendTrailingSlash(&path);
      appendTrailingSlash(&oldPath);      
      
			fuppesThreadLockMutex(&mutex);
			m_pEventHandler->FamEvent(eventType, path, name, oldPath, oldName);
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
    virtual bool addWatch(std::string path) { return true; }
};

#ifdef HAVE_INOTIFY
class CInotifyMonitor: public CFileAlterationMonitor
{
  friend fuppesThreadCallback WatchLoop(void* arg);
  
  public:
    CInotifyMonitor(IFileAlterationMonitor* pEventHandler);
    virtual ~CInotifyMonitor();
  
    bool  addWatch(std::string path);
    void  removeWatch(std::string path);
    void  moveWatch(std::string fromPath, std::string toPath);
    
  private:
    Inotify*                                m_pInotify;  
    fuppesThread                            m_MonitorThread;    
    // path, watch
    std::map<std::string, InotifyWatch*>    m_watches;    
};
#endif

#endif // _FILEALTERATIONMONITOR_H
