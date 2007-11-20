/***************************************************************************
 *            Common.h
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

#ifndef _COMMON_H
#define _COMMON_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <sys/types.h>

#include <string>
#include <sstream>
#include <assert.h>
#include <exception>
#include <stdarg.h>

#include <libxml/xmlwriter.h>

#ifdef WIN32

/* This must be defined to use InitializeCriticalSectionAndSpinCount() */
//#define _WIN32_WINNT 0x0410 // Windows 98 or later

#pragma comment(lib,"Wsock32.lib") 
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")

#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <shlwapi.h>

#endif

class EException: public std::exception
{
  public:
    EException(std::string p_sException, char* p_szFile, int p_nLine) : std::exception()
    {       
      std::stringstream sRes;
      sRes << p_sException << " (" << p_szFile << ", " << p_nLine << ")";
      m_sException = sRes.str();
    };
    
    EException(const std::string p_sFile, int p_nLine, const char* p_szEx, ...) : std::exception()
    {       
      va_list args;
      char buffer[1024];
      va_start(args, p_szEx);
      vsnprintf(buffer, sizeof(buffer), p_szEx, args);
      va_end(args);
      m_sException = buffer;
    };
  
    ~EException() throw() {};
      
    std::string What() { return m_sException; };
    
  private:
    std::string m_sException;
};


#define ASSERT assert

const std::string MIME_TYPE_TEXT_HTML = "text/html";

bool FileExists(std::string p_sFileName);
bool IsFile(std::string p_sFileName);
bool DirectoryExists(std::string p_sDirName);
bool IsDirectory(std::string p_sDirName);

std::string StringReplace(std::string p_sIn, std::string p_sSearch, std::string p_sReplace);
std::string ExtractFileExt(std::string p_sFileName);
std::string ExtractFilePath(std::string p_sFileName);
std::string TruncateFileExt(std::string p_sFileName);
bool ExtractFolderFromPath(std::string p_sPath, std::string* p_sFolder);
std::string TrimFileName(std::string p_sFileName, unsigned int p_nMaxLength, bool p_bTruncateFileExt = false);
std::string TrimWhiteSpace(std::string s);
std::string MD5Sum(std::string p_sFileName);


std::string ToLower(std::string p_sInput);
std::string ToUpper(std::string p_sInput);
bool SplitURL(std::string p_sURL, std::string* p_sIPAddress, unsigned int* p_nPort);
std::string Base64Decode(const std::string p_sInputString);

unsigned int HexToInt(std::string sHex);
std::string SQLEscape(std::string p_sValue);

std::string ToUTF8(std::string p_sValue, std::string p_sEncoding = "");


void fuppesSleep(unsigned int p_nMilliseconds);


#ifdef WIN32
  typedef SOCKET fuppesSocket;  
#else
  typedef int fuppesSocket;
#endif

bool fuppesSocketSetNonBlocking(fuppesSocket p_SocketHandle);
int  fuppesSocketClose(fuppesSocket p_SocketHandle);


#ifdef WIN32
  typedef HANDLE            fuppesThread;
  typedef CRITICAL_SECTION  fuppesThreadMutex;
#else
  typedef pthread_t         fuppesThread;
  typedef pthread_mutex_t   fuppesThreadMutex;
#endif

int  fuppesThreadCancel(fuppesThread p_ThreadHandle);
bool fuppesThreadClose(fuppesThread p_ThreadHandle);

void fuppesThreadInitMutex(fuppesThreadMutex* p_ThreadMutex);
void fuppesThreadDestroyMutex(fuppesThreadMutex* p_ThreadMutex);

void fuppesThreadLockMutex(fuppesThreadMutex* p_ThreadMutex);
void fuppesThreadUnlockMutex(fuppesThreadMutex* p_ThreadMutex);



#ifdef WIN32
  typedef HINSTANCE  fuppesLibHandle;
  typedef FARPROC    fuppesProcHandle;
#else
  #include <dlfcn.h>
  
  typedef void*      fuppesLibHandle;
  typedef void*      fuppesProcHandle;
#endif

fuppesLibHandle   FuppesLoadLibrary(std::string p_sLibName);
fuppesProcHandle  FuppesGetProcAddress(fuppesLibHandle p_LibHandle, std::string p_sProcName);
bool              FuppesCloseLibrary(fuppesLibHandle p_LibHandle);




#ifdef WIN32
  #if SIZEOF_UNSIGNED_LONG_INT == 8
  typedef unsigned long int fuppes_off_t;  
	#elif SIZEOF_UNSIGNED_LONG_LONG_INT == 8
	typedef unsigned long long int fuppes_off_t;  
	#endif	
#else
  typedef off_t fuppes_off_t;
#endif




#ifdef WIN32

/* Common */
//#define upnpSleep               Sleep
#define upnpPathDelim           "\\"

/* Sockets */
#define upnpSocket              SOCKET
#define upnpSocketSuccess       0
#define upnpSocketClose         closesocket
#define upnpSocketFlag(_x_)     const char _x_[256] = ""


/* Threads */
//#define fuppesThread                                        HANDLE
#define fuppesThreadStart(_handle_, _callback_)             _handle_ = CreateThread(NULL, 0, &_callback_, this, 0, NULL)
//#define fuppesThreadClose(_handle_)                         WaitForSingleObject(_handle_, INFINITE); CloseHandle(_handle_)
//#define fuppesThreadCancel(_handle_, _exit_code_)           TerminateThread(_handle_, _exit_code_);
#define fuppesThreadExit()                                  ExitThread(0);
#define fuppesThreadStartArg(_handle_, _callback_, _arg_)   _handle_ = CreateThread(NULL, 0, &_callback_, &_arg_, 0, NULL)
#define fuppesThreadCallback                                DWORD WINAPI

//#define fuppesThreadMutex                                   CRITICAL_SECTION
//#define fuppesThreadInitMutex(_mutex_)                      InitializeCriticalSection(_mutex_) //AndSpinCount(_mutex_, 0x80000400)
//#define fuppesThreadLockMutex(_mutex_)                      EnterCriticalSection(_mutex_)
//#define fuppesThreadUnlockMutex(_mutex_)                    LeaveCriticalSection(_mutex_)
//#define fuppesThreadDestroyMutex(_mutex_)                   DeleteCriticalSection(_mutex_)

#else



#include <pthread.h>
#include <unistd.h>

/* Common */
//#define upnpSleep               usleep
#define upnpPathDelim           "/"

/* Sockets */
#define upnpSocket              int
#define upnpSocketSuccess       -1
#define upnpSocketClose(_socket_)         close(_socket_)

/* Threads */
//#define fuppesThread                                        pthread_t
#define fuppesThreadStart(_handle_, _callback_)             pthread_create(&_handle_, NULL, &_callback_, this);
//#define fuppesThreadClose(_handle_)                         pthread_join(_handle_, NULL);
//#define fuppesThreadCancel(_handle_, _exit_code_)           pthread_cancel(_handle_);
#define fuppesThreadExit()                                  pthread_exit(NULL);
#define fuppesThreadStartArg(_handle_, _callback_, _arg_)   pthread_create(&_handle_, NULL, &_callback_, &_arg_);
#define fuppesThreadCallback                                void*

//#define fuppesThreadMutex                                   pthread_mutex_t
//#define fuppesThreadInitMutex(_mutex_)                      pthread_mutex_init(_mutex_, NULL)
//#define fuppesThreadLockMutex(_mutex_)                      pthread_mutex_lock(_mutex_)
//#define fuppesThreadUnlockMutex(_mutex_)                    pthread_mutex_unlock(_mutex_)
//#define fuppesThreadDestroyMutex(_mutex_)                   pthread_mutex_destroy(_mutex_)

#endif

#endif // _COMMON_H
