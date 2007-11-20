/***************************************************************************
 *            UPnPBase.cpp
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

#include "Common/Common.h"
#include "UPnPBase.h"
#include "SharedLog.h"

/* constructor */
CUPnPBase::CUPnPBase(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL)
{
  /* Save data */
  m_nUPnPDeviceType = nType;
  m_sHTTPServerURL  = p_sHTTPServerURL;
}

std::string	CUPnPBase::GetUPnPDeviceTypeAsString()
{
  std::string sResult;
	
  // Set string for the corrseponding device type
  switch(m_nUPnPDeviceType)
  {
    case UPNP_DEVICE_MEDIA_SERVER:			  
	  sResult = "MediaServer";       
	  break;
    case UPNP_DEVICE_MEDIA_RENDERER:
      sResult = "MediaRenderer";
	  break;
    case UPNP_SERVICE_CONTENT_DIRECTORY:  
	  sResult = "ContentDirectory";  
	  break;
    case UPNP_SERVICE_CONNECTION_MANAGER:
	  sResult = "ConnectionManager";
	  break;
	case UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR:
	  sResult = "XMSMediaReceiverRegistrar";
	  break;
    default:
      CSharedLog::Shared()->Log(L_DBG, "unhandled UPnP device type", __FILE__, __LINE__);
      sResult = "unknown";
      break;
  }
	
  return sResult;
}
