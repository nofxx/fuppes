/***************************************************************************
 *            UPnPBase.cpp
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "Common/Common.h"
#include "UPnPBase.h"
#include "SharedLog.h"

/*===============================================================================
 CLASS CUPnPBase
===============================================================================*/

/* <PROTECTED> */

/* constructor */
CUPnPBase::CUPnPBase(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL)
{
  /* Save data */
  m_nUPnPDeviceType = nType;
  m_sHTTPServerURL  = p_sHTTPServerURL;
}

/* <\PROTECTED> */

/* <PUBLIC> */

/*===============================================================================
 GET
===============================================================================*/

/* GetUPnPDeviceTypeAsString */
std::string	CUPnPBase::GetUPnPDeviceTypeAsString()
{
	std::string sResult;
	
	/* Set string for the corrseponding device type */
  switch(m_nUPnPDeviceType)
	{
		case UPNP_DEVICE_TYPE_ROOT_DEVICE:        
		  sResult = "RootDevice";        
			break;
		case UPNP_DEVICE_TYPE_MEDIA_SERVER:			  
		  sResult = "MediaServer";       
			break;
    case UPNP_DEVICE_TYPE_CONTENT_DIRECTORY:  
		  sResult = "ContentDirectory";  
			break;
    case UPNP_DEVICE_TYPE_CONNECTION_MANAGER:
		  sResult = "ConnectionManager";
			break;
		case UPNP_SERVICE_TYPE_X_MS_MEDIA_RECEIVER_REGISTRAR:
		  sResult = "XMSMediaReceiverRegistrar";
			break;
    default:
		  CSharedLog::Shared()->Log(L_EXTENDED_ERR, "unhandled UPnP device type", __FILE__, __LINE__);
			sResult = "unknown";
			break;
	}
	
	return sResult;
}

/* <\PUBLIC> */
