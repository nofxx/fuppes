/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            SharedConfig.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include <iostream>

#include "SharedConfig.h"

#include "Common/Common.h"
#include "Common/File.h"
#include "Common/Directory.h"
#include "Common/UUID.h"
#include "Common/Process.h"
#include "SharedLog.h"

#include "Transcoding/TranscodingMgr.h"
#include "DeviceSettings/DeviceIdentificationMgr.h"
#include "Plugins/Plugin.h"

#include "Configuration/DefaultConfig.h"

#include <sys/types.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#ifdef WIN32
#else
#include <sys/utsname.h>
#endif

using namespace std;
using namespace fuppes;

static void PrintConfigReadErrors(int error);

CSharedConfig* CSharedConfig::m_Instance = 0;

CSharedConfig* CSharedConfig::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedConfig();
	return m_Instance;
}

CSharedConfig::CSharedConfig()
{
  m_sFileName = CONFIG_NAME;
  m_pDoc = NULL;

  pathFinder = new PathFinder();
  pathFinder->SetupDefaultPaths();

  /*pluginDirectories = new PluginDirectories();
  pluginDirectories->SetupDefaultPaths();*/

  // Create all of the smaller objects
  m_sharedObjects = new SharedObjects();
  networkSettings = new NetworkSettings();
  globalSettings = new GlobalSettings();
  m_virtualFolders = new VirtualFolders();
  deviceMapping = new DeviceMapping();
  contentDirectory = new ContentDirectory();
  databaseSettings = new DatabaseSettings();
  transcodingSettings = new TranscodingSettings();
}

CSharedConfig::~CSharedConfig()
{  
	delete CDeviceIdentificationMgr::Shared();
	delete CTranscodingMgr::Shared();

  // delete all of the smaller objects
  delete m_sharedObjects;
  delete networkSettings;
  delete globalSettings;
  delete deviceMapping;
  delete m_virtualFolders;
  delete contentDirectory;
  delete databaseSettings;
  delete transcodingSettings;
  
  //delete pluginDirectories;
  delete pathFinder;

	CProcessMgr::uninit();
	CPluginMgr::uninit();
}

bool CSharedConfig::FindConfigPaths(void)
{
  m_sBaseConfigFile = pathFinder->findInPath(m_sFileName, File::readable);
  if(m_sBaseConfigFile.empty()) {
    CSharedLog::Log(L_NORM, __FILE__, __LINE__, "Could not find a readable config file.");
    return false;
  }
  
  Log::log(Log::config, Log::normal, __FILE__, __LINE__, "Shared (Base) Config File: %s", m_sBaseConfigFile.c_str());
  cout << "Shared (Base) Config File:" << m_sBaseConfigFile << endl;
  return true;
}

bool CSharedConfig::CreateConfigFile(void)
{
  // find a writable directory in the path to see if we can do that first
  m_sBaseConfigFile = pathFinder->findInPath("", Directory::writable); 
  if (!m_sBaseConfigFile.empty()) {
    m_sBaseConfigFile += m_sFileName;
    
    // Try and write the default config file
    if(!WriteDefaultConfig(m_sBaseConfigFile)) {
      CSharedLog::Log(L_NORM, __FILE__, __LINE__, "could not write default configuration to %s", m_sBaseConfigFile.c_str());
    } else {
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, "wrote default configuration to %s", m_sBaseConfigFile.c_str());
      return true;
    }
  }

  // create config dir because it might not have been 
  // - the default config dir should always be the first one in the list
  string sConfigDir = pathFinder->DefaultPath();
  if(Directory::create(sConfigDir)) {
    sConfigDir += m_sFileName;
    // Try and write the default config file
    if(!WriteDefaultConfig(sConfigDir)) {
      CSharedLog::Log(L_NORM, __FILE__, __LINE__, "could not write default configuration to %s", sConfigDir.c_str());
    }
    else {
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, "wrote default configuration to %s", sConfigDir.c_str());
      return true;
    }
  }

  return false;
}

#ifdef WIN32
bool CSharedConfig::SetupConfig(std::string applicationDir)
{   
	applicationDir = ExtractFilePath(applicationDir);

	if((applicationDir.length() > 1) && 
		 (applicationDir.substr(applicationDir.length() - 1).compare(upnpPathDelim) != 0)) {
    applicationDir += upnpPathDelim;
	}
  if(m_dataDir.empty())  
    m_dataDir 	= applicationDir + "data/";

  //pluginDirectories->AddPluginPath(applicationDir);
  m_pluginDirectory = applicationDir;
#else
bool CSharedConfig::SetupConfig()
{ 
  if(m_dataDir.empty())  
  	m_dataDir 	= string(FUPPES_DATADIR) + "/";

  //pluginDirectories->AddPluginPath(string(FUPPES_PLUGINDIR) + "/");
  m_pluginDirectory = string(FUPPES_PLUGINDIR) + "/";
#endif

  // this sets the default search path for config files
  // and then sets them
  if(!FindConfigPaths()) {
    if(!CreateConfigFile()) {
      // We have a serious problem, we should probably quit now
      return false;
    }
  }

  // read the config file
  if(!ReadConfigFile()) {
    Log::error(Log::config, Log::normal, __FILE__, __LINE__, "The config file failed to load properly.");
    return false;
	}

  // read OS information
  GetOSInfo();
  
	// init device settings when 
	// os and network info is available
	CDeviceIdentificationMgr::Shared()->Initialize();		
		
  // transcoding mgr must be initialized
  // by the main thread so let's do it
  // when everytihng else is correct
  CTranscodingMgr::Shared();

	CProcessMgr::init();

	CPluginMgr::init();
	
  return true;
}

bool CSharedConfig::Refresh()
{  
  // TODO: What needs to be cleaned here before a refresh?

  // ... and read the config file
  return ReadConfigFile();
}

void CSharedConfig::AddFuppesInstance(CFuppes* pFuppes)
{
  /* Add the instance to the list */
  m_vFuppesInstances.push_back(pFuppes);
}

CFuppes* CSharedConfig::GetFuppesInstance(unsigned int p_nIndex)
{
  return m_vFuppesInstances[p_nIndex];
}

unsigned int CSharedConfig::GetFuppesInstanceCount()
{
  return m_vFuppesInstances.size();
}

string CSharedConfig::GetAppName()
{
  return "FUPPES";
}

string CSharedConfig::GetAppFullname()
{
  return "Free UPnP Entertainment Service";
}

string CSharedConfig::GetAppVersion()
{
	return FUPPES_VERSION;
}

string CSharedConfig::GetUUID()
{
	if(m_sUUID.empty()) {
    bool foundDir = false;
		if(globalSettings->UseFixedUUID()) {
      string uuidDir = pathFinder->findInPath("", Directory::exists);
      if (!uuidDir.empty()) {
			    m_sUUID = GenerateUUID(uuidDir + UUID_NAME);
          foundDir = true;
      }
		}
		
    if(!foundDir) { // if you could not find a place to put the file or if no file should be made
			m_sUUID = GenerateUUID();
		}
	}
	
  return m_sUUID; 
}

bool CSharedConfig::ReadConfigFile()
{
  if(m_pDoc != NULL) delete m_pDoc;
  m_pDoc = new CXMLDocument();

  // Load the XML file
  if(!m_pDoc->LoadFromFile(m_sBaseConfigFile)) {    
    //*p_psErrorMsg = "parse error"; log instead
    delete m_pDoc;
    m_pDoc = NULL;
    return false;
  }

  CXMLNode* pRootNode = m_pDoc->RootNode();
  if(pRootNode == NULL) {
    //*p_psErrorMsg = "parse error";
    delete m_pDoc;
    m_pDoc = NULL;
    return false;
  }
  
  string sVersion = pRootNode->Attribute("version");
  if(sVersion.compare(NEEDED_CONFIGFILE_VERSION) != 0) {
    //*p_psErrorMsg = "configuration deprecated";
    PrintConfigReadErrors(READERROR_CONFIG_DEPRECATED);
    delete m_pDoc;
    m_pDoc = NULL;
    return false;
  }
  
  // These have to be updated with the right nodes
  CXMLNode* pTmp;

  pTmp = pRootNode->FindNodeByName("shared_objects");
  if (pTmp) {	
    m_sharedObjects->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_SHARED_OBJECTS);
  }

  pTmp = pRootNode->FindNodeByName("network");
  if (pTmp) {	
    networkSettings->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_NETWORK);
    delete m_pDoc;
    m_pDoc = NULL;
    return false;
  }

  pTmp = pRootNode->FindNodeByName("global_settings");
  if (pTmp) {	
	  globalSettings->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_GLOBAL_SETTINGS);
  }

  pTmp = pRootNode->FindNodeByName("vfolders");
  if (pTmp) {	
    m_virtualFolders->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_VFOLDER_SETTINGS);
    delete m_pDoc;
    m_pDoc = NULL;
    return false;
  }
  
  pTmp = pRootNode->FindNodeByName("device_mapping");
  if (pTmp) {	
    deviceMapping->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_DEVICE_MAPPING);
    delete m_pDoc;
    m_pDoc = NULL;
    return false;
  }

  pTmp = pRootNode->FindNodeByName("content_directory");
  if (pTmp) {	
    contentDirectory->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_CONTENT_DIRECTORY);
  }

  pTmp = pRootNode->FindNodeByName("database");
  if (pTmp) {	
    databaseSettings->Init(pTmp);
  } else if(databaseSettings->UseDefaultSettings()) {
    PrintConfigReadErrors(READERROR_DATABASE_DEFAULT);
  } else {
    PrintConfigReadErrors(READERROR_DATABASE_FAIL);
    delete m_pDoc;
    m_pDoc = NULL;
    return false;
  }

  pTmp = pRootNode->FindNodeByName("transcoding");
  if (pTmp) {	
	  transcodingSettings->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_TRANSCODING);
  }

  /*
  pTmp = pRootNode->FindNodeByName("plugin_directories");
  if (pTmp) {	
	  pluginDirectories->Init(pTmp);
  } else {
    PrintConfigReadErrors(READERROR_PLUGINDIRS);
  }*/

  return true;
}

static void PrintConfigReadErrors(int error) {
  switch (error) {
    case READERROR_CONFIG_DEPRECATED:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "ERROR: The config file is deprecated.");
      break;
    case READERROR_SHARED_OBJECTS:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "Warning: Could not find any shared_objects in the config file. It seems unlikely that this would be desirable.");
      break;
    case READERROR_NETWORK:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "ERROR: The Network settings could not be found in the config file. They are required.");
      break;
    case READERROR_GLOBAL_SETTINGS:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "Warning: The Global settings could not be read.");
      break;
    case READERROR_VFOLDER_SETTINGS:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "Warning: The vfolder settings could not be read.");
      break;
    case READERROR_DEVICE_MAPPING:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "ERROR: A device mapping could not be found and one is required. Even an empty device mapping will work. (The empty device mapping makes everything use the default device)");
      break;
    case READERROR_CONTENT_DIRECTORY:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "Warning: The Content Directory settings could not be read.");
      break;
    case READERROR_DATABASE_DEFAULT:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "Warning: The Database settings could not be read but the default settings are being used.");
      break;
    case READERROR_DATABASE_FAIL:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "ERROR: The Database Settings could not be found and the default ones did not work. Fuppes needs a database to run so these settings need to be avaliable.");
      break;
    case READERROR_TRANSCODING:
      Log::error(Log::config, Log::normal, __FILE__, __LINE__, "Warning: The Transcoding settings could not be read.");
      break;
    case READERROR_PLUGINDIRS:
      Log::log(Log::config, Log::normal, __FILE__, __LINE__, "Warning: The Plugin directory settings could not be read.");
      break;
    default:
      // wow...an error on the error
      break;
  }
}

void CSharedConfig::PrintTranscodingSettings()
{
  CTranscodingMgr::Shared()->PrintTranscodingSettings();    
}

std::string CSharedConfig::GetOSName()
{
  return m_sOSName;
}

std::string CSharedConfig::GetOSVersion()
{
  return m_sOSVersion;
}

void CSharedConfig::GetOSInfo()
{
  #ifdef WIN32
  m_sOSName = "Windows";

  OSVERSIONINFO osinfo;
  osinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!GetVersionEx(&osinfo)) {
    m_sOSVersion = "?";
  }
  else {
    iint nMajor = osinfo.dwMajorVersion;
    int nMinor = osinfo.dwMinorVersion;
    int nBuild = osinfo.dwBuildNumber; 
    
    stringstream sVer;
    sVer << nMajor << "." << nMinor << "." << nBuild;    
    m_sOSVersion = sVer.str();
  }
  #else  
  struct utsname sUtsName;
  uname(&sUtsName);  
  m_sOSName    = sUtsName.sysname;
  m_sOSVersion = sUtsName.release;  
  #endif
}

static int nTmpCnt = 0;

std::string CSharedConfig::CreateTempFileName()
{
  stringstream sResult;
  sResult << globalSettings->GetTempDir() << nTmpCnt;
  nTmpCnt++;
  return sResult.str().c_str();
}


bool CSharedConfig::isAlbumArtFile(const std::string fileName) // static
{
	string name = ToLower(fileName);
#warning todo: What is left to do?
	if(name.compare("cover.jpg") == 0 ||
		 name.compare("cover.png") == 0 ||
		 name.compare(".folder.jpg") == 0 ||
		 name.compare(".folder.png") == 0 ||
     name.compare("folder.jpg") == 0 ||
		 name.compare("folder.png") == 0 ||
		 name.compare("front.jpg") == 0 ||
		 name.compare("front.png") == 0)
		return true;
	
	return false;
}

std::string CSharedConfig::getAlbumArtFiles() // static
{
  string result = "";
  
  fuppes::StringList ext;
  ext.push_back("jpg");
  ext.push_back("jpeg");
  ext.push_back("png");

  fuppes::StringList file;
  file.push_back("cover");
  file.push_back(".folder");
  file.push_back("folder");
  file.push_back("front");
  
  StringListIterator iterExt;
  StringListIterator iterFile;
  for(iterExt = ext.begin(); iterExt != ext.end(); iterExt++) {

    for(iterFile = file.begin(); iterFile != file.end(); iterFile++) {

      if(result.length() > 0)
        result += ",";
      result += "'" + *iterFile + "." + *iterExt + "'";
    }
  }
  
  return result;
}

bool CSharedConfig::WriteDefaultConfig(std::string p_sFileName)
{
  return WriteDefaultConfigFile(p_sFileName);
}

bool CSharedConfig::Save(void) {
  return m_pDoc->Save();
}
