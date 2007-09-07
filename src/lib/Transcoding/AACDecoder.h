/***************************************************************************
 *            AACDecoder.h
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
#ifdef  HAVE_FAAD
 
#ifndef _AACDECODER_H
#define _AACDECODER_H

#include "WrapperBase.h"
#include <faad.h>

#ifdef __cplusplus
extern "C"
{
  /* NeAACDecHandle NeAACDecOpen(void); */
  typedef NeAACDecHandle (*NeAACDecOpen_t)(void);

  /* NeAACDecConfigurationPtr NeAACDecGetCurrentConfiguration(NeAACDecHandle hDecoder); */
  typedef NeAACDecConfigurationPtr (*NeAACDecGetCurrentConfiguration_t)(NeAACDecHandle);

  /* unsigned char NeAACDecSetConfiguration(NeAACDecHandle hDecoder, NeAACDecConfigurationPtr config); */
  typedef unsigned char (*NeAACDecSetConfiguration_t)(NeAACDecHandle, NeAACDecConfigurationPtr);
  
  /* long NeAACDecInit(NeAACDecHandle hDecoder,
                              unsigned char *buffer,
                              unsigned long buffer_size,
                              unsigned long *samplerate,
                              unsigned char *channels); */
  typedef long (*NeAACDecInit_t)(NeAACDecHandle, unsigned char*,
                              unsigned long, unsigned long*,
                              unsigned char*);
  
  /* void NEAACDECAPI NeAACDecClose(NeAACDecHandle hDecoder); */
  typedef void (*NeAACDecClose_t)(NeAACDecHandle);

  /* void* NeAACDecDecode(NeAACDecHandle hDecoder,
                                 NeAACDecFrameInfo *hInfo,
                                 unsigned char *buffer,
                                 unsigned long buffer_size);*/
  typedef void* (*NeAACDecDecode_t)(NeAACDecHandle, NeAACDecFrameInfo*,
                                 unsigned char*, unsigned long);  
  
}
#endif // __cplusplus


class CAACDecoder: public CAudioDecoderBase
{
  public:
    CAACDecoder();
    virtual ~CAACDecoder();
  
    bool LoadLib();
    bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails);
    void CloseFile();
    long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead);

    unsigned int NumPcmSamples();
  
  private:
    
    NeAACDecHandle            AACDecoder;
    NeAACDecFrameInfo         AACFrameInfo;
    NeAACDecConfigurationPtr  AACConfig;
    
    unsigned char*  m_Buffer;
    FILE*           m_pFileHandle;
    int             m_nFileLength;
    unsigned int    m_nOffset;
  
    NeAACDecOpen_t                      m_NeAACDecOpen;
    NeAACDecGetCurrentConfiguration_t   m_NeAACDecGetCurrentConfiguration;
    NeAACDecSetConfiguration_t          m_NeAACDecSetConfiguration;
    NeAACDecInit_t                      m_NeAACDecInit;
    NeAACDecDecode_t                    m_NeAACDecDecode;
    NeAACDecClose_t                     m_NeAACDecClose;
};

#endif // _AACDECODER_H
#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
