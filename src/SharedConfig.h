/***************************************************************************
 *            SharedConfig.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>
#include <vector>

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

using namespace std;

typedef enum tagFILE_KIND
{
  FILE_KIND_UNKNOWN  =  0,
  FILE_KIND_AUDIO    =  1,
  FILE_KIND_VIDEO    =  2
}FILE_KIND;

struct DisplaySettings
{
  bool bShowTranscodingTypeInItemNames;
  bool bShowDirNamesInFirstLevel;
};

/*===============================================================================
 CLASS CSharedConfig
===============================================================================*/

class CSharedConfig
{

/* <PROTECTED> */

protected:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

		CSharedConfig();    

/* <\PROTECTED> */

/* <PUBLIC> */

	public:
    ~CSharedConfig();

/*===============================================================================
 INSTANCE
===============================================================================*/

  static CSharedConfig* Shared();

/*===============================================================================
 INIT
===============================================================================*/

  bool SetupConfig();
  bool Refresh();

  void PrintTranscodingSettings();
  
/*===============================================================================
 GET
===============================================================================*/

  std::string GetAppName();
	std::string GetAppFullname();
	std::string GetAppVersion();
	
	std::string GetHostname();
	std::string GetUUID();
  
  std::string GetOSName();
  std::string GetOSVersion();
	
	std::string GetIPv4Address() { return m_sIP; }
  bool SetIPv4Address(std::string p_sIPAddress);
  
	unsigned int GetHTTPPort() { return m_nHTTPPort; }
  bool SetHTTPPort(unsigned int p_nHTTPPort);
  
  std::string GetConfigDir();
  
  std::string GetSharedDir(unsigned int p_nDirIdx);
  unsigned int SharedDirCount();
  
  bool IsSupportedFileExtension(std::string p_sFileExtension);
  bool IsTranscodingEnabled() { return m_bTranscodingEnabled; }  
  bool IsTranscodingExtension(std::string p_sFileExt);
  
  unsigned int AllowedIPCount();
  std::string GetAllowedIP(unsigned int p_nIdx);
  bool IsAllowedIP(std::string p_sIPAddress);
  
  bool AddAllowedIP(std::string p_sIPAddress);
  bool RemoveAllowedIP(unsigned int p_nIndex);
  
  
  FILE_KIND GetFileKindByExtension(std::string p_sFileExtension);
  DisplaySettings GetDisplaySettings() { return m_DisplaySettings; }
  
  unsigned int GetMaxFileNameLength() { return m_nMaxFileNameLength; }
  void SetMaxFileNameLength(unsigned int p_nMaxFileNameLenght);
  
  bool AddSharedDirectory(std::string p_sDirectory);
  bool RemoveSharedDirectory(unsigned int p_nIndex);
/* <\PUBLIC> */
	
/* <PRIVATE> */

private:

/*===============================================================================
 INSTANCE
===============================================================================*/
    
  static CSharedConfig* m_Instance;

/*===============================================================================
 MEMBERS
===============================================================================*/
  
  /* xml config nodes */
  xmlDocPtr   m_pDoc;
  xmlNode*    m_pSharedDirNode;
  xmlNode*    m_pContentDirNode;
  xmlNode*    m_pNetSettingsNode;

  std::string m_sConfigVersion;
  std::string m_sConfigFileName;

  std::string m_sHostname;
	std::string m_sIP;
  std::string m_sUUID;
  std::string m_sOSName;
  std::string m_sOSVersion;

  std::string m_sLameVersion;
  std::string m_sVorbisVersion;
  std::string m_sMpcVersion;
  std::string m_sFlacVersion;

  unsigned int m_nMaxFileNameLength;

  std::vector<std::string> m_vSharedDirectories;
  std::vector<std::string> m_vAllowedIPs;
  unsigned int m_nHTTPPort;
  bool m_bTranscodingEnabled;
  bool m_bLameAvailable;
  bool m_bVorbisAvailable;
  bool m_bMusePackAvailable;
  bool m_bFlacAvailable;  
  
  bool m_bTranscodeVorbis;
  bool m_bTranscodeMusePack;
  bool m_bTranscodeFlac;  

  DisplaySettings m_DisplaySettings;

/*===============================================================================
 HELPER
===============================================================================*/

  bool ReadConfigFile(bool p_bIsInit);
  bool ResolveHostAndIP();
  bool ResolveIPByHostname();
  bool ResolveIPByInterface(std::string p_sInterfaceName);  
  bool WriteDefaultConfig(std::string p_sFileName);
  void CheckForTranscodingLibs();
  bool GetOSInfo();
  

/* <\PRIVATE> */

};

#endif /* _SHAREDCONFIG_H */
