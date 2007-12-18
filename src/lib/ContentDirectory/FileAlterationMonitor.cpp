/***************************************************************************
 *            FileAlterationMonitor.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <fuppes@ulrich-voelkel.de>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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
  
  #if defined(HAVE_GAMIN)
  pResult = new CGaminMonitor(pEventHandler);
  #endif
  
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
  InotifyWatch* pWatch = new InotifyWatch(p_sDirectory, IN_CREATE | IN_MODIFY | IN_DELETE | IN_MOVE);
  try {
    m_pInotify->Add(pWatch);
  }
  catch(InotifyException &ex) {
    cout << "exception: " << ex.GetMessage() << endl;
  }
  m_lWatches.push_back(pWatch);
  
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
    }    
  }
  
  
  
  /*int rc = 0;
	struct timeval  timeout;
	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;
	struct timeval* timeout_ptr = &timeout;
	
	fd_set read_fd;
	FD_ZERO(&read_fd);
	FD_SET(pInotify->m_nInotifyFd, &read_fd);
	
	struct inotify_event event;
	
	int bytes_to_read = 0;
	
  cout << "inotify loop" << endl;
  
	while (true) {
	  rc = select(pInotify->m_nInotifyFd + 1, &read_fd, NULL, NULL, timeout_ptr); 
	  if(rc > 0) {
  	  cout << "select: " << rc << endl;  	  
	    do {
		    rc = ioctl(pInotify->m_nInotifyFd, FIONREAD, &bytes_to_read);
	    } while (!rc && bytes_to_read < sizeof(struct inotify_event));
  	  
  	  cout << "bytes to read: " << bytes_to_read << endl;
  	  
	    rc = read(pInotify->m_nInotifyFd, &event, bytes_to_read); // sizeof(struct inotify_event));
	    if(rc > 0) {
	      cout << "read: " << rc << endl;
	      cout << event.name << " :: " << event.mask << endl;
	    }
	  }
	}*/
  
  fuppesThreadExit();
}
  
#endif // HAVE_INOTIFY


#ifdef HAVE_GAMIN
CGaminMonitor::CGaminMonitor(IFileAlterationMonitor* pEventHandler):
  CFileAlterationMonitor(pEventHandler)
{
  FAMOpen(&m_FAMConnection);    
  FAMMonitorDirectory(&m_FAMConnection, "/home/ulrich/Desktop/famtest", &m_FAMRequest, NULL);  
    
  FAMEvent event;

  while(true) {
    FAMNextEvent(&m_FAMConnection, &event);    
    cout << "file/dir: " << event.filename << " changed: " << event.code << "." << endl;
  }
}

CGaminMonitor::~CGaminMonitor()
{
  FAMCancelMonitor(&m_FAMConnection, &m_FAMRequest);
  FAMClose(&m_FAMConnection);
}

bool CGaminMonitor::AddDirectory(std::string p_sDirectory)
{
  FAMMonitorDirectory(&m_FAMConnection, p_sDirectory, &m_FAMRequest, NULL);  
}
#endif // HAVE_GAMIN
