/***************************************************************************
 *            FaadWrapper.cpp
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
#include "FaadWrapper.h"
#ifdef HAVE_FAAD

#include "../SharedLog.h"
#include "../SharedConfig.h"
#include <iostream>

#define MAX_CHANNELS 6 

using namespace std;

CFaadWrapper::CFaadWrapper()
{
  cout << __FILE__ << " :: constructor" << endl;
}

CFaadWrapper::~CFaadWrapper()
{
  cout << __FILE__ << " :: destructor" << endl;
}
  
bool CFaadWrapper::LoadLib()
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

  m_faacDecOpen = (faacDecOpen_t)FuppesGetProcAddress(m_LibHandle, "faacDecOpen");
  if(!m_faacDecOpen) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'faacDecOpen'", __FILE__, __LINE__);   
    return false;
  }
  
  m_faacDecGetCurrentConfiguration = (faacDecGetCurrentConfiguration_t)FuppesGetProcAddress(m_LibHandle, "faacDecGetCurrentConfiguration");
  if(!m_faacDecGetCurrentConfiguration) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'faacDecGetCurrentConfiguration'", __FILE__, __LINE__);   
  }
  
  m_faacDecSetConfiguration = (faacDecSetConfiguration_t)FuppesGetProcAddress(m_LibHandle, "faacDecSetConfiguration");
  if(!m_faacDecSetConfiguration) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'faacDecSetConfiguration'", __FILE__, __LINE__);   
  }
  
  m_faacDecInit = (faacDecInit_t)FuppesGetProcAddress(m_LibHandle, "faacDecInit");
  if(!m_faacDecInit) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'faacDecInit'", __FILE__, __LINE__);   
    return false;
  }
  
  m_faacDecDecode = (faacDecDecode_t)FuppesGetProcAddress(m_LibHandle, "faacDecDecode");
  if(!m_faacDecDecode) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'faacDecDecode'", __FILE__, __LINE__);   
    return false;
  }
  
  m_faacDecClose = (faacDecClose_t)FuppesGetProcAddress(m_LibHandle, "faacDecClose");
  if(!m_faacDecClose) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'faacDecClose'", __FILE__, __LINE__);   
    return false;
  }
    
  return true;
};

bool CFaadWrapper::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{
  cout << __FILE__ << " :: open : " << p_sFileName << endl;
  
  AACDecoder  = m_faacDecOpen();
  AACConfig   = m_faacDecGetCurrentConfiguration(AACDecoder);
  
  // 5.1 -> stereo
  AACConfig->downMatrix   = true;
  AACConfig->outputFormat = FAAD_FMT_16BIT;
  
  m_faacDecSetConfiguration(AACDecoder, AACConfig);
  
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
  
  m_nBufferSize = FAAD_MIN_STREAMSIZE * MAX_CHANNELS;
  if(!(m_Buffer = (unsigned char*)malloc(m_nBufferSize))) {
    fprintf(stderr, "Memory allocation error\n");
    return false;
  }
  memset(m_Buffer, 0, m_nBufferSize);
  
  fread(m_Buffer, 1, m_nBufferSize, m_pFileHandle);
  
  m_bIsMp4 = false;
  if(m_Buffer[4] == 'f' && m_Buffer[5] == 't' && m_Buffer[6] == 'y' && m_Buffer[7] == 'p') {
    m_bIsMp4 = true;
  }
  
  unsigned long	  samplerate;
  unsigned char		channels;  
  
  m_nBytesConsumed =  m_faacDecInit(AACDecoder, m_Buffer, m_nBufferSize, &samplerate, &channels);
  if(m_nBytesConsumed < 0) {
    cout << "AACDecoder :: error init decoder" << endl;
    return false;
  }
  else if(m_nBytesConsumed > 0) {
    memmove(m_Buffer, &m_Buffer[m_nBytesConsumed], m_nBufferSize - m_nBytesConsumed); 
    fread(&m_Buffer[m_nBufferSize - m_nBytesConsumed], 1, m_nBytesConsumed, m_pFileHandle);
    m_nBytesConsumed = 0;
  }
  
  return true;
}

void CFaadWrapper::CloseFile()
{
  cout << __FILE__ << " :: close" << endl;
  
  fclose(m_pFileHandle);  
  m_faacDecClose(AACDecoder);
  free(m_Buffer);
}



static int write_audio_16bit(void *sample_buffer, unsigned int samples, char* p_PcmOut)
{
  int ret;
  unsigned int i;
  short *sample_buffer16 = (short*)sample_buffer;
  char *data = (char*)malloc(samples * 16 * sizeof(char)/8);

  for (i = 0; i < samples; i++) {
    data[i*2] = (char)(sample_buffer16[i] & 0xFF);
    data[i*2+1] = (char)((sample_buffer16[i] >> 8) & 0xFF);
  }
  //ret = fwrite(data, samples, aufile->bits_per_sample/8, aufile->sndfile);

  memcpy(p_PcmOut, data, samples * 16 * sizeof(char)/8);
  ret = samples * 16 * sizeof(char)/8;
  
  free(data);
  return ret;
}



long CFaadWrapper::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{  
  cout << __FILE__ << " :: decode" << endl;
    
  if(m_nBufferSize > 0) {  
    m_pSampleBuffer = m_faacDecDecode(AACDecoder, &AACFrameInfo, m_Buffer, m_nBufferSize);    
                             
    cout << "frm info: samples: " << AACFrameInfo.samples << " bytes: " << AACFrameInfo.bytesconsumed << endl;
       
    if(AACFrameInfo.error > 0) {
      cout << "error :: " << faacDecGetErrorMessage(AACFrameInfo.error) << endl;
      return -1;
    }
    
    write_audio_16bit(m_pSampleBuffer, AACFrameInfo.samples, p_PcmOut);
    
    
    //memcpy(p_PcmOut, pResult, AACFrameInfo.bytesconsumed);  
    *p_nBytesRead = AACFrameInfo.bytesconsumed; 
    m_nBufferSize -= AACFrameInfo.bytesconsumed;
    
    m_nBytesConsumed = AACFrameInfo.bytesconsumed;
    
    if(m_nBytesConsumed > 0) {      
      memmove(m_Buffer, &m_Buffer[m_nBytesConsumed], m_nBufferSize - m_nBytesConsumed); 
      fread(&m_Buffer[m_nBufferSize - m_nBytesConsumed], 1, m_nBytesConsumed, m_pFileHandle);
      m_nBytesConsumed = 0;
    }
    
    return AACFrameInfo.samples  * 16/8;
  }
  
  return -1;
}

unsigned int CFaadWrapper::NumPcmSamples()
{
  return 0;
}

#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
