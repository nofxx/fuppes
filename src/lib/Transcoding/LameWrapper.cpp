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
#include "../SharedConfig.h"
#include <iostream>
#include <sstream>

const std::string LOGNAME = "LameWrapper";

using namespace std;

struct LAME_BITRATE_MAPPING_t LAME_BITRATE_MAPPINGS[] = {  
  {320, 3.0},
  {256, 5.0},
  {192, 7.0},
  {160, 9.0},
  {128, 11.0},
  {0, 0}
};

CLameWrapper::CLameWrapper():CAudioEncoderBase()
{
  m_LibHandle = NULL;
  m_nBitRate = 0;
  m_nSampleRate = 0;
  m_nChannels = 2;
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
  std::string sLibName = "lame_enc.dll";
  #else  
  std::string sLibName = "libmp3lame.so.0";
  #endif
  
  if(!CSharedConfig::Shared()->LameLibName().empty()) {
    sLibName = CSharedConfig::Shared()->LameLibName();
  }  
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening " + sLibName);
  m_LibHandle = FuppesLoadLibrary(sLibName);  
  
  
  if(!m_LibHandle) {
    CSharedLog::Shared()->Log(L_EXT, "cannot open library", __FILE__, __LINE__);
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
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'get_lame_version'", __FILE__, __LINE__);   
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
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_print_config'", __FILE__, __LINE__);
  }  
  
  m_LameSetCompressionRatio = (LameSetCompressionRatio_t)FuppesGetProcAddress(m_LibHandle, "lame_set_compression_ratio");
  if(!m_LameSetCompressionRatio) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_set_compression_ratio'", __FILE__, __LINE__);   
  }
  
  m_LameGetCompressionRatio = (LameGetCompressionRatio_t)FuppesGetProcAddress(m_LibHandle, "lame_get_compression_ratio");
  if(!m_LameGetCompressionRatio) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_get_compression_ratio'", __FILE__, __LINE__);   
  }
  
  m_LameSetBrate = (LameSetBrate_t)FuppesGetProcAddress(m_LibHandle, "lame_set_brate");
  if(!m_LameSetBrate) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_set_brate'", __FILE__, __LINE__);   
  }
  
  m_LameGetBrate = (LameGetBrate_t)FuppesGetProcAddress(m_LibHandle, "lame_get_brate");
  if(!m_LameGetBrate) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_get_brate'", __FILE__, __LINE__);   
  }
  
  m_LameSetMode = (LameSetMode_t)FuppesGetProcAddress(m_LibHandle, "lame_set_mode");
  if(!m_LameSetMode) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_set_mode'", __FILE__, __LINE__);   
  } 
  
  m_LameSetQuality = (LameSetQuality_t)FuppesGetProcAddress(m_LibHandle, "lame_set_quality");
  if(!m_LameSetQuality) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_set_quality'", __FILE__, __LINE__);   
  }
  
  m_LameGetQuality = (LameGetQuality_t)FuppesGetProcAddress(m_LibHandle, "lame_get_quality");
  if(!m_LameGetQuality) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_get_quality'", __FILE__, __LINE__);   
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
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'lame_close'", __FILE__, __LINE__);
    return false;
  }
  
  
  // id3
  m_Id3TagInit = (Id3TagInit_t)FuppesGetProcAddress(m_LibHandle, "id3tag_init");
  if(!m_Id3TagInit) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_init'", __FILE__, __LINE__);
    //return false;
  }

  m_Id3TagV1Only = (Id3TagV1Only_t)FuppesGetProcAddress(m_LibHandle, "id3tag_v1_only");
  if(!m_Id3TagV1Only) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_v1_only'", __FILE__, __LINE__);
    //return false;
  }

  m_Id3TagV2Only = (Id3TagV1Only_t)FuppesGetProcAddress(m_LibHandle, "id3tag_v2_only");
  if(!m_Id3TagV2Only) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_v2_only'", __FILE__, __LINE__);
    //return false;
  }  
  
  m_Id3TagAddV2 = (Id3TagAddV2_t)FuppesGetProcAddress(m_LibHandle, "id3tag_add_v2");
  if(!m_Id3TagAddV2) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_add_v2'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagPadV2 = (Id3TagPadV2_t)FuppesGetProcAddress(m_LibHandle, "id3tag_pad_v2");
  if(!m_Id3TagPadV2) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_pad_v2'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetTitle = (Id3TagSetTitle_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_title");
  if(!m_Id3TagSetTitle) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_set_title'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetArtist = (Id3TagSetArtist_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_artist");
  if(!m_Id3TagSetArtist) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_set_artist'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetAlbum = (Id3TagSetAlbum_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_album");
  if(!m_Id3TagSetAlbum) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_set_album'", __FILE__, __LINE__);
    //return false;
  }
  
  m_Id3TagSetTrack = (Id3TagSetAlbum_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_track");
  if(!m_Id3TagSetTrack) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_set_track'", __FILE__, __LINE__);
    //return false;
  }  
  
  m_Id3TagSetGenre = (Id3TagSetGenre_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_genre");
  if(!m_Id3TagSetGenre) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'id3tag_set_genre'", __FILE__, __LINE__);
    //return false;
  }   
  
  m_LameGlobalFlags = m_LameInit();
  return true;
}

void CLameWrapper::Init()
{
  if(m_Id3TagInit) {
    m_Id3TagInit(m_LameGlobalFlags);
  }
  
  bool bCreateId3v1 = false;
  
  if(m_Id3TagSetTitle && !m_pSessionInfo->m_sTitle.empty()) {
    m_Id3TagSetTitle(m_LameGlobalFlags, m_pSessionInfo->m_sTitle.c_str());
    bCreateId3v1 = true;
  }
  
  if(m_Id3TagSetArtist && !m_pSessionInfo->m_sArtist.empty()) {
    m_Id3TagSetArtist(m_LameGlobalFlags, m_pSessionInfo->m_sArtist.c_str());
    bCreateId3v1 = true;
  }

  if(m_Id3TagSetAlbum && !m_pSessionInfo->m_sAlbum.empty()) {
    m_Id3TagSetAlbum(m_LameGlobalFlags, m_pSessionInfo->m_sAlbum.c_str());
    bCreateId3v1 = true;
  }
  
  if(m_Id3TagSetGenre && !m_pSessionInfo->m_sGenre.empty()) {
    m_Id3TagSetGenre(m_LameGlobalFlags, m_pSessionInfo->m_sGenre.c_str());
    bCreateId3v1 = true;
  }
  
  if(m_Id3TagSetTrack && !m_pSessionInfo->m_sOriginalTrackNumber.empty()) {
    m_Id3TagSetTrack(m_LameGlobalFlags, m_pSessionInfo->m_sOriginalTrackNumber.c_str());
    bCreateId3v1 = true;
  }
  
  if(m_Id3TagV2Only) {
    m_Id3TagV2Only(m_LameGlobalFlags);
  } 

  //http://de.wikipedia.org/wiki/Liste_der_ID3-Genres
      
  /*Offset 	Länge 	Bedeutung
        0 	3 	Kennung "TAG" zur Kennzeichnung eines ID3v1-Blocks
        3 	30 	Songtitel
       33 	30 	Künstler/Interpret
       63 	30 	Album
       93 	4 	Erscheinungsjahr
       97 	29 	Beliebiger Kommentar
      126   1   Track no.
      127 	1 	Genre */ 
  
  if(bCreateId3v1) {
    sprintf(szMp3Tail, "TAG");    
    sprintf(&szMp3Tail[3],  "%30s", m_pSessionInfo->m_sTitle.c_str());
    sprintf(&szMp3Tail[33], "%30s", m_pSessionInfo->m_sArtist.c_str());
    sprintf(&szMp3Tail[63], "%30s", m_pSessionInfo->m_sAlbum.c_str());
    sprintf(&szMp3Tail[93], "    ");
    sprintf(&szMp3Tail[97], "%29s", "");
    sprintf(&szMp3Tail[126], "%d", 0);
    sprintf(&szMp3Tail[127], "%d", 0);
  }
  else {    
    const string sFakeMp3Tail = 
      "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
      "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
      "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
      "qqqqqqqqqqqqqqqqqqo=";
    
    string sBinFake = Base64Decode(sFakeMp3Tail);      
    memcpy(szMp3Tail, sBinFake.c_str(), 128);                 
  }
  
  if(m_LameSetMode) {
    m_LameSetMode(m_LameGlobalFlags, STEREO);
  }  
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

void CLameWrapper::SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings)
{
  // bitrate
  if(pTranscodingSettings->AudioBitRate() > 0 && m_LameSetCompressionRatio) {
        
    LAME_BITRATE_MAPPING_t* mapping = LAME_BITRATE_MAPPINGS;
    float fBitrate = 11.0; // 128 kbit
    
    while(mapping->nBitRate > 0) {      
      if(mapping->nBitRate == pTranscodingSettings->AudioBitRate()) {
        fBitrate = mapping->fLameRate;
        break;
      }      
      mapping++;
    }
    
    if(m_LameSetCompressionRatio) {
      if(m_LameSetCompressionRatio(m_LameGlobalFlags, fBitrate) == 0)
        m_nBitRate = pTranscodingSettings->AudioBitRate();
    }
  }
  
  if(m_nBitRate == 0) {
    m_nBitRate = 128;
  }  
  
  // samplerate
  if(pTranscodingSettings->AudioSampleRate() > 0) {
    m_nSampleRate = pTranscodingSettings->AudioSampleRate();
  }
  else {
    m_nSampleRate = 44100;
  }
    
  // quality
  if((pTranscodingSettings->LameQuality() != -1) && m_LameSetQuality) {
    m_LameSetQuality(m_LameGlobalFlags, pTranscodingSettings->LameQuality());
  }
  
  
  m_nChannels = 2;
}

/*void CLameWrapper::SetCompressionRatio(LAME_BITRATE p_nCompressionRatio)
{
  if(m_LameSetCompressionRatio) {
    m_LameSetCompressionRatio(m_LameGlobalFlags, p_nCompressionRatio);
  }
}*/


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
  float bitrate    = m_nBitRate * 1000.0; //
  float samplerate = m_nSampleRate * 1.0; //.0;
 
  
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

void CLameWrapper::GetMp3Tail(char* p_szBuffer)
{
  memcpy(p_szBuffer, szMp3Tail, 128);
}

#endif // HAVE_LAME
#endif // DISABLE_TRANSCODING
