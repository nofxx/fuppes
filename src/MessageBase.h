/***************************************************************************
 *            MessageBase.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 
#ifndef _MESSAGEBASE_H
#define _MESSAGEBASE_H

#include "win32.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>

class CMessageBase
{
	protected:
		CMessageBase(std::string);
	  ~CMessageBase();
	
	public:
		sockaddr_in GetLocalEndPoint();
	  void        SetLocalEndPoint(sockaddr_in);
	
		sockaddr_in GetRemoteEndPoint();
	  void        SetRemoteEndPoint(sockaddr_in);		
	
		std::string GetContent();	
  
	protected:
	  sockaddr_in m_LocalEp;
		sockaddr_in m_RemoteEp;
	  std::string m_sContent;
};

#endif /* _MESSAGEBASE_H */
