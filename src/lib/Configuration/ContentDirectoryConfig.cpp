#include <cassert>

#include "ContentDirectoryConfig.h"
#include "../SharedConfig.h"

using namespace std;


void ContentDirectory::InitVariables(void) {
  m_sLocalCharset = "UTF-8";
}

bool ContentDirectory::Read(void)
{
  assert(pStart != NULL);

  CXMLNode* pTmp;
  for(int i = 0; i < pStart->ChildCount(); i++) {
    pTmp = pStart->ChildNode(i);
    if(pTmp->Name().compare("local_charset") == 0) {
      m_sLocalCharset = pTmp->Value();
    }
  }

  return true;
}

void ContentDirectory::SetLocalCharset(std::string p_sLocalCharset)
{
  assert(pStart != NULL);

  CXMLNode* pTmp = pStart->FindNodeByName("local_charset");
  if(pTmp) {
    pTmp->Value(p_sLocalCharset);
    CSharedConfig::Shared()->Save();  
    m_sLocalCharset = p_sLocalCharset;
  } 
}

