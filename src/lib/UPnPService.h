/***************************************************************************
 *            UPnPService.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _UPNPSERVICE_H
#define _UPNPSERVICE_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "UPnPBase.h"
#include "HTTP/HTTPMessage.h"

class CUPnPService: public CUPnPBase
{
  protected:

    /** constructor
     *  @param  nType  the device type
     *  @param  p_sHTTPServerURL  URL of the HTTP server
     */
    CUPnPService(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL);
		
    /** destructor
     */
    virtual ~CUPnPService();
	
  public:
    
    /** returns the service description as string
     *  @return  the service description
     */
    virtual std::string GetServiceDescription() = 0;

    virtual void HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut) = 0;
};

#endif // _UPNPSERVICE_H
