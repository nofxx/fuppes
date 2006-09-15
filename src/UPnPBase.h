/***************************************************************************
 *            UPnPBase.h
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
 
#ifndef _UPNPBASE_H
#define _UPNPBASE_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>

/*===============================================================================
 DEFINITIONS
===============================================================================*/

typedef enum tagUPNP_DEVICE_TYPE
{
  UPNP_DEVICE_TYPE_UNKNOWN,
  UPNP_DEVICE_TYPE_ROOT_DEVICE,
  UPNP_DEVICE_TYPE_MEDIA_SERVER,
  UPNP_DEVICE_TYPE_MEDIA_RENDERER,
  UPNP_DEVICE_TYPE_CONTENT_DIRECTORY,
  UPNP_DEVICE_TYPE_RENDERING_CONTROL,
  UPNP_DEVICE_TYPE_CONNECTION_MANAGER,
  UPNP_DEVICE_TYPE_AV_TRANSPORT
}UPNP_DEVICE_TYPE;

/*===============================================================================
 CLASS CUPnPBase
===============================================================================*/

class CUPnPBase
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
  CUPnPBase(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL);  

/* <\PROTECTED> */ 
  
/* <PUBLIC> */

public:

/*===============================================================================
 GET
===============================================================================*/
  
  /** returns the device type
  *  @return device type as string
  */
  std::string GetUPnPDeviceTypeAsString();

/* <\PUBLIC> */

/* <PROTECTED> */
  
protected:

/*===============================================================================
 MEMBERS
===============================================================================*/

  std::string m_sHTTPServerURL;

  UPNP_DEVICE_TYPE m_nUPnPDeviceType;
/* <\PROTECTED> */

/* <PRIVATE> */

private:

/*===============================================================================
 MEMBERS
===============================================================================*/

  

/* <\PRIVATE> */

};

#endif /* _UPNPBASE_H */
