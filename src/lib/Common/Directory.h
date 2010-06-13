/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Directory.h
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

#ifndef _DIRECTORY_H
#define _DIRECTORY_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../../../include/fuppes_types.h"


#ifdef WIN32

#else

#endif

#include <string>
#include <vector>

#include <dirent.h>


namespace fuppes {

class DirEntry;

typedef std::vector<DirEntry> DirEntryList;

class Directory
{
  public:
    static bool create(std::string directory);
    static bool remove(std::string directory);

    static bool exists(std::string directory);
    static bool readable(std::string directory);
    static bool writable(std::string directory);
    static bool searchable(std::string directory);
    static bool hidden(std::string directory);
		
		Directory(std::string path);
		bool open();
		DirEntryList dirEntryList();
		void close();
		
		static std::string appendTrailingSlash(std::string directory);
		static std::string removeTrailingSlash(std::string directory);

	private:

		std::string		m_path;
		DIR*  				m_dir;
};


class DirEntry
{
	friend class Directory;
	
	public:
		
		enum Type {
			Directory,
			File,
			Symlink			
		};

		DirEntry::Type	type() { return m_type; }
		std::string			path() { return m_path; }
		std::string			name() { return m_name; }

		std::string absolutePath() { return (m_type == DirEntry::Directory ? m_path : m_path + m_name); }
		
		DirEntry& operator=(const DirEntry& entry) {
			m_type = entry.m_type;
			
			m_path = entry.m_path;
			m_name = entry.m_name;      
	  	return *this;
	  }

		
	private:
		DirEntry::Type		m_type;
		
		std::string				m_path;
		std::string				m_name;

};

}

#endif // _DIRECTORY_H
