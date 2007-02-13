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

  pSettings = new CDeviceSettings("XBox 360");
	pSettings->m_bShowPlaylistAsFile = true;
	pSettings->m_bXBox360Support = true;
	pSettings->m_slUserAgents.push_back("XBox/2.0.\\d+.\\d+ UPnP/1.0 XBox/2.0.\\d+.\\d+");
	m_Settings.push_back(pSettings);

  /*pSettings = new CDeviceSettings("Noxon audio");
	pSettings->m_bShowPlaylistAsFile = true;
	pSettings->m_bXBox360Support = true;
	pSettings->m_slUserAgents.push_back("Mozilla/4.0 \\(compatible\\)");
	pSettings->m_slUserAgents.push_back("WinampMPEG/2.8");
	pSettings->m_slIPAddresses.push_back("192.168.0.22");
	m_Settings.push_back(pSettings);	
	
  pSettings = new CDeviceSettings("Telegent TG 100");
	pSettings->m_bShowPlaylistAsFile = false;
	pSettings->m_bXBox360Support = false;
	//pSettings->m_slUserAgents.push_back("Cybetran ...");
	pSettings->m_slIPAddresses.push_back("192.168.0.23");
	m_Settings.push_back(pSettings);*/

  // default settings
  m_pDefaultSettings = new CDeviceSettings("default");
	m_pDefaultSettings->m_bShowPlaylistAsFile = true;
	m_pDefaultSettings->m_bXBox360Support = false;
}

CDeviceIdentificationMgr::~CDeviceIdentificationMgr()
{
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

	cout << __FILE__ << " " << __LINE__ << ": found device \"" << pDeviceMessage->GetDeviceSettings()->m_sDeviceName << "\"" << endl; 
}