#ifndef __DEVICEMAPPING_H
#define __DEVICEMAPPING_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <vector>
#include <string>
#include <set>
#include "ConfigSettings.h"
#include "../DeviceSettings/DeviceSettings.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"

struct mapping {
  std::string value, vfolder;
  CDeviceSettings* device;
};

class DeviceMapping : public ConfigSettings
{
  public:
    friend class CDeviceIdentificationMgr;

    virtual bool Read(void);

    void requiredVirtualDevices(std::set<std::string>* vfolders);

  private:
    // device mappings
    std::vector<struct mapping> macAddrs;
    std::vector<struct mapping> ipAddrs;
};

#endif
