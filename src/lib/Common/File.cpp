/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            File.cpp
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


#include "File.h"

#include <sys/stat.h>
#include <sys/types.h>

using namespace fuppes;


File::File(std::string fileName)
{
	m_fileName = fileName;
	m_openMode = Closed;
}

bool File::open(File::OpenMode mode)
{
	std::fstream::openmode openmode;

	if(mode & Read)
		openmode |= std::fstream::in;
	if(mode & Write)
		openmode |= std::fstream::out;
	if(mode & Truncate)
		openmode |= std::fstream::trunc;
	if(mode & Append)
		openmode |= std::fstream::app;
	if(!mode & Text)
		openmode |= std::fstream::binary;
	
	m_fstream.open(m_fileName.c_str(), openmode);
	return m_fstream.is_open();
}

void File::close()
{
	m_fstream.close();
}

fuppes_off_t File::size()
{
	struct stat Stat;  
  if(stat(m_fileName.c_str(), &Stat) != 0)
		return 0;

	return Stat.st_size;
}

bool File::getline(std::string& line)
{
  if(!m_fstream.is_open())
    return false;

  return std::getline(m_fstream, line);
}


bool File::exists(std::string fileName) // static
{
	struct stat Stat;  
  return (stat(fileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode) != 0);
}
