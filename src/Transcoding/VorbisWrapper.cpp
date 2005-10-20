/***************************************************************************
 *            VorbisWrapper.cpp
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

#ifndef DISABLE_TRANSCODING
#ifndef DISABLE_VORBIS

#include "VorbisWrapper.h"
#include <sstream>
#include <iostream>

using namespace std;

const std::string LOGNAME = "VorbisWrapper";

CVorbisDecoder::CVorbisDecoder()
{
  /* determine endianness (clever trick courtesy of Nicholas Devillard,
   * (http://www.eso.org/~ndevilla/endian/) */
  int testvar = 1;
  if(*(char *)&testvar)
    m_nEndianess = 0;  // little endian
  else
    m_nEndianess = 1;  // big endian  
}

CVorbisDecoder::~CVorbisDecoder()
{
  FuppesCloseLibrary(m_LibHandle);
}
  
bool CVorbisDecoder::LoadLib()
{
  #ifdef WIN32
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening vorbisfile");
  m_LibHandle = FuppesLoadLibrary("vorbisfile.dll");  
  #else
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening libvorbis");
  m_LibHandle = FuppesLoadLibrary("libvorbisfile.so");
  #endif
  if(!m_LibHandle)
  {
    stringstream sLog;
    sLog << "cannot open library: "; // dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
  
  //m_OvOpen = (OvOpen_t)FuppesGetProcAddress(m_LibHandle, "ov_open");
  m_OvOpen = (OvOpen_t)dlsym(m_LibHandle, "ov_open");
  if(!m_OvOpen)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_open': "; // << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_OvInfo = (OvInfo_t)FuppesGetProcAddress(m_LibHandle, "ov_info");
  if(!m_OvInfo)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_info': "; // << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_OvComment = (OvComment_t)FuppesGetProcAddress(m_LibHandle, "ov_comment");
  if(!m_OvComment)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_comment': "; // << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    //return false;
  }  
  
  m_OvRead = (OvRead_t)FuppesGetProcAddress(m_LibHandle, "ov_read");
  if(!m_OvRead)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_read': "; // << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  m_OvClear = (OvClear_t)FuppesGetProcAddress(m_LibHandle, "ov_clear");
  if(!m_OvClear)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'ov_clear': "; // << dlerror();
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }
  
  return true;
}

bool CVorbisDecoder::OpenFile(std::string p_sFileName)
{
  if ((m_pVorbisFileHandle = fopen(p_sFileName.c_str(), "r")) == NULL)
  {
    fprintf(stderr, "Cannot open %s\n", p_sFileName.c_str()); 
    return false;
  }
  
  if(m_OvOpen(m_pVorbisFileHandle, &m_VorbisFile, NULL, 0) < 0) 
  {
    fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");      
    return false;
  }	 

  m_pVorbisInfo = ov_info(&m_VorbisFile, -1);
  m_OvInfo(&m_VorbisFile, -1);
     
  char **ptr = m_OvComment(&m_VorbisFile,-1)->user_comments;
  while(*ptr)
  {
    fprintf(stderr,"%s\n",*ptr);
    ++ptr;
  }
  fprintf(stderr,"\nBitstream is %d channel, %ldHz\n", m_pVorbisInfo->channels, m_pVorbisInfo->rate);
  //fprintf(stderr,"\nDecoded length: %ld samples\n", (long)ov_pcm_total(&m_VorbisFile, -1));
  fprintf(stderr,"Encoded by: %s\n\n", m_OvComment(&m_VorbisFile,-1)->vendor);
  
  return true;
}

void CVorbisDecoder::CloseFile()
{
  m_OvClear(&m_VorbisFile);  
}

long CVorbisDecoder::DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize)
{  
  int bitstream = 0; 
  int bytesRead = m_OvRead(&m_VorbisFile, p_PcmOut, p_nSize, m_nEndianess, 2, 1, &bitstream);
  
  if(bytesRead == 0)
  {
      /* todo: error handling */
    return -1;
  }
  else if(bytesRead < 0) 
  {
    /* todo: error handling */
    /* error in the stream */
    return -1;
  }
  else 
  {
    if(bitstream != 0)
      return -1;
    
    /* calc samples an encode */
    long samplesRead = bytesRead / m_pVorbisInfo->channels / sizeof(short int);
    return samplesRead;
  }  
}

#endif /* DISABLE_VORBIS */
#endif /* DISABLE_TRANSCODING */