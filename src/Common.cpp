/***************************************************************************
 *            Common.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel & Thomas Schnitzler
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include "Common.h"
#include "RegEx.h"

#include <cstdio>
#ifdef WIN32
#include <sys/stat.h>
#else
#include <dirent.h>
#endif
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

bool FileExists(std::string p_sFileName)
{
  std::fstream fFile;
  bool bResult = false;
  
  fFile.open(p_sFileName.c_str(), std::ios::in);  
  bResult = (fFile.fail() != 1);
  fFile.close();
  
  return bResult;
}

bool IsFile(std::string p_sFileName)
{
  return FileExists(p_sFileName);
}
#ifdef WIN32
bool DirectoryExists(std::string p_sDirName)
{
  // Convert string
  const char* pszDirName = p_sDirName.c_str();
  
  // Get file information
  struct _stat info;
  memset(&info, 0, sizeof(info));

  // Check directory exists
  _stat(pszDirName, &info);
  if(0 == (info.st_mode & _S_IFDIR))
    return false;

  return true;
}
#else
bool DirectoryExists(std::string p_sDirName)
{
  DIR* pDir;
  if((pDir = opendir(p_sDirName.c_str())) != NULL)
  {
    closedir(pDir);
    return true;
  }
  else
  {
    return false;
  }  
}
#endif

bool IsDirectory(std::string p_sDirName)
{
  return DirectoryExists(p_sDirName);
}

std::string ExtractFileExt(std::string p_sFileName)
{
  RegEx rxExt("\\.([\\w|\\n]+)$");
  std::string sResult = "";
  if(rxExt.Search(p_sFileName.c_str()))
  {
    do
    {
      sResult = rxExt.Match(1);  
    } while(rxExt.SearchAgain());
    
  }  
  return sResult;
}

std::string ToLower(std::string p_sInput)
{
  for(unsigned int i = 0; i < p_sInput.length(); i++)
  {
    p_sInput[i] = tolower(p_sInput[i]);
  }  
  return p_sInput;
}
