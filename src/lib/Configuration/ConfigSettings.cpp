#include <assert.h>
#include "ConfigSettings.h"


ConfigSettings::ConfigSettings() {
  pStart = NULL;
}

bool ConfigSettings::Init(CXMLNode* devRoot) {
  assert(devRoot != NULL);

  pStart = devRoot;

  // Some others that you do not want in the constructor
  // But maybe the constructor for each object will prove 
  // the best place for them. Take a look at this and if,
  // once the dust settles, there is no reason to put them
  // in the constructor, then place them there instead. 
  // However, be warned that this will mean that the object cannot
  // be reset, just recreated, to get it back into an initial state.
  InitVariables();

  bool result = Read();

  result &= InitPostRead();

  return result;
}
