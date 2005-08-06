/***************************************************************************
 *            UPnPDevice.h
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
 
#ifndef _UPNPDEVICE_H
#define _UPNPDEVICE_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>
#include <vector>
#include <libxml/xmlwriter.h>

#include "UPnPBase.h"
#include "UPnPService.h"

using namespace std;

/*===============================================================================
 CLASS CUPnPDevice
===============================================================================*/

class CUPnPDevice: public CUPnPBase
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
  CUPnPDevice(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL);		

/* <\PROTECTED> */

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/
  
  /** constructor
   */
  CUPnPDevice();

  /** destructor
   */
  ~CUPnPDevice();

/*===============================================================================
 BUILD DEVICE
===============================================================================*/

  /** gets a device description from a specific URL and builds the device
   *  @param  p_sDescriptionURL URL where we can get the device description message
   *  @return returns true on success otherwise false
   */
  bool BuildFromDescriptionURL(std::string p_sDescriptionURL);

  /** adds a UPnP service to this device
   *  @param  pService the service to add to the device
   */
  void AddUPnPService(CUPnPService* pService);

/*===============================================================================
 GET
===============================================================================*/

  /** returns the count of all services for this device
   *  @return returns the count or 0
   */
  int GetUPnPServiceCount();

  /** returns a pointer to a UPnP service
   *  @param  p_nIndex  index of the service to get
   *  @return  pointer to the service or NULL
   */
  CUPnPService* GetUPnPService(int p_nIndex);

  /** returns the device type of this device
   *  @return  the device type (see UPNP_DEVICE_TYPE enumeration)
   */
  UPNP_DEVICE_TYPE GetDeviceType();	  
	
  /** returns the whole device description
   *  @return  the device descripition as string
   */
  std::string GetDeviceDescription();		

  /** returns the friendly name of this device
   *  @return  name of the device
   */
  std::string GetFriendlyName() { return m_sFriendlyName; }
  
  /** returns the device's UUID
   *  @return the UUID
   */  
  std::string GetUUID() { return m_sUUID; }
  
/* <\PUBLIC> */

/* <PROTECTED> */

protected:

/*===============================================================================
 MEMBERS
===============================================================================*/

  std::string   m_sFriendlyName;
  std::string   m_sManufacturer;
  std::string   m_sManufacturerURL;
  std::string   m_sModelDescription;
  std::string   m_sModelName;
  std::string   m_sModelNumber;
  std::string   m_sModelURL;
  std::string   m_sSerialNumber;
  std::string   m_sUUID;
  std::string   m_sUPC;
  std::string   m_sPresentationURL;

/* <\PROTECTED> */

	private:

/* <PRIVATE> */

/*===============================================================================
 MEMBERS
===============================================================================*/

	UPNP_DEVICE_TYPE           m_nUPnPDeviceType;			
	std::vector<CUPnPService*> m_vUPnPServices;

/*===============================================================================
 HELPER
===============================================================================*/
  
  /** gets all information from a device description
  *  @param  p_sDescription string with the device description
  *  @return returns true on success otherwise false
  */
  bool ParseDescription(std::string p_sDescription);

/* <\PRIVATE> */

};

#endif /* _UPNPDEVICE_H */
