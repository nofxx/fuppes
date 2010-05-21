#include <cassert>

#include "DeviceConfigFile.h"
#include "DeviceMapping.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"
#include "../Configuration/PathFinder.h"

using namespace std;
using namespace fuppes;

void PrintSetupDeviceErrorMessages(int error, string deviceName);

bool DeviceMapping::Read(void) 
{
  assert(pStart != NULL);

  CDeviceIdentificationMgr* identMgr = CDeviceIdentificationMgr::Shared();
  CXMLNode* pDevice = NULL;
  CDeviceConfigFile* devConf = new CDeviceConfigFile();
  int error;

  // Setup the default device
  error = devConf->SetupDevice(identMgr->GetSettingsForInitialization("default"), string("default"));
  if (error) {
    // Not being able to load the default device is a fatal error
    PrintSetupDeviceErrorMessages(error, string("default"));
    delete devConf;
    return false;
  }

  string name;
  struct mapping temp;
  for(int i = 0; i < pStart->ChildCount(); ++i) {
    pDevice = pStart->ChildNode(i);

    // setup the mapping
    temp.value = pDevice->Attribute("value");
    // make the right device for it
    string devName = pDevice->Attribute("device");
    // by default if there is no device field then it is the default device.
    if (devName.empty()) devName = "default";
    temp.device = identMgr->GetSettingsForInitialization(devName);
    temp.vfolder = pDevice->Attribute("vfolder");

    // you have to map to an IP or Mac Addr (and we should add in ranges too)
    if(!temp.value.empty()) {
      // Now load all of the settings for the device
      error = devConf->SetupDevice(temp.device, devName);
      if(error) { // that is meant to be a single equals
        PrintSetupDeviceErrorMessages(error, devName);
      } else {
        // Add it to our lists
        if(pDevice->Name().compare("mac") == 0) {
          macAddrs.push_back(temp);
        } else if(pDevice->Name().compare("ip") == 0) {
          ipAddrs.push_back(temp);
        } else {
          // only 'mac' or 'ip' fields allowed in device_mapping
          CSharedLog::Log(L_NORM, __FILE__, __LINE__, "'%s not loaded: '%s' is an invalid name for a field in device_mapping.",
              devName.c_str(),
              pDevice->Name().c_str());
        }
      }
    }
  }

  delete devConf;

  return true;
}

// The error messages are messy so this prints them out and makes the logic clearer
void PrintSetupDeviceErrorMessages(int error, string deviceName) {
  assert(error > SETUP_DEVICE_SUCCESS);

  string deviceFile = deviceName + DEVICE_EXT;

  if(deviceName.compare("default") == 0) {
    switch (error) {
      case SETUP_DEVICE_NOT_FOUND:
        Log::error(Log::config, Log::normal, __FILE__, __LINE__, "The default config file %s was not found in the search path. This is a fatal error, a default config is required. Try 'locate %s' in a terminal to see where it got to.",
            deviceFile.c_str(), 
            deviceFile.c_str());
        break;
      case SETUP_DEVICE_LOAD_FAILED:
        Log::error(Log::config, Log::normal, __FILE__, __LINE__, "The config file '%s' for '%s' was found but could not be loaded. You need a default config so this error means something is wrong and FUPPES will not run in these conditions. Try 'locate %s' in a terminal to see where it got to.",
        deviceFile.c_str(), 
        deviceName.c_str(),
        deviceFile.c_str());
        break;
      default:
        Log::error(Log::config, Log::normal, __FILE__, __LINE__, "An undefined error ocurred while trying to load the default configuration file. It had the error code: %d",
            error);
        break;
    }
  } else {
    switch (error) {
      case SETUP_DEVICE_NOT_FOUND:
        Log::error(Log::config, Log::normal, __FILE__, __LINE__, "The config file '%s' for '%s' was not found. We will be skipping this file and using the defaults instead. Try 'locate %s' in a terminal to see where it got to.",
          deviceFile.c_str(), 
          deviceName.c_str(),
          deviceFile.c_str());
        break;
      case SETUP_DEVICE_LOAD_FAILED:
        Log::error(Log::config, Log::normal, __FILE__, __LINE__, "The config file '%s' for '%s' was found but could not be loaded. We will be skipping this load and using the defaults instead. Try 'locate %s' in a terminal to see where it got to.",
          deviceFile.c_str(), 
          deviceName.c_str(),
          deviceFile.c_str());
        break;
      default:
        Log::error(Log::config, Log::normal, __FILE__, __LINE__, "An undefined error ocurred while trying to load '%s'. It had the error code: %d",
            deviceFile.c_str(),
            error);
        break;
    }
  }
}

void DeviceMapping::requiredVirtualDevices(set<string>* vfolders) {
  assert(vfolders != NULL);
  
  for(vector<struct mapping>::const_iterator it = macAddrs.begin(); it != macAddrs.end(); ++it) {
     vfolders->insert(it->vfolder);
  }

  for(vector<struct mapping>::const_iterator it = ipAddrs.begin(); it != ipAddrs.end(); ++it) {
     vfolders->insert(it->vfolder);
  }
}
