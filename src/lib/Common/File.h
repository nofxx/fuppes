/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            File.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifndef _FILE_H
#define _FILE_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../../../include/fuppes_types.h"


#ifdef WIN32

#else

#endif

#include <string>
#include <fstream>

namespace fuppes {

class File
{
  public:

	  enum OpenMode {
		  Closed		= 0,
		  Read			= 1,
		  Write			= 2,
		  ReadWrite = Read | Write,
		  Truncate  = 4,
		  Append		= 8,
		  Text			= 16
	  };

    enum Flags {
      None        = 0,
      Readable    = 1,
      Writable    = 2,
      Executable  = 4
    };
    
		File(std::string fileName = "");
    ~File();
    void          setFileName(std::string fileName);
		bool          open(File::OpenMode mode);
		void          close();
		fuppes_off_t  size();
    bool          isOpen();

    bool          seek(fuppes_off_t);
    fuppes_off_t  read(char* buffer, fuppes_off_t length);
    bool          getline(std::string& line);
    fuppes_off_t  write(char* buffer, fuppes_off_t length);

    static bool remove(std::string fileName);
    
    static bool exists(std::string fileName);
    static bool readable(std::string fileName);
    static bool writable(std::string fileName);
    static bool executable(std::string fileName);

    static time_t lastModified(std::string fileName);
    
	private:
		std::string   m_fileName;
		OpenMode			m_openMode;
    //std::fstream  m_fstream;
    FILE*         m_file;
};

}

#endif // _FILE_H
