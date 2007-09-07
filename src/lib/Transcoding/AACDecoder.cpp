/***************************************************************************
 *            AACDecoder.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef DISABLE_TRANSCODING
#include "AACDecoder.h"
#ifdef HAVE_FAAD

#include "../SharedLog.h"
#include "../SharedConfig.h"
#include <iostream>

#define MAX_CHANNELS 6 

using namespace std;

CAACDecoder::CAACDecoder()
{
  cout << __FILE__ << " :: constructor" << endl;
}

CAACDecoder::~CAACDecoder()
{
  cout << __FILE__ << " :: destructor" << endl;
}
  
bool CAACDecoder::LoadLib()
{
  cout << __FILE__ << " :: load lib" << endl;
  
  #ifdef WIN32
  std::string sLibName = "faad.dll";
  #else  
  std::string sLibName = "libfaad.so.0";
  #endif
  
  if(!CSharedConfig::Shared()->FaadLibName().empty()) {
    sLibName = CSharedConfig::Shared()->FaadLibName();
  }  
  
  CSharedLog::Shared()->Log(L_EXTENDED, "try opening " + sLibName, __FILE__, __LINE__);
  m_LibHandle = FuppesLoadLibrary(sLibName);  
  
  
  if(!m_LibHandle) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot open library " + sLibName, __FILE__, __LINE__);
		cout << "[WARNING :: AACDecoder] cannot open library " << sLibName << endl;
    return false;
  }

  m_NeAACDecOpen = (NeAACDecOpen_t)FuppesGetProcAddress(m_LibHandle, "NeAACDecOpen");
  if(!m_NeAACDecOpen) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'NeAACDecOpen'", __FILE__, __LINE__);   
    return false;
  }
  
  m_NeAACDecGetCurrentConfiguration = (NeAACDecGetCurrentConfiguration_t)FuppesGetProcAddress(m_LibHandle, "NeAACDecGetCurrentConfiguration");
  if(!m_NeAACDecGetCurrentConfiguration) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'NeAACDecGetCurrentConfiguration'", __FILE__, __LINE__);   
  }
  
  m_NeAACDecSetConfiguration = (NeAACDecSetConfiguration_t)FuppesGetProcAddress(m_LibHandle, "NeAACDecSetConfiguration");
  if(!m_NeAACDecSetConfiguration) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'NeAACDecSetConfiguration'", __FILE__, __LINE__);   
  }
  
  m_NeAACDecInit = (NeAACDecInit_t)FuppesGetProcAddress(m_LibHandle, "NeAACDecInit");
  if(!m_NeAACDecInit) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'NeAACDecInit'", __FILE__, __LINE__);   
    return false;
  }
  
  m_NeAACDecDecode = (NeAACDecDecode_t)FuppesGetProcAddress(m_LibHandle, "NeAACDecDecode");
  if(!m_NeAACDecDecode) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'NeAACDecDecode'", __FILE__, __LINE__);   
    return false;
  }
  
  m_NeAACDecClose = (NeAACDecClose_t)FuppesGetProcAddress(m_LibHandle, "NeAACDecClose");
  if(!m_NeAACDecClose) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'NeAACDecClose'", __FILE__, __LINE__);   
    return false;
  }
    
  return true;
};

bool CAACDecoder::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{
  cout << __FILE__ << " :: open" << endl;
  
  AACDecoder  = m_NeAACDecOpen();
  AACConfig   = m_NeAACDecGetCurrentConfiguration(AACDecoder);
  
  // 5.1 -> stereo
  AACConfig->downMatrix = 1;
  
  m_NeAACDecSetConfiguration(AACDecoder, AACConfig);
  
  if((m_pFileHandle = fopen(p_sFileName.c_str(), "rb")) == NULL) {
    fprintf(stderr, "Cannot open %s\n", p_sFileName.c_str()); 
    return false;
  }
  
	#ifdef _WIN32
  _setmode(_fileno(m_pFileHandle), _O_BINARY);
  #endif
  
  fseek(m_pFileHandle, 0, SEEK_END);
  m_nFileLength = ftell(m_pFileHandle);
  fseek(m_pFileHandle, 0, SEEK_SET);
  
  m_nOffset = 1;
  
  return true;
}

void CAACDecoder::CloseFile()
{
  cout << __FILE__ << " :: close" << endl;
  
  m_NeAACDecClose(AACDecoder);
  free(m_Buffer);
}

long CAACDecoder::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{
  
  //AACFrameInfo;
  
  cout << __FILE__ << " :: decode" << endl;
  
  unsigned long nBufferSize = FAAD_MIN_STREAMSIZE * MAX_CHANNELS;
  
  if(!(m_Buffer = (unsigned char*)malloc(nBufferSize))) {
    fprintf(stderr, "Memory allocation error\n");
    return 0;
  }
  memset(m_Buffer, 0, nBufferSize);
  
  unsigned long  samplerate;
  unsigned char  channels;
    
  void* pResult;
 
  int nBytesRead = fread(m_Buffer, m_nOffset, nBufferSize, m_pFileHandle);
  
     // err = init < 0   
     
  if(m_nOffset == 1) {
      cout << "init: " << 
       m_NeAACDecInit(AACDecoder, m_Buffer, nBufferSize, &samplerate, &channels) << endl;
  }  
    
  //m_nOffset += nBytesRead;
  
    
    pResult = m_NeAACDecDecode(AACDecoder,
                             &AACFrameInfo,
                             m_Buffer,
                             nBufferSize);
                             
    cout << "frm info: samples: " << AACFrameInfo.samples <<
      " bytes: " << AACFrameInfo.bytesconsumed << endl;
 
    m_nOffset += AACFrameInfo.bytesconsumed;
  
  memcpy(p_PcmOut, pResult, AACFrameInfo.bytesconsumed);
  
  *p_nBytesRead = AACFrameInfo.bytesconsumed; //nBytesRead;
        /* void* NeAACDecDecode(NeAACDecHandle hDecoder,
                                 NeAACDecFrameInfo *hInfo,
                                 unsigned char *buffer,
                                 unsigned long buffer_size);*/

  return AACFrameInfo.samples; //AACFrameInfo.bytesconsumed;
}

unsigned int CAACDecoder::NumPcmSamples()
{
  return 0;
}

#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
