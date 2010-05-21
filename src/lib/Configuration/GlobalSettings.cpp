#include <cassert>

#include "GlobalSettings.h"
#include "../SharedConfig.h"

#include "../Common/Common.h"
#include "../Common/Directory.h"

using namespace std;
using namespace fuppes;

void GlobalSettings::InitVariables(void) {
	m_useFixedUUID 	= false;

  // setup temp dir
  if(m_sTempDir.empty()) {
    #ifdef WIN32
    m_sTempDir = getenv("TEMP") + string("\\fuppes\\");
    #else
    char* szTmp = getenv("TEMP");
    if(szTmp != NULL)
      m_sTempDir = string(szTmp) + "/fuppes/";
    else
      m_sTempDir = "/tmp/fuppes/";      
    #endif
  }

  m_sTempDir = Directory::appendTrailingSlash(m_sTempDir);	
  Directory::create(m_sTempDir);
}

std::string GlobalSettings::GetFriendlyName()
{
  CSharedConfig* parent =CSharedConfig::Shared(); 
  if(m_sFriendlyName.empty()) {
    m_sFriendlyName = parent->GetAppName() + " " + parent->GetAppVersion() + " (" + parent->networkSettings->GetHostname() + ")";
  }
  
  return m_sFriendlyName;
}

bool GlobalSettings::Read(void)
{
  assert(pStart != NULL);

  CXMLNode* pTmp;
  for(int i = 0; i < pStart->ChildCount(); ++i) {
    pTmp = pStart->ChildNode(i);

    if(pTmp->Name().compare("temp_dir") == 0) {
      if(pTmp->Value().length() > 0) {
        m_sTempDir = pTmp->Value();
        appendTrailingSlash(&m_sTempDir);
      }
    }
		else if(pTmp->Name().compare("use_fixed_uuid") == 0) {
      m_useFixedUUID = (pTmp->Value().compare("true") == 0);
    }	
    else if(pTmp->Name().compare("trash_dir") == 0) {
      if(pTmp->Value().length() > 0) {
        m_sTrashDir = pTmp->Value();
        appendTrailingSlash(&m_sTrashDir);
      }
    }	
  }  

  return true;
}

