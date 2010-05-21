#ifndef __GLOBALSETTINGS_H
#define __GLOBALSETTINGS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include "ConfigSettings.h"

class GlobalSettings : public ConfigSettings
{
  public:
    virtual bool Read(void);

		void SetTempDir(std::string p_sTempDir) { m_sTempDir = p_sTempDir; }
		std::string GetTempDir() { return m_sTempDir; }
		std::string TrashDir() { return m_sTrashDir; }

    std::string GetFriendlyName(void);
    void        SetFriendlyName(std::string p_sFriendlyName) { m_sFriendlyName = p_sFriendlyName; }

    bool        UseFixedUUID(void) { return m_useFixedUUID; }
  private:
    virtual void InitVariables(void);

    std::string   m_sFriendlyName;
		std::string		m_sTempDir;
		bool					m_useFixedUUID;
		std::string   m_sTrashDir;
};

#endif
