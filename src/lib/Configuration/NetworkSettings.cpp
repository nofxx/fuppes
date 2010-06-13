#ifndef WIN32
//#include <unistd.h>
//#include <sys/param.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#include <cassert>

#include "NetworkSettings.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"

#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../Common/Exception.h"

using namespace std;
using namespace fuppes;

NetworkSettings::NetworkSettings()
{  
  // ./configure --enable-default-http-port=PORT
  #ifdef DEFAULT_HTTP_PORT
  m_nHTTPPort = DEFAULT_HTTP_PORT;
  #else
  m_nHTTPPort = 0;
  #endif
  m_sNetInterface = "";  
}

NetworkSettings::~NetworkSettings()
{
}



bool NetworkSettings::InitPostRead() {
  // Network settings
  return ResolveHostAndIP();
}

bool NetworkSettings::SetHTTPPort(unsigned int p_nHTTPPort)
{
  assert(pStart != NULL);

  if(p_nHTTPPort > 0 && p_nHTTPPort <= 1024) {
    CSharedLog::Shared()->UserError("please set port to \"0\" or a number greater \"1024\"");
    return false;
  }
  
  CXMLNode* pTmp = pStart->FindNodeByName("http_port");
  if(pTmp) {
    pTmp->Value(p_nHTTPPort);
    CSharedConfig::Shared()->Save();  
    m_nHTTPPort = p_nHTTPPort;
  }  
	return true;
}

void NetworkSettings::SetNetInterface(std::string p_sNetInterface)
{
  assert(pStart != NULL);

  CXMLNode* pTmp = pStart->FindNodeByName("interface");
  if(pTmp) {
    pTmp->Value(p_sNetInterface);
    if(!CSharedConfig::Shared()->Save()) {
      CSharedLog::Log(L_NORM, __FILE__, __LINE__, "[network settings] Error: interface save failed.");
    }
    m_sNetInterface = p_sNetInterface;
  } 
}

bool NetworkSettings::IsAllowedIP(std::string p_sIPAddress)
{
  if(p_sIPAddress.compare(m_sIP) == 0) return true; // the host's address is always allowed to access
  if (AllowedIPCount() == 0) return true;           // if no allowed ip is set all addresses are allowed

  for(unsigned int i = 0; i < AllowedIPCount(); i++) { 
		string pattern = GetAllowedIP(i);
		pattern = StringReplace(pattern, ".*", ".[0-255]");  // 192.*.0.* => 192.[0-255].0.[0-255]
		pattern = StringReplace(pattern, "*", "255"); // 192.168.0.[10-*] => 192.168.0.[10-255]

		RegEx rxIp(pattern.c_str());
		if(rxIp.Search(p_sIPAddress.c_str()))
		  return true;
  }
  
  return false;  
}

bool NetworkSettings::AddAllowedIP(std::string p_sIpAddress)
{
  assert(pStart != NULL);

  pStart->AddChild("ip", p_sIpAddress);
  Read();
  return CSharedConfig::Shared()->Save();
}

bool NetworkSettings::RemoveAllowedIP(int p_nIndex)
{
  assert(p_nIndex >= 0);
  assert(pStart != NULL);

  int nIdx = 0;
  
  CXMLNode* pObj = pStart->FindNodeByName("allowed_ips");
  CXMLNode* pTmp;
  for(int i = 0; i < pObj->ChildCount(); i++) {
    pTmp = pObj->ChildNode(i);
    
    if(pTmp->Name().compare("ip") == 0) {    
      if(nIdx == p_nIndex) {
        pObj->RemoveChild(i);
        // TODO: Double Check that this save and read are in the right order
        // you have removed the child so it would make sense to save and then
        // read but i think that it might be okay because you only load the config
        // file once. If it does not matter though then we should probably do save then 
        // read just to be safe.
        Read();
        return CSharedConfig::Shared()->Save();
      }        
    }
  }  

  return false;
}

bool NetworkSettings::ResolveHostAndIP()
{
  bool bNew = false;
  
  // get hostname
  char szName[MAXHOSTNAMELEN + 1];
  memset(szName, 0, MAXHOSTNAMELEN + 1);
  int  nRet = gethostname(szName, MAXHOSTNAMELEN);  
  if(nRet != 0) {
    throw fuppes::Exception(__FILE__, __LINE__, "can't resolve hostname");
  }
  m_sHostname = szName;

  // get interface
  if(m_sNetInterface.empty()) ResolveIPByHostname();

  // empty or localhost
  if(m_sNetInterface.empty()) {
    if(m_sIP.empty() || (m_sIP.compare("127.0.0.1") == 0)) {
        
    string sMsg;
    
    if(m_sIP.compare("127.0.0.1") == 0) {
      sMsg = string("detected ip 127.0.0.1. it's possible, maybe for testing, but senseless.\n");
    }
        
		#ifdef WIN32
		sMsg += string("Please enter the ip address of your lan adapter:\n");
		#else
    sMsg += string("Please enter the ip address or name (e.g. eth0, wlan1, ...) of your lan adapter:\n");
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
      SetNetInterface(m_sNetInterface);
    }
    return true;
  }
  else {    
    if(ResolveIPByInterface(m_sNetInterface)) {
      if(bNew) {
        SetNetInterface(m_sNetInterface);
      }
      return true;
    }
  } 
	
	return false;
}

bool NetworkSettings::ResolveIPByHostname()
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

bool NetworkSettings::ResolveIPByInterface(std::string p_sInterfaceName)
{
  #ifdef WIN32
  return true;
  #else
  struct ifreq ifa;
  struct sockaddr_in *saddr;
  int       fd;
  char str[INET_ADDRSTRLEN + 1];
  
  strcpy (ifa.ifr_name, p_sInterfaceName.c_str());
  if(((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) || ioctl(fd, SIOCGIFADDR, &ifa)) {
		cout << "[ERROR] can't resolve ip from interface \"" << p_sInterfaceName << "\"." << endl;
		return false;
	}
  saddr = (struct sockaddr_in*)&(ifa.ifr_addr);
  m_sIP = inet_ntop(AF_INET, &(saddr->sin_addr), str, INET_ADDRSTRLEN);
  close(fd);
  return true;
  #endif
}

bool NetworkSettings::Read(void)
{
  assert(pStart != NULL);
  int i, j;  
  
  m_lAllowedIps.clear();
  
  for(i = 0; i < pStart->ChildCount(); i++) {
      
    if(pStart->ChildNode(i)->Name().compare("interface") == 0) {
      if(pStart->ChildNode(i)->Value().length() > 0) {
        m_sNetInterface = pStart->ChildNode(i)->Value();
      }        
    }
    else if(pStart->ChildNode(i)->Name().compare("http_port") == 0) {
      if(pStart->ChildNode(i)->Value().length() > 0) {
        m_nHTTPPort = atoi(pStart->ChildNode(i)->Value().c_str());
      } 
    }
    else if(pStart->ChildNode(i)->Name().compare("allowed_ips") == 0) {
      for(j = 0; j < pStart->ChildNode(i)->ChildCount(); j++) {
        if(pStart->ChildNode(i)->ChildNode(j)->Name().compare("ip") == 0) {
          m_lAllowedIps.push_back(pStart->ChildNode(i)->ChildNode(j)->Value());
        }
      }          
    }     
    
  }  

  return true;
}

