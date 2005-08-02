/***************************************************************************
 *            UPnPDevice.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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

#include <string>
#include <vector>
#include <libxml/xmlwriter.h>

#include "UPnPBase.h"
#include "UPnPService.h"

using namespace std;

class CUPnPDevice: public CUPnPBase
{
  protected:
	  CUPnPDevice(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL);		
  
	public:
    CUPnPDevice();
    ~CUPnPDevice();
    bool BuildFromDescriptionURL(std::string p_sDescriptionURL);
  
		void AddUPnPService(CUPnPService*);
    int  GetUPnPServiceCount();
    CUPnPService* GetUPnPService(int);	
	
		UPNP_DEVICE_TYPE GetDeviceType();	  
		std::string			GetDeviceDescription();		
	
    std::string GetFriendlyName() { return m_sFriendlyName; }
  
	private:
		UPNP_DEVICE_TYPE m_nUPnPDeviceType;			
		std::vector<CUPnPService*> m_vUPnPServices;    
    bool ParseDescription(std::string p_sDescription);
  
	protected:
		std::string   m_sFriendlyName;
	  std::string   m_sManufacturer;
	  std::string   m_sManufacturerURL;
	  std::string   m_sModelDescription;
	  std::string   m_sModelName;
	  std::string   m_sModelNumber;
	  std::string   m_sModelURL;
	  std::string   m_sSerialNumber;
	  std::string   m_sUDN;
	  std::string   m_sUPC;
	  std::string   m_sPresentationURL;
};

#endif /* _UPNPDEVICE_H */
