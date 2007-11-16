/***************************************************************************
 *            DeviceIdentificationMgr.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifndef _DEVICEIDENTIFICATIONMGR_H
#define _DEVICEIDENTIFICATIONMGR_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "DeviceSettings.h"
#include "../HTTP/HTTPMessage.h"
#include <list>

class CDeviceIdentificationMgr
{
  public:
	  static CDeviceIdentificationMgr* Shared();
    ~CDeviceIdentificationMgr();

		void Initialize();
		
		void IdentifyDevice(CHTTPMessage* pDeviceMessage);
    CDeviceSettings* GetSettingsForInitialization(std::string p_sDeviceName);
    void PrintSettings(std::string* p_sOut = NULL);
  
    CDeviceSettings* DefaultDevice() { return m_pDefaultSettings; }
  
  private:
		CDeviceIdentificationMgr();
	  static CDeviceIdentificationMgr* m_pInstance;
		
    void PrintSetting(CDeviceSettings* pSettings, std::string* p_sOut = NULL);

  
		CDeviceSettings* m_pDefaultSettings;
		std::list<CDeviceSettings*> m_Settings;
		std::list<CDeviceSettings*>::const_iterator m_SettingsIt;

};

#endif // _DEVICEIDENTIFICATIONMGR_H
