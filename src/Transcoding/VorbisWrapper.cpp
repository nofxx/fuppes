/***************************************************************************
 *            VorbisWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#ifndef DISABLE_VORBIS

#include "VorbisWrapper.h"
#include <sstream>
#include <iostream>

#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif

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
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
}
  
bool CVorbisDecoder::LoadLib()
{
  #ifdef WIN32
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening vorbisfile.dll");
  m_LibHandle = FuppesLoadLibrary("vorbisfile.dll");  
  #else
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening libvorbis");
  m_LibHandle = FuppesLoadLibrary("libvorbisfile.so");
  #endif
  if(!m_LibHandle)
  {
    stringstream sLog;
    sLog << "cannot open library";
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
    //return false;
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
  
  return true;
}

bool CVorbisDecoder::OpenFile(std::string p_sFileName)
{
  if ((m_pVorbisFileHandle = fopen(p_sFileName.c_str(), "r")) == NULL)
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
     
  /*char **ptr = m_OvComment(&m_VorbisFile,-1)->user_comments;
  while(*ptr)
  {
    fprintf(stderr,"%s\n",*ptr);
    ++ptr;
  }
  fprintf(stderr,"\nBitstream is %d channel, %ldHz\n", m_pVorbisInfo->channels, m_pVorbisInfo->rate);
  //fprintf(stderr,"\nDecoded length: %ld samples\n", (long)ov_pcm_total(&m_VorbisFile, -1));
  fprintf(stderr,"Encoded by: %s\n\n", m_OvComment(&m_VorbisFile,-1)->vendor);*/
  
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
