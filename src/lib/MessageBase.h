/***************************************************************************
 *            MessageBase.h
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
 
#ifndef _MESSAGEBASE_H
#define _MESSAGEBASE_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "Common/Common.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>

using namespace std;

class CMessageBase
{
  protected:
  	CMessageBase();
	  virtual ~CMessageBase();

  public:
    virtual bool SetMessage(std::string p_sMessage);
    virtual bool SetHeader(std::string p_sHeader);

	  std::string GetContent()         { return m_sContent;                     }
    std::string	GetMessage()         { return m_sMessage;                     }
    std::string GetHeader()          { return m_sHeader;                      }
    std::string GetRemoteIPAddress() { return inet_ntoa(m_RemoteEp.sin_addr); }

    sockaddr_in GetLocalEndPoint()	 { return m_LocalEp;                      }
    sockaddr_in GetRemoteEndPoint()  { return m_RemoteEp;                     }
		
	  void        SetLocalEndPoint(sockaddr_in);
	  void        SetRemoteEndPoint(sockaddr_in);		
  

  protected:

	  sockaddr_in m_LocalEp;
		sockaddr_in m_RemoteEp;
	  std::string m_sContent;
    std::string m_sHeader;
    std::string m_sMessage;

};

#endif // _MESSAGEBASE_H
