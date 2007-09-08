/***************************************************************************
 *            FaadWrapper.h
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
 
#ifndef _FAADWRAPPER_H
#define _FAADWRAPPER_H

#include "WrapperBase.h"
#include <faad.h>

#ifdef __cplusplus
extern "C"
{
  /* faacDecHandle faacDecOpen(void); */
  typedef faacDecHandle (*faacDecOpen_t)(void);

  /* faacDecConfigurationPtr faacDecGetCurrentConfiguration(faacDecHandle hDecoder); */
  typedef faacDecConfigurationPtr (*faacDecGetCurrentConfiguration_t)(faacDecHandle);

  /* unsigned char faacDecSetConfiguration(NeAACDecHandle hDecoder, faacDecConfigurationPtr config); */
  typedef unsigned char (*faacDecSetConfiguration_t)(faacDecHandle, faacDecConfigurationPtr);
  
  /* long faacDecInit(faacDecHandle hDecoder,
                              unsigned char *buffer,
                              unsigned long buffer_size,
                              unsigned long *samplerate,
                              unsigned char *channels); */
  typedef long (*faacDecInit_t)(faacDecHandle, unsigned char*,
                              unsigned long, unsigned long*,
                              unsigned char*);
  
  /* void faacDecClose(faacDecHandle hDecoder); */
  typedef void (*faacDecClose_t)(faacDecHandle);

  /* void* faacDecDecode(faacDecHandle hDecoder,
                                 faacDecFrameInfo *hInfo,
                                 unsigned char *buffer,
                                 unsigned long buffer_size);*/
  typedef void* (*faacDecDecode_t)(faacDecHandle, faacDecFrameInfo*,
                                 unsigned char*, unsigned long);  
  
}
#endif // __cplusplus


class CFaadWrapper: public CAudioDecoderBase
{
  public:
    CFaadWrapper();
    virtual ~CFaadWrapper();
  
    bool LoadLib();
    bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails);
    void CloseFile();
    long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead);

    unsigned int NumPcmSamples();
  
  private:
    
    faacDecHandle            AACDecoder;
    faacDecFrameInfo         AACFrameInfo;
    faacDecConfigurationPtr  AACConfig;
    
    unsigned char*  m_Buffer;
    FILE*           m_pFileHandle;
    int             m_nFileLength;    
  
    unsigned long   m_nBufferSize;
    int             m_nBytesConsumed;
    bool            m_bIsMp4;
    void*           m_pSampleBuffer;
  
    faacDecOpen_t                      m_faacDecOpen;
    faacDecGetCurrentConfiguration_t   m_faacDecGetCurrentConfiguration;
    faacDecSetConfiguration_t          m_faacDecSetConfiguration;
    faacDecInit_t                      m_faacDecInit;
    faacDecDecode_t                    m_faacDecDecode;
    faacDecClose_t                     m_faacDecClose;
};

#endif // _FAADWRAPPER_H
#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
