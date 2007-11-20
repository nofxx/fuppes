/***************************************************************************
 *            VorbisWrapper.cpp
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
#include "VorbisWrapper.h"

#ifdef HAVE_VORBIS

#include "../SharedConfig.h"
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

using namespace std;

const std::string LOGNAME = "VorbisWrapper";

CVorbisDecoder::CVorbisDecoder():CAudioDecoderBase()
{
}

CVorbisDecoder::~CVorbisDecoder()
{
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
}
  
bool CVorbisDecoder::LoadLib()
{
  #ifdef WIN32
  std::string sLibName = "libvorbisfile-3.dll";
  #else  
  
  #ifndef HAVE_TREMOR
  std::string sLibName = "libvorbisfile.so";
  #elif HAVE_TREMOR
  std::string sLibName = "libvorbisidec.so";
  #endif  

  #endif
  
  if(!CSharedConfig::Shared()->VorbisLibName().empty()) {
    sLibName = CSharedConfig::Shared()->VorbisLibName();
  }  
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening " + sLibName);
  m_LibHandle = FuppesLoadLibrary(sLibName);   
  
  if(!m_LibHandle)
  {
    stringstream sLog;
    sLog << "cannot open library " << sLibName;
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
  
  m_OvOpen = (OvOpen_t)FuppesGetProcAddress(m_LibHandle, "ov_open");
  if(!m_OvOpen)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_open'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_OvInfo = (OvInfo_t)FuppesGetProcAddress(m_LibHandle, "ov_info");
  if(!m_OvInfo)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_info'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_OvComment = (OvComment_t)FuppesGetProcAddress(m_LibHandle, "ov_comment");
  if(!m_OvComment)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_comment'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());    
  }  
  
  m_OvRead = (OvRead_t)FuppesGetProcAddress(m_LibHandle, "ov_read");
  if(!m_OvRead)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_read'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_OvClear = (OvClear_t)FuppesGetProcAddress(m_LibHandle, "ov_clear");
  if(!m_OvClear)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_clear'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_OvPcmTotal = (OvPcmTotal_t)FuppesGetProcAddress(m_LibHandle, "ov_pcm_total");
  if(!m_OvPcmTotal) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'ov_pcm_total'", __FILE__, __LINE__);
    return false;
  }  
  
  return true;
}

bool CVorbisDecoder::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{
  
  if ((m_pVorbisFileHandle = fopen(p_sFileName.c_str(), "rb")) == NULL)
  {
    fprintf(stderr, "Cannot open %s\n", p_sFileName.c_str()); 
    return false;
  }
  
	#ifdef _WIN32
  _setmode(_fileno(m_pVorbisFileHandle), _O_BINARY);
  #endif
  
  if(m_OvOpen(m_pVorbisFileHandle, &m_VorbisFile, NULL, 0) < 0) 
  {
    fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");      
    return false;
  }

  m_pVorbisInfo = m_OvInfo(&m_VorbisFile, -1);
     
  pAudioDetails->nNumChannels   = m_pVorbisInfo->channels;
  pAudioDetails->nSampleRate    = m_pVorbisInfo->rate;
  pAudioDetails->nNumPcmSamples = m_OvPcmTotal(&m_VorbisFile, -1);

  /*char **ptr = m_OvComment(&m_VorbisFile,-1)->user_comments;
  while(*ptr)
  {
    fprintf(stderr,"%s\n",*ptr);
    ++ptr;
  }
  fprintf(stderr,"Encoded by: %s\n\n", m_OvComment(&m_VorbisFile,-1)->vendor); */
  
  return true;
}

void CVorbisDecoder::CloseFile()
{
  m_OvClear(&m_VorbisFile);  
}

long CVorbisDecoder::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{ 
  int bitstream = 0; 
  int nBytesConsumed = m_OvRead(&m_VorbisFile, p_PcmOut, p_nBufferSize, m_nOutEndianess, 2, 1, &bitstream);  
  
  // eof
  if(nBytesConsumed == 0)
  {    
    return -1;
  }
  else if(nBytesConsumed < 0) 
  {
    // error in the stream
    if(nBytesConsumed == OV_HOLE)
      CSharedLog::Shared()->Log(L_EXT,"OV_HOLE", __FILE__, __LINE__);
    else if(nBytesConsumed == OV_EBADLINK)    
      CSharedLog::Shared()->Log(L_EXT,"OV_EBADLINK", __FILE__, __LINE__);
    else {
      CSharedLog::Shared()->Log(L_EXT,"unknown stream error", __FILE__, __LINE__);      
    }    
    return -1;
  }
  else 
  {
    if(bitstream != 0)
      return -1;
    
    *p_nBytesRead = nBytesConsumed;
    
    // calc samples and return
    long samplesRead = nBytesConsumed / m_pVorbisInfo->channels / sizeof(short int);
    return samplesRead;
  }  
}

unsigned int CVorbisDecoder::NumPcmSamples()
{  
  return m_OvPcmTotal(&m_VorbisFile, -1);  
}

#endif // HAVE_VORBIS
#endif // DISABLE_TRANSCODING
