#include <cassert>

#include "TranscodingSettings.h"

using namespace std;

bool TranscodingSettings::Read(void)
{
  assert(pStart != NULL);

  CXMLNode* pTmp;
  for(int i = 0; i < pStart->ChildCount(); i++) {    
    pTmp = pStart->ChildNode(i);

    if(pTmp->Name().compare("lame_libname") == 0) {
      m_sLameLibName = pTmp->Value();
    }
    else if(pTmp->Name().compare("twolame_libname") == 0) {
      m_sTwoLameLibName = pTmp->Value();
    }
    else if(pTmp->Name().compare("vorbis_libname") == 0) {
      m_sVorbisLibName = pTmp->Value();
    }
    else if(pTmp->Name().compare("flac_libname") == 0) {
      m_sFlacLibName = pTmp->Value();
    }
    else if(pTmp->Name().compare("mpc_libname") == 0) {
      m_sMpcLibName = pTmp->Value();
    }
    else if(pTmp->Name().compare("faad_libname") == 0) {
      m_sFaadLibName = pTmp->Value();	  
    }
    else if(pTmp->Name().compare("mp4ff_libname") == 0) {
      m_sMp4ffLibName = pTmp->Value();	  
    }	  
  }

  return true;
}

