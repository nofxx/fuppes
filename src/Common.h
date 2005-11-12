/***************************************************************************
 *            Common.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
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

#ifndef _COMMON_H
#define _COMMON_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>
#include <assert.h>

#ifdef WIN32

/* T.S.NOTE: This must be defined to use InitializeCriticalSectionAndSpinCount() */
#define _WIN32_WINNT 0x0410 /* Windos 98 * or later */

#pragma comment(lib,"Wsock32.lib") 
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"shlwapi.lib")

#include <Winsock2.h>
#include <Ws2tcpip.h>
#include <shlwapi.h> /* For PathXXX functions */

#endif

/*===============================================================================
 MACROS
===============================================================================*/

#define SAFE_DELETE(_x_)            if(_x_){delete(_x_); _x_ = NULL;}
#define BOOL_CHK_RET_POINTER(_x_)   assert(_x_); if(!_x_) return false
#define STRING_CHK_RET_POINTER(_x_) assert(_x_); if(!_x_) return ""
#define VOID_CHK_RET_POINTER(_x_)   assert(_x_); if(!_x_) return

/*===============================================================================
 DEFINITIONS
===============================================================================*/

#define ASSERT assert

/*===============================================================================
 CONSTATNS
===============================================================================*/

/*
 * mime types
 */

const std::string MIME_TYPE_TEXT_HTML = "text/html";

/* image types */
const std::string MIME_TYPE_IMAGE_PNG = "image/png";

/* video types */
const std::string MIME_TYPE_VIDEO_X_MSVIDEO = "video/x-msvideo";
const std::string MIME_TYPE_VIDEO_MPEG      = "video/mpeg";

/* TODO */

/*===============================================================================
 File Functions
===============================================================================*/

bool FileExists(std::string p_sFileName);
bool IsFile(std::string p_sFileName);
bool DirectoryExists(std::string p_sDirName);
bool IsDirectory(std::string p_sDirName);
std::string ExtractFileExt(std::string p_sFileName);
std::string TruncateFileExt(std::string p_sFileName);

/*===============================================================================
 String Functions
===============================================================================*/

std::string ToLower(std::string p_sInput);
bool SplitURL(std::string p_sURL, std::string* p_sIPAddress, unsigned int* p_nPort);
std::string Base64Decode(const std::string p_sInputString);


/*===============================================================================
 Common Functions
===============================================================================*/
void fuppesSleep(unsigned int p_nMilliseconds);

/*===============================================================================
 Socket definitions and functions
===============================================================================*/

#ifdef WIN32
  typedef SOCKET fuppesSocket;  
#else
  typedef int fuppesSocket;
#endif

bool fuppesSocketSetNonBlocking(fuppesSocket p_SocketHandle);


/*===============================================================================
 Thread definitions and functions
===============================================================================*/

#ifdef WIN32
  typedef HANDLE            fuppesThread;
  typedef CRITICAL_SECTION  fuppesThreadMutex;
#else
  typedef pthread_t         fuppesThread;
  typedef pthread_mutex_t   fuppesThreadMutex;
#endif

bool fuppesThreadClose(fuppesThread p_ThreadHandle);



/*===============================================================================
 Library definitions and functions
===============================================================================*/

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


/*===============================================================================
 WIN32 specific definitions
===============================================================================*/

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
#define fuppesThreadInitMutex(_mutex_)                      InitializeCriticalSectionAndSpinCount(_mutex_, 0x80000400)
#define fuppesThreadLockMutex(_mutex_)                      EnterCriticalSection(_mutex_)
#define fuppesThreadUnlockMutex(_mutex_)                    LeaveCriticalSection(_mutex_)
#define fuppesThreadDestroyMutex(_mutex_) 

#else

/*===============================================================================
 NOT WIN32 specific definitions
===============================================================================*/

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
//#define fuppesThreadCancel(_handle_)                        pthread_cancel(_handle_);
#define fuppesThreadExit()                                  pthread_exit(NULL);
#define fuppesThreadStartArg(_handle_, _callback_, _arg_)   pthread_create(&_handle_, NULL, &_callback_, &_arg_);
#define fuppesThreadCallback                                void*

//#define fuppesThreadMutex                                   pthread_mutex_t
#define fuppesThreadInitMutex(_mutex_)                      pthread_mutex_init(_mutex_, NULL)
#define fuppesThreadLockMutex(_mutex_)                      pthread_mutex_lock(_mutex_)
#define fuppesThreadUnlockMutex(_mutex_)                    pthread_mutex_unlock(_mutex_)
#define fuppesThreadDestroyMutex(_mutex_)                   pthread_mutex_destroy(_mutex_)

#endif

#endif /* _COMMON_H */
