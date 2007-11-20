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
#include "../SharedConfig.h"
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

void ReplaceDescriptionVars(std::string* p_sValue)
{
	string sValue = *p_sValue;
	string::size_type pos;

	// version (%v)
	while((pos = sValue.find("%v")) != string::npos) {
		sValue = sValue.replace(pos, 2, CSharedConfig::Shared()->GetAppVersion());
	}
	// short name (%s)
	while((pos = sValue.find("%s")) != string::npos) {
		sValue = sValue.replace(pos, 2, CSharedConfig::Shared()->GetAppName());
	}	
	// hostname (%h)
	while((pos = sValue.find("%h")) != string::npos) {
		sValue = sValue.replace(pos, 2, CSharedConfig::Shared()->GetHostname());
	}
		
  *p_sValue = sValue;
}

void CDeviceIdentificationMgr::Initialize()
{
	ReplaceDescriptionVars(&m_pDefaultSettings->MediaServerSettings()->FriendlyName);
	ReplaceDescriptionVars(&m_pDefaultSettings->MediaServerSettings()->ModelName);
	ReplaceDescriptionVars(&m_pDefaultSettings->MediaServerSettings()->ModelNumber);
	ReplaceDescriptionVars(&m_pDefaultSettings->MediaServerSettings()->ModelDescription);
	ReplaceDescriptionVars(&m_pDefaultSettings->MediaServerSettings()->SerialNumber);	
		
	CDeviceSettings* pSettings;
	for(m_SettingsIt = m_Settings.begin(); 
			m_SettingsIt != m_Settings.end(); 
			m_SettingsIt++)
	{
	  pSettings = *m_SettingsIt;

		ReplaceDescriptionVars(&pSettings->MediaServerSettings()->FriendlyName);
		ReplaceDescriptionVars(&pSettings->MediaServerSettings()->ModelName);
		ReplaceDescriptionVars(&pSettings->MediaServerSettings()->ModelNumber);
		ReplaceDescriptionVars(&pSettings->MediaServerSettings()->ModelDescription);
		ReplaceDescriptionVars(&pSettings->MediaServerSettings()->SerialNumber);
	}	
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
	
	if(!pDeviceMessage->DeviceSettings()) {
  	pDeviceMessage->DeviceSettings(m_pDefaultSettings);    
  }

  CSharedLog::Log(L_EXT, __FILE__, __LINE__,
    "device settings \"%s\"\n\tip: %s\n\tuser agent: %s",
    pDeviceMessage->DeviceSettings()->m_sDeviceName.c_str(),
    pDeviceMessage->GetRemoteIPAddress().c_str(),
    pDeviceMessage->m_sUserAgent.c_str());
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

void CDeviceIdentificationMgr::PrintSetting(CDeviceSettings* pSettings, std::string* p_sOut) {
  
  CFileSettings* pFileSet;
  stringstream sTmp;
  
  if(!p_sOut) {  
    cout << "device: " << pSettings->m_sDeviceName << endl;
    cout << "  release delay: " << pSettings->nDefaultReleaseDelay << endl;
    cout << "  file_settings: " << endl;
  }
  else {
    *p_sOut += "<table>";
    *p_sOut += "<tr><th colspan=\"2\">device: " + pSettings->m_sDeviceName + "</th></tr>";
    
    sTmp << pSettings->nDefaultReleaseDelay << endl;
    *p_sOut += "<tr><td>release delay</td>";
    *p_sOut += "<td>" + sTmp.str() + "</td></tr>";
    sTmp.str("");
    
    *p_sOut += "<tr><th colspan=\"2\">file settings</th></tr>";
  }
  
  for(pSettings->m_FileSettingsIterator = pSettings->m_FileSettings.begin();
      pSettings->m_FileSettingsIterator != pSettings->m_FileSettings.end();
      pSettings->m_FileSettingsIterator++) {
          
    pFileSet = pSettings->m_FileSettingsIterator->second;
    
    if(!p_sOut) {        
      cout << "    ext: " << pSettings->m_FileSettingsIterator->first << endl; 
      cout << "    dlna: " << pFileSet->DLNA() << endl;
      cout << "    mime-type: " << pFileSet->MimeType() << endl;
      cout << "    upnp-type: " << pFileSet->ObjectType() << endl;
    }
    else {  
      *p_sOut += "<tr><th colspan=\"2\">ext: " + pSettings->m_FileSettingsIterator->first + " </th></tr>";
      *p_sOut += "<tr><td>dlna</td>";
      *p_sOut += "<td>" + pFileSet->DLNA() + "</td></tr>";
      *p_sOut += "<tr><td>mime/type</td>";
      *p_sOut += "<td>" + pFileSet->MimeType() + "</td></tr>";
      *p_sOut += "<tr><td>upnp type</td>";
      *p_sOut += "<td>" + pFileSet->ObjectTypeAsStr() + "</td></tr>";
    }


    if(pFileSet->pTranscodingSettings) {
      if(!p_sOut) {
        cout << "  transcode: " << endl;
        cout << "    ext: " << pFileSet->pTranscodingSettings->sExt << endl;
        cout << "    dlna: " << pFileSet->pTranscodingSettings->sDLNA << endl;
        cout << "    mime-type: " << pFileSet->pTranscodingSettings->sMimeType << endl;      
        cout << "    release delay: " << pFileSet->ReleaseDelay() << endl;
      }
      else {
        *p_sOut += "<tr><th colspan=\"2\">transcode</th></tr>";
        *p_sOut += "<tr><td>ext</td>";
        *p_sOut += "<td>" + pFileSet->pTranscodingSettings->sExt + "</td></tr>";  
        *p_sOut += "<tr><td>dlna</td>";
        *p_sOut += "<td>" + pFileSet->pTranscodingSettings->DLNA() + "</td></tr>";
        *p_sOut += "<tr><td>mime/type</td>";
        *p_sOut += "<td>" + pFileSet->pTranscodingSettings->MimeType() + "</td></tr>";
        
        sTmp << pFileSet->ReleaseDelay();
        *p_sOut += "<tr><td>release delay</td>";
        *p_sOut += "<td>" + sTmp.str() + "</td></tr>";
        sTmp.str("");
      }
    }
    else if(pFileSet->pImageSettings) {
      if(!p_sOut) {
        cout << "  resize: " << endl;
        cout << "    height: " << pFileSet->pImageSettings->nHeight << endl;
        cout << "    width: " << pFileSet->pImageSettings->nWidth << endl;
        cout << "    greater: " << pFileSet->pImageSettings->bGreater << endl;
        cout << "    less: " << pFileSet->pImageSettings->bLess << endl;
      }
      else {
        *p_sOut += "<tr><th colspan=\"2\">resize</th></tr>";
        sTmp << pFileSet->pImageSettings->nHeight;
        *p_sOut += "<tr><td>height</td>";
        *p_sOut += "<td>" + sTmp.str() + "</td></tr>";
        sTmp.str("");
        sTmp << pFileSet->pImageSettings->nWidth;
        *p_sOut += "<tr><td>width</td>";
        *p_sOut += "<td>" + sTmp.str() + "</td></tr>";
        sTmp.str("");
        
        *p_sOut += "<tr><td>greater</td>";
        if(pFileSet->pImageSettings->bGreater)
          *p_sOut += "<td>true</td></tr>";
        else
          *p_sOut += "<td>false</td></tr>";
        
        *p_sOut += "<tr><td>less</td>";
        if(pFileSet->pImageSettings->bLess)
          *p_sOut += "<td>true</td></tr>";
        else
          *p_sOut += "<td>false</td></tr>";
      }
    }
    else {
      if(!p_sOut) {
        cout << "  no transcoding/resizing" << endl;
      }
      else {
        *p_sOut += "<tr><td colspan=\"2\">no transcoding/resizing</td></tr>";
      }
    }
          
    if(!p_sOut) {
      cout << endl;
    }
    else {
      *p_sOut += "<tr><td colspan=\"2\">&nbsp;</td></tr>";
    }
  }
  
  if(p_sOut) {
    *p_sOut += "</table>";
  }
  
}

void CDeviceIdentificationMgr::PrintSettings(std::string* p_sOut)
{
  CDeviceSettings* pSettings;
  
  if(!p_sOut) {
    cout << "device settings" << endl;
  }
  
  PrintSetting(m_pDefaultSettings, p_sOut);
  
  for(m_SettingsIt = m_Settings.begin(); 
      m_SettingsIt != m_Settings.end(); 
      m_SettingsIt++)	{    

    pSettings = *m_SettingsIt;
      
    PrintSetting(pSettings, p_sOut);    
  }
  
}
