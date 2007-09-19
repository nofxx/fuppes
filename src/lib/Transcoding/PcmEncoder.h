/***************************************************************************
 *            PcmEncoder.h
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

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif
 
#ifndef DISABLE_TRANSCODING

#ifndef _PCMENCODER_H
#define _PCMENCODER_H

#include "WrapperBase.h"

class CPcmEncoder: public CAudioEncoderBase
{
  public:
    CPcmEncoder();
    virtual ~CPcmEncoder();
    
    bool LoadLib() { return true; }
    void Init() { }
  
    void SetAudioDetails(CAudioDetails* pAudioDetails);
    void SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings);
  
    int   EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead);
    int   Flush() { return 0; }
    unsigned char* GetEncodedBuffer() { return m_sBuffer; }
  
    unsigned int GuessContentLength(unsigned int p_nNumPcmSamples);
  
  private:
    unsigned char*   m_sBuffer; //[1024 * 1024];  
    unsigned int     m_nBufferSize;
  
    int nSampleRate;
    int nNumChannels;
    int nNumSamples;    
};

#endif // _PCMENCODER_H

#endif // DISABLE_TRANSCODING
