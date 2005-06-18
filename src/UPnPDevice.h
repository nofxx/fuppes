/***************************************************************************
 *            UPnPDevice.h
 * 
 *  Copyright  2005  Ulrich Völkel
 *  mail@ulrich-voelkel.de
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

#include "UPnPService.h"

using namespace std;

enum eUPnPDeviceType
{
	dt_root_device,
	dt_media_server
};

class CUPnPDevice
{
  protected:
	  CUPnPDevice(eUPnPDeviceType);
		~CUPnPDevice();

	public:
		void AddUPnPService(CUPnPService*);
    int  GetUPnPServiceCount();
    CUPnPService* GetUPnPService(int);	
	
		eUPnPDeviceType GetDeviceType();
	  std::string			GetDeviceTypeAsString();	
		std::string			GetDescription();
		void 						SetHTTPServerURL(std::string);
	
	private:
		eUPnPDeviceType m_UPnPDeviceType;	
		std::string     m_sHTTPServerURL;
		std::vector<CUPnPService*> m_vUPnPServices;
	
	protected:
		string m_sFriendlyName;
	  string m_sManufacturer;
	  string m_sManufacturerURL;
	  string m_sModelDescription;
	  string m_sModelName;
	  string m_sModelNumber;
	  string m_sModelURL;
	  string m_sSerialNumber;
	  string m_sUDN;
	  string m_sUPC;
	  string m_sPresentationURL;
};

#endif /* _UPNPDEVICE_H */
