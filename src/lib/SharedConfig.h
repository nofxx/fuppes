/***************************************************************************
 *            SharedConfig.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
#ifndef _SHAREDCONFIG_H
#define _SHAREDCONFIG_H


/* OS dependent defines */
// gcc -dM -E - < /dev/null
#if defined(WIN32)
  #define FUPPES_TARGET_WIN32
#elif defined(__GNUC__) && defined(__LINUX__)
  #define FUPPES_TARGET_LINUX
#elif defined(__APPLE__)
  #define FUPPES_TARGET_MAC_OSX
#endif


#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

#include "Transcoding/TranscodingMgr.h"

using namespace std;

/*typedef struct {
  bool bShowTranscodingTypeInItemNames;
  bool bShowDirNamesInFirstLevel;
  bool bShowPlaylistsAsContainers;
} DisplaySettings_t;*/


class CFuppes;

class CSharedConfig
{
  protected:
		CSharedConfig(); 

	public:
    ~CSharedConfig();
    static CSharedConfig* Shared();		



  bool SetupConfig();
  bool Refresh();

  void PrintTranscodingSettings();
  

  std::string GetAppName();
	std::string GetAppFullname();
	std::string GetAppVersion();
	
	std::string GetHostname();
	std::string GetUUID();
  
  std::string GetOSName();
  std::string GetOSVersion();
  
  std::string GetLocalCharset();
  bool SetLocalCharset(std::string p_sCharset);
	
	std::string GetIPv4Address() { return m_sIP; }
  bool SetIPv4Address(std::string p_sIPAddress);
  
	unsigned int GetHTTPPort() { return m_nHTTPPort; }
  bool SetHTTPPort(unsigned int p_nHTTPPort);
  
	std::string GetConfigDir();
  std::string GetConfigFileName() { return m_sConfigFileName; }
  
  std::string GetSharedDir(unsigned int p_nDirIdx);
  unsigned int SharedDirCount();  
  
  unsigned int AllowedIPCount();
  std::string GetAllowedIP(unsigned int p_nIdx);
  bool IsAllowedIP(std::string p_sIPAddress);
  
  bool AddAllowedIP(std::string p_sIPAddress);
  bool RemoveAllowedIP(unsigned int p_nIndex);
  
    
  //DisplaySettings_t GetDisplaySettings() { return m_DisplaySettings; }
  
  /*unsigned int GetMaxFileNameLength() { return m_nMaxFileNameLength; }
  void SetMaxFileNameLength(unsigned int p_nMaxFileNameLenght); */
  
  bool AddSharedDirectory(std::string p_sDirectory);
  bool RemoveSharedDirectory(unsigned int p_nIndex);
  
  //bool SetPlaylistRepresentation(std::string p_sRepresentation);  

  void AddFuppesInstance(CFuppes* pFuppes);
	
  CFuppes* GetFuppesInstance(unsigned int p_nIndex);
  unsigned int GetFuppesInstanceCount();
  
	//bool IsXBox360SupportEnabled() { return m_bXBox360Support; }
  bool UseImageMagick() { return m_bUseImageMagick; }


private:

  static CSharedConfig* m_Instance;

  // xml config nodes
  xmlDocPtr   m_pDoc;
  xmlNode*    m_pSharedDirNode;
  xmlNode*    m_pContentDirNode;
  xmlNode*    m_pNetSettingsNode;
  xmlNode*    m_pTranscodingSettingsNode;

  // member vars
  std::string m_sConfigVersion;  
  std::string m_sConfigFileName;

  std::string m_sHostname;
	std::string m_sIP;
  std::string m_sUUID;
  std::string m_sOSName;
  std::string m_sOSVersion;
  std::string m_sLocalCharset;
	std::string m_sTempDir;


  //unsigned int m_nMaxFileNameLength;

  std::vector<std::string> m_vSharedDirectories;
  std::vector<std::string> m_vAllowedIPs;
  unsigned int m_nHTTPPort;
  
	bool m_bUseImageMagick;

  //DisplaySettings_t m_DisplaySettings;


  std::vector<CFuppes*> m_vFuppesInstances;


  bool ReadConfigFile();
  bool ResolveHostAndIP();
  bool ResolveIPByHostname();
  bool ResolveIPByInterface(std::string p_sInterfaceName);  
  bool WriteDefaultConfig(std::string p_sFileName);  
  void GetOSInfo();
  
};

#endif // _SHAREDCONFIG_H
