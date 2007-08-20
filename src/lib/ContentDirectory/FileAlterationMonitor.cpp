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
  #ifdef HAVE_GAMIN
  return new CGaminMonitor(pEventHandler);
  #else
  return NULL;
  #endif
}

#ifdef HAVE_INOTIFY
CInotifyMonitor::CInotifyMonitor(IFileSystemMonitor* pEventHandler):
  CFileSystemMonitor(pEventHandler)
{
  int inotify_fd = inotify_init();
	if (inotify_fd < 0)	{
		cout << "error inotify_init()" << endl;
	}
	
	int wd = inotify_add_watch(inotify_fd, "/home/ulrich/Desktop/test/", IN_CREATE);
	cout << "wd: " << wd << endl;	
	
	usleep(1000 * 100);
	
	int rc = 0;
	struct timeval  timeout;
	timeout.tv_sec  = 10;
	timeout.tv_usec = 0;
	struct timeval* timeout_ptr = &timeout;
	
	fd_set read_fd;
	FD_ZERO(&read_fd);
	FD_SET(inotify_fd, &read_fd);
	
	struct inotify_event event;
	
	int bytes_to_read = 0;
	
	while (true)
	{
	  rc = select(inotify_fd + 1, &read_fd, NULL, NULL, timeout_ptr); 
	  if(rc > 0) {
  	  cout << "select: " << rc << endl;
  	  
	    do {
		    rc = ioctl(inotify_fd, FIONREAD, &bytes_to_read);
	    } while (!rc && bytes_to_read < sizeof(struct inotify_event));
  	  
  	  cout << "bytes to read: " << bytes_to_read << endl;
  	  
	    rc = read(inotify_fd, &event, bytes_to_read); // sizeof(struct inotify_event));
	    if(rc > 0) {
	      cout << "read: " << rc << endl;
	      cout << event.name << " :: " << event.mask << endl;
	    }
	  }
	}
	
	inotify_rm_watch(inotify_fd, wd);
	
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
