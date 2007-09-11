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

/*
 * this file contains code from FAAD2 version 2.0
 *
 * FAAD2 - Freeware Advanced Audio (AAC) Decoder including SBR decoding
 * Copyright (C) 2003-2004 M. Bakker, Ahead Software AG, http://www.nero.com
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifndef DISABLE_TRANSCODING
#ifdef HAVE_FAAD

#ifndef _FAAD_WRAPPER_H
#define _FAAD_WRAPPER_H

#include <string>

#include <faad.h>

#ifdef HAVE_MP4FF_H
#include <mp4ff.h>
#endif

#include "WrapperBase.h"

#define FAAD_MAX_CHANNELS 6 /* make this higher to support files with
                          more channels */


#ifdef __cplusplus 
extern "C" 
{ 
  /* faacDecHandle faacDecOpen(void); */ 
  typedef faacDecHandle (*faacDecOpen_t)(void); 

  /* faacDecConfigurationPtr faacDecGetCurrentConfiguration(faacDecHandle hDecoder); */ 
  typedef faacDecConfigurationPtr (*faacDecGetCurrentConfiguration_t)(faacDecHandle); 
 
  /* char* faacDecGetErrorMessage(unsigned char errcode); */
  typedef char* (*faacDecGetErrorMessage_t)(unsigned char);
  
  /* unsigned char faacDecSetConfiguration(NeAACDecHandle hDecoder, faacDecConfigurationPtr config); */ 
  typedef unsigned char (*faacDecSetConfiguration_t)(faacDecHandle, faacDecConfigurationPtr); 
   
  /* long faacDecInit(faacDecHandle hDecoder, 
                              unsigned char *buffer, 
                              unsigned long buffer_size, 
                              unsigned long *samplerate, 
                              unsigned char *channels); */ 
  typedef long (*faacDecInit_t)(faacDecHandle, unsigned char*, 
                              unsigned long, unsigned long*, unsigned char*);
  
  /* char faacDecInit2(faacDecHandle hDecoder, unsigned char *pBuffer,
                         unsigned long SizeOfDecoderSpecificInfo,
                         unsigned long *samplerate, unsigned char *channels); */ 
  typedef char(*faacDecInit2_t)(faacDecHandle, unsigned char*,
                         unsigned long, unsigned long*, unsigned char*);
  
  /* void faacDecClose(faacDecHandle hDecoder); */ 
  typedef void (*faacDecClose_t)(faacDecHandle); 
 
  /* void* faacDecDecode(faacDecHandle hDecoder, 
                                 faacDecFrameInfo *hInfo, 
                                 unsigned char *buffer, 
                                 unsigned long buffer_size);*/ 
  typedef void* (*faacDecDecode_t)(faacDecHandle, faacDecFrameInfo*, 
                                 unsigned char*, unsigned long); 
  
  
  /* char AudioSpecificConfig(unsigned char *pBuffer,
                                 unsigned long buffer_size,
                                 mp4AudioSpecificConfig *mp4ASC); */
  typedef char (*AudioSpecificConfig_t)(unsigned char*, unsigned long, mp4AudioSpecificConfig*);
  
  
  #ifdef HAVE_MP4FF_H
  
  /* int32_t mp4ff_read_sample(mp4ff_t *f, const int track, const int sample,
                          unsigned char **audio_buffer,  unsigned int *bytes); */  
  typedef int32_t (*mp4ff_read_sample_t)(mp4ff_t*, const int, const int,
                          unsigned char**,  unsigned int*);
  
  /* int32_t mp4ff_time_scale(const mp4ff_t *f, const int track); */
  typedef int32_t (*mp4ff_time_scale_t)(const mp4ff_t*, const int);
  
  /* int32_t mp4ff_num_samples(const mp4ff_t *f, const int track); */
  typedef int32_t (*mp4ff_num_samples_t)(const mp4ff_t*, const int);
  
  /* mp4ff_t *mp4ff_open_read(mp4ff_callback_t *f); */
  typedef mp4ff_t* (*mp4ff_open_read_t)(mp4ff_callback_t*);
  
  /* void mp4ff_close(mp4ff_t *f); */
  typedef void (*mp4ff_close_t)(mp4ff_t*);
  
  /* int32_t mp4ff_get_decoder_config(const mp4ff_t *f, const int track,
                             unsigned char** ppBuf, unsigned int* pBufSize); */
  typedef int32_t (*mp4ff_get_decoder_config_t)(const mp4ff_t*, const int,
                             unsigned char**, unsigned int*);
  
  /* int32_t mp4ff_total_tracks(const mp4ff_t *f); */
  typedef int32_t (*mp4ff_total_tracks_t)(const mp4ff_t*);
  
  /* int32_t mp4ff_get_sample_duration(const mp4ff_t *f, const int32_t track, const int32_t sample); */
  typedef int32_t (*mp4ff_get_sample_duration_t)(const mp4ff_t*, const int32_t, const int32_t);  
  
  #endif // HAVE_MP4FF_H
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
    FILE*           m_pFileHandle;
    unsigned int    m_nFileLength;
    unsigned int    m_nBufferSize;
    unsigned char*  m_Buffer;
    bool            m_bIsMp4;
    unsigned int    m_nBytesConsumed;
  
    unsigned int    m_nFileRest;
  
    faacDecHandle             hDecoder;
    faacDecFrameInfo          frameInfo;
    faacDecConfigurationPtr   config;  
  
    bool first_time;
  
    #ifdef HAVE_MP4FF_H
    mp4ff_callback_t*         mp4cb;
    mp4ff_t*                  infile;
    #endif
  
    bool            InitAACDecoder();
    #ifdef HAVE_MP4FF_H
    bool            InitMp4Decoder();
    #endif // HAVE_MP4FF_H
    
    void            FillBuffer();
  
    int             adts_parse(int *bitrate, float *length);
    #ifdef HAVE_MP4FF_H
    int             GetAACTrack(mp4ff_t *infile);
    #endif // HAVE_MP4FF_H
  
    int DecodeAACfile(char* p_PcmOut);  
    #ifdef HAVE_MP4FF_H
    int DecodeMP4file(char* p_PcmOut);
    #endif
  
    int write_audio_16bit(char* p_PcmOut, void *sample_buffer, unsigned int samples);
  
    long  sampleId;
    long  numSamples;
  
    int             track;
    unsigned long   samplerate;
    unsigned int    useAacLength;
    unsigned int    framesize;
    unsigned long   timescale;  
    unsigned int    initial;
  
  
  
    faacDecOpen_t                       m_faacDecOpen; 
    faacDecGetErrorMessage_t            m_faacDecGetErrorMessage;
    faacDecGetCurrentConfiguration_t    m_faacDecGetCurrentConfiguration; 
    faacDecSetConfiguration_t           m_faacDecSetConfiguration; 
    faacDecInit_t                       m_faacDecInit;
    faacDecInit2_t                      m_faacDecInit2;
    faacDecDecode_t                     m_faacDecDecode; 
    faacDecClose_t                      m_faacDecClose;
    AudioSpecificConfig_t               m_AudioSpecificConfig;
  
    
    #ifdef HAVE_MP4FF_H
    fuppesLibHandle  m_mp4ffLibHandle;
  
    mp4ff_read_sample_t                 m_mp4ff_read_sample;
    mp4ff_time_scale_t                  m_mp4ff_time_scale;
    mp4ff_num_samples_t                 m_mp4ff_num_samples;
    mp4ff_open_read_t                   m_mp4ff_open_read;
    mp4ff_close_t                       m_mp4ff_close;
    mp4ff_get_decoder_config_t          m_mp4ff_get_decoder_config;
    mp4ff_total_tracks_t                m_mp4ff_total_tracks;
    mp4ff_get_sample_duration_t         m_mp4ff_get_sample_duration;
    #endif // HAVE_MP4FF_H
};

#endif // _FAAD_WRAPPER_H

#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
