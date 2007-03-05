/***************************************************************************
 *            UPnPBase.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include <string>

typedef enum {
  UPNP_DEVICE_UNKNOWN,
  //UPNP_DEVICE_TYPE_ROOT_DEVICE,
  UPNP_DEVICE_MEDIA_SERVER,
  UPNP_DEVICE_MEDIA_RENDERER,
  UPNP_SERVICE_CONTENT_DIRECTORY,
  UPNP_SERVICE_RENDERING_CONTROL,
  UPNP_SERVICE_CONNECTION_MANAGER,
  UPNP_SERVICE_AV_TRANSPORT,
  UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR
}UPNP_DEVICE_TYPE;

class CUPnPBase
{

protected:

  /** constructor
  *  @param  nType  the device type
  *  @param  p_sHTTPServerURL  URL of the HTTP server
  */
  CUPnPBase(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL);  


public:

  /** returns the device type
  *  @return device type as string
  */
  std::string GetUPnPDeviceTypeAsString();
	UPNP_DEVICE_TYPE GetUPnPDeviceType() { return m_nUPnPDeviceType; }

  
protected:

  std::string m_sHTTPServerURL;

  UPNP_DEVICE_TYPE m_nUPnPDeviceType;


};

#endif // _UPNPBASE_H 
