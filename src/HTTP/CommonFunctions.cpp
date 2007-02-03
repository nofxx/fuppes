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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "CommonFunctions.h"

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
  int  nErrNo = 0;
  bool bWouldBlock = false; 
    
  // send loop
  do
  {    
    // send again
    nLastSend = send(p_Socket, pBuffer[nFullSend], p_nLength - nFullSend);
    
    bWouldBlock = false;
    #ifdef WIN32
    nErrNo = WSAGetLastError();
    bWouldBlock = (nErrNo == 10035);    
    #else
    nErrNo = errno;
    bWouldBlock = (nErrNo == EAGAIN);
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
      fuppesSleep(100);      
    

      
  } while ((nRes < 0) || (nFullSend < p_nLength) || bWouldBlock);    
  
  return nFullSend;
}
