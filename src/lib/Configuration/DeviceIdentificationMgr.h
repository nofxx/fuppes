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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _DEVICEIDENTIFICATIONMGR_H
#define _DEVICEIDENTIFICATIONMGR_H

#include "DeviceSettings.h"
#include "../HTTP/HTTPMessage.h"
#include <list>

class CDeviceIdentificationMgr
{
  public:
	  static CDeviceIdentificationMgr* Shared();
    ~CDeviceIdentificationMgr();

    void IdentifyDevice(CHTTPMessage* pDeviceMessage);

  private:
		CDeviceIdentificationMgr();
	  static CDeviceIdentificationMgr* m_pInstance;
		
		CDeviceSettings* m_pDefaultSettings;
		std::list<CDeviceSettings*> m_Settings;
		std::list<CDeviceSettings*>::const_iterator m_SettingsIt;

};

#endif // _DEVICEIDENTIFICATIONMGR_H
