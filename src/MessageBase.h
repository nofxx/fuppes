/***************************************************************************
 *            MessageBase.h
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
 
#ifndef _MESSAGEBASE_H
#define _MESSAGEBASE_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "Common/Common.h"

#ifndef WIN32
#include <arpa/inet.h>
#include <sys/types.h>
#include <netinet/in.h>
#endif

#include <string>

using namespace std;

/*===============================================================================
 CLASS CMessageBase
===============================================================================*/

class CMessageBase
{

/* <PROTECTED> */

	protected:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

		CMessageBase();
	  virtual ~CMessageBase();

/* <\PROTECTED> */

/* <PUBLIC> */

public:

/*===============================================================================
 INIT
===============================================================================*/

  virtual bool SetMessage(std::string p_sMessage);

/*===============================================================================
 GET MESSAGE DATA
===============================================================================*/

	std::string GetContent()         { return m_sContent;                     }
  std::string	GetMessage()         { return m_sMessage;                     }
  std::string GetHeader()          { return m_sHeader;                      }
  std::string GetRemoteIPAddress() { return inet_ntoa(m_RemoteEp.sin_addr); }

/*===============================================================================
 ENDPOINT
===============================================================================*/

  sockaddr_in GetLocalEndPoint()	 { return m_LocalEp;                      }
  sockaddr_in GetRemoteEndPoint()  { return m_RemoteEp;                     }
		
	void        SetLocalEndPoint(sockaddr_in);
	void        SetRemoteEndPoint(sockaddr_in);		
  
/* <\PUBLIC> */

/* <PROTECTED> */

protected:

/*===============================================================================
 MEMBERS
===============================================================================*/

	  sockaddr_in m_LocalEp;
		sockaddr_in m_RemoteEp;
	  std::string m_sContent;
    std::string m_sHeader;
    std::string m_sMessage;

/* <\PROTECTED> */

  private:
    void TrySplitMessage();

};

#endif /* _MESSAGEBASE_H */
