/***************************************************************************
 *            TwoLameEncoder.cpp
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
#include "TwoLameEncoder.h"
#ifdef HAVE_TWOLAME
 
#include "../SharedLog.h"
#include "../SharedConfig.h"
#include <iostream>
#include <sstream>

using namespace std;

CTwoLameEncoder::CTwoLameEncoder()
{  
}

CTwoLameEncoder::~CTwoLameEncoder()
{
  if(m_LibHandle) {
    if(m_TwoLameClose)
      m_TwoLameClose(&m_TwoLameOptions);
    FuppesCloseLibrary(m_LibHandle);
  }
}

bool CTwoLameEncoder::LoadLib()
{ 
  #ifdef WIN32
  std::string sLibName = "twolame.dll";
  #else  
  std::string sLibName = "libtwolame.so";
  #endif
  
  if(!CSharedConfig::Shared()->TwoLameLibName().empty()) {
    sLibName = CSharedConfig::Shared()->TwoLameLibName();
  }  
  
  CSharedLog::Shared()->Log(L_EXT, "try opening " + sLibName, __FILE__, __LINE__);
  m_LibHandle = FuppesLoadLibrary(sLibName);   
  
  
  if(!m_LibHandle) {
    CSharedLog::Shared()->Log(L_EXT, "cannot open library", __FILE__, __LINE__);
    return false;
  }
   
    
  // twolame_init()
  m_TwoLameInit = (TwoLameInit_t)FuppesGetProcAddress(m_LibHandle, "twolame_init");
  if(!m_TwoLameInit) {        
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_init'", __FILE__, __LINE__);
    return false;
  }
  
  // twolame_set_num_channels()
  m_TwoLameSetNumChannels = (TwoLameSetNumChannels_t)FuppesGetProcAddress(m_LibHandle, "twolame_set_num_channels");
  if(!m_TwoLameSetNumChannels) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_set_num_channels'", __FILE__, __LINE__);    
    return false;
  } 
  
  // twolame_set_out_samplerate()
  m_TwoLameSetOutSamplerate = (TwoLameSetOutSamplerate_t)FuppesGetProcAddress(m_LibHandle, "twolame_set_out_samplerate");
  if(!m_TwoLameSetOutSamplerate) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_set_out_samplerate'", __FILE__, __LINE__);    
    return false;
  } 
  
    // twolame_set_in_samplerate()
  m_TwoLameSetInSamplerate = (TwoLameSetInSamplerate_t)FuppesGetProcAddress(m_LibHandle, "twolame_set_in_samplerate");
  if(!m_TwoLameSetInSamplerate) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_set_in_samplerate'", __FILE__, __LINE__);    
    return false;
  } 
  
  // twolame_set_bitrate()
  m_TwoLameSetBitrate = (TwoLameSetBitrate_t)FuppesGetProcAddress(m_LibHandle, "twolame_set_bitrate");
  if(!m_TwoLameSetBitrate) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_set_bitrate'", __FILE__, __LINE__);    
    return false;
  }

  
  // twolame_init_params()
  m_TwoLameInitParams = (TwoLameInitParams_t)FuppesGetProcAddress(m_LibHandle, "twolame_init_params");
  if(!m_TwoLameInitParams) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_init_params'", __FILE__, __LINE__);    
    return false;
  } 
  
  // twolame_encode_buffer_interleaved()
  m_TwoLameEncodeBufferInterleaved = (TwoLameEncodeBufferInterleaved_t)FuppesGetProcAddress(m_LibHandle, "twolame_encode_buffer_interleaved");
  if(!m_TwoLameEncodeBufferInterleaved) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_encode_buffer_interleaved'", __FILE__, __LINE__);    
    return false;
  } 
    
  // twolame_encode_flush()
  m_TwoLameEncodeFlush = (TwoLameEncodeFlush_t)FuppesGetProcAddress(m_LibHandle, "twolame_encode_flush");
  if(!m_TwoLameEncodeFlush) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_encode_flush'", __FILE__, __LINE__);    
    return false;
  } 
  
  // twolame_close()
  m_TwoLameClose = (TwoLameClose_t)FuppesGetProcAddress(m_LibHandle, "twolame_close");
  if(!m_TwoLameClose) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_close'", __FILE__, __LINE__);    
    return false;
  } 
  
  
  /*m_TwoLameSetCompressionRatio = (TwoLameSetCompressionRatio_t)FuppesGetProcAddress(m_LibHandle, "lame_set_compression_ratio");
  if(!m_LameSetCompressionRatio)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_set_compression_ratio'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    //return false;
  } */
  
  
  // twolame_get_version()
  m_TwoLameGetVersionName = (TwoLameGetVersionName_t)FuppesGetProcAddress(m_LibHandle, "twolame_get_version_name");
  if(!m_TwoLameGetVersionName) {        
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_get_version_name'", __FILE__, __LINE__);    
  }
  
  // twolame_print_config()
  m_TwoLamePrintConfig = (TwoLamePrintConfig_t)FuppesGetProcAddress(m_LibHandle, "twolame_print_config");
  if(!m_TwoLamePrintConfig) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'twolame_print_config'", __FILE__, __LINE__);    
  }
  
    
  m_TwoLameOptions = m_TwoLameInit();
  return true;
}

void CTwoLameEncoder::Init()
{
  cout << "CTwoLameEncoder::Init()" << endl;
  
  cout << "twolame_set_num_channels()" << endl;
  m_TwoLameSetNumChannels(m_TwoLameOptions, 2);
  
  m_TwoLameSetInSamplerate(m_TwoLameOptions, 44100); //44100);
  m_TwoLameSetOutSamplerate(m_TwoLameOptions, 44100); //48000);
  
  //m_TwoLameSetBitrate(m_TwoLameOptions, 160);
 
  cout << "twolame_init_params()" << endl;
  m_TwoLameInitParams(m_TwoLameOptions); 
}

void CTwoLameEncoder::PrintConfig()
{
  if(m_TwoLamePrintConfig)    
    m_TwoLamePrintConfig(m_TwoLameOptions);
}

std::string CTwoLameEncoder::GetVersion()
{
  if (m_TwoLameGetVersionName)
    return m_TwoLameGetVersionName();
  else
    return "unknown";
}

/*void CTwoLameEncoder::SetBitrate(LAME_BITRATE p_nBitrate)
{
  cout << "CLameWrapper::SetBitrate()" << endl;
  if(m_TwoLameSetCompressionRatio)
    m_TwoLameSetCompressionRatio(m_TwoLameOptions, p_nBitrate);
} */

void CTwoLameEncoder::SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings)
{
  #warning todo
}

int CTwoLameEncoder::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead)
{
  return m_TwoLameEncodeBufferInterleaved(m_TwoLameOptions, p_PcmIn, p_nNumSamples, (unsigned char*)m_sMp3Buffer, TWOLAME_MAX_BUFFER);
}

int CTwoLameEncoder::Flush()
{
  return m_TwoLameEncodeFlush(m_TwoLameOptions, m_sMp3Buffer, TWOLAME_MAX_BUFFER);
}

#endif // HAVE_TWOLAME
#endif // DISABLE_TRANSCODING
