#ifndef __NETWORK_SETTINGS_H
#define __NETWORK_SETTINGS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <vector>
#include <string>
#include "ConfigSettings.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN     256
#endif

class NetworkSettings : public ConfigSettings
{
  public:
    NetworkSettings();
    virtual ~NetworkSettings();
    
    virtual bool Read(void);

    std::string GetHostname(void) { return m_sHostname; }
    std::string GetIPv4Address(void) { return m_sIP; }
    std::string GetNetInterface(void) { return m_sNetInterface; }
    void SetNetInterface(std::string p_sNetInterface);
  
    unsigned int GetHTTPPort() { return m_nHTTPPort; }
    bool SetHTTPPort(unsigned int p_nHTTPPort);
    
    //  allowed ip
    unsigned int AllowedIPCount() { return m_lAllowedIps.size(); }
    bool IsAllowedIP(std::string p_sIPAddress);  
    bool AddAllowedIP(std::string p_sIPAddress);
    std::string GetAllowedIP(unsigned int p_nIdx) { return m_lAllowedIps[p_nIdx]; }
    bool RemoveAllowedIP(int p_nIndex);  

  private:
    virtual void InitVariables(void) { }
    virtual bool InitPostRead(void);
    
    std::string   m_sHostname;
    std::string   m_sIP;
    std::string   m_sNetInterface;    
    unsigned int  m_nHTTPPort;

    std::vector<std::string>  m_lAllowedIps;
  
    bool ResolveHostAndIP();
    bool ResolveIPByHostname();
    bool ResolveIPByInterface(std::string p_sInterfaceName);
};

#endif
