/***************************************************************************
 *            LameWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef DISABLE_TRANSCODING
 
#include "LameWrapper.h"
#include "../SharedLog.h"
#include <iostream>
#include <sstream>

const std::string LOGNAME = "LameWrapper";

using namespace std;

CLameWrapper::CLameWrapper()
{  
}

CLameWrapper::~CLameWrapper()
{
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
}

bool CLameWrapper::LoadLib()
{ 
  #ifdef WIN32
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening lame_enc.dll");
  m_LibHandle = FuppesLoadLibrary("lame_enc.dll");
  #else
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening libmp3lame.so");
  m_LibHandle = FuppesLoadLibrary("libmp3lame.so.0");
  if(!m_LibHandle)
    m_LibHandle = FuppesLoadLibrary("libmp3lame.so");
  #endif
  if(!m_LibHandle)
  {
    stringstream sLog;
    sLog << "cannot open library";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }   
   
    
  m_LameInit = (LameInit_t)FuppesGetProcAddress(m_LibHandle, "lame_init");
  if(!m_LameInit)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_init'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }

  
  m_LameGetVersion = (LameGetVersion_t)FuppesGetProcAddress(m_LibHandle, "get_lame_version");
  if(!m_LameGetVersion)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'get_lame_version'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    //return false;
  }
  
  m_LameInitParams = (LameInitParams_t)FuppesGetProcAddress(m_LibHandle, "lame_init_params");
  if(!m_LameInitParams)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_init_params'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_LamePrintConfig = (LamePrintConfig_t)FuppesGetProcAddress(m_LibHandle, "lame_print_config");
  if(!m_LamePrintConfig)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_print_config'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    //return false;
  }  
  
  m_LameSetCompressionRatio = (LameSetCompressionRatio_t)FuppesGetProcAddress(m_LibHandle, "lame_set_compression_ratio");
  if(!m_LameSetCompressionRatio)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_set_compression_ratio'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    //return false;
  }  
  
  m_LameEncodeBufferInterleaved = (LameEncodeBufferInterleaved_t)FuppesGetProcAddress(m_LibHandle, "lame_encode_buffer_interleaved");
  if(!m_LameEncodeBufferInterleaved)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_encode_buffer_interleaved'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
    
  m_LameEncodeFlush = (LameEncodeFlush_t)FuppesGetProcAddress(m_LibHandle, "lame_encode_flush");
  if(!m_LameEncodeFlush)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_encode_flush'";
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
  FuppesCloseLibrary(m_LibHandle);
}

std::string CLameWrapper::GetVersion()
{
  if (m_LameGetVersion)
    return m_LameGetVersion();
  else
    return "unknown";
}

void CLameWrapper::SetBitrate(LAME_BITRATE p_nBitrate)
{
  if(m_LameSetCompressionRatio)
    m_LameSetCompressionRatio(m_LameGlobalFlags, p_nBitrate);
}

int CLameWrapper::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples)
{
  return m_LameEncodeBufferInterleaved(m_LameGlobalFlags, p_PcmIn, p_nNumSamples, (unsigned char*)m_sMp3Buffer, LAME_MAXMP3BUFFER);
}

int CLameWrapper::Flush()
{
  return m_LameEncodeFlush(m_LameGlobalFlags, m_sMp3Buffer, LAME_MAXMP3BUFFER);
}

#endif /* DISABLE_TRANSCODING */
