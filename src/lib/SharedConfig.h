/***************************************************************************
 *            SharedConfig.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "Configuration/ConfigFile.h"

class CFuppes;

class CSharedConfig
{
  protected:
		CSharedConfig(); 

	public:
    ~CSharedConfig();
    static CSharedConfig* Shared();

    bool SetupConfig(std::string p_sConfigFileName = "");
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
  
    // shared dir
    int SharedDirCount();
    std::string GetSharedDir(int p_nIdx);  
    void AddSharedDirectory(std::string p_sDirectory);
    void RemoveSharedDirectory(int p_nIdx);
  
    // shared iTunes
    int SharedITunesCount();
    std::string GetSharedITunes(int p_nIdx);  
    void AddSharedITunes(std::string p_sITunes);
    void RemoveSharedITunes(int p_nIdx);
  
    // allowed ip
    unsigned int AllowedIPCount();
    std::string GetAllowedIP(unsigned int p_nIdx);
    bool IsAllowedIP(std::string p_sIPAddress);  
    bool AddAllowedIP(std::string p_sIPAddress);
    bool RemoveAllowedIP(unsigned int p_nIndex);  
  
  
    unsigned int GetFuppesInstanceCount();
    CFuppes* GetFuppesInstance(unsigned int p_nIndex);
    void AddFuppesInstance(CFuppes* pFuppes);
  
  	
    bool UseImageMagick() { return m_pConfigFile->UseImageMagick(); }
    bool UseTaglib()      { return m_pConfigFile->UseTaglib(); }
    bool UseLibAvFormat() { return m_pConfigFile->UseLibAvFormat(); }

  private:
    static CSharedConfig* m_Instance;    
      
    CConfigFile*  m_pConfigFile;
    std::string   m_sConfigFileName;

    std::string   m_sHostname;
    std::string   m_sIP;
    std::string   m_sUUID;
    std::string   m_sOSName;
    std::string   m_sOSVersion;  
    std::string   m_sTempDir;
    unsigned int  m_nHTTPPort;

    std::vector<CFuppes*> m_vFuppesInstances;

    bool ReadConfigFile();
    bool ResolveHostAndIP();
    bool ResolveIPByHostname();
    bool ResolveIPByInterface(std::string p_sInterfaceName);
    void GetOSInfo();  
};

#endif // _SHAREDCONFIG_H
