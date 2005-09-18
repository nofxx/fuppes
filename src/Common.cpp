/***************************************************************************
 *            Common.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
#include <fcntl.h>

using namespace std;

/*===============================================================================
 File Functions
===============================================================================*/

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
  /* Convert string */
  const char* pszDirName = p_sDirName.c_str();
  
  /* Get file information */
  struct _stat info;
  memset(&info, 0, sizeof(info));

  /* Check directory exists */
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

/*===============================================================================
 String Functions
===============================================================================*/

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

bool SplitURL(std::string p_sURL, std::string* p_sIPAddress, unsigned int* p_nPort)
{
  RegEx rxSplit("[http://]*([0-9|\\.]+):*([0-9]*)");
  if(rxSplit.Search(p_sURL.c_str()))
  {    
    (*p_sIPAddress) = rxSplit.Match(1);
    if(rxSplit.SubStrings() == 3)
      *p_nPort = atoi(rxSplit.Match(2));
    else
      *p_nPort = 80;
    
    return true;
  }
  else
  {
    return false;
  }
}

/* BASE64 decoding
   taken from: http://www.adp-gmbh.ch/cpp/common/base64.html
   modified to handle line breaks
*/
static inline bool IsBase64(unsigned char c) {
  return (isalnum(c) || (c == '+') || (c == '/'));
}

static const std::string base64_chars = 
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

std::string Base64Decode(const std::string p_sInputString)
{
  int nSize = p_sInputString.size();
  int i     = 0;
  int j     = 0;
  int in_   = 0;
  unsigned char char_array_4[4];
  unsigned char char_array_3[3];
  std::string   sResult;

  while(nSize-- &&  (p_sInputString[in_] != '=') && 
        (IsBase64(p_sInputString[in_]) ||
        (p_sInputString[in_] == '\n')  ||
        (p_sInputString[in_] == '\r'))
        )
  {
    /* continue line break */
    if ((p_sInputString[in_] == '\n') || (p_sInputString[in_] == '\r'))
    {
      in_++;
      continue;
    }

    char_array_4[i] = p_sInputString[in_];
    i++;
    in_++;

    if (i == 4) 
    {
      for (i = 0; i <4; i++)
        char_array_4[i] = base64_chars.find(char_array_4[i]);

      char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
      char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
      char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

      for (i = 0; (i < 3); i++)
        sResult += char_array_3[i];

      i = 0;
    }
  }

  if (i)
  {
    for (j = i; j <4; j++)
      char_array_4[j] = 0;

    for (j = 0; j <4; j++)
      char_array_4[j] = base64_chars.find(char_array_4[j]);

    char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
    char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
    char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

    for (j = 0; (j < i - 1); j++)
      sResult += char_array_3[j];
  }

  return sResult;  
}
/* end BASE64 decoding */


/*===============================================================================
 Common Functions
===============================================================================*/

void fuppesSleep(unsigned int p_nMilliseconds)
{
  #ifdef WIN32
    Sleep(p_nMillisecinds);
  #else
    usleep(p_nMilliseconds * 1000);
  #endif
}

/*===============================================================================
 Socket functions
===============================================================================*/

bool fuppesSocketSetNonBlocking(upnpSocket p_SocketHandle)
{
  int opts;
	opts = fcntl(p_SocketHandle, F_GETFL);
	if (opts < 0) {
    return false;
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(p_SocketHandle, F_SETFL,opts) < 0) {		
    return false;
	} 
  return true;
}

/*===============================================================================
 Thread functions
===============================================================================*/

bool fuppesThreadClose(fuppesThread p_ThreadHandle)
{
  #ifdef WIN32  
  WaitForSingleObject(p_ThreadHandle, INFINITE);
  CloseHandle(p_ThreadHandle);
  #else    
  bool bResult = true;
  int nErrNo = pthread_join(p_ThreadHandle, NULL);
  if (nErrNo != 0)
  {
    bResult = false;
    switch(nErrNo)
    {
      case EINVAL:
        cout << "pthread_join() :: " << nErrNo << " EINVAL = handle does not refer to a joinable thread" << endl;      
        break;
      case ESRCH:
        cout << "pthread_join() :: " << nErrNo << " ESRCH = No thread found with the given thread handle" << endl;
        break;
      case EDEADLK:
        cout << "pthread_join() :: " << nErrNo << " EDEADLK = deadlock detected" << endl;      
        break;        
    }
    fflush(stdout);
  }
  return bResult;  
  #endif
}
