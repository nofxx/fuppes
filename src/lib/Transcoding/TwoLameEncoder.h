/***************************************************************************
 *            TwoLameEncoder.h
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
#ifdef  HAVE_TWOLAME
 
#ifndef _TWOLAMEENCODER_H
#define _TWOLAMEENCODER_H

#include "WrapperBase.h"
#include <twolame.h>

#define TWOLAME_MAX_BUFFER 32768

extern "C"
{ 
  typedef twolame_options TwoLameOptions;
  
  /* twolame_options*	twolame_init(void) */
  typedef TwoLameOptions* (*TwoLameInit_t)();
  
  // int 	twolame_set_num_channels (twolame_options *glopts, int num_channels)
  typedef int (*TwoLameSetNumChannels_t)(twolame_options*, int);
  
  //int 	twolame_set_out_samplerate (twolame_options *glopts, int samplerate)
  typedef int (*TwoLameSetOutSamplerate_t)(twolame_options*, int);
  
  //int 	twolame_set_in_samplerate (twolame_options *glopts, int samplerate)
  typedef int (*TwoLameSetInSamplerate_t)(twolame_options*, int);
  
  //int 	twolame_set_bitrate (twolame_options *glopts, int bitrate)
  typedef int (*TwoLameSetBitrate_t)(twolame_options*, int);
  
  /* void lame_init_params(lame_global_flags*) */
  typedef void     (*TwoLameInitParams_t)(twolame_options*);
  
  
  /* int 	twolame_encode_buffer_interleaved (twolame_options *glopts, const short int pcm[], int num_samples, unsigned char *mp2buffer, int mp2buffer_size) */  
  typedef int (*TwoLameEncodeBufferInterleaved_t)(twolame_options*, short int[], int, unsigned char*, int);                    
               
  // int 	twolame_encode_flush (twolame_options *glopts, unsigned char *mp2buffer, int mp2buffer_size)
  typedef int (*TwoLameEncodeFlush_t)(twolame_options*, unsigned char*, int); 
  
  // void twolame_close(twolame_options **glopts);
  typedef void (*TwoLameClose_t)(twolame_options**);
  
 /* const char* get_lame_version_name() */  
  typedef const char*        (*TwoLameGetVersionName_t)();
  /* void lame_print_config(lame_global_flags*) */
  typedef void               (*TwoLamePrintConfig_t)(twolame_options*);  
  //typedef void               (*TwoLameSetCompressionRatio_t)(twolame_options*, float);
    
}

class CTwoLameEncoder: public CAudioEncoderBase
{
  public:
    CTwoLameEncoder();
    ~CTwoLameEncoder();
    bool LoadLib();
  
    void Init();
    void PrintConfig();
    std::string GetVersion();
    //void SetBitrate(LAME_BITRATE p_nBitrate);
    
    void SetTranscodingSettings(CTranscodingSettings* pTranscodingSettings);
  
    int   EncodeInterleaved(short int p_PcmIn[], int p_nNumSamples, int p_nBytesRead);
    int   Flush();
    unsigned char* GetEncodedBuffer() { return m_sMp3Buffer; }
    
  
    #warning todo
    unsigned int GuessContentLength(unsigned int p_nNumPcmSamples) { return 0; }
    
  private:
    fuppesLibHandle  m_LibHandle;
    TwoLameOptions*  m_TwoLameOptions;
    unsigned char    m_sMp3Buffer[TWOLAME_MAX_BUFFER];
  
    TwoLameInit_t        m_TwoLameInit;
    TwoLameSetNumChannels_t m_TwoLameSetNumChannels;
    TwoLameSetOutSamplerate_t m_TwoLameSetOutSamplerate;
    TwoLameSetInSamplerate_t   m_TwoLameSetInSamplerate;
    TwoLameSetBitrate_t       m_TwoLameSetBitrate;
  
    TwoLameInitParams_t  m_TwoLameInitParams; 
    
    TwoLameEncodeBufferInterleaved_t m_TwoLameEncodeBufferInterleaved;
    TwoLameEncodeFlush_t             m_TwoLameEncodeFlush;
  
    TwoLameClose_t         m_TwoLameClose;
  
    // optional
    TwoLameGetVersionName_t  m_TwoLameGetVersionName;
    TwoLamePrintConfig_t m_TwoLamePrintConfig;  
    //TwoLameSetCompressionRatio_t     m_TwoLameSetCompressionRatio;
};

#endif // _TWOLAMEENCODER_H
#endif // HAVE_TWOLAME
#endif // DISABLE_TRANSCODING
