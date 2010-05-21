#ifndef __PATHFINDER_H
#define __PATHFINDER_H


#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include <vector>

#define DEVICE_EXT ".cfg"
#define VFOLDER_EXT ".cfg"

#define DEVICE_DIRECTORY string("devices/")
#define VFOLDER_DIRECTORY string("vfolders/")

class PathFinder 
{
  public:
    void SetupDefaultPaths(void);
    std::string DefaultPath(void);

    std::string findInPath(std::string fileName, bool (*exists)(std::string), std::string extraPath = "");
    std::string findDeviceInPath(std::string device, bool (*exists)(std::string));
    std::string findVFolderInPath(std::string device, bool (*exists)(std::string));

    void AddConfigPath(std::string p_sConfigDir);

    void walker(bool (*step)(std::string));
  private:
    std::string devicesPath, vfolderPath; // the extra paths for device files and vfolder files
    std::vector<std::string> m_sConfigPath;
};

#endif
