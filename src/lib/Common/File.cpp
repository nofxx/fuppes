/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            File.cpp
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


#include "File.h"

#include <sys/stat.h>
#include <sys/types.h>

#ifndef WIN32
#include <unistd.h>
#include <stdio.h>
#endif

using namespace fuppes;
using namespace std;

File::File(std::string fileName)
{
	m_fileName = fileName;
	m_openMode = Closed;
#ifndef WIN32
  m_file = NULL;
#endif
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
#ifndef WIN32
	std::string openmode;

	if(mode & Read)
		openmode += "r";
	if(mode & Write)
		openmode += "w";
	/*if(mode & Truncate)
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
  
#else
  ios_base::openmode openmode = ios_base::in;

	if(mode & Read)
		openmode |= ios_base::in;
	if(mode & Write)
		openmode |= ios_base::out;
	if(mode & Truncate)
		openmode |= ios_base::trunc;
	if(mode & Append)
		openmode |= ios_base::app;
	if(!(mode & Text))
		openmode |= ios_base::binary;
  
	m_fstream.open(m_fileName.c_str(), openmode);
#endif
  
  return isOpen();
}

bool File::isOpen()
{
#ifndef WIN32
  return (m_file != NULL);
#else
  return m_fstream.is_open();
#endif
}

void File::close()
{
#ifndef WIN32
  fclose(m_file);
  m_file = NULL;
#else
	m_fstream.close();
#endif
}

bool File::seek(fuppes_off_t offset)
{ 
  if(!isOpen())
    return false;
  
#ifndef WIN32  
  return (fseeko(m_file, offset, SEEK_SET) == 0);
#else
  m_fstream.clear();
  m_fstream.seekg(offset, ios::beg);
  return m_fstream.good();
#endif
}

fuppes_off_t File::read(char* buffer, fuppes_off_t length)
{
  if(!isOpen())
    return false;

#ifndef WIN32
  return fread(buffer, 1, length, m_file);
#else
  m_fstream.clear();
  m_fstream.read(buffer, length);
  return m_fstream.good();
#endif
}

fuppes_off_t File::write(char* buffer, fuppes_off_t length)
{
  if(!isOpen())
    return false;

#ifndef WIN32
  return fwrite(buffer, 1, length, m_file);
#else
  m_fstream.clear();
  m_fstream.write(buffer, length);
  return m_fstream.good();
#endif
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
  if(!isOpen())
    return false;

#ifdef WIN32
  m_fstream.clear();
  char buffer[1000];
  m_fstream.getline(buffer, 1000);
  if(m_fstream.good()) {
    line = buffer;
  }
  return m_fstream.good();
#else
  
  
  fuppes_off_t start;
  start = ftello(m_file);
  int c;
  do {
    c = fgetc(m_file);
    if (c == 10 || c == 13) {
      break;
    }
  } while (c != EOF);

  fuppes_off_t end;
  end = ftello(m_file);

  if(end > start) {
    seek(start);

    char* buffer = new char[end - start + 1];
    read(buffer, end - start);
    buffer[end - start] = '\0';
    line = buffer;
    delete[] buffer;
    return true;    
  }

  return false;
#endif
}


bool File::exists(std::string fileName) // static
{
	struct stat Stat;  
  return (stat(fileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode) != 0);
}

bool File::readable(std::string fileName) // static
{
	struct stat Stat;
  bool isReadable = false;

  if(stat(fileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode)) {
#ifndef WIN32
    // check other then group
    if(Stat.st_mode & S_IROTH) {
      isReadable = true;  // I am an other
    }
    if(!isReadable && (Stat.st_mode & S_IRGRP)) {
      if (getgid() == Stat.st_gid) {
        isReadable = true;
      }
    }
    if(!isReadable && (Stat.st_mode & S_IRUSR)) {
      if (getuid() == Stat.st_uid) {
        isReadable = true;
      }
    }
#else
    // TODO i'm pretty sure that you can read and write to all windows files, correct me if i'm wrong
    isReadable = true;
#endif
  }

  return isReadable;
}

bool File::writable(std::string fileName) // static
{
	struct stat Stat;
  bool isWritable = false;

  if(stat(fileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode)) {
#ifndef WIN32
    // check other then group
    if(Stat.st_mode & S_IWOTH) {
      isWritable = true;  // I am an other
    }
    if(!isWritable && (Stat.st_mode & S_IWGRP)) {
      if (getgid() == Stat.st_gid) {
        isWritable = true;
      }
    }
    if(!isWritable && (Stat.st_mode & S_IWUSR)) {
      if (getuid() == Stat.st_uid) {
        isWritable = true;
      }
    }
#else
    // TODO i'm pretty sure that you can read and write to all windows files, correct me if i'm wrong
    isWritable = true;
#endif
  }

  return isWritable;
}

bool File::executable(std::string fileName) // static
{
	struct stat Stat;
  bool isExecutable = false;

  if(stat(fileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode)) {
#ifndef WIN32
    // check other then group
    if(Stat.st_mode & S_IXOTH) {
      isExecutable = true;  // I am an other
    }
    if(!isExecutable && (Stat.st_mode & S_IXGRP)) {
      if (getgid() == Stat.st_gid) {
        isExecutable = true;
      }
    }
    if(!isExecutable && (Stat.st_mode & S_IXUSR)) {
      if (getuid() == Stat.st_uid) {
        isExecutable = true;
      }
    }
#else
    // TODO i'm pretty sure that you can read and write to all windows files, correct me if i'm wrong
    isExecutable = true;
#endif
  }

  return isExecutable;
}


time_t File::lastModified(std::string fileName) // static
{
  struct stat Stat;
  if(stat(fileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode)) {
    return Stat.st_ctime;
  }

  return 0;
}
