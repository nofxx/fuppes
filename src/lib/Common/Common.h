/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Common.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifndef _COMMON_H
#define _COMMON_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <sys/types.h>

#include <string>
#include <sstream>
#include <assert.h>

#include <libxml/xmlwriter.h>

#ifdef WIN32

/* This must be defined to use InitializeCriticalSectionAndSpinCount() */
//#define _WIN32_WINNT 0x0410 // Windows 98 or later

#pragma comment(lib,"Wsock32.lib") 
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")

#include <winsock2.h>
#include <ws2tcpip.h>
#include <shlwapi.h>

#else

#include <pthread.h>

#endif

#ifdef WIN32
  #if SIZEOF_LONG_INT == 8
  typedef long int fuppes_off_t;  
	#elif SIZEOF_LONG_LONG_INT == 8
	typedef long long int fuppes_off_t;  
	#endif	
#else
  typedef off_t fuppes_off_t;
#endif


#define ASSERT assert

const std::string MIME_TYPE_TEXT_HTML = "text/html";

bool FileExists(std::string p_sFileName);
bool IsFile(std::string p_sFileName);
bool DirectoryExists(std::string p_sDirName);
bool IsDirectory(std::string p_sDirName);
bool CreateDirectory(std::string dir);

std::string StringReplace(std::string p_sIn, std::string p_sSearch, std::string p_sReplace);
std::string ExtractFileExt(std::string p_sFileName);
std::string ExtractFilePath(std::string p_sFileName);
std::string TruncateFileExt(std::string p_sFileName);
bool ExtractFolderFromPath(std::string p_sPath, std::string* p_sFolder);
std::string TrimFileName(std::string p_sFileName, unsigned int p_nMaxLength);
std::string TrimWhiteSpace(std::string s);
std::string MD5Sum(std::string p_sFileName);

void        appendTrailingSlash(std::string* value);
std::string appendTrailingSlash(std::string value);


std::string ToLower(std::string p_sInput);
std::string ToUpper(std::string p_sInput);
bool SplitURL(std::string p_sURL, std::string* p_sIPAddress, unsigned int* p_nPort);
int  Base64Decode(const std::string p_sInputString, char* p_szOutBuffer, int p_nBufSize);

unsigned int HexToInt(std::string sHex);
std::string SQLEscape(std::string p_sValue);

std::string ToUTF8(std::string p_sValue, std::string p_sEncoding = "");

std::string URLEncodeValueToPlain(std::string p_sValue);

void fuppesSleep(unsigned int p_nMilliseconds);

fuppes_off_t getFileSize(std::string fileName);

fuppes_off_t strToOffT(std::string value);

#ifdef WIN32
  typedef SOCKET fuppesSocket;  
#else
  typedef int fuppesSocket;
#endif

bool fuppesSocketSetNonBlocking(fuppesSocket p_SocketHandle);
int  fuppesSocketClose(fuppesSocket p_SocketHandle);


#ifdef WIN32
//  typedef HANDLE            fuppesThread;
  typedef CRITICAL_SECTION  fuppesThreadMutex;
#else
//  typedef pthread_t         fuppesThread;
  typedef pthread_mutex_t   fuppesThreadMutex;
#endif

/*int  fuppesThreadCancel(fuppesThread p_ThreadHandle);
bool fuppesThreadClose(fuppesThread p_ThreadHandle);*/

void fuppesThreadInitMutex(fuppesThreadMutex* p_ThreadMutex);
void fuppesThreadDestroyMutex(fuppesThreadMutex* p_ThreadMutex);

void fuppesThreadLockMutex(fuppesThreadMutex* p_ThreadMutex);
void fuppesThreadUnlockMutex(fuppesThreadMutex* p_ThreadMutex);


class MutexLocker2
{
	public:
		MutexLocker2(fuppesThreadMutex* mutex) {
			m_mutex = mutex;
			fuppesThreadLockMutex(m_mutex);
		}
		~MutexLocker2() {
			fuppesThreadUnlockMutex(m_mutex);
		}
		
	private:
		fuppesThreadMutex* m_mutex;
};


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

/* Common */
//#define upnpSleep               Sleep
#define upnpPathDelim           "\\"

/* Sockets */
/*#define upnpSocket              SOCKET
#define upnpSocketSuccess       0
#define upnpSocketClose         closesocket
#define upnpSocketFlag(_x_)     const char _x_[256] = ""*/


/* Threads */
//#define fuppesThread                                        HANDLE
//#define fuppesThreadStart(_handle_, _callback_)             _handle_ = CreateThread(NULL, 0, &_callback_, this, 0, NULL)
//#define fuppesThreadClose(_handle_)                         WaitForSingleObject(_handle_, INFINITE); CloseHandle(_handle_)
//#define fuppesThreadCancel(_handle_, _exit_code_)           TerminateThread(_handle_, _exit_code_);
//#define fuppesThreadExit()                                  ExitThread(0);
//#define fuppesThreadStartArg(_handle_, _callback_, _arg_)   _handle_ = CreateThread(NULL, 0, &_callback_, &_arg_, 0, NULL)
//#define fuppesThreadCallback                                DWORD WINAPI

//#define fuppesThreadMutex                                   CRITICAL_SECTION
//#define fuppesThreadInitMutex(_mutex_)                      InitializeCriticalSection(_mutex_) //AndSpinCount(_mutex_, 0x80000400)
//#define fuppesThreadLockMutex(_mutex_)                      EnterCriticalSection(_mutex_)
//#define fuppesThreadUnlockMutex(_mutex_)                    LeaveCriticalSection(_mutex_)
//#define fuppesThreadDestroyMutex(_mutex_)                   DeleteCriticalSection(_mutex_)

#else



/*#include <pthread.h>
#include <unistd.h>*/

/* Common */
//#define upnpSleep               usleep
#define upnpPathDelim           "/"

/* Sockets */
/*#define upnpSocket              int
#define upnpSocketSuccess       -1
#define upnpSocketClose(_socket_)         close(_socket_)*/

/* Threads */
//#define fuppesThread                                        pthread_t
//#define fuppesThreadStart(_handle_, _callback_)             pthread_create(&_handle_, NULL, &_callback_, this);
//#define fuppesThreadClose(_handle_)                         pthread_join(_handle_, NULL);
//#define fuppesThreadCancel(_handle_, _exit_code_)           pthread_cancel(_handle_);
//#define fuppesThreadExit()                                  pthread_exit(NULL);
//#define fuppesThreadStartArg(_handle_, _callback_, _arg_)   pthread_create(&_handle_, NULL, &_callback_, &_arg_);
//#define fuppesThreadCallback                                void*

//#define fuppesThreadMutex                                   pthread_mutex_t
//#define fuppesThreadInitMutex(_mutex_)                      pthread_mutex_init(_mutex_, NULL)
//#define fuppesThreadLockMutex(_mutex_)                      pthread_mutex_lock(_mutex_)
//#define fuppesThreadUnlockMutex(_mutex_)                    pthread_mutex_unlock(_mutex_)
//#define fuppesThreadDestroyMutex(_mutex_)                   pthread_mutex_destroy(_mutex_)

#endif

#endif // _COMMON_H
