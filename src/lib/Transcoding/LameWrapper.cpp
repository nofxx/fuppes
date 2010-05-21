/***************************************************************************
 *            LameWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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
#include <string.h>


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
	m_LameGlobalFlags = NULL;
}

CLameWrapper::~CLameWrapper()
{
  if(m_LibHandle) {
    if(m_LameClose && m_LameGlobalFlags) {
      m_LameClose(m_LameGlobalFlags);
		}
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
  
  if(!CSharedConfig::Shared()->transcodingSettings->LameLibName().empty()) {
    sLibName = CSharedConfig::Shared()->transcodingSettings->LameLibName();
  }  
  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "try opening %s", sLibName.c_str());
  m_LibHandle = FuppesLoadLibrary(sLibName);  
  
  
  if(!m_LibHandle) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot open library");
    return false;
  }
   
  
  // lame_init()
  m_LameInit = (LameInit_t)FuppesGetProcAddress(m_LibHandle, "lame_init");
  if(!m_LameInit) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_init'"); 
    return false;
  }

  // get_lame_version()
  m_LameGetVersion = (LameGetVersion_t)FuppesGetProcAddress(m_LibHandle, "get_lame_version");
  if(!m_LameGetVersion) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'get_lame_version'");
  }
  
  m_LameInitParams = (LameInitParams_t)FuppesGetProcAddress(m_LibHandle, "lame_init_params");
  if(!m_LameInitParams) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_init_params'");
    return false;
  }
  
  m_LamePrintConfig = (LamePrintConfig_t)FuppesGetProcAddress(m_LibHandle, "lame_print_config");
  if(!m_LamePrintConfig) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_print_config'");
  }  
  
  m_LameSetCompressionRatio = (LameSetCompressionRatio_t)FuppesGetProcAddress(m_LibHandle, "lame_set_compression_ratio");
  if(!m_LameSetCompressionRatio) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_set_compression_ratio'");
  }
  
  m_LameGetCompressionRatio = (LameGetCompressionRatio_t)FuppesGetProcAddress(m_LibHandle, "lame_get_compression_ratio");
  if(!m_LameGetCompressionRatio) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_get_compression_ratio'");
  }
  
  m_LameSetBrate = (LameSetBrate_t)FuppesGetProcAddress(m_LibHandle, "lame_set_brate");
  if(!m_LameSetBrate) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_set_brate'");
  }
  
  m_LameGetBrate = (LameGetBrate_t)FuppesGetProcAddress(m_LibHandle, "lame_get_brate");
  if(!m_LameGetBrate) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_get_brate'");
  }
  
  m_LameSetMode = (LameSetMode_t)FuppesGetProcAddress(m_LibHandle, "lame_set_mode");
  if(!m_LameSetMode) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_set_mode'");
  } 
  
  m_LameSetQuality = (LameSetQuality_t)FuppesGetProcAddress(m_LibHandle, "lame_set_quality");
  if(!m_LameSetQuality) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_set_quality'");
  }
  
  m_LameGetQuality = (LameGetQuality_t)FuppesGetProcAddress(m_LibHandle, "lame_get_quality");
  if(!m_LameGetQuality) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_get_quality'");
  }
  
  m_LameEncodeBufferInterleaved = (LameEncodeBufferInterleaved_t)FuppesGetProcAddress(m_LibHandle, "lame_encode_buffer_interleaved");
  if(!m_LameEncodeBufferInterleaved) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_encode_buffer_interleaved'");
    return false;
  } 
    
  m_LameEncodeFlush = (LameEncodeFlush_t)FuppesGetProcAddress(m_LibHandle, "lame_encode_flush");
  if(!m_LameEncodeFlush) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_encode_flush'");
    return false;
  } 
    
  // lame_close()
  m_LameClose = (LameClose_t)FuppesGetProcAddress(m_LibHandle, "lame_close");
  if(!m_LameClose) {     
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'lame_close'");
    return false;
  }
  
  
  // id3
  m_Id3TagInit = (Id3TagInit_t)FuppesGetProcAddress(m_LibHandle, "id3tag_init");
  if(!m_Id3TagInit) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_init'");
  }

  m_Id3TagV1Only = (Id3TagV1Only_t)FuppesGetProcAddress(m_LibHandle, "id3tag_v1_only");
  if(!m_Id3TagV1Only) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_v1_only'");
  }

  m_Id3TagV2Only = (Id3TagV1Only_t)FuppesGetProcAddress(m_LibHandle, "id3tag_v2_only");
  if(!m_Id3TagV2Only) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_v2_only'");
  }  
  
  m_Id3TagAddV2 = (Id3TagAddV2_t)FuppesGetProcAddress(m_LibHandle, "id3tag_add_v2");
  if(!m_Id3TagAddV2) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_add_v2'");
  }
  
  m_Id3TagPadV2 = (Id3TagPadV2_t)FuppesGetProcAddress(m_LibHandle, "id3tag_pad_v2");
  if(!m_Id3TagPadV2) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_pad_v2'");
  }
  
  m_Id3TagSetTitle = (Id3TagSetTitle_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_title");
  if(!m_Id3TagSetTitle) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_set_title'");
  }
  
  m_Id3TagSetArtist = (Id3TagSetArtist_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_artist");
  if(!m_Id3TagSetArtist) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_set_artist'");
  }
  
  m_Id3TagSetAlbum = (Id3TagSetAlbum_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_album");
  if(!m_Id3TagSetAlbum) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_set_album'");
  }
  
  m_Id3TagSetTrack = (Id3TagSetAlbum_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_track");
  if(!m_Id3TagSetTrack) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_set_track'");
  }  
  
  m_Id3TagSetGenre = (Id3TagSetGenre_t)FuppesGetProcAddress(m_LibHandle, "id3tag_set_genre");
  if(!m_Id3TagSetGenre) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "cannot load symbol 'id3tag_set_genre'");
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
    
    //string sBinFake = Base64Decode(sFakeMp3Tail);      
    //memcpy(szMp3Tail, sBinFake.c_str(), 128);                 
		Base64Decode(sFakeMp3Tail, szMp3Tail, sizeof(szMp3Tail));
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


int CLameWrapper::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int /*p_nBytesRead*/)
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
