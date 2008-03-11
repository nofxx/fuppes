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
  
  return pResult;
}

#ifdef HAVE_INOTIFY

fuppesThreadCallback WatchLoop(void* arg);

CInotifyMonitor::CInotifyMonitor(IFileAlterationMonitor* pEventHandler):
  CFileAlterationMonitor(pEventHandler)
{
  m_pInotify = new Inotify();
  m_MonitorThread = (fuppesThread)NULL;
}

CInotifyMonitor::~CInotifyMonitor()
{
  std::list<InotifyWatch*>::iterator iter;
  
  for(iter = m_lWatches.begin(); iter != m_lWatches.end(); iter++) {
    //inotify_rm_watch(m_nInotifyFd, iter);
  }
  
  fuppesThreadCancel(m_MonitorThread);
  fuppesThreadClose(m_MonitorThread);
}
  
bool CInotifyMonitor::AddDirectory(std::string p_sDirectory)
{
	//p_sDirectory = p_sDirectory.substr(0, p_sDirectory.length()-1);
	cout << "create watch: " << p_sDirectory << endl;
  try {
		InotifyWatch* pWatch = new InotifyWatch(p_sDirectory, IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVE);
    m_pInotify->Add(pWatch);
		m_lWatches.push_back(pWatch);
  }
  catch(InotifyException &ex) {
    cout << "exception: " << ex.GetMessage() << endl;
  }
  
  if(!m_MonitorThread) {
    fuppesThreadStart(m_MonitorThread, WatchLoop);
  }

  return true;
}
  
fuppesThreadCallback WatchLoop(void* arg)    
{
  CInotifyMonitor* pInotify = (CInotifyMonitor*)arg;
  InotifyEvent event;
  
  while(true) {
  
    cout << "wait for events" << endl;
    
    try {    
      pInotify->m_pInotify->WaitForEvents();
    }
    catch(InotifyException &ex) {
      cout << "exception" << ex.GetMessage() << endl;
    }
    
    cout << "got " << pInotify->m_pInotify->GetEventCount() << " events" << endl;
    
    while(pInotify->m_pInotify->GetEvent(&event)) {
      cout << "event: " << event.GetName() << endl;
      string sDump;
      event.DumpTypes(sDump);
      cout << sDump << endl << endl;

			string dir;
			
			if(event.IsType(IN_ISDIR)) {
				cout << "is dir: " << event.GetWatch()->GetPath() << endl;
				
				if(event.IsType(IN_CREATE) || event.IsType(IN_MOVED_TO)) {
					
					dir = event.GetWatch()->GetPath() + event.GetName() + "/";					
					pInotify->AddDirectory(dir);					
					pInotify->famEvent(FAM_DIR_NEW, 
														event.GetWatch()->GetPath(),
														event.GetName());
				}				
			}
			
			/*IN_MOVED_TO,IN_ISDIR
			IN_CREATE,IN_ISDIR
			IN_DELETE,IN_ISDIR
			IN_MOVED_FROM,IN_ISDIR*/

    }
  }
  
  fuppesThreadExit();
}
  
#endif // HAVE_INOTIFY
