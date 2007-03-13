/***************************************************************************
 *            FileSystemMonitor.h
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

#ifndef _FILESYSTEMMONITOR_H
#define _FILESYSTEMMONITOR_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifdef HAVE_INOTIFY
#include <sys/inotify.h>
#endif

class IFileSystemMonitor
{
};

class CFileSystemMonitor
{
  protected:
    CFileSystemMonitor(IFileSystemMonitor* pEventHandler) 
      { m_pEventHandler = pEventHandler; }
    
    IFileSystemMonitor* m_pEventHandler;
};

#ifdef HAVE_INOTIFY
class CInotifyMonitor: public CFileSystemMonitor
{
  public:
    CInotifyMonitor(IFileSystemMonitor* pEventHandler);
};
#endif

#endif // _FILESYSTEMMONITOR_H
