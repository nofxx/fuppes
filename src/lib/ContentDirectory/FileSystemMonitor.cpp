/***************************************************************************
 *            FileSystemMonitor.cpp
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "FileSystemMonitor.h"

#include <iostream>
using namespace std;

#ifdef HAVE_INOTIFY
CInotifyMonitor::CInotifyMonitor(IFileSystemMonitor* pEventHandler):
  CFileSystemMonitor(pEventHandler)
{
  int inotify_fd = inotify_init();
	if (inotify_fd < 0)	{
		cout << "error inotify_init()" << endl;
	}
}
#endif // HAVE_INOTIFY
