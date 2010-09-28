#include <cassert>

#include "DatabaseSettings.h"
#include "../SharedConfig.h"

#include "../Common/Common.h"
#include "../Common/Directory.h"
#include "../Common/File.h"

using namespace std;
using namespace fuppes;

DatabaseSettings::DatabaseSettings()
{
  m_dbConnectionParams.readonly = false;
  m_dbConnectionParams.filename = "";
  m_dbConnectionParams.type = "sqlite3";
}

DatabaseSettings::~DatabaseSettings()
{
}


bool DatabaseSettings::InitPostRead() {

  // set dbfilename if necessary  
  if(dbConnectionParams().type == "sqlite3" && dbConnectionParams().filename.empty()) {
    #ifdef WIN32
    setDbFilename(string(getenv("APPDATA")) + "\\FUPPES\\fuppes.db");
    #else
    if(Directory::writable(Directory::appendTrailingSlash(FUPPES_LOCALSTATEDIR)))
      setDbFilename(Directory::appendTrailingSlash(FUPPES_LOCALSTATEDIR) + "fuppes.db");
    else
      setDbFilename(string(getenv("HOME")) + "/.fuppes/fuppes.db");
    #endif
  }
   
  return true;
}

bool DatabaseSettings::UseDefaultSettings(void) {
  return InitPostRead();
}

bool DatabaseSettings::Read(void) 
{
  assert(pStart != NULL);

  m_dbConnectionParams.type = ToLower(pStart->Attribute("type"));

  CXMLNode* pTmp;
  for(int i = 0; i < pStart->ChildCount(); i++) {
    pTmp = pStart->ChildNode(i);

    if(pTmp->Name().compare("filename") == 0) {
      m_dbConnectionParams.filename = pTmp->Value();
    }
    
    else if(pTmp->Name().compare("hostname") == 0) {
      m_dbConnectionParams.hostname = pTmp->Value();
    }
    else if(pTmp->Name().compare("username") == 0) {
      m_dbConnectionParams.username = pTmp->Value();
    }
    else if(pTmp->Name().compare("password") == 0) {
      m_dbConnectionParams.password = pTmp->Value();
    }
    else if(pTmp->Name().compare("dbname") == 0) {
      m_dbConnectionParams.dbname = pTmp->Value();
    }
    else if(pTmp->Name().compare("readonly") == 0) {
      m_dbConnectionParams.readonly = (pTmp->Value() == "true");
    }
  }

  return true;
}

