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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
 
#ifndef _SHAREDCONFIG_H
#define _SHAREDCONFIG_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

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

    bool SetupConfig();
    bool Refresh();

    void PrintTranscodingSettings();  

    std::string GetAppName();
    std::string GetAppFullname();
    std::string GetAppVersion();
	
    std::string FriendlyName();
    void        FriendlyName(std::string p_sFriendlyName);
    
  
    std::string GetHostname();
    std::string GetUUID();
  
    std::string GetOSName();
    std::string GetOSVersion();
  
    std::string GetLocalCharset();
    bool SetLocalCharset(std::string p_sCharset);
	
    std::string GetIPv4Address() { return m_sIP; }
    std::string GetNetInterface() { return m_sNetInterface; }
    bool SetNetInterface(std::string p_sNetInterface);
  
    unsigned int GetHTTPPort() { return m_nHTTPPort; }
    bool SetHTTPPort(unsigned int p_nHTTPPort);
  
    //std::string GetConfigDir();
  
    void SetConfigDir(std::string p_sConfigDir) { m_sConfigDir = p_sConfigDir; }
  
    void SetConfigFileName(std::string p_sConfigFileName) { m_sConfigFileName = p_sConfigFileName; }  
    std::string GetConfigFileName() { return m_sConfigFileName; }
  
    void SetDbFileName(std::string p_sDbFileName) { m_sDbFileName = p_sDbFileName; }
    std::string GetDbFileName() { return m_sDbFileName; }
  
    void SetVFolderConfigFileName(std::string p_sVFolderFileName) { m_sVFolderFileName = p_sVFolderFileName; }
    std::string GetVFolderConfigFileName() { return m_sVFolderFileName; }
  
		void TempDir(std::string p_sTempDir) { m_sTempDir = p_sTempDir; }
		std::string TempDir() { return m_sTempDir; }
		
		
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
  
  	
		CConfigFile* ConfigFile() { return m_pConfigFile; }
		
    bool UseImageMagick() { return m_pConfigFile->UseImageMagick(); }
    bool UseTaglib()      { return m_pConfigFile->UseTaglib(); }
    bool UseLibAvFormat() { return m_pConfigFile->UseLibAvFormat(); }

    std::string LameLibName() { return m_pConfigFile->LameLibName(); }
    std::string TwoLameLibName() { return m_pConfigFile->TwoLameLibName(); }
    std::string VorbisLibName() { return m_pConfigFile->VorbisLibName(); }
    std::string MpcLibName() { return m_pConfigFile->MpcLibName(); }
    std::string FlacLibName() { return m_pConfigFile->FlacLibName(); }
    std::string FaadLibName() { return m_pConfigFile->FaadLibName(); }
	std::string Mp4ffLibName() { return m_pConfigFile->Mp4ffLibName(); }
  
    
    std::string CreateTempFileName();
  
  private:
    static CSharedConfig* m_Instance;    
      
    std::string   m_sFriendlyName;
  
    CConfigFile*  m_pConfigFile;
    std::string   m_sConfigDir;
    std::string   m_sConfigFileName;
    std::string   m_sDbFileName;
    std::string   m_sVFolderFileName;

    std::string   m_sHostname;
    std::string   m_sIP;
    std::string   m_sNetInterface;
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
