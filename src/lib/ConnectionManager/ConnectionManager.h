/***************************************************************************
 *            ConnectionManager.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _CONNECTIONMANAGER_H
#define _CONNECTIONMANAGER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../UPnPService.h"
#include <string>

class CConnectionManager: public CUPnPService
{
	public:
    CConnectionManager(std::string p_sHTTPServerURL);
  
    std::string GetServiceDescription();
    
    void HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut);
};

#endif /* _CONNECTIONMANAGER_H */
