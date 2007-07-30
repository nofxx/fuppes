/***************************************************************************
 *            LameWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "LameWrapper.h"
#ifdef HAVE_LAME

#include "../SharedLog.h"
#include <iostream>
#include <sstream>

const std::string LOGNAME = "LameWrapper";

using namespace std;

CLameWrapper::CLameWrapper()
{
  m_LibHandle = NULL;
}

CLameWrapper::~CLameWrapper()
{
  if(m_LibHandle) {
    if(m_LameClose)
      m_LameClose(m_LameGlobalFlags);
    FuppesCloseLibrary(m_LibHandle);
  }
}

bool CLameWrapper::LoadLib()
{ 
	#ifdef WIN32
  string sLibName[1];
	sLibName[0] = "lame_enc.dll";
	#else
	string sLibName[3];
	sLibName[0] = "libmp3lame.so.0";
	sLibName[1] = "libmp3lame.so";
	sLibName[2] = "libmp3lame.dylib";
	#endif

  /*string* sLib = sLibName;
	cout << sizeof(sLibName) << endl;
	fuppesSleep(2000);
	while(sLib) {
	  cout << *sLib << endl;
	  sLib++;
		fuppesSleep(2000);
	}*/

  //cout << "LAME" << endl;

  #ifdef WIN32
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening lame_enc.dll");
  m_LibHandle = FuppesLoadLibrary("lame_enc.dll");
  #else
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening libmp3lame.so");
  m_LibHandle = FuppesLoadLibrary("libmp3lame.so.0");
  if(!m_LibHandle)
    m_LibHandle = FuppesLoadLibrary("/opt/local/lib/libmp3lame.dylib");
  #endif
  
  if(!m_LibHandle) {
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot open library", __FILE__, __LINE__);
		cout << "[WARNING :: LameWrapper] cannot open library" << endl;
    return false;
  }   
   
  
  // lame_init()
  m_LameInit = (LameInit_t)FuppesGetProcAddress(m_LibHandle, "lame_init");
  if(!m_LameInit) {
    stringstream sLog;
    sLog << "cannot load symbol 'lame_init'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }

  // get_lame_version()
  m_LameGetVersion = (LameGetVersion_t)FuppesGetProcAddress(m_LibHandle, "get_lame_version");
  if(!m_LameGetVersion) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'get_lame_version'", __FILE__, __LINE__);   
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
  if(!m_LamePrintConfig) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'lame_print_config'", __FILE__, __LINE__);
  }  
  
  m_LameSetCompressionRatio = (LameSetCompressionRatio_t)FuppesGetProcAddress(m_LibHandle, "lame_set_compression_ratio");
  if(!m_LameSetCompressionRatio) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'lame_set_compression_ratio'", __FILE__, __LINE__);   
  }  
  
  m_LameSetMode = (LameSetMode_t)FuppesGetProcAddress(m_LibHandle, "lame_set_mode");
  if(!m_LameSetMode) {
    CSharedLog::Shared()->Log(L_EXTENDED_WARN, "cannot load symbol 'lame_set_mode'", __FILE__, __LINE__);   
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
    
  // lame_close()
  m_LameClose = (LameClose_t)FuppesGetProcAddress(m_LibHandle, "lame_close");
  if(!m_LameClose) {     
    CSharedLog::Shared()->Log(L_EXTENDED_ERR, "cannot load symbol 'lame_close'", __FILE__, __LINE__);
    return false;
  }
  
  
  // id3
  m_Id3TagInit = (Id3TagInit_t)FuppesGetProcAddress(m_LibHandle, "id3tag_init");
  if(!m_Id3TagInit) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_init'", __FILE__, __LINE__);
    //return false;
  }

  m_Id3TagV1Only = (Id3TagV1Only_t)FuppesGetProcAddress(m_LibHandle, "id3tag_v1_only");
  if(!m_Id3TagV1Only) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_v1_only'", __FILE__, __LINE__);
    //return false;
  }

  m_Id3TagV2Only = (Id3TagV1Only_t)FuppesGetProcAddress(m_LibHandle, "id3tag_v2_only");
  if(!m_Id3TagV2Only) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_v2_only'", __FILE__, __LINE__);
    //return false;
  }  
  
  m_Id3TagAddV2 = (Id3TagAddV2_t)FuppesGetProcAddress(m_LibHandle, "id3tag_add_v2");
  if(!m_Id3TagAddV2) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_add_v2'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagPadV2 = (Id3TagPadV2_t)FuppesGetProcAddress(m_LibHandle, "id3tag_pad_v2");
  if(!m_Id3TagPadV2) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_pad_v2'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetTitle = (Id3TagSetTitle_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_title");
  if(!m_Id3TagSetTitle) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_set_title'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetArtist = (Id3TagSetArtist_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_artist");
  if(!m_Id3TagSetArtist) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_set_artist'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetAlbum = (Id3TagSetAlbum_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_album");
  if(!m_Id3TagSetAlbum) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_set_album'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetTrack = (Id3TagSetAlbum_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_track");
  if(!m_Id3TagSetTrack) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_set_track'", __FILE__, __LINE__);
    //return false;
  }  
  
  m_Id3TagSetGenre = (Id3TagSetGenre_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_genre");
  if(!m_Id3TagSetGenre) {
    CSharedLog::Shared()->Log(L_WARNING, "cannot load symbol 'id3tag_set_genre'", __FILE__, __LINE__);
    //return false;
  }   
  
  m_LameGlobalFlags = m_LameInit();
  return true;
}

void CLameWrapper::Init()
{
  m_Id3TagInit(m_LameGlobalFlags);  

  if(!m_pSessionInfo->m_sTitle.empty()) {
    m_Id3TagSetTitle(m_LameGlobalFlags, m_pSessionInfo->m_sTitle.c_str());
  }
  
  if(!m_pSessionInfo->m_sArtist.empty()) {
    m_Id3TagSetArtist(m_LameGlobalFlags, m_pSessionInfo->m_sArtist.c_str());
  }

  if(!m_pSessionInfo->m_sAlbum.empty()) {
    m_Id3TagSetAlbum(m_LameGlobalFlags, m_pSessionInfo->m_sAlbum.c_str());
  }
  
  if(!m_pSessionInfo->m_sGenre.empty()) {
    m_Id3TagSetGenre(m_LameGlobalFlags, m_pSessionInfo->m_sGenre.c_str());
  }
  
  if(!m_pSessionInfo->m_sOriginalTrackNumber.empty()) {
    m_Id3TagSetTrack(m_LameGlobalFlags, m_pSessionInfo->m_sOriginalTrackNumber.c_str());
  }
  
  m_Id3TagV2Only(m_LameGlobalFlags);
  //m_Id3TagAddV2(m_LameGlobalFlags);
  //m_Id3TagPadV2(m_LameGlobalFlags); 
  
  
  m_LameSetMode(m_LameGlobalFlags, STEREO);
  
  m_LameInitParams(m_LameGlobalFlags);  
}

void CLameWrapper::PrintConfig()
{
  if(m_LamePrintConfig)    
    m_LamePrintConfig(m_LameGlobalFlags);
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

int CLameWrapper::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead)
{
  return m_LameEncodeBufferInterleaved(m_LameGlobalFlags, p_PcmIn, p_nNumSamples, (unsigned char*)m_sMp3Buffer, LAME_MAXMP3BUFFER);
}

int CLameWrapper::Flush()
{  
  return m_LameEncodeFlush(m_LameGlobalFlags, m_sMp3Buffer, LAME_MAXMP3BUFFER);  
}

unsigned int CLameWrapper::GuessContentLength(unsigned int p_nNumPcmSamples)
{
  //#warning todo
  float bitrate = 128000.0;
  float samplerate = 44100.0;

 /* float duration = p_nNumPcmSamples / samplerate;
  cout << "duration: " << duration << " s" << endl;
  
  float size = bitrate * duration;
  cout << "size: " << size << " bits - ";
  size /= 8;
  cout << size << " bytes" << endl;
  
  cout << "size + const: " << size + 1218 << endl;*/
  
  //cout << "lame: " << p_nNumPcmSamples << " :: " << (unsigned int)p_nNumPcmSamples * (bitrate/8.0)/samplerate + 4 * 1152 * (bitrate/8.0)/samplerate + 512 << endl;
  
  return (unsigned int)p_nNumPcmSamples * (bitrate/8.0)/samplerate + 4 * 1152 * (bitrate/8.0)/samplerate + 512;
}

#endif // HAVE_LAME
#endif // DISABLE_TRANSCODING
