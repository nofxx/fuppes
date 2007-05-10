/***************************************************************************
 *            CommonFunctions.cpp
 *
 *  FUPPES - the Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net> 
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

#include "CommonFunctions.h"

#ifndef WIN32
#include <errno.h>
#include <sys/errno.h>

#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

// win32 and os x have no MSG_NOSIGNAL
// mac os x uses setsockopt(SO_NOSIGPIPE) instead
// win32 does not need this at all
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

int fuppesSocketSend(fuppesSocket p_Socket, const char* pBuffer, int p_nLength)
{
  int  nLastSend = 0;
  int  nFullSend = 0;
  bool bWouldBlock = false; 
    
  // send loop
  do
  {    
    // send
    nLastSend = send(p_Socket, &pBuffer[nFullSend], p_nLength - nFullSend, MSG_NOSIGNAL);
    
    bWouldBlock = false;
    #ifdef WIN32
    bWouldBlock = (WSAGetLastError() == WSAEWOULDBLOCK);    
    #else
    bWouldBlock = (errno == EAGAIN);
    #endif
    
    // incomplete
    if(nLastSend > 0)
      nFullSend += nLastSend; 
    
    // complete
    if(nFullSend == p_nLength)
      return nFullSend;    
    
    // error
    if((nLastSend < 0) && !bWouldBlock)
      return -1;

    // would block
    if(bWouldBlock)
      fuppesSleep(10);      

  } while ((nLastSend < 0) || (nFullSend < p_nLength) || bWouldBlock);    
  
  return nFullSend;
}
