#include <cassert>
#include "PathFinder.h"
#include "../Common/Common.h"
#include "../Common/Directory.h"

using namespace std;
using namespace fuppes;

void PathFinder::SetupDefaultPaths(void) {
  m_sConfigPath.clear();

  #ifdef WIN32
  m_sConfigPath.push_back(string(getenv("APPDATA")) + "\\FUPPES\\");
  #else
  // .fuppes has higher priority than /etc/
  m_sConfigPath.push_back(string(getenv("HOME")) + "/.fuppes/");
  m_sConfigPath.push_back(Directory::appendTrailingSlash(FUPPES_SYSCONFDIR));
  #endif

  devicesPath = DEVICE_DIRECTORY;
  vfolderPath = VFOLDER_DIRECTORY;
}

string PathFinder::DefaultPath(void) {
  assert(m_sConfigPath.size() > 0);

  return m_sConfigPath.front();
}

void PathFinder::AddConfigPath(std::string p_sConfigDir) { 
  if(!p_sConfigDir.empty()) m_sConfigPath.insert(m_sConfigPath.begin(), p_sConfigDir); 
}

string PathFinder::findInPath(string fileName, bool (*exists)(string), string extraPath) {
  assert(exists != NULL);

  bool found = false;
  string tempName = "";
  for(vector<string>::const_iterator it = m_sConfigPath.begin(); !found && it != m_sConfigPath.end(); ++it) {
    tempName = *it  + extraPath + fileName;
    if((*exists)(tempName)) {
      found = true;
      break;
    }
  }

  if (found) return tempName;
  return string("");
}

string PathFinder::findDeviceInPath(string device, bool (*exists)(string)) {
  assert(exists != NULL);
  return findInPath(device + DEVICE_EXT, exists, appendTrailingSlash(devicesPath));
}

string PathFinder::findVFolderInPath(string device, bool (*exists)(string)) {
  assert(exists != NULL);
  return findInPath(device + VFOLDER_EXT, exists, appendTrailingSlash(vfolderPath));
}

void PathFinder::walker(bool (*step)(string)) {
  Directory::walk(&m_sConfigPath, step);
}
