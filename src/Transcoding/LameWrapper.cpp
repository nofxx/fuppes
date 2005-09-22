/***************************************************************************
 *            LameWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "LameWrapper.h"
#include "../SharedLog.h"
#include <iostream>
#include <sstream>

const std::string LOGNAME = "LameWrapper";

using namespace std;

CLameWrapper::CLameWrapper()
{
}

bool CLameWrapper::LoadLibrary()
{  
  CSharedLog::Shared()->Log(LOGNAME, "try opening libmp3lame.so");
  m_LibHandle = dlopen("libmp3lame.so", RTLD_LAZY);    
  if(!m_LibHandle)
  {
    stringstream sLog;
    sLog << "cannot open library: " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }    

  m_LameInit = (LameInit_t)dlsym(m_LibHandle, "lame_init");
  if(!m_LameInit)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_init': " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_LameGetVersion = (LameGetVersion_t)dlsym(m_LibHandle, "get_lame_version");
  if(!m_LameGetVersion)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'get_lame_version': " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_LameInitParams = (LameInitParams_t)dlsym(m_LibHandle, "lame_init_params");
  if(!m_LameInitParams)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_init_params': " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_LamePrintConfig = (LamePrintConfig_t)dlsym(m_LibHandle, "lame_print_config");
  if(!m_LamePrintConfig)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_print_config': " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }  
  
  m_LameSetCompressionRatio = (LameSetCompressionRatio_t)dlsym(m_LibHandle, "lame_set_compression_ratio");
  if(!m_LameSetCompressionRatio)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_set_compression_ratio': " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }  
  
  m_LameEncodeBufferInterleaved = (LameEncodeBufferInterleaved_t)dlsym(m_LibHandle, "lame_encode_buffer_interleaved");
  if(!m_LameEncodeBufferInterleaved)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_encode_buffer_interleaved': " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
    
  m_LameEncodeFlush = (LameEncodeFlush_t)dlsym(m_LibHandle, "lame_encode_flush");
  if(!m_LameEncodeFlush)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_encode_flush': " << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
    
  m_LameGlobalFlags = m_LameInit();  
  return true;
}

void CLameWrapper::Init()
{
  m_LameInitParams(m_LameGlobalFlags); 
}

void CLameWrapper::PrintConfig()
{
  cout << "Closing library" << endl;
  dlclose(m_LibHandle);
}

std::string CLameWrapper::GetVersion()
{
  return m_LameGetVersion();
}

void CLameWrapper::SetBitrate(LAME_BITRATE p_nBitrate)
{
  m_LameSetCompressionRatio(m_LameGlobalFlags, p_nBitrate);
}

int CLameWrapper::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples)
{
  return m_LameEncodeBufferInterleaved(m_LameGlobalFlags, p_PcmIn, p_nNumSamples, (char*)m_sMp3Buffer, LAME_MAXMP3BUFFER);
}

int CLameWrapper::Flush()
{
  return lame_encode_flush(m_LameGlobalFlags, m_sMp3Buffer, LAME_MAXMP3BUFFER);
}
