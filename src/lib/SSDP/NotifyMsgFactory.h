/***************************************************************************
 *            NotifyMsgFactory.h
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
 
#ifndef _NOTIFYMSGFACTORY_H
#define _NOTIFYMSGFACTORY_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>

typedef enum tagMESSAGE_TYPE
{
  MESSAGE_TYPE_UNKNOWN              =  0,
  MESSAGE_TYPE_USN                  =  1,
  MESSAGE_TYPE_ROOT_DEVICE          =  2,
  MESSAGE_TYPE_CONNECTION_MANAGER   =  3,
  MESSAGE_TYPE_CONTENT_DIRECTORY    =  4,
  MESSAGE_TYPE_MEDIA_SERVER         =  5
}MESSAGE_TYPE;

class CNotifyMsgFactory
{
  public:		
    CNotifyMsgFactory(std::string p_sHTTPServerURL); 

		std::string msearch();	
	  std::string notify_alive(MESSAGE_TYPE);	
	  std::string notify_bye_bye(MESSAGE_TYPE);
    std::string GetMSearchResponse(MESSAGE_TYPE);		

private:

    static CNotifyMsgFactory* instance;
	  std::string               m_sHTTPServerURL;

    static std::string type_to_string(MESSAGE_TYPE);

};

#endif // _NOTIFYMSGFACTORY_H
