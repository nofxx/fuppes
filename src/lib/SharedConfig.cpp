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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
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

#include "Common/Common.h"
#include "Common/UUID.h"
#include "Common/RegEx.h"
#include "SharedLog.h"

#include "Transcoding/TranscodingMgr.h"

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
#define MAXHOSTNAMELEN     64
#endif

using namespace std;

const std::string FUPPES_VERSION = "0.7.2";

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

bool CSharedConfig::SetupConfig(std::string p_sConfigFileName)
{  
  m_sConfigFileName = p_sConfigFileName;
  
  // read config file
  if(!ReadConfigFile()) {
    return false;
	}
  
	#warning todo: system checks (e.g. XP firewall)

	/*stringstream sTemp;
	#ifdef WIN32
	sTemp << getenv("TEMP") << "\\fuppes\\";
	m_sTempDir = sTemp.str();
	if(!DirectoryExists(m_sTempDir)) 
		CreateDirectory(m_sTempDir.c_str(), NULL);
	#else
  char* szTmp = getenv("TEMP");
	if(szTmp != NULL)
	  sTemp << szTmp << "/fuppes/";
	else
	  sTemp << "/tmp/fuppes/";
	m_sTempDir = sTemp.str();
	if(!DirectoryExists(m_sTempDir)) 
		mkdir(m_sTempDir.c_str(), S_IRWXU | S_IRWXG);
	#endif
	cout << "TEMP: " << m_sTempDir << "." << endl; */
	
	
  // Network settings
  if(!ResolveHostAndIP()) {  
    return false;
  }
  
  // read OS information
  GetOSInfo();
  
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

string CSharedConfig::GetHostname()
{
  return m_sHostname;
}

string CSharedConfig::GetUUID()
{
  return m_sUUID; 
}

bool CSharedConfig::SetIPv4Address(std::string p_sIPAddress)
{
  m_pConfigFile->IpAddress(p_sIPAddress);
  m_sIP = p_sIPAddress;
}
	
bool CSharedConfig::SetHTTPPort(unsigned int p_nHTTPPort)
{
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
  return m_pConfigFile->SharedDir(p_nIdx);
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



std::string CSharedConfig::GetConfigDir()
{
  stringstream sResult;
  #ifdef WIN32
  sResult << getenv("APPDATA") << "\\Free UPnP Entertainment Service\\";
  #else
  sResult << getenv("HOME") << "/.fuppes/";
  #endif
  return sResult.str();
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
  bool bResult = true;
  std::string sErrorMsg;
  if(m_pConfigFile == NULL) {
    m_pConfigFile = new CConfigFile();
  }
  
  // get config file name
  if(m_sConfigFileName.length() == 0) {
    stringstream sFileName;
    stringstream sDir;
    
    #ifdef WIN32
    sDir << getenv("APPDATA") << "\\Free UPnP Entertainment Service\\";
    sFileName << sDir.str() << "fuppes.cfg";
    if(!DirectoryExists(sDir.str())) 
      CreateDirectory(sDir.str().c_str(), NULL);
    #else
    sDir << getenv("HOME") << "/.fuppes/";
    sFileName << sDir.str() << "fuppes.cfg";
    if(!DirectoryExists(sDir.str())) 
      mkdir(sDir.str().c_str(), S_IRWXU | S_IRWXG);
    #endif
    
    m_sConfigFileName = sFileName.str();     
  }
  
  // write default config
  if(!FileExists(m_sConfigFileName)) {
    if(!m_pConfigFile->WriteDefaultConfig(m_sConfigFileName)) {
      CSharedLog::Shared()->Log(L_ERROR, string("could not write default configuration to ") + m_sConfigFileName, __FILE__, __LINE__);  
    }
    else {
      CSharedLog::Shared()->Log(L_NORMAL, string("wrote default configuration to ") + m_sConfigFileName, __FILE__, __LINE__);  
    }
  }
  
  // load config file
  if(m_pConfigFile->Load(m_sConfigFileName, &sErrorMsg) != CF_OK) {
    CSharedLog::Shared()->Log(L_ERROR, sErrorMsg, __FILE__, __LINE__);
    return false;
  }
  
  // get values from config
  m_sIP       = m_pConfigFile->IpAddress();
  if(m_pConfigFile->HttpPort() > 0) {
    m_nHTTPPort = m_pConfigFile->HttpPort();
  }
  
  return true;
}

bool CSharedConfig::ResolveHostAndIP()
{
  char name[MAXHOSTNAMELEN];
  
  int nRet = gethostname(name, MAXHOSTNAMELEN);
  if(0 == nRet)
  {   
    m_sHostname = name;
    
		if((m_sIP == "") || (m_sIP.compare("127.0.0.1") == 0))
		  ResolveIPByHostname();
		
    if((m_sIP == "") || (m_sIP.compare("127.0.0.1") == 0))
    {
			if(m_sIP.compare("127.0.0.1") == 0)
			  cout << "detected ip 127.0.0.1. it's possible but sensless." << endl;
		
		  string sIface;
		  #ifdef WIN32
			cout << "please enter the ip address of your lan adapter" << endl;
			cin >> sIface;
			if(sIface.length() > 0) {
			  m_sIP = sIface;
        m_pConfigFile->IpAddress(m_sIP);
			  return true;
      }
			#else
    	cout << "please enter the ip address or name (e.g. eth0, wlan1, ...) of your lan adapter" << endl;			
			cin >> sIface;
			RegEx rxIP("\\d+\\.\\d+\\.\\d+\\.\\d");
			if(rxIP.Search(sIface.c_str())) {
			  m_sIP = rxIP.Match(0);
        m_pConfigFile->IpAddress(m_sIP);
				return true;
			}
			else {
		    if(ResolveIPByInterface(sIface)) {
          m_pConfigFile->IpAddress(m_sIP);
          return true;
        }
        else {
          return false;
        }
			}
			#endif
    }
    else    
      return true;
  }
  else {
    cout << "[ERROR] can't resolve hostname" << endl;
		return false;
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
