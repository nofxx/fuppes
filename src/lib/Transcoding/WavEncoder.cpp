/***************************************************************************
 *            WavEncoder.cpp
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

#include "WavEncoder.h"

#include <iostream>
using namespace std;

#ifndef DISABLE_TRANSCODING

CWavEncoder::CWavEncoder()
{  
  m_sBuffer = NULL;
}

CWavEncoder::~CWavEncoder()
{
  if(m_sBuffer != NULL) {
    free(m_sBuffer);
  }
}

int CWavEncoder::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead)
{
  int nOffset = 0;
  
  //cout << "WAV: " << p_nBytesRead << endl;
  
  if(m_sBuffer != NULL) {
    //free(m_sBuffer);
    if(m_nBufferSize < p_nBytesRead) {
      m_sBuffer = (unsigned char*)realloc(m_sBuffer, p_nBytesRead);
      m_nBufferSize = p_nBytesRead;
    }
  }
  else {
    // first call. let's write the wav header
    WriteFileHeader();
    //f.write((const char*)headbuf, 44);
    
    m_sBuffer = (unsigned char*)malloc(p_nBytesRead + 44);
    m_nBufferSize = p_nBytesRead + 44;
    nOffset = 44;
    memcpy(m_sBuffer, headbuf, 44);
  }

  //f.write((const char*)p_PcmIn, p_nBytesRead);  
  memcpy(&m_sBuffer[nOffset], p_PcmIn, p_nBytesRead);
  
  return p_nBytesRead;
}

/*
  most of the following lines are taken from oggdec.c
  from the vorbis-tools-1.1.1 package (GPLv2)  
*/

#define WRITE_U32(buf, x) *(buf)     = (unsigned char)((x)&0xff);\
                          *((buf)+1) = (unsigned char)(((x)>>8)&0xff);\
                          *((buf)+2) = (unsigned char)(((x)>>16)&0xff);\
                          *((buf)+3) = (unsigned char)(((x)>>24)&0xff);

#define WRITE_U16(buf, x) *(buf)     = (unsigned char)((x)&0xff);\
                          *((buf)+1) = (unsigned char)(((x)>>8)&0xff);

void CWavEncoder::WriteFileHeader()
{
  int bits = 16;
  
  unsigned int size = 0x7fffffff;
  if(m_pAudioDetails->nPcmSize > 0) {
    size = m_pAudioDetails->nPcmSize;
  }
  int channels    = m_pAudioDetails->nChannels; //ov_info(vf,0)->channels;
  int samplerate  = m_pAudioDetails->nSampleRate; //ov_info(vf,0)->rate;
  int bytespersec = channels*samplerate*bits/8;
  int align = channels*bits/8;
  int samplesize = bits;

  unsigned int knownlength = m_pAudioDetails->nPcmSize;
  
  if(knownlength && knownlength*bits/8*channels < size)
      size = (unsigned int)(knownlength*bits/8*channels+44) ;

    memcpy(headbuf, "RIFF", 4);
    WRITE_U32(headbuf+4, size-8);
    memcpy(headbuf+8, "WAVE", 4);
    memcpy(headbuf+12, "fmt ", 4);
    WRITE_U32(headbuf+16, 16);
    WRITE_U16(headbuf+20, 1); /* format */
    WRITE_U16(headbuf+22, channels);
    WRITE_U32(headbuf+24, samplerate);
    WRITE_U32(headbuf+28, bytespersec);
    WRITE_U16(headbuf+32, align);
    WRITE_U16(headbuf+34, samplesize);
    memcpy(headbuf+36, "data", 4);
    WRITE_U32(headbuf+40, size - 44);  
}

void CWavEncoder::SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings)
{
  #warning todo
}

unsigned int CWavEncoder::GuessContentLength(unsigned int p_nNumPcmSamples)
{
  if(p_nNumPcmSamples == 0) {
    return 0;
  }
  else {
    #warning todo: (knownlength*bits/8*channels+44) ;
    return (p_nNumPcmSamples * 4) + 44;
  }
}

#endif // DISABLE_TRANSCODING
