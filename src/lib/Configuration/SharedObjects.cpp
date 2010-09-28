#include <cassert>

#include "SharedObjects.h"
#include "../SharedConfig.h"

#include "../Common/Common.h"

using namespace std;

// shared dirs
bool SharedObjects::Read(void)
{
  assert(pStart != NULL);

  //cout << "SharedObjects read: " << pStart->name() << endl;
  
  // You may call read more than once
  m_lSharedDirs.clear();    
  m_lSharedITunes.clear();


  for(m_sharedObjectsIter = m_sharedObjects.begin();
      m_sharedObjectsIter != m_sharedObjects.end();
      m_sharedObjectsIter++) {
    delete *m_sharedObjectsIter;
  }
  m_sharedObjects.clear();
  
  
  // now actually perform the read
  SharedObject* obj;
  CXMLNode* pTmp;
	for(int i = 0; i < pStart->ChildCount(); ++i) {
    pTmp = pStart->ChildNode(i);
    
		if(pTmp->Name().compare("dir") == 0) {
			m_lSharedDirs.push_back(pTmp->Value());

      obj = new SharedObject();
      obj->m_type = SharedObject::directory;
      obj->m_path = pTmp->Value();
      m_sharedObjects.push_back(obj);
		}
		else if(pTmp->Name().compare("itunes") == 0) {
			m_lSharedITunes.push_back(pTmp->Value());

      obj = new SharedObject();
      obj->m_type = SharedObject::itunes;
      obj->m_path = pTmp->Value();
      m_sharedObjects.push_back(obj);
		}
    
	}

  return true;
}

int SharedObjects::SharedDirCount()
{
  return m_lSharedDirs.size();
}

std::string SharedObjects::GetSharedDir(int p_nIdx)
{
  string sDir = m_lSharedDirs[p_nIdx];
  if(sDir.length() > 1 && sDir.substr(sDir.length() - 1).compare(upnpPathDelim) != 0) {
    sDir += upnpPathDelim;
  }
  return sDir.c_str();
}

void SharedObjects::AddSharedDirectory(std::string p_sDirName)
{
  assert(pStart != NULL);
  
  p_sDirName = ToUTF8(p_sDirName);

  pStart->AddChild("dir", p_sDirName);
  Read();
  CSharedConfig::Shared()->Save();
}


void SharedObjects::RemoveSharedDirectory(int p_nIdx)
{
  assert(pStart != NULL);

  int nIdx = 0;
  CXMLNode* pTmp;
  for(int i = 0; i < pStart->ChildCount(); ++i) {
    pTmp = pStart->ChildNode(i);
    if(pTmp->Name().compare("dir") == 0) {
      if(nIdx == p_nIdx) {
        pStart->RemoveChild(i);
        Read();
        CSharedConfig::Shared()->Save();
        break;
      }      
      ++nIdx;
    }    
  }
}

// shared iTunes
void SharedObjects::AddSharedITunes(std::string p_sITunes)
{
  assert(pStart != NULL);

  p_sITunes = ToUTF8(p_sITunes);
  pStart->AddChild("itunes", p_sITunes); 
  Read();
  CSharedConfig::Shared()->Save();
}

void SharedObjects::RemoveSharedITunes(int p_nIdx)
{
  assert(pStart != NULL);

  int nIdx = 0;
  
  CXMLNode* pTmp;
  for(int i = 0; i < pStart->ChildCount(); ++i) {
    pTmp = pStart->ChildNode(i);
    if(pTmp->Name().compare("itunes") == 0) {
      if(nIdx == p_nIdx) {
        pStart->RemoveChild(i);
        Read();
        CSharedConfig::Shared()->Save();
        break;
      }      
      ++nIdx;
    }    
  }  
}


void SharedObjects::removeSharedObject(int idx)
{
}

