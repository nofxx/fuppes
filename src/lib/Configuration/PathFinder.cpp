/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            PathFinder.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2010 Robert Massaioli <robertmassaioli@gmail.com>
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

#include "PathFinder.h"
#include "../Common/Directory.h"

using namespace std;
using namespace fuppes;


PathFinder* PathFinder::m_instance = NULL;


void PathFinder::init() // static
{
  assert(m_instance == NULL);
  m_instance = new PathFinder();
}

void PathFinder::uninit() // static
{
  assert(m_instance != NULL);
  delete m_instance;
  m_instance = NULL;
}

PathFinder* PathFinder::instance() // static
{
  assert(m_instance != NULL);
  return m_instance;
}


PathFinder::PathFinder()
{
  #ifdef WIN32
  m_paths.push_back(string(getenv("APPDATA")) + "\\FUPPES\\");
  #else
  // .fuppes has higher priority than /etc/
  m_paths.push_back(string(getenv("HOME")) + "/.fuppes/");
  m_paths.push_back(Directory::appendTrailingSlash(FUPPES_SYSCONFDIR));
  #endif

  devicesPath = DEVICE_DIRECTORY;
  vfolderPath = VFOLDER_DIRECTORY;
}

/*
string PathFinder::defaultPath(void)
{
  assert(m_paths.size() > 0);
  return m_paths.front();
}
*/

void PathFinder::addConfigPath(std::string path) // static
{ 
  if(!path.empty())
    instance()->m_paths.insert(instance()->m_paths.begin(), path); 
}

string PathFinder::findInPath(std::string fileName, File::Flags flags /*= File::Readable*/, std::string extra /*= ""*/)
{
  bool found = false;
  string tempName = "";
  vector<string>::const_iterator it;
  for(it = m_paths.begin(); it != m_paths.end(); ++it) {
    tempName = *it;
    tempName += extra;
    tempName += fileName;
    if(File::exists(tempName)) {
      #warning todo check flags
      found = true;
      break;     
    }
  }

  return (found ? tempName : "");
}

string PathFinder::findDeviceInPath(string device)
{
  return findInPath(device + DEVICE_EXT, File::Readable, appendTrailingSlash(devicesPath));
}

string PathFinder::findVFolderInPath(string device)
{
  return findInPath(device + VFOLDER_EXT, File::Readable, appendTrailingSlash(vfolderPath));
}

/*
void PathFinder::walker(bool (*step)(string)) {
  Directory::walk(&m_paths, step);
}
*/