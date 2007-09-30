/***************************************************************************
 *            PcmEncoder.cpp
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

#include "PcmEncoder.h"

#include <iostream>
using namespace std;

#ifndef DISABLE_TRANSCODING

CPcmEncoder::CPcmEncoder():CAudioEncoderBase()
{
  m_sBuffer = NULL;  
  m_nBufferSize = 0;
  m_nInEndianess = E_LITTLE_ENDIAN;
}

CPcmEncoder::~CPcmEncoder()
{  
  if(m_sBuffer != NULL) {
    free(m_sBuffer);
  }
}

void CPcmEncoder::SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings)
{
}

void CPcmEncoder::SetAudioDetails(CAudioDetails* pAudioDetails)
{
  CAudioEncoderBase::SetAudioDetails(pAudioDetails);
  
  nSampleRate  = m_pAudioDetails->nSampleRate;
  nNumChannels = m_pAudioDetails->nNumChannels;
  nNumSamples  = m_pAudioDetails->nNumPcmSamples;  
}

int CPcmEncoder::EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead)
{
  if(!m_sBuffer) {
    m_sBuffer = (unsigned char*)malloc(p_nBytesRead * sizeof(unsigned char*));
    m_nBufferSize = p_nBytesRead;
  }  
  
  if(m_nBufferSize < p_nBytesRead) {
    m_sBuffer = (unsigned char*)realloc(m_sBuffer, p_nBytesRead * sizeof(unsigned char*));
  }
  
  memcpy(m_sBuffer, p_PcmIn, p_nBytesRead);
  
  return p_nBytesRead;
}

unsigned int CPcmEncoder::GuessContentLength(unsigned int p_nNumPcmSamples)
{
  if(p_nNumPcmSamples == 0) {
    return 0;
  }
  else {    
    nNumSamples = p_nNumPcmSamples;
    
    unsigned int  knownlength = -1;
    unsigned int  size        = nNumSamples;
    int           bits        = 16;
    int           channels    = nNumChannels;
        
    if(knownlength*bits/8*channels < size) {
      size = (unsigned int)(knownlength*bits/8*channels);
    }

    return size;
  }
}

#endif // DISABLE_TRANSCODING
