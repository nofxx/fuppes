#ifndef __CONTENT_DIRECTORY_SETTINGS_H
#define __CONTENT_DIRECTORY_SETTINGS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include "ConfigSettings.h"

class ContentDirectory : public ConfigSettings
{
  public:
    virtual bool Read(void);

    std::string GetLocalCharset() { return m_sLocalCharset; }
    void        SetLocalCharset(std::string p_sLocalCharset);

    /*
    bool UseImageMagick() { return m_pConfigFile->UseImageMagick(); }
    bool UseTaglib()      { return m_pConfigFile->UseTaglib(); }
    bool UseLibAvFormat() { return m_pConfigFile->UseLibAvFormat(); }
    */

  private:
    virtual void InitVariables(void);

    std::string             m_sLocalCharset;
};

#endif
