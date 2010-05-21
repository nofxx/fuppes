/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            FileAlterationMonitor.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007-2010 Ulrich Völkel <fuppes@ulrich-voelkel.de>
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

#include "../SharedLog.h"
using namespace fuppes;

//#ifdef WIN32
#include <iostream>
using namespace std;
//#endif


void CFileAlterationMonitor::famEvent(CFileAlterationEvent* event) 
{
  if(m_pEventHandler == NULL)
    return;
  
	Log::log(Log::fam, Log::debug, __FILE__, __LINE__, CFileAlterationEvent::toString(event));  
  //fuppesThreadLockMutex(&mutex);
	m_pEventHandler->famEvent(event);
	//fuppesThreadUnlockMutex(&mutex);
}



/*CFileAlterationMgr* CFileAlterationMgr::m_Instance = 0;

CFileAlterationMgr* CFileAlterationMgr::Shared() // static
{
  if(m_Instance == 0) {
    m_Instance = new CFileAlterationMgr();
  }
  return m_Instance;  
}

void CFileAlterationMgr::deleteInstance() // static
{
  if(m_Instance == 0)
    return;
  delete m_Instance;
  m_Instance = NULL;
}*/

CFileAlterationMonitor* CFileAlterationMgr::CreateMonitor(IFileAlterationMonitor* pEventHandler)
{
  CFileAlterationMonitor* pResult = NULL;
  
  #if defined(HAVE_INOTIFY)
  pResult = new CInotifyMonitor(pEventHandler);
  #endif
  
	#ifdef WIN32
  pResult = new CWindowsFileMonitor(pEventHandler);
	#endif
	
  // if no real monitor is available we return a
  // dummy monitor that does nothing but the
  // content database does not need to care whether
  // fam is available or not
  if(pResult == NULL) {
    pResult = new CDummyMonitor(pEventHandler);
  }
  
  return pResult;
}

#ifdef HAVE_INOTIFY
CInotifyMonitor::CInotifyMonitor(IFileAlterationMonitor* pEventHandler):
  CFileAlterationMonitor(pEventHandler)
{
  m_pInotify = new Inotify();
  m_active = true;
}

CInotifyMonitor::~CInotifyMonitor()
{
  close();
  
  std::map<std::string, InotifyWatch*>::iterator iter;
  for(iter = m_watches.begin(); iter != m_watches.end(); iter++) {
	  m_pInotify->Remove(iter->second);
    delete iter->second;
  }
  m_watches.clear();
  
  delete m_pInotify;
}
  
bool CInotifyMonitor::addWatch(std::string path)
{  
  appendTrailingSlash(&path);
  cout << "add watch: " << path << endl;
  if(m_watches.find(path) != m_watches.end()) {
    //cout << "watch already exists: " << path << endl;
    return false;
  }

	Log::log(Log::fam, Log::extended, __FILE__, __LINE__, "add watch \"%s\"", path.c_str());

  InotifyWatch* pWatch = NULL;
  try { // IN_UNMOUNT
		pWatch = new InotifyWatch(path, IN_CREATE | IN_DELETE | IN_MOVE | IN_CLOSE_WRITE); // IN_MODIFY 
    m_pInotify->Add(pWatch);
    m_watches[path] = pWatch;
  }
  catch(InotifyException &ex) {
    //cout << "addWatch :: exception: " << ex.GetMessage() << endl << path << endl;
    if(pWatch)
      delete pWatch;
		Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "addWatch :: exception \"%s\"", ex.GetMessage().c_str());
  }
  
	if(!this->running()) {
    this->start();
  }

  return true;
}
  
void CInotifyMonitor::removeWatch(std::string path)
{
  appendTrailingSlash(&path);
  cout << "remove watch: " << path << endl;
	Log::log(Log::fam, Log::extended, __FILE__, __LINE__, "remove watch \"%s\"", path.c_str());
	
  std::map<std::string, InotifyWatch*>::iterator iter;
  if((iter = m_watches.find(path)) == m_watches.end()) {
    //cout << "watch not found: " << path << endl;
    return;
  }


  std::string tmpPath;

  // iterate over all watches ...
  for(iter = m_watches.begin();
      iter != m_watches.end(); ) {

    // ... check if the watche's path contains 'path'
    tmpPath = iter->first;
    if(tmpPath.length() >= path.length() &&
       tmpPath.substr(0, path.length()).compare(path) == 0) {

       cout << "delete watch: " << iter->first << endl;
         
       // delete the watch
       m_pInotify->Remove(iter->second);
       delete iter->second;

       // remove the map entry
       m_watches.erase(iter++);
    }
    else {
      ++iter;
    }
        
  }
  

  /*
	try {
	  m_pInotify->Remove(iter->second);
		delete iter->second;  
	  m_watches.erase(iter);  
  }
  catch(InotifyException &ex) {
		//cout << "removeWatch :: exception: " << ex.GetMessage() << endl << path << endl;
		Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "removeWatch :: exception \"%s\"", ex.GetMessage().c_str());
  } 
*/
}

void CInotifyMonitor::moveWatch(std::string fromPath, std::string toPath)
{
  appendTrailingSlash(&fromPath);
  appendTrailingSlash(&toPath);

  cout << "move watch: " << fromPath << " to: " << toPath << endl;

  std::map<std::string, InotifyWatch*>::iterator iter;
  if((iter = m_watches.find(fromPath)) == m_watches.end()) {
    //cout << "watch not found: " << path << endl;
    return;
  }


  string path;
  InotifyWatch* watch;

  std::list<InotifyWatch*> tmpStore;
  std::list<InotifyWatch*>::iterator tmpStoreIter;

  // iterate over all watches ...
  for(iter = m_watches.begin();
      iter != m_watches.end(); ) {

    // ... check if the path contains 'fromPath'
    path = iter->first;
    if(path.length() >= fromPath.length() &&
       path.substr(0, fromPath.length()).compare(fromPath) == 0) {

       // modify the watch and store it in a temp list 
       watch = iter->second;
       path = toPath + path.substr(fromPath.length(), path.length());
       watch->SetPath(path);
       cout << "set watch path: " << path << endl;
       tmpStore.push_back(watch);

       // remove the map entry
       m_watches.erase(iter++);
    }
    else {
      ++iter;
    }
        
  }

  // append watches from our temp list
  for(tmpStoreIter = tmpStore.begin();
      tmpStoreIter != tmpStore.end();
      ++tmpStoreIter) {
    watch = *tmpStoreIter;
    m_watches[watch->GetPath()] = watch;
  }
  
  
  
/*  removeWatch(fromPath);
  addWatch(toPath);
  */
}

//fuppesThreadCallback WatchLoop(void* arg)    
void CInotifyMonitor::run()
{
  CInotifyMonitor* pInotify = this; //(CInotifyMonitor*)arg;
  InotifyEvent event;
  InotifyEvent peek;
  
  
  std::string   absEventPath;
  unsigned int  movedFromCookie = 0;
  std::string   movedFromPath;
  std::string   movedFromFile;
  bool          movedFromIsDir = false;
  
  CFileAlterationEvent   famEvent;
  // path, event
  std::map<std::string, CFileAlterationEvent*>            events;
  std::map<std::string, CFileAlterationEvent*>::iterator  eventsIter;  

  size_t numEvents;

  pInotify->m_pInotify->SetNonBlock(true);
  
  while(!this->stopRequested()) {
  
    // wait for events
    try {    
      pInotify->m_pInotify->WaitForEvents();
    }
    catch(InotifyException &ex) {
			Log::log(Log::fam, Log::normal, __FILE__, __LINE__, "exception \"%s\"", ex.GetMessage().c_str());
    }

    if(this->stopRequested())
      break;
    
    numEvents = pInotify->m_pInotify->GetEventCount();

    if(numEvents == 0) {
      msleep(100);
      continue;
    }    
    Log::log(Log::fam, Log::debug, __FILE__, __LINE__, "got %d events", numEvents);

    // process events
    while(pInotify->m_pInotify->GetEvent(&event)) {


      std::string types;
      event.DumpTypes(types);
      //cout << "event: " << "cookie: " << event.GetCookie() << " types: " << types << endl;
      Log::log(Log::fam, Log::debug, __FILE__, __LINE__, 
        "event :: cookie: %d types: %s", event.GetCookie(), types.c_str());


      if(event.IsType(IN_IGNORED)) {
        //cout << "inotify: IN_IGNORED" << endl;        
        /*string sDump;
        event.DumpTypes(sDump);
        cout << "cookie: " << event.GetCookie() << " mask: " << sDump << endl;*/        
        continue;
      } 

      
      // build absolute path
      absEventPath = event.GetWatch()->GetPath() + event.GetName();            
      if(event.IsType(IN_ISDIR)) {
        absEventPath = Directory::appendTrailingSlash(absEventPath);
      }

      
      
      // IN_CREATE
			if(event.IsType(IN_CREATE)) {

        cout << "CREATE EVENT: " << absEventPath << endl;
        
        // directories just send a CREATE event so we can
        // throw the event right now
        if(event.IsType(IN_ISDIR)) {
          pInotify->addWatch(absEventPath);
          
          famEvent.m_type  = FAM_CREATE;
          famEvent.m_isDir = true;
          famEvent.m_path  = event.GetWatch()->GetPath();
          famEvent.m_file  = event.GetName();
          
          pInotify->famEvent(&famEvent);
        }
        // the file's CREATE event is always followed by a
        // CLOSE event. therefore we queue it and wait for CLOSE
        else {
    
          events[absEventPath] = new CFileAlterationEvent();
          events[absEventPath]->m_type  = FAM_CREATE;
          events[absEventPath]->m_isDir = false;
          events[absEventPath]->m_path  = event.GetWatch()->GetPath();
          events[absEventPath]->m_file  = event.GetName();
        }        
        
			} // IN_CREATE

      
      // IN_DELETE
      else if(event.IsType(IN_DELETE)) {
        
        cout << "DELETE EVENT: " << absEventPath << endl;

        if(event.IsType(IN_ISDIR)) {
          pInotify->removeWatch(absEventPath);
        }
        
        famEvent.m_type = FAM_DELETE;
        famEvent.m_path = event.GetWatch()->GetPath();
        famEvent.m_file = event.GetName();        
        famEvent.m_isDir = event.IsType(IN_ISDIR);
        pInotify->famEvent(&famEvent);

      } // IN_DELETE        

      
      // IN_CLOSE_WRITE
      else if(event.IsType(IN_CLOSE_WRITE)) {
        //cout << "object closed: " << eventPath << " cookie: " << event.GetCookie() << endl;

        cout << "CLOSE_WRITE EVENT: " << absEventPath << endl;
        
        // check if there is a create event and throw it ...
        eventsIter = events.find(absEventPath);
        if(eventsIter != events.end()) {
          
          CFileAlterationEvent* evt = eventsIter->second;
          pInotify->famEvent(evt);
          delete evt;
          events.erase(eventsIter); 
        }
        // ... else we have a (file) modify event
        /*else {
          famEvent.m_type  = FAM_MODIFY;
          famEvent.m_isDir = false;
          famEvent.m_path  = event.GetWatch()->GetPath();
          famEvent.m_file  = event.GetName();
          pInotify->famEvent(&famEvent);
        }*/
        
      } // IN_CLOSE_WRITE
      
      
      // IN_MOVED_FROM
      else if(event.IsType(IN_MOVED_FROM)) {        
        //cout << "object moved from: " << eventPath << " [NEW]" << endl;
        
        movedFromCookie = event.GetCookie();
        movedFromPath   = event.GetWatch()->GetPath();
        movedFromFile   = event.GetName();
        movedFromIsDir  = event.IsType(IN_ISDIR);

        // if there is an other event in the queue an 
        // if it is a MOVED_TO with the same cookie
        // the dir is moved inside the shared dirs
        if(pInotify->m_pInotify->PeekEvent(&peek) &&
           peek.IsType(IN_MOVED_TO) && 
           peek.GetCookie() == event.GetCookie()) {
           // the event is thrown in the next round of this loop
           continue;
        }
        // else the dir is moved outside the shared dirs
        // which means we throw a delete event and remove the watch
        else {

          famEvent.m_type  = FAM_DELETE;
          famEvent.m_path  = movedFromPath;
          famEvent.m_file  = movedFromFile;        
          famEvent.m_isDir = movedFromIsDir;
          movedFromCookie = 0;

          if(movedFromIsDir) {
            pInotify->removeWatch(Directory::appendTrailingSlash(movedFromPath + movedFromFile));
          }

          pInotify->famEvent(&famEvent);          
        }
        
      } // IN_MOVED_FROM
      
      // IN_MOVED_TO
      else if(event.IsType(IN_MOVED_TO)) {

        //cout << "object moved to: " << eventPath << " [NEW]" << endl;
        
        if(event.GetCookie() == movedFromCookie) {
          //cout << "moved already watched object from : " << movedFromPath + movedFromFile << " to: " << eventPath << " [MOVE]" << endl;            

          famEvent.m_type  = FAM_MOVE;
          famEvent.m_isDir = event.IsType(IN_ISDIR);
          famEvent.m_path  = event.GetWatch()->GetPath();
          famEvent.m_file  = event.GetName();
          famEvent.m_oldPath  = movedFromPath;
          famEvent.m_oldFile  = movedFromFile;

          if(event.IsType(IN_ISDIR)) {
            pInotify->moveWatch(famEvent.m_oldPath + famEvent.m_oldFile, 
                                famEvent.m_path + famEvent.m_file);
          }
          
          pInotify->famEvent(&famEvent);          

          movedFromCookie = 0;
        }
        else {
          //cout << "new object moved in to: " << eventPath << " [NEW]" << endl;
          
          famEvent.m_type  = FAM_CREATE;
          famEvent.m_isDir = event.IsType(IN_ISDIR);
          famEvent.m_path  = event.GetWatch()->GetPath();
          famEvent.m_file  = event.GetName();                 
                     
          if(event.IsType(IN_ISDIR)) {
            pInotify->addWatch(absEventPath); 
          }
          pInotify->famEvent(&famEvent); 
        }

      }  // IN_MOVED_TO   
      

    } // while getEvents
    
  }
  
  //fuppesThreadExit();
}
  
#endif // HAVE_INOTIFY


#ifdef WIN32

/*
http://msdn.microsoft.com/en-us/library/aa365261(VS.85).aspx
http://msdn.microsoft.com/en-us/library/aa365465(VS.85).aspx
ReadDirectoryChangesW
*/

CWindowsFileMonitor::CWindowsFileMonitor(IFileAlterationMonitor* pEventHandler)
:CFileAlterationMonitor(pEventHandler)
{
  m_active = true;
}

CWindowsFileMonitor::~CWindowsFileMonitor()
{
  close();
}
  
bool CWindowsFileMonitor::addWatch(std::string path)
{
  appendTrailingSlash(&path);
	//cout << "create watch: " << path << endl;


	// todo: create watch and store in m_watches map
	
	
	if(!this->running()) {
    this->start();
  }
}
  
void CWindowsFileMonitor::removeWatch(std::string path)
{
  appendTrailingSlash(&path);
  //cout << "remove watch: " << path << endl;

	// todo: destroy watch and remove from m_watches map
	
  
}

void CWindowsFileMonitor::moveWatch(std::string fromPath, std::string toPath)
{
  appendTrailingSlash(&fromPath);
  appendTrailingSlash(&toPath);

  //cout << "move watch: " << fromPath << " to: " << toPath << endl;

  removeWatch(fromPath);
  addWatch(toPath);
}

void CWindowsFileMonitor::run()
{
	/*while(!this->stopRequested() {
		
		// todo: watch loop
		
	}*/
}

#endif // WIN32


#ifdef HAVE_KQUEUE

/*
http://wiki.netbsd.se/kqueue_tutorial
http://developer.apple.com/Mac/library/documentation/Darwin/Reference/ManPages/man2/kqueue.2.html
 */

#endif // HAVE_KQUEUE