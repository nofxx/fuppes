/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Directory.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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


#include "Directory.h"

#include <sys/stat.h>
#include <sys/types.h>

#ifdef WIN32
#include <string.h>
#endif

using namespace fuppes;

#define SLASH "/"
#define BACKSLASH "\\"

bool Directory::exists(std::string directory) // static
{
#ifdef WIN32
	directory = removeTrailingSlash(directory);
	
  struct _stat info;
  memset(&info, 0, sizeof(info));

  _stat(directory.c_str(), &info);
  return ((info.st_mode & _S_IFDIR) != 0);	
#else
	directory = appendTrailingSlash(directory);
	
	struct stat Stat;  
  return (stat(directory.c_str(), &Stat) == 0 && S_ISDIR(Stat.st_mode) != 0);
#endif
}

std::string Directory::appendTrailingSlash(std::string directory) // static
{
	if(directory.length() <= 2)
		return directory;

	if(directory.substr(directory.length() - 1).compare(SLASH) != 0 &&
     directory.substr(directory.length() - 1).compare(BACKSLASH) != 0) {
		directory += SLASH;
	}

	return directory;
}

std::string Directory::removeTrailingSlash(std::string directory) // static
{
	if(directory.length() <= 2)
		return directory;

	if(directory.substr(directory.length() - 1).compare(SLASH) == 0 ||
     directory.substr(directory.length() - 1).compare(BACKSLASH) == 0) {
		return directory.substr(0, directory.length() - 1);
	}

	return directory;
}

