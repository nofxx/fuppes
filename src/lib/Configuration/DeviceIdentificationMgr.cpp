/***************************************************************************
 *            DeviceIdentificationMgr.cpp
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
 
#include "DeviceIdentificationMgr.h"
#include "../SharedLog.h"
#include <iostream>

using namespace std;
 
CDeviceIdentificationMgr* CDeviceIdentificationMgr::m_pInstance = 0;

CDeviceIdentificationMgr* CDeviceIdentificationMgr::Shared()
{
  if(m_pInstance == 0)
	  m_pInstance = new CDeviceIdentificationMgr();
	return m_pInstance;
}

CDeviceIdentificationMgr::CDeviceIdentificationMgr()
{
  #warning todo: read settings from config
	CDeviceSettings* pSettings = NULL;

  /* default settings :: CDeviceSettings::CDeviceSettings()
	
	   m_bShowPlaylistAsContainer  = false;
	   m_bXBox360Support					 = false;
	   m_nMaxFileNameLength			   = 0;
		 
		 m_ImageSettings.bResize = false;
		 m_ImageSettings.bResizeIfLarger = false;
		 m_ImageSettings.nMaxWidth = 0;
		 m_ImageSettings.nMinWidth = 0;
	*/

  // Microsoft Xbox 360
  pSettings = new CDeviceSettings("Xbox 360");
	pSettings->m_slUserAgents.push_back("Xbox/2.0.\\d+.\\d+ UPnP/1.0 Xbox/2.0.\\d+.\\d+");
	pSettings->m_bXBox360Support = true;
	m_Settings.push_back(pSettings);

  // Terratec Noxon audio 1
  pSettings = new CDeviceSettings("Noxon audio");
	pSettings->m_slIPAddresses.push_back("192.168.0.22");
	pSettings->m_bShowPlaylistAsContainer = true;
	m_Settings.push_back(pSettings);	
	
	// Telegent TG 100
  pSettings = new CDeviceSettings("Telegent TG 100");
	pSettings->m_slIPAddresses.push_back("192.168.0.23");
	pSettings->m_nMaxFileNameLength = 101;
	
	pSettings->m_ImageSettings.bResize    = true;
	pSettings->m_ImageSettings.nMaxWidth  = 20;
	pSettings->m_ImageSettings.nMaxHeight = 40;
	m_Settings.push_back(pSettings);

  // default settings
  m_pDefaultSettings = new CDeviceSettings("default");
}

CDeviceIdentificationMgr::~CDeviceIdentificationMgr()
{
  #warning todo: cleanup
  delete m_pDefaultSettings;
}

void CDeviceIdentificationMgr::IdentifyDevice(CHTTPMessage* pDeviceMessage)
{
  //cout << __FILE__ << " " << __LINE__ << endl << pDeviceMessage->GetHeader() << endl;
	
	CDeviceSettings* pSettings;
	for(m_SettingsIt = m_Settings.begin(); m_SettingsIt != m_Settings.end(); m_SettingsIt++)
	{
	  pSettings = *m_SettingsIt;
		//cout << pDeviceMessage->m_sUserAgent << endl;
		
		if(pSettings->HasIP(pDeviceMessage->GetRemoteIPAddress())) {
		  pDeviceMessage->SetDeviceSettings(pSettings);
			break;
		}
		
		if(pSettings->HasUserAgent(pDeviceMessage->m_sUserAgent)) {
		  pDeviceMessage->SetDeviceSettings(pSettings);
			break;
		}
		
	}
	
	if(!pDeviceMessage->GetDeviceSettings())
  	pDeviceMessage->SetDeviceSettings(m_pDefaultSettings);

  CSharedLog::Shared()->Log(L_EXTENDED, pDeviceMessage->GetDeviceSettings()->m_sDeviceName, __FILE__, __LINE__);
}
