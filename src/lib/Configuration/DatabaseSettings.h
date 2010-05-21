#ifndef __DATABASE_SETTINGS_H
#define __DATABASE_SETTINGS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>

#include "ConfigSettings.h"
#include "../Plugins/Plugin.h"

class DatabaseSettings : public ConfigSettings
{
  public:
    virtual bool Read(void);

    CConnectionParams dbConnectionParams() { return m_dbConnectionParams; }
    void setDbFilename(std::string filename) { m_dbConnectionParams.filename = filename; }

    bool UseDefaultSettings(void);

  private:
    virtual void InitVariables(void);
    virtual bool InitPostRead(void);

    CConnectionParams m_dbConnectionParams;
};

#endif
