#ifndef __SHARED_OBJECTS_H
#define __SHARED_OBJECTS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <vector>
#include <string>
#include "ConfigSettings.h"

class SharedObjects : public ConfigSettings
{
  public:
    virtual bool Read(void);

    //  shared dir
    int SharedDirCount();
    std::string GetSharedDir(int p_nIdx);  
    void AddSharedDirectory(std::string p_sDirectory);
    void RemoveSharedDirectory(int p_nIdx);
    
    //  shared iTunes
    int SharedITunesCount() { return m_lSharedITunes.size(); }
    std::string GetSharedITunes(int p_nIdx) { return m_lSharedITunes[p_nIdx]; }
    void AddSharedITunes(std::string p_sITunes);
    void RemoveSharedITunes(int p_nIdx);

  private:
    std::vector<std::string>  m_lSharedDirs;
    std::vector<std::string>  m_lSharedITunes;
};

#endif
