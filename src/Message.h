/***************************************************************************
 *            Message.h
 *
 *  Sun Jun 12 18:19:54 2005
 *  Copyright  2005  Ulrich Völkel
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
 
#ifndef _MESSAGE_H
#define _MESSAGE_H

#include "win32.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>

class CMessage
{
	public:
		CMessage(std::string);
	  ~CMessage();
	
	public:
		sockaddr_in get_local_ep();
	  void set_local_ep(sockaddr_in);
	
		sockaddr_in get_remote_ep();
	  void set_remote_ep(sockaddr_in);		
	
		std::string GetContent();
	
	private:
	  sockaddr_in local_ep;
		sockaddr_in remote_ep;
	  std::string m_sContent;
};

#endif /* _MESSAGE_H */
