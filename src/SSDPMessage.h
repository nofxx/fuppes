/***************************************************************************
 *            SSDPMessage.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
#ifndef _SSDPMESSAGE_H
#define _SSDPMESSAGE_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "MessageBase.h"
#include <string>

/*===============================================================================
 DEFINITIONS
===============================================================================*/

typedef enum tagSSDP_MESSAGE_TYPE
{
  SSDP_MESSAGE_TYPE_UNKNOWN,
  SSDP_MESSAGE_TYPE_M_SEARCH,
  SSDP_MESSAGE_TYPE_M_SEARCH_RESPONSE,
  SSDP_MESSAGE_TYPE_NOTIFY_ALIVE,
  SSDP_MESSAGE_TYPE_NOTIFY_BYEBYE
}SSDP_MESSAGE_TYPE;

/*===============================================================================
 CLASS CSSDPMessage
===============================================================================*/

class CSSDPMessage: public CMessageBase
{
	
/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

		CSSDPMessage();
	  virtual ~CSSDPMessage();

/*===============================================================================
 MESSAGES
===============================================================================*/

  virtual void SetMessage(std::string p_sMessage);

/*===============================================================================
 GET
===============================================================================*/

    std::string GetLocation() { return m_sLocation; }
    std::string GetUUID()     { return m_sUUID;     }    
    std::string GetDeviceID();
    SSDP_MESSAGE_TYPE GetMessageType() { return m_nMessageType; }     
    
/* <\PUBLIC> */

/* <PRIVATE> */

private:

/*===============================================================================
 MEMBERS
===============================================================================*/

  std::string m_sLocation;
  std::string m_sServer;
  std::string m_sST;
  std::string m_sNTS;
  std::string m_sUSN;
  std::string m_sUUID;
  SSDP_MESSAGE_TYPE m_nMessageType;

/* <\PRIVATE> */

};

#endif /* _SSDPMESSAGE_H */
