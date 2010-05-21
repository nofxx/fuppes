#ifndef __CONFIG_SETTINGS__
#define __CONFIG_SETTINGS__


#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../Common/XMLParser.h"

class ConfigSettings
{
  public:
    ConfigSettings();
    virtual bool Init(CXMLNode* devRoot);

    // if you call this function more than once it should act the same 
    // way every time. That probably means clearing lists out.
    virtual bool Read(void) = 0;
  protected:
    CXMLNode* pStart;             // for the other function calls

    // This function is the one that you rewrite when you want to 
    // set some variables at the start but you don't want to overwrite
    // Init()
    virtual void InitVariables(void) {}
    virtual bool InitPostRead(void) { return true; }
};

#endif
