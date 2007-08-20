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
		  pDeviceMessage->DeviceSettings(pSettings);
			break;
		}
		
		if(pSettings->HasUserAgent(pDeviceMessage->m_sUserAgent)) {
		  pDeviceMessage->DeviceSettings(pSettings);
			break;
		}		
	}
	
	if(!pDeviceMessage->DeviceSettings())
  	pDeviceMessage->DeviceSettings(m_pDefaultSettings);

  CSharedLog::Shared()->Log(L_EXTENDED, pDeviceMessage->DeviceSettings()->m_sDeviceName, __FILE__, __LINE__);
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
  if(!pSettings) {
    pSettings = new CDeviceSettings(p_sDeviceName, m_pDefaultSettings);
    m_Settings.push_back(pSettings);
  } 
    
  return pSettings;
}

void CDeviceIdentificationMgr::PrintSetting(CDeviceSettings* pSettings) {
  
  CFileSettings  * pFileSet;
  
  cout << "device: " << pSettings->m_sDeviceName << endl;
  cout << "  release delay: " << pSettings->nDefaultReleaseDelay << endl;
  cout << "  file_settings: " << endl;  
  
  for(pSettings->m_FileSettingsIterator = pSettings->m_FileSettings.begin();
      pSettings->m_FileSettingsIterator != pSettings->m_FileSettings.end();
      pSettings->m_FileSettingsIterator++) {
          
    pFileSet = pSettings->m_FileSettingsIterator->second;
    
    cout << "    ext: " << pSettings->m_FileSettingsIterator->first << endl; 
    //cout << "    ext: " << pFileSet->sExt << endl;
    cout << "    dlna: " << pFileSet->sDLNA << endl;
    cout << "    mime-type: " << pFileSet->sMimeType << endl;
    cout << "    upnp-type: " << pFileSet->ObjectType() << endl;
          
    if(pFileSet->pTranscodingSettings) {
      cout << "  transcode: " << endl;
      
      cout << "    ext: " << pFileSet->pTranscodingSettings->sExt << endl;
      cout << "    dlna: " << pFileSet->pTranscodingSettings->sDLNA << endl;
      cout << "    mime-type: " << pFileSet->pTranscodingSettings->sMimeType << endl;      
      cout << "    release delay: " << pFileSet->ReleaseDelay() << endl;
    }
    else if(pFileSet->pImageSettings) {
      cout << "  resize: " << endl;
      
      cout << "    height: " << pFileSet->pImageSettings->nHeight << endl;
      cout << "    width: " << pFileSet->pImageSettings->nWidth << endl;
      cout << "    greater: " << pFileSet->pImageSettings->bGreater << endl;
      cout << "    less: " << pFileSet->pImageSettings->bLess << endl;
    }
    else {
      cout << "  no transcoding/resizing" << endl;
    }
          
    cout << endl;
  }
}

void CDeviceIdentificationMgr::PrintSettings()
{
  CDeviceSettings* pSettings;
  
  cout << "device settings" << endl;
  
  PrintSetting(m_pDefaultSettings);
  
  for(m_SettingsIt = m_Settings.begin(); 
      m_SettingsIt != m_Settings.end(); 
      m_SettingsIt++)	{    

    pSettings = *m_SettingsIt;
      
    PrintSetting(pSettings);    
  }
  
}
