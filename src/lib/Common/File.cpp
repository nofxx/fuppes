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
  m_file = NULL;
}

File::~File()
{
  if(isOpen())
    close();
}

void File::setFileName(std::string fileName)
{
  m_fileName = fileName;
}

bool File::open(File::OpenMode mode)
{
	std::string openmode;

	if(mode & Read)
		openmode += "r";
	/*if(mode & Write)
		openmode += "a";
	if(mode & Truncate)
		openmode |= "w";
	if(mode & Append)
		openmode += "a";*/
	if(!mode & Text)
		openmode += "b";
	
/*
"r"	Open a file for reading. The file must exist.
"w"	Create an empty file for writing. If a file with the same name already exists its content is erased and the file is treated as a new empty file.
"a"	Append to a file. Writing operations append data at the end of the file. The file is created if it does not exist.
"r+"	Open a file for update both reading and writing. The file must exist.
"w+"	Create an empty file for both reading and writing. If a file with the same name already exists its content is erased and the file is treated as a new empty file.
"a+"	Open a file for reading and appending. All writing operations are performed at the end of the file, protecting the previous content to be overwritten. You can reposition (fseek, rewind) the internal pointer to anywhere in the file for reading, but writing operations will move it back to the end of file. The file is created if it does not exist.
*/

  m_file = fopen(m_fileName.c_str(), openmode.c_str());
  return isOpen();;

	/*m_fstream.open(m_fileName.c_str(), openmode);
	return m_fstream.is_open();*/
}

bool File::isOpen()
{
  return (m_file != NULL);
}

void File::close()
{
  fclose(m_file);
  m_file = NULL;
	//m_fstream.close();
}

bool File::seek(fuppes_off_t offset)
{
  if(!m_file)
    return false;

  return (fseeko(m_file, offset, SEEK_SET) == 0);
}

fuppes_off_t File::read(char* buffer, fuppes_off_t length)
{
  if(!m_file)
    return false;

  return fread(buffer, 1, length, m_file);
}

fuppes_off_t File::size()
{
	struct stat Stat;  
  if(stat(m_fileName.c_str(), &Stat) != 0)
		return 0;

	return Stat.st_size;
}

/*bool File::getline(std::string& line)
{
  if(!m_fstream.is_open())
    return false;

  return std::getline(m_fstream, line);
}*/


bool File::exists(std::string fileName) // static
{
	struct stat Stat;  
  return (stat(fileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode) != 0);
}
