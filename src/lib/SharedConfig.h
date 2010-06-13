/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            SharedConfig.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

//#define CONFIG_NAME "fuppes.cfg"
#define VFOLDER_DIR "vfolders"
#define DEVICE_DIR "devices"

#define UUID_NAME "UUID.txt"

// Errors that could happen when reading the global config file
#define READERROR_CONFIG_DEPRECATED 1
#define READERROR_SHARED_OBJECTS    2
#define READERROR_NETWORK           3
#define READERROR_GLOBAL_SETTINGS   4
#define READERROR_VFOLDER_SETTINGS  5
#define READERROR_DEVICE_MAPPING    6
#define READERROR_CONTENT_DIRECTORY 7
#define READERROR_DATABASE_DEFAULT  8
#define READERROR_DATABASE_FAIL     9
#define READERROR_TRANSCODING       10
#define READERROR_PLUGINDIRS        11

#include <string>
#include <vector>

#include "Configuration/PathFinder.h"
#include "Configuration/SharedObjects.h"
#include "Configuration/NetworkSettings.h"
#include "Configuration/GlobalSettings.h"
#include "Configuration/DeviceMapping.h"
#include "Configuration/ContentDirectoryConfig.h"
#include "Configuration/DatabaseSettings.h"
#include "Configuration/TranscodingSettings.h"

#include "DeviceSettings/DeviceSettings.h"
#include "../../include/fuppes_db_connection_plugin.h"

class CFuppes;

// SharedConfig is way too huge, it needs to have many smaller modules worth of settings.
class CSharedConfig
{
  protected:
		CSharedConfig(); 

	public:
    ~CSharedConfig();
    static CSharedConfig* Shared();

#ifdef WIN32
    bool SetupConfig(std::string applicationDir);
#else
		bool SetupConfig();
#endif	
    bool Refresh();

    void PrintTranscodingSettings();  

    // The Object in control of paths
    PathFinder* pathFinder;

		// there is absolutely no need for more than one plugin dir
    //PluginDirectories* pluginDirectories;
		std::string				pluginDirectory() { return m_pluginDirectory; }
		void							setPluginDirectory(std::string dir) { m_pluginDirectory = dir; }

    // Make the smaller objects that we will use
    static SharedObjects* sharedObjects() { return Shared()->m_sharedObjects; }
    NetworkSettings* networkSettings;
    GlobalSettings* globalSettings;
    static VirtualFolders* virtualFolders() { return Shared()->m_virtualFolders; }
    DeviceMapping* deviceMapping;
    ContentDirectory* contentDirectory;
    DatabaseSettings* databaseSettings;
    TranscodingSettings* transcodingSettings;

    
    
    // Instance Settings
    std::string GetAppName();
    std::string GetAppFullname();
    std::string GetAppVersion();
    std::string GetOSName();
    std::string GetOSVersion();

    // Command Line Options
    void SetConfigFileName(std::string p_sConfigFileName) {
      if (!p_sConfigFileName.empty()) m_sFileName = p_sConfigFileName;
    }  
    std::string GetConfigFileName() { return m_sFileName; }

    //std::string GetConfigDir();

		void dataDir(std::string dir) { m_dataDir = appendTrailingSlash(dir); }
		std::string dataDir() { return m_dataDir; }
		
    std::string GetUUID();
  
		// album art
		static bool isAlbumArtFile(const std::string fileName);
    static std::string getAlbumArtFiles();

    
    unsigned int GetFuppesInstanceCount();
    CFuppes* GetFuppesInstance(unsigned int p_nIndex);
    void AddFuppesInstance(CFuppes* pFuppes);
    
    std::string CreateTempFileName();

    bool WriteDefaultConfig(std::string p_sFileName);

    // Flushes all unsaved changes to the XMLFile
    bool Save(void);

  private:
    static CSharedConfig* m_Instance;    
    CXMLDocument* m_pDoc;

    
    SharedObjects* m_sharedObjects;
    VirtualFolders* m_virtualFolders;

    
    std::string   m_sFileName;
    std::string   m_sBaseConfigFile;

    std::string   m_sUUID;
    std::string   m_sOSName;
    std::string   m_sOSVersion;  
		std::string		m_dataDir;

    std::vector<CFuppes*> m_vFuppesInstances;


		std::string				m_pluginDirectory;
		
    // Actually parsing the config files
    bool ReadConfigFile();

    bool FindConfigPaths(void);
    bool CreateConfigFile(void);

    void GetOSInfo();  
};

#endif // _SHAREDCONFIG_H
