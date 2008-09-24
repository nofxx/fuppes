/***************************************************************************
 *            FileAlterationMonitor.cpp
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

#include "FileAlterationMonitor.h"

#ifdef HAVE_INOTIFY
#include <sys/ioctl.h>
#include <sys/inotify.h>
#endif

#include <iostream>
using namespace std;

CFileAlterationMgr* CFileAlterationMgr::m_Instance = 0;

CFileAlterationMgr* CFileAlterationMgr::Shared()
{
  if(m_Instance == 0) {
    m_Instance = new CFileAlterationMgr();
  }
  return m_Instance;  
}

CFileAlterationMonitor* CFileAlterationMgr::CreateMonitor(IFileAlterationMonitor* pEventHandler)
{
  CFileAlterationMonitor* pResult = NULL;
  
  #if defined(HAVE_INOTIFY)
  pResult = new CInotifyMonitor(pEventHandler);
  #endif
  
  // if no real monitor is available we return a
  // dummy monitor that does nothing but the
  // content database does not need to care wether
  // fam is available or not
  if(pResult == NULL) {
    pResult = new CDummyMonitor(pEventHandler);
  }
  
  return pResult;
}

#ifdef HAVE_INOTIFY

fuppesThreadCallback WatchLoop(void* arg);

CInotifyMonitor::CInotifyMonitor(IFileAlterationMonitor* pEventHandler):
  CFileAlterationMonitor(pEventHandler)
{
  m_pInotify = new Inotify();
  m_MonitorThread = (fuppesThread)NULL;
  m_active = true;
}

CInotifyMonitor::~CInotifyMonitor()
{
 /* std::list<InotifyWatch*>::iterator iter;  
  for(iter = m_lWatches.begin(); iter != m_lWatches.end(); iter++) {
    //inotify_rm_watch(m_nInotifyFd, iter);
  }*/

  fuppesThreadCancel(m_MonitorThread);
  fuppesThreadClose(m_MonitorThread);
  
  delete m_pInotify;
}
  
bool CInotifyMonitor::addWatch(std::string path)
{
  appendTrailingSlash(&path);
  
	//p_sDirectory = p_sDirectory.substr(0, p_sDirectory.length()-1);
	//cout << "create watch: " << path << endl;
  
  if(m_watches.find(path) != m_watches.end()) {
    cout << "watch already exists: " << path << endl;
    return false;
  }
  
  try {    
		InotifyWatch* pWatch = new InotifyWatch(path, IN_CREATE | IN_DELETE | IN_MOVE | IN_MODIFY);
    m_pInotify->Add(pWatch);
    m_watches[path] = pWatch;
  }
  catch(InotifyException &ex) {
    cout << "exception: " << ex.GetMessage() << endl;
  }
  
  if(!m_MonitorThread) {
    fuppesThreadStart(m_MonitorThread, WatchLoop);
  }

  return true;
}
  
void CInotifyMonitor::removeWatch(std::string path)
{
  appendTrailingSlash(&path);
  //cout << "remove watch: " << path << endl;
  
  std::map<std::string, InotifyWatch*>::iterator iter;
  if((iter = m_watches.find(path)) == m_watches.end()) {
    cout << "watch not found: " << path << endl;
    return;
  }
  
  m_pInotify->Remove(iter->second);
  delete iter->second;  
  m_watches.erase(iter);  
}

void CInotifyMonitor::moveWatch(std::string fromPath, std::string toPath)
{
  appendTrailingSlash(&fromPath);
  appendTrailingSlash(&toPath);

  //cout << "move watch: " << fromPath << " to: " << toPath << endl;
  
  removeWatch(fromPath);
  addWatch(toPath);
}

fuppesThreadCallback WatchLoop(void* arg)    
{
  CInotifyMonitor* pInotify = (CInotifyMonitor*)arg;
  InotifyEvent event;
  
  
  std::string eventPath;
  int         movedFromCookie = 0;
  std::string movedFromPath;
  std::string movedFromName;
  bool        movedFromIsDir = false;
  
  while(true) {
  
    //cout << "wait for events" << endl;
    
    try {    
      pInotify->m_pInotify->WaitForEvents();
    }
    catch(InotifyException &ex) {
      cout << "exception" << ex.GetMessage() << endl;
    }
    
    //cout << "got " << pInotify->m_pInotify->GetEventCount() << " events" << endl;
    
    while(pInotify->m_pInotify->GetEvent(&event)) {
      
      if(event.IsType(IN_IGNORED)) {
        continue;
      }        

      FAM_EVENT_TYPE  type;
      
      string sDump;
      event.DumpTypes(sDump);      

      eventPath = event.GetWatch()->GetPath() + event.GetName();      
      
      //cout << "event: " << eventPath << endl;
      //cout << "cookie: " << event.GetCookie() << " mask: " << sDump << endl;
      
				
      // IN_CREATE
			if(event.IsType(IN_CREATE)) {        
        //cout << "object created: " << eventPath << " [NEW]" << endl;
        
        if(event.IsType(IN_ISDIR))
   				pInotify->addWatch(eventPath);
        event.IsType(IN_ISDIR) ? type = FAM_DIR_NEW : type = FAM_FILE_NEW;
				pInotify->famEvent(type, event.GetWatch()->GetPath(), event.GetName());       
			}
      
      // IN_MOVED_FROM
      else if(event.IsType(IN_MOVED_FROM)) {
        
        // moved from is still set so the associated file was moved
        // to outside the watched dirs
        if(movedFromCookie != 0) {
          //cout << "object moved outside watched dirs: " << movedFromPath + movedFromName << " [DEL]" << endl;
          pInotify->removeWatch(movedFromPath + movedFromName);
          
          movedFromIsDir ? type = FAM_DIR_DEL : type = FAM_FILE_DEL;
          pInotify->famEvent(type, movedFromPath, movedFromName);
          movedFromCookie = 0;
        }

        movedFromCookie = event.GetCookie();
        movedFromPath   = event.GetWatch()->GetPath();
        movedFromName   = event.GetName();
        movedFromIsDir  = event.IsType(IN_ISDIR);
      }
      
      // IN_MOVED_TO
      else if(event.IsType(IN_MOVED_TO)) {

        if(event.GetCookie() == movedFromCookie) {
          //cout << "moved already watched object from : " << movedFromPath + movedFromName << " to: " << eventPath << " [MOVE]" << endl;            

          if(event.IsType(IN_ISDIR))
            pInotify->moveWatch(movedFromPath + movedFromName, eventPath);          
          event.IsType(IN_ISDIR) ? type = FAM_DIR_MOVE : type = FAM_FILE_MOVE;          
          pInotify->famEvent(type, event.GetWatch()->GetPath(), event.GetName(), movedFromPath, movedFromName);
          movedFromCookie = 0;
        }
        else {
          //cout << "new object moved in to: " << eventPath << " [NEW]" << endl;
          if(event.IsType(IN_ISDIR))
  				  pInotify->addWatch(eventPath);
          event.IsType(IN_ISDIR) ? type = FAM_DIR_NEW : type = FAM_FILE_NEW;
  				pInotify->famEvent(type,	event.GetWatch()->GetPath(), event.GetName());
        }
      }
      
      // IN_DELETE
      else if(event.IsType(IN_DELETE)) {
        //cout << "object deleted: " << eventPath << endl;
        if(event.IsType(IN_ISDIR))
    			pInotify->removeWatch(eventPath);
        event.IsType(IN_ISDIR) ? type = FAM_DIR_DEL : type = FAM_FILE_DEL;
  			pInotify->famEvent(type,	event.GetWatch()->GetPath(), event.GetName());
      }
        

      //cout << endl;
    } // while getEvents
    
  }
  
  fuppesThreadExit();
}
  
#endif // HAVE_INOTIFY
