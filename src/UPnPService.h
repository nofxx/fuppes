/***************************************************************************
 *            UPnPService.h
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
 
#ifndef _UPNPSERVICE_H
#define _UPNPSERVICE_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "UPnPBase.h"

/*===============================================================================
 CLASS CUPnPService
===============================================================================*/

class CUPnPService: public CUPnPBase
{

/* <PROTECTED> */

protected:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/
  
  /** constructor
  *  @param  nType  the device type
  *  @param  p_sHTTPServerURL  URL of the HTTP server
  */
  CUPnPService(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL);
		
  /** destructor
  */
  virtual ~CUPnPService();
	
/* <\PROTECTED> */

/* <PUBLIC> */

public:

/*===============================================================================
 GET
===============================================================================*/

  /** returns the service description as string
  *  @return  the service description
  */
  virtual std::string GetServiceDescription();

/* <\PUBLIC> */

};

#endif /* _UPNPSERVICE_H */
