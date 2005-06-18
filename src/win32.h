/***************************************************************************
 *            win32.h
 *
 *  Copyright  2005  Ulrich Völkel & Thomas Schnitzler
 *  mail@ulrich-voelkel.de
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

#ifndef _WIN32_Hschnitzler
#define _WIN32_H

#ifdef WIN32

#pragma comment(lib,"Wsock32.lib") 
#pragma comment(lib,"Ws2_32.lib")

#include <Winsock2.h>
#include <Ws2tcpip.h>

// Common
#define upnpSleep Sleep

// Sockets
#define upnpSocket              SOCKET
#define upnpSocketSuccess       0
#define upnpSocketClose         closesocket
#define upnpSocketFlag(_x_)     const char _x_[256] = ""

// Threads
#define upnpThread              HANDLE
#define upnpThreadStart(_handle_, _callback_) _handle_ = CreateThread(NULL, 0, &_callback_, this, 0, NULL)
#define upnpThreadCallback      DWORD WINAPI
//#define upnpThreadExitCallback  return 0

#else

// Common
#define upnpSleep               usleep

// Sockets
#define upnpSocket              int
#define upnpSocketSuccess       -1

#define upnpSocketClose         close
#define upnpSocketFlag(_x_)     int* _x_

// Threads
#define upnpThread              pthread_t
#define upnpThreadStart(_handle_, _callback_) pthread_create(&_handle_, NULL, &_callback_, this);
#define upnpThreadCallback      void*
//#define upnpThreadExitCallback  return

#endif

#endif /* _WIN32_H */
