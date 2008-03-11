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
  FAM_DIR_NEW,
  FAM_FILE_MOD,
  FAM_DIR_MOD,
  FAM_FILE_DEL,
  FAM_DIR_DEL  
}FAM_EVENT_TYPE;

class IFileAlterationMonitor
{
  public:
			virtual void FamEvent(FAM_EVENT_TYPE eventType, std::string path, std::string name) = 0;
};

class CFileAlterationMonitor
{
  public:
		virtual ~CFileAlterationMonitor() {
			fuppesThreadDestroyMutex(&mutex);
		}
		
    virtual bool AddDirectory(std::string p_sDirectory) = 0;
  
		void famEvent(FAM_EVENT_TYPE eventType, std::string path, std::string name)	{
			if(!m_pEventHandler) {
				return;
			}
			
			fuppesThreadLockMutex(&mutex);
			m_pEventHandler->FamEvent(eventType, path, name);
			fuppesThreadUnlockMutex(&mutex);
		}
			
  protected:
    CFileAlterationMonitor(IFileAlterationMonitor* pEventHandler) { 
			fuppesThreadInitMutex(&mutex);
			m_pEventHandler = pEventHandler; 
		}
			
    IFileAlterationMonitor* m_pEventHandler;
			
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

#ifdef HAVE_INOTIFY
class CInotifyMonitor: public CFileAlterationMonitor
{
  friend fuppesThreadCallback WatchLoop(void* arg);
  
  public:
    CInotifyMonitor(IFileAlterationMonitor* pEventHandler);
    virtual ~CInotifyMonitor();
  
    bool  AddDirectory(std::string p_sDirectory);
    
  private:
    fuppesThread    m_MonitorThread;  
    std::list<InotifyWatch*>  m_lWatches;
  
    //int   m_nInotifyFd;
    Inotify*        m_pInotify;
};
#endif

#endif // _FILEALTERATIONMONITOR_H
