/***************************************************************************
 *            SharedConfig.cpp
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

#include <iostream>

#include "SharedConfig.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/param.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#include "../version.h"

#include "Common/Common.h"
#include "Common/UUID.h"
#include "Common/RegEx.h"
#include "SharedLog.h"

#include "Transcoding/TranscodingMgr.h"
#include "DeviceSettings/DeviceIdentificationMgr.h"

#include <sys/types.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#ifdef WIN32
#else
#include <sys/utsname.h>
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN     256
#endif

using namespace std;

//const std::string FUPPES_VERSION = "0.7.2a-20070529";

CSharedConfig* CSharedConfig::m_Instance = 0;

CSharedConfig* CSharedConfig::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedConfig();
	return m_Instance;
}

CSharedConfig::CSharedConfig()
{
  m_pConfigFile = NULL;
  m_sUUID       = GenerateUUID();
  
  // ./configure --enable-default-http-port=PORT
  #ifdef DEFAULT_HTTP_PORT
  m_nHTTPPort = DEFAULT_HTTP_PORT;
  #else
  m_nHTTPPort = 0;
  #endif
}

CSharedConfig::~CSharedConfig()
{  
  if(m_pConfigFile != NULL) {
    delete m_pConfigFile;
  }
}

bool CSharedConfig::SetupConfig()
{   
  // set config dir
  if(m_sConfigDir.empty()) {
    #ifdef WIN32
    m_sConfigDir = string(getenv("APPDATA")) + "\\FUPPES\\";
    #else
    m_sConfigDir = string(getenv("HOME")) + "/.fuppes/";
    #endif  
  }  
		
  // build file names
  if(m_sConfigFileName.empty()) {
    m_sConfigFileName = m_sConfigDir + "fuppes.cfg";
  }
  
  if(m_sDbFileName.empty()) {
    m_sDbFileName = m_sConfigDir + "fuppes.db";
  }
  
  if(m_sVFolderFileName.empty()) {
    m_sVFolderFileName = m_sConfigDir + "vfolder.cfg";
  }
  
  // read the config file
  if(!ReadConfigFile()) {
    return false;
	}
  
	#warning todo: system checks (e.g. XP firewall)

  // setup temp dir
  if(m_sTempDir.empty()) {
    #ifdef WIN32
    m_sTempDir = getenv("TEMP") + string("\\fuppes\\");
    #else
    char* szTmp = getenv("TEMP");
    if(szTmp != NULL)
      m_sTempDir = string(szTmp) + "/fuppes/";
    else
      m_sTempDir = "/tmp/fuppes/";      
    #endif
  }
	else {
		if((m_sTempDir.length() > 1) && 
			 (m_sTempDir.substr(m_sTempDir.length() - 1).compare(upnpPathDelim) != 0)) {
      m_sTempDir += upnpPathDelim;
		}
	}
	
	if(!DirectoryExists(m_sTempDir)) {
    #ifdef WIN32
		CreateDirectory(m_sTempDir.c_str(), NULL);
    #else
		mkdir(m_sTempDir.c_str(), S_IRWXU | S_IRWXG);
    #endif
  }
	
  // Network settings
  if(!ResolveHostAndIP()) {  
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
		
  return true;
}

bool CSharedConfig::Refresh()
{  
  delete m_pConfigFile;
  m_pConfigFile = NULL;
  
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

void CSharedConfig::FriendlyName(std::string p_sFriendlyName)
{
  m_sFriendlyName = p_sFriendlyName;
}

std::string CSharedConfig::FriendlyName()
{
  if(m_sFriendlyName.empty()) {
    m_sFriendlyName = GetAppName() + " " + GetAppVersion() + " (" + GetHostname() + ")";
  }
  
  return m_sFriendlyName;
}

string CSharedConfig::GetHostname()
{
  return m_sHostname;
}

string CSharedConfig::GetUUID()
{
  return m_sUUID; 
}

bool CSharedConfig::SetNetInterface(std::string p_sNetInterface)
{
  m_pConfigFile->NetInterface(p_sNetInterface);
  m_sNetInterface = p_sNetInterface;
}
	
bool CSharedConfig::SetHTTPPort(unsigned int p_nHTTPPort)
{
  if(p_nHTTPPort > 0 && p_nHTTPPort <= 1024) {
    CSharedLog::Shared()->UserError("please set port to \"0\" or a number greater \"1024\"");
    return false;
  }
  
  m_pConfigFile->HttpPort(p_nHTTPPort);
  m_nHTTPPort = p_nHTTPPort;
}



// shared dirs
int CSharedConfig::SharedDirCount()
{
  return m_pConfigFile->SharedDirCount();
}

std::string CSharedConfig::GetSharedDir(int p_nIdx)
{
  string sDir = m_pConfigFile->SharedDir(p_nIdx);
  if(sDir.length() > 1 && sDir.substr(sDir.length() - 1).compare(upnpPathDelim) != 0) {
    sDir += upnpPathDelim;
  }
  return sDir;
}

void CSharedConfig::AddSharedDirectory(std::string p_sDirectory)
{  
  m_pConfigFile->AddSharedDir(ToUTF8(p_sDirectory));
}

void CSharedConfig::RemoveSharedDirectory(int p_nIdx)
{
  m_pConfigFile->RemoveSharedDir(p_nIdx);
}


// shared iTunes
int CSharedConfig::SharedITunesCount()
{
  return m_pConfigFile->SharedITunesCount();
}

std::string CSharedConfig::GetSharedITunes(int p_nIdx)
{
  return m_pConfigFile->SharedITunes(p_nIdx);  
}

void CSharedConfig::AddSharedITunes(std::string p_sITunes)
{
  m_pConfigFile->AddSharedITunes(ToUTF8(p_sITunes));
}

void CSharedConfig::RemoveSharedITunes(int p_nIdx)
{
  m_pConfigFile->RemoveSharedITunes(p_nIdx);
}






bool CSharedConfig::IsAllowedIP(std::string p_sIPAddress)
{
  // the host's address is always allowed to access
  if(p_sIPAddress.compare(m_sIP) == 0)
    return true;
  
  // if no allowed ip is set all addresses are allowed
  bool bResult = (AllowedIPCount() == 0);
    
  for(unsigned int i = 0; i < AllowedIPCount(); i++) { 
    if(GetAllowedIP(i).compare(p_sIPAddress) == 0) {
      bResult = true;
      break;
    }
  }
  
  return bResult;  
}

unsigned int CSharedConfig::AllowedIPCount()
{
  return m_pConfigFile->AllowedIpsCount(); //m_vAllowedIPs.size();
}

std::string CSharedConfig::GetAllowedIP(unsigned int p_nIdx)
{
  return m_pConfigFile->AllowedIp(p_nIdx); //m_vAllowedIPs[p_nIdx];
}

bool CSharedConfig::AddAllowedIP(std::string p_sIPAddress)
{
  m_pConfigFile->AddAllowedIp(p_sIPAddress);
}

bool CSharedConfig::RemoveAllowedIP(unsigned int p_nIndex)
{
  m_pConfigFile->RemoveAllowedIp(p_nIndex);
}



std::string CSharedConfig::GetLocalCharset()
{  
  return m_pConfigFile->LocalCharset();
}

bool CSharedConfig::SetLocalCharset(std::string p_sCharset)
{
  m_pConfigFile->LocalCharset(p_sCharset);
}

bool CSharedConfig::ReadConfigFile()
{
  string sErrorMsg;
  
  if(m_pConfigFile == NULL) {
    m_pConfigFile = new CConfigFile();
  }
  
  // create config dir
  string sConfigDir = ExtractFilePath(m_sConfigFileName);
  if(!DirectoryExists(sConfigDir)) {
    #ifdef WIN32
    CreateDirectory(sConfigDir.c_str(), NULL);
    #else
    mkdir(sConfigDir.c_str(), S_IRWXU | S_IRWXG);
    #endif
  }
  
  // write default config
  if(!FileExists(m_sConfigFileName)) {
    if(!m_pConfigFile->WriteDefaultConfig(m_sConfigFileName)) {
      CSharedLog::Shared()->Log(L_NORM, string("could not write default configuration to ") + m_sConfigFileName, __FILE__, __LINE__);  
    }
    else {
      CSharedLog::Shared()->Log(L_EXT, string("wrote default configuration to ") + m_sConfigFileName, __FILE__, __LINE__);  
    }
  }
  
  // load config file
  if(m_pConfigFile->Load(m_sConfigFileName, &sErrorMsg) != CF_OK) {
    CSharedLog::Shared()->Log(L_NORM, sErrorMsg, __FILE__, __LINE__);
    return false;
  }

  // get values from config
  if(m_pConfigFile->HttpPort() > 0) {
    m_nHTTPPort = m_pConfigFile->HttpPort();
  }

	if(m_sTempDir.empty()) {
		m_sTempDir = m_pConfigFile->TempDir();
	}
  
  return true;
}

bool CSharedConfig::ResolveHostAndIP()
{
  bool bNew = false;
  
  // get hostname
  char szName[MAXHOSTNAMELEN]; 
  int  nRet = gethostname(szName, MAXHOSTNAMELEN);  
  if(nRet != 0) {
    throw EException("can't resolve hostname", __FILE__, __LINE__);
  }
  m_sHostname = szName;

  // get interface
  m_sNetInterface = m_pConfigFile->NetInterface();
  if(m_sNetInterface.empty()) {
    ResolveIPByHostname();    
  }

  // empty or localhost
  if(m_sNetInterface.empty()) {
        
    if(m_sIP.empty() || (m_sIP.compare("127.0.0.1") == 0)) {
        
    string sMsg;
    
    if(m_sIP.compare("127.0.0.1") == 0) {
      sMsg = string("detected ip 127.0.0.1. it's possible but senseless.\n");
    }
        
		#ifdef WIN32
		sMsg += string("please enter the ip address of your lan adapter:\n");
		#else
    sMsg += string("please enter the ip address or name (e.g. eth0, wlan1, ...) of your lan adapter:\n");
    #endif    
    m_sNetInterface = CSharedLog::Shared()->UserInput(sMsg);
    bNew = true;
    
    }
    else {
      m_sNetInterface = m_sIP;
    }    
  }
    
  if(m_sNetInterface.empty()) {
    return false;
  }
  
  
  // ip or iface name   
  RegEx rxIP("\\d+\\.\\d+\\.\\d+\\.\\d");
	if(rxIP.Search(m_sNetInterface.c_str())) {
    m_sIP = m_sNetInterface;
    if(bNew) {
      m_pConfigFile->NetInterface(m_sNetInterface);
    }
    return true;
  }
  else {    
    if(ResolveIPByInterface(m_sNetInterface)) {
      if(bNew) {
        m_pConfigFile->NetInterface(m_sNetInterface);
      }
      return true;
    }
  } 
}

bool CSharedConfig::ResolveIPByHostname()
{
    in_addr* addr;
    struct hostent* host;
     
    host = gethostbyname(m_sHostname.c_str());
    if(host != NULL)
    {
      addr = (struct in_addr*)host->h_addr;
      m_sIP = inet_ntoa(*addr);
      return true;
    }
    else {
			return false;
		}
}

bool CSharedConfig::ResolveIPByInterface(std::string p_sInterfaceName)
{
  #ifdef WIN32
  return true;
  #else
  struct ifreq ifa;
  struct sockaddr_in *saddr;
  int       fd;
  
  strcpy (ifa.ifr_name, p_sInterfaceName.c_str());
  if(((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) || ioctl(fd, SIOCGIFADDR, &ifa)) {
		cout << "[ERROR] can't resolve ip from interface \"" << p_sInterfaceName << "\"." << endl;
		return false;
	}
  saddr = (struct sockaddr_in*)&ifa.ifr_addr;
  m_sIP = inet_ntoa(saddr->sin_addr);  
  //cout << "address of iface: " << p_sInterfaceName << " = " << m_sIP << endl;  
  return true;
  #endif
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
    int nMajor = osinfo.dwMajorVersion;
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
  sResult << m_sTempDir << nTmpCnt;
  nTmpCnt++;
  return sResult.str();
}
