#include <cassert>

#include "DatabaseSettings.h"
#include "../SharedConfig.h"

#include "../Common/Common.h"
#include "../Common/Directory.h"
#include "../Common/File.h"

using namespace std;
using namespace fuppes;

void DatabaseSettings::InitVariables(void) {
  m_dbConnectionParams.readonly = false;
}

bool DatabaseSettings::InitPostRead() {
  PathFinder* pf = CSharedConfig::Shared()->pathFinder;

  // set dbfilename if necessary  
  if(dbConnectionParams().type == "sqlite3") {
    if(dbConnectionParams().filename.empty()) setDbFilename(DB_NAME);

    // now append a path to the object
    string temp = pf->findInPath(dbConnectionParams().filename, File::writable);
    if(!temp.empty()) {
      setDbFilename(temp);
    } else {
      temp = pf->findInPath("", Directory::writable); // find a writable directory in the path
      if(!temp.empty()) {
        setDbFilename(temp + DB_NAME);
      }

      if(dbConnectionParams().filename.empty()) return false;
    }
  }
  // TODO there should be something here that says wether or not loading mysql worked

  return true;
}

bool DatabaseSettings::UseDefaultSettings(void) {
  // set it up and get it ready
  m_dbConnectionParams.readonly = false;
  m_dbConnectionParams.type = "sqlite3";
  m_dbConnectionParams.filename = "";

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

