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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
  m_pDefaultSettings = new CDeviceSettings("default");
}

CDeviceIdentificationMgr::~CDeviceIdentificationMgr()
{
  #warning todo: cleanup
  delete m_pDefaultSettings;
}

void CDeviceIdentificationMgr::IdentifyDevice(CHTTPMessage* pDeviceMessage)
{
	CDeviceSettings* pSettings;
	for(m_SettingsIt = m_Settings.begin(); m_SettingsIt != m_Settings.end(); m_SettingsIt++)
	{
	  pSettings = *m_SettingsIt;		
		
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


CDeviceSettings* CDeviceIdentificationMgr::GetSettingsForInitialization(std::string p_sDeviceName)
{
  // check if device exists
  CDeviceSettings* pSettings = NULL;
  
  if(p_sDeviceName.compare("default") == 0) {
    return m_pDefaultSettings;
  }
  
	for(m_SettingsIt = m_Settings.begin(); m_SettingsIt != m_Settings.end(); m_SettingsIt++)	{    
    
    if((*m_SettingsIt)->m_sDeviceName.compare(p_sDeviceName) == 0) {
      pSettings = *m_SettingsIt;
      break;
    }
  }
  
  // create new setting
  if(pSettings == NULL) {
    
    pSettings = new CDeviceSettings(p_sDeviceName);
    
    // copy default settings
    pSettings->m_bShowPlaylistAsContainer = m_pDefaultSettings->m_bShowPlaylistAsContainer;
    pSettings->m_bXBox360Support          = m_pDefaultSettings->m_bXBox360Support;
    pSettings->m_nMaxFileNameLength       = m_pDefaultSettings->m_nMaxFileNameLength;
    pSettings->m_DisplaySettings.bShowChildCountInTitle = m_pDefaultSettings->m_DisplaySettings.bShowChildCountInTitle;
    pSettings->m_bDLNAEnabled             = m_pDefaultSettings->m_bDLNAEnabled;
    
    m_Settings.push_back(pSettings);
  } 
    
  return pSettings;
}
