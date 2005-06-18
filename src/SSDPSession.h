/***************************************************************************
 *            SSDPSession.h
 * 
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
 
#ifndef _SSDPSESSION_H
#define _SSDPSESSION_H

#include <string>

#include "UDPSocket.h"
#include "SSDPMessage.h"

class CSSDPSession: public IUDPSocket
{
	public:	
		void OnUDPSocketReceive(CUDPSocket*, CSSDPMessage*);
	
	protected:
		CSSDPSession();
	  virtual ~CSSDPSession();
	
	  void send_multicast(std::string);
	  void send_unicast(std::string);
	  
	  void begin_receive_unicast();	  

	  virtual void start();	  
	
		int timeout;
	  CUDPSocket* udp;	  
};

class CMSearchSession: public CSSDPSession
{
	public:
	  CMSearchSession();
    ~CMSearchSession();
	
	  void start();
	  sockaddr_in  GetLocalEndPoint();
};

#endif /* _SSDPSESSION_H */
