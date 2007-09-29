/***************************************************************************
 *            Common.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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


#ifndef HAVE_CONFIG_H
#define ICONV_SECOND_ARG const char**
#endif

#include "Common.h"
#include "RegEx.h"
#include "md5.h"

#include "../SharedConfig.h"

#include <cstdio>

#ifndef WIN32
#include <dirent.h>
#endif
#include <sys/stat.h>
#include <sys/types.h>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <fcntl.h>

#ifdef HAVE_ICONV
#include <iconv.h>
#endif

#include <errno.h>

using namespace std;

bool FileExists(std::string p_sFileName)
{
  struct stat Stat;  
  return (stat(p_sFileName.c_str(), &Stat) == 0 && S_ISREG(Stat.st_mode));
}

bool IsFile(std::string p_sFileName)
{ 
  return FileExists(p_sFileName);
}

#ifdef WIN32
bool DirectoryExists(std::string p_sDirName)
{
  // remove trailing backslash
  if((p_sDirName.length() > 2) &&
     (p_sDirName.substr(p_sDirName.length() - 1).compare(upnpPathDelim) == 0) &&
     (p_sDirName.substr(p_sDirName.length() - 2, 1).compare(":") != 0)
    ) {
    p_sDirName = p_sDirName.substr(0, p_sDirName.length() - 1);                                           
  }
  
  // Get file information
  struct _stat info;
  memset(&info, 0, sizeof(info));

  // Check directory exists
  _stat(p_sDirName.c_str(), &info);
  if(0 == (info.st_mode & _S_IFDIR)) {
    return false;
  }
  
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

std::string MD5Sum(std::string p_sFileName)
{
  std::fstream fsFile;   
  int nFileSize = 0;  
  int nRead = 0;
  char szBuffer[200];
  
  md5_state_t state;
	md5_byte_t  digest[16];
	char hex_output[16 * 2 + 1];  
  int  di;
  
  fsFile.open(p_sFileName.c_str(), ios::binary|ios::in);
  if(fsFile.fail() != 1)
  { 
    fsFile.seekg(0, ios::end); 
    nFileSize = streamoff(fsFile.tellg()); 
    fsFile.seekg(0, ios::beg);
        
    md5_init(&state);
    while(nFileSize > 0)
    {    
      if(nFileSize < 200)
        nRead = nFileSize;
      else
        nRead = 200;     
      
      fsFile.read(szBuffer, nRead);
      md5_append(&state, (const md5_byte_t *)szBuffer, nRead);
      
      nFileSize -= nRead;
    }
    
    md5_finish(&state, digest);
  }	
  
  cout << "md5" << endl;
  
	for (di = 0; di < 16; ++di)
	    sprintf(hex_output + di * 2, "%02x", digest[di]);
  
  cout << hex_output << endl;
  
  return hex_output;
}

std::string StringReplace(std::string p_sIn, std::string p_sSearch, std::string p_sReplace)
{
  std::string p_sResult;
	if(p_sIn.find(p_sSearch) == std::string::npos)
	  return p_sIn;
	
  while(p_sIn.find(p_sSearch) != std::string::npos)
	{
	  p_sResult += p_sIn.substr(0, p_sIn.find(p_sSearch)) + p_sReplace;
		p_sIn      = p_sIn.substr(p_sIn.find(p_sSearch) + p_sSearch.length(), p_sIn.length());
	}
	p_sResult += p_sIn;
	
	return p_sResult;
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
  return ToLower(sResult);
}

std::string ExtractFilePath(std::string p_sFileName)
{
  p_sFileName = p_sFileName.substr(0, p_sFileName.length() - ExtractFileExt(p_sFileName).length());
  while((p_sFileName.length() > 0) && (p_sFileName.substr(p_sFileName.length() - 1, 1).compare(upnpPathDelim) != 0))
  {
    p_sFileName = p_sFileName.substr(0, p_sFileName.length() -1);    
  }
  return p_sFileName;
}

std::string TruncateFileExt(std::string p_sFileName)
{
  std::string sExt = ExtractFileExt(p_sFileName);  
  if((sExt.length() == 0) && (p_sFileName.substr(p_sFileName.length() - 1, 1).compare(".") != 0))
    return p_sFileName;
  
  std::string sResult = p_sFileName.substr(0, p_sFileName.length() - sExt.length() - 1);
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

std::string ToUpper(std::string p_sInput)
{
  for(unsigned int i = 0; i < p_sInput.length(); i++)
  {
    p_sInput[i] = toupper(p_sInput[i]);
  }  
  return p_sInput;
}

bool ExtractFolderFromPath(std::string p_sPath, std::string* p_sFolder)
{
  if(p_sPath.substr(p_sPath.length() - 1, 1).compare(upnpPathDelim) == 0) {
    p_sPath = p_sPath.substr(0, p_sPath.length() - 1);
  }
    
  string::size_type pos;
  if((pos = p_sPath.find_last_of(upnpPathDelim)) != string::npos) {
    pos++;
    *p_sFolder = p_sPath.substr(pos, p_sPath.length() - pos);
    return true;
  }
  else {  
    return false;
  }
}

std::string TrimFileName(std::string p_sFileName, unsigned int p_nMaxLength, bool p_bTruncateFileExt)
{ 
  if((p_nMaxLength == 0) || (p_sFileName.length() <= p_nMaxLength))
    return p_sFileName;
  
  std::string  sExt  = "";
  std::string  sFile = TruncateFileExt(p_sFileName);
  unsigned int nLen = 0;
  
  if(p_bTruncateFileExt) {
    nLen = p_nMaxLength;
  }
  else {
    sExt = ExtractFileExt(p_sFileName);
    nLen = p_nMaxLength - sExt.length() - 1;
  }    
  
  sFile = sFile.substr(0, nLen);
  
  if(!p_bTruncateFileExt) {
    sFile = sFile + "." + sExt;
  }
  
  return sFile;
}

std::string TrimWhiteSpace(std::string s)
{
  const std::string drop = " ";
  std::string r = s.erase(s.find_last_not_of(drop)+1);
  return r.erase(0,r.find_first_not_of(drop));
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

/* BASE64 decoding */
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
    /* continue on line break */
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


unsigned int HexToInt(std::string sHex)
{  
  /* remove leading "0" */   
  if(sHex.find_first_not_of("0", 0) <= sHex.length())
    sHex = sHex.substr(sHex.find_first_not_of("0", 0), sHex.length());
  
  /* taken from: http://bdn.borland.com/article/0,1410,17203,00.html */  
  int n = 0;         // position in string
  int m = 0;         // position in digit[] to shift
  int count;         // loop index
  unsigned int intValue = 0;  // integer value of hex string
  int digit[10];     // hold values to convert
  while (n < 9) {
     if (sHex[n]=='\0')
        break;
     if (sHex[n] > 0x29 && sHex[n] < 0x40 ) //if 0 to 9
        digit[n] = sHex[n] & 0x0f;            //convert to int
     else if (sHex[n] >='a' && sHex[n] <= 'f') //if a to f
        digit[n] = (sHex[n] & 0x0f) + 9;      //convert to int
     else if (sHex[n] >='A' && sHex[n] <= 'F') //if A to F
        digit[n] = (sHex[n] & 0x0f) + 9;      //convert to int
     else break;
    n++;
  }
  count = n;
  m = n - 1;
  n = 0;
  while(n < count) {
     // digit[n] is value of hex digit at position n
     // (m << 2) is the number of positions to shift
     // OR the bits into return value
     intValue = intValue | (digit[n] << (m << 2));
     m--;   // adjust the position to set
     n++;   // next digit to process
  }

  return intValue;
}

std::string SQLEscape(std::string p_sValue)
{
  int nPos     = -2;
  int nLastPos = -1;
  do
  {
    if(nPos != -2)
      nLastPos = nPos;
    
    nPos = p_sValue.find('\'', nPos + 2);  
    if((nPos > -1) && (nPos != nLastPos))
      p_sValue = p_sValue.replace(nPos, 1, "\'\'");
    
  } while((nPos > -1) && (nPos != nLastPos));
  
  return p_sValue;
}

std::string ToUTF8(std::string p_sValue, std::string p_sEncoding)
{
  #ifdef HAVE_ICONV
	if(xmlCheckUTF8((const unsigned char*)p_sValue.c_str()))
    return p_sValue;
   
	iconv_t icv; 
	 
	if(p_sEncoding.length() == 0) {
    if(CSharedConfig::Shared()->GetLocalCharset().compare("UTF-8") == 0)
      return p_sValue;
  
		icv = iconv_open("UTF-8", CSharedConfig::Shared()->GetLocalCharset().c_str());
	}
	else {
    if(p_sEncoding.compare("UTF-8") == 0)
      return p_sValue;
  
		icv = iconv_open("UTF-8", p_sEncoding.c_str());
	}
	
  if(icv < 0)  
    return p_sValue;  
  
  size_t nInbytes  = p_sValue.length(); 
  char* szInBuf    = new char[p_sValue.length() + 1];
  memcpy(szInBuf, p_sValue.c_str(), p_sValue.length());
  szInBuf[p_sValue.length()] = '\0';

  size_t nOutbytes = p_sValue.length() * 2;
  char* szOutBuf   = new char[p_sValue.length() * 2 + 1];  
  char* pOutBuf    = szOutBuf;  
  memset(szOutBuf, 0, p_sValue.length() * 2 + 1);
  
  iconv(icv, (ICONV_SECOND_ARG)&szInBuf, &nInbytes, &pOutBuf, &nOutbytes);
  p_sValue = szOutBuf;  
    
  iconv_close(icv); 
  
  delete[] szOutBuf;
  //delete[] szInBuf;
  #endif
	
  return p_sValue;
}



void fuppesSleep(unsigned int p_nMilliseconds)
{
  #ifdef WIN32
  Sleep(p_nMilliseconds);
  #else
  if(p_nMilliseconds < 1000)
    usleep(p_nMilliseconds * 1000);
  else
    sleep(p_nMilliseconds / 1000);  
  #endif
}


bool fuppesSocketSetNonBlocking(upnpSocket p_SocketHandle)
{
  #ifdef WIN32     
  int nonblocking = 1;
  if(ioctlsocket(p_SocketHandle, FIONBIO, (unsigned long*) &nonblocking) != 0)
    return false;
  #else     
  int opts;
	opts = fcntl(p_SocketHandle, F_GETFL);
	if (opts < 0) {
    return false;
	}
	opts = (opts | O_NONBLOCK);
	if (fcntl(p_SocketHandle, F_SETFL,opts) < 0) {		
    return false;
	} 
	#endif
  return true;
}

int fuppesSocketClose(fuppesSocket p_SocketHandle)
{
  #ifdef WIN32
  return closesocket(p_SocketHandle);
  #else
  return close(p_SocketHandle);
  #endif  
}


bool fuppesThreadClose(fuppesThread p_ThreadHandle)
{     
  #ifdef WIN32  
  bool bResult = false;
  DWORD nErrNo = WaitForSingleObject(p_ThreadHandle, 500);
  switch(nErrNo)
  {
    case WAIT_ABANDONED:
      /*cout << "WAIT_ABANDONED :: " << nErrNo << endl;
      fflush(stdout);*/
      break;
    case WAIT_OBJECT_0:
      //cout << "WAIT_OBJECT_0 :: " << nErrNo << endl;
      CloseHandle(p_ThreadHandle);
      bResult = true;
      break;
    case WAIT_TIMEOUT:
      /*cout << "fuppesThreadClose() :: WAIT_TIMEOUT (" << nErrNo << ")" << endl;      
      fflush(stdout);*/
      break;
    case WAIT_FAILED:
      /*cout << "fuppesThreadClose() :: WAIT_FAILED (" << nErrNo << ")" << endl;
      fflush(stdout);*/
      break;
    default:
      cout << "fuppesThreadClose - DEFAULT :: " << nErrNo << endl;      
      fflush(stdout);      
      break;            
  }
  return bResult;
  #else    
  bool bResult = true;
  int  nErrNo  = pthread_join(p_ThreadHandle, NULL);  
  if (nErrNo != 0)
  {
    bResult = false;
    switch(nErrNo)
    {
      /*case EINVAL:
        cout << "pthread_join() :: " << nErrNo << " EINVAL = handle does not refer to a joinable thread" << endl;      
        break;
      case ESRCH:
        cout << "pthread_join() :: " << nErrNo << " ESRCH = No thread found with the given thread handle" << endl;
        break;
      case EDEADLK:
        cout << "pthread_join() :: " << nErrNo << " EDEADLK = deadlock detected" << endl;      
        break;*/
    }
    fflush(stdout);
  }
  return bResult;  
  #endif
}


int fuppesThreadCancel(fuppesThread p_ThreadHandle)
{
  #ifdef WIN32
	int nExitCode;
	TerminateThread(p_ThreadHandle, nExitCode);
  return nExitCode;
	#else
	return pthread_cancel(p_ThreadHandle);  
	#endif
}

void fuppesThreadInitMutex(fuppesThreadMutex* p_ThreadMutex)
{
  #ifdef WIN32
  InitializeCriticalSection(p_ThreadMutex);
  //InitializeCriticalSectionAndSpinCount(p_ThreadMutex, 0x80000400);
  #else
  pthread_mutex_init(p_ThreadMutex, NULL);
  #endif  
}

void fuppesThreadDestroyMutex(fuppesThreadMutex* p_ThreadMutex)
{
  #ifdef WIN32
  DeleteCriticalSection(p_ThreadMutex);
  #else
  pthread_mutex_destroy(p_ThreadMutex);
  #endif
}

void fuppesThreadLockMutex(fuppesThreadMutex* p_ThreadMutex)
{
  #ifdef WIN32
  #warning todo: check if mutex is initialized (uninitialized mutexes will crash on win32)
  EnterCriticalSection(p_ThreadMutex);
  #else
  pthread_mutex_lock(p_ThreadMutex);
  #endif  
}

void fuppesThreadUnlockMutex(fuppesThreadMutex* p_ThreadMutex)
{
  #ifdef WIN32
  #warning todo: check if mutex is initialized (uninitialized mutexes will crash on win32)
  LeaveCriticalSection(p_ThreadMutex);
  #else
  pthread_mutex_unlock(p_ThreadMutex);
  #endif  
}


fuppesLibHandle FuppesLoadLibrary(std::string p_sLibName)
{
  #ifdef WIN32
    return LoadLibrary(p_sLibName.c_str());
  #else
    return dlopen(p_sLibName.c_str(), RTLD_LAZY);
  #endif
}

fuppesProcHandle  FuppesGetProcAddress(fuppesLibHandle p_LibHandle, std::string p_sProcName)
{
  #ifdef WIN32
  return GetProcAddress(p_LibHandle, p_sProcName.c_str());
  #else
  return dlsym(p_LibHandle, p_sProcName.c_str());
  #endif
}

bool FuppesCloseLibrary(fuppesLibHandle p_LibHandle)
{
  #ifdef WIN32
  return FreeLibrary(p_LibHandle);  
  #else
  return dlclose(p_LibHandle);
  #endif
}
