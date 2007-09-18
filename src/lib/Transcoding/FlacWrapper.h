/***************************************************************************
 *            FlacWrapper.h
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
 
#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifndef DISABLE_TRANSCODING
#ifdef  HAVE_FLAC
 
#ifndef _FLACWRAPPER_H
#define _FLACWRAPPER_H

#include "WrapperBase.h"

#ifdef HAVE_FLAC_FILEDECODER // <= 1.1.2
#include <FLAC/file_decoder.h>
#else // >= 1.1.3
#include <FLAC/stream_decoder.h>
#endif

#ifdef __cplusplus
extern "C"
{
  #ifdef HAVE_FLAC_FILEDECODER
  /* FLAC__FileDecoder* FLAC__file_decoder_new() */  
  typedef FLAC__FileDecoder* (*FLACFileDecoderNew_t)();
  
  /* void FLAC__file_decoder_delete(FLAC__FileDecoder *decoder) */
  typedef void (*FLACFileDecoderDelete_t)(FLAC__FileDecoder*);
  
  /* FLAC__bool FLAC__file_decoder_set_filename(FLAC__FileDecoder *decoder, const char *value) */
  typedef FLAC__bool (*FLACFileDecoderSetFilename_t)(FLAC__FileDecoder*, const char*);
  
  /* FLAC__bool	FLAC__file_decoder_set_write_callback(FLAC__FileDecoder *decoder, FLAC__FileDecoderWriteCallback value) */
  typedef FLAC__bool (*FLACFileDecoderSetWriteCallback_t)(FLAC__FileDecoder*, FLAC__FileDecoderWriteCallback);
  
  /* FLAC__bool	FLAC__file_decoder_set_metadata_callback(FLAC__FileDecoder *decoder, FLAC__FileDecoderMetadataCallback value) */
  typedef FLAC__bool (*FLACFileDecoderSetMetadataCallback_t)(FLAC__FileDecoder*, FLAC__FileDecoderMetadataCallback);

  /* FLAC__bool FLAC__file_decoder_set_error_callback(FLAC__FileDecoder *decoder, FLAC__FileDecoderErrorCallback value) */
  typedef FLAC__bool (*FLACFileDecoderSetErrorCallback_t)(FLAC__FileDecoder*, FLAC__FileDecoderErrorCallback);
  
  /* FLAC__bool FLAC__file_decoder_set_client_data(FLAC__FileDecoder *decoder, void *value) */
  typedef FLAC__bool (*FLACFileDecoderSetClientData_t)(FLAC__FileDecoder*, void*);
  
  /* FLAC__FileDecoderState FLAC__file_decoder_init(FLAC__FileDecoder *decoder) */
  typedef FLAC__FileDecoderState (*FLACFileDecoderInit_t)(FLAC__FileDecoder*);
   
  /* FLAC__bool FLAC__file_decoder_process_until_end_of_metadata(FLAC__FileDecoder *decoder) */
  typedef FLAC__bool (*FLACFileDecoderProcessUntilEndOfMetadata_t)(FLAC__FileDecoder*);
   
  /* FLAC__bool	FLAC__file_decoder_process_single(FLAC__FileDecoder *decoder) */
  typedef FLAC__bool (*FLACFileDecoderProcessSingle_t)(FLAC__FileDecoder*);
  
  /* FLAC__FileDecoderState 	FLAC__file_decoder_get_state (const FLAC__FileDecoder *decoder) */
  typedef FLAC__FileDecoderState (*FLACFileDecoderGetState_t)(const FLAC__FileDecoder*);
  
  /* FLAC__bool	FLAC__file_decoder_finish(FLAC__FileDecoder *decoder) */
  typedef FLAC__bool (*FLACFileDecoderFinish_t)(FLAC__FileDecoder*);  
  #else
  
  // FLAC__StreamDecoder* FLAC__stream_decoder_new(void) 
  typedef FLAC__StreamDecoder* (*FLAC_StreamDecoderNew_t)();
  
  /* FLAC__StreamDecoderInitStatus FLAC__stream_decoder_init_file(
          FLAC__StreamDecoder* decoder,
          const char *  	filename,
          FLAC__StreamDecoderWriteCallback  	write_callback,
          FLAC__StreamDecoderMetadataCallback  	metadata_callback,
          FLAC__StreamDecoderErrorCallback  	error_callback,
          void *  	client_data)   */
  typedef FLAC__StreamDecoderInitStatus (*FLAC_StreamDecoderInitFile_t)
          (FLAC__StreamDecoder* decoder,
           const char* ilename,
           FLAC__StreamDecoderWriteCallback    write_callback,
           FLAC__StreamDecoderMetadataCallback metadata_callback,
           FLAC__StreamDecoderErrorCallback    error_callback,
           void* client_data);
  
  // FLAC__bool FLAC__stream_decoder_process_until_end_of_metadata(FLAC__StreamDecoder* decoder) 
  typedef FLAC__bool (*FLAC_StreamDecoderProcessUntilEndOfMetadata_t)(FLAC__StreamDecoder* decoder);
  
  // FLAC__bool FLAC__stream_decoder_process_single(FLAC__StreamDecoder* decoder) 
  typedef FLAC__bool (*FLAC_StreamDecoderProcessSingle_t)(FLAC__StreamDecoder* decoder);
  
  // FLAC__StreamDecoderState FLAC__stream_decoder_get_state(const FLAC__StreamDecoder* decoder) 
  typedef FLAC__StreamDecoderState (*FLAC_StreamDecoderGetState_t)(const FLAC__StreamDecoder* decoder);
    
  // FLAC__bool FLAC__stream_decoder_finish(FLAC__StreamDecoder* decoder) 
  typedef FLAC__bool (*FLAC_StreamDecoderFinish_t)(FLAC__StreamDecoder* decoder);
  
  // void FLAC__stream_decoder_delete(FLAC__StreamDecoder* decoder) 
  typedef void (*FLAC_StreamDecoderDelete_t)(FLAC__StreamDecoder* decoder);
  
  #endif
}
#endif // __cplusplus

typedef struct flac_data_s {
  #ifdef HAVE_FLAC_FILEDECODER
  FLAC__FileDecoder *decoder;
  #else
  FLAC__StreamDecoder *decode;
  #endif
  
  FLAC__uint64  total_samples;
  unsigned int  channels;
  unsigned int  bits_per_sample;
  
  char *buffer;
  int buffer_size;  
  unsigned int sample_rate;
  long position;
  long duration;
} flac_data_t;




class CFLACDecoder: public CAudioDecoderBase
{
  public:
    CFLACDecoder();
    virtual ~CFLACDecoder();
  
    bool LoadLib();
    bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails);
    void CloseFile();
    long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead);

    unsigned int NumPcmSamples();
  
    char* m_pPcmOut;
    long  m_nBytesConsumed;
    long  m_nSamplesRead;
      
    flac_data_t* m_pFLACData;    
  
  private:
    bool             m_bEOF;
  
    #ifdef HAVE_FLAC_FILEDECODER
    FLAC__FileDecoder*                    m_pFLACFileDecoder;  
    FLACFileDecoderNew_t                  m_FLACFileDecoderNew;
    FLACFileDecoderDelete_t               m_FLACFileDecoderDelete;
    FLACFileDecoderSetFilename_t          m_FLACFileDecoderSetFilename;
    FLACFileDecoderSetWriteCallback_t     m_FLACFileDecoderSetWriteCallback;
    FLACFileDecoderSetMetadataCallback_t  m_FLACFileDecoderSetMetadataCallback;
    FLACFileDecoderSetErrorCallback_t     m_FLACFileDecoderSetErrorCallback;
    FLACFileDecoderSetClientData_t        m_FLACFileDecoderSetClientData;
    FLACFileDecoderInit_t                 m_FLACFileDecoderInit;
    FLACFileDecoderProcessUntilEndOfMetadata_t m_FLACFileDecoderProcessUntilEndOfMetadata;
    FLACFileDecoderProcessSingle_t        m_FLACFileDecoderProcessSingle;
    FLACFileDecoderGetState_t             m_FLACFileDecoderGetState;
    FLACFileDecoderFinish_t               m_FLACFileDecoderFinish;
    #else
    FLAC__StreamDecoder*                  m_pFLAC_StreamDecoder;
    FLAC_StreamDecoderNew_t               m_FLAC_StreamDecoderNew;
    FLAC_StreamDecoderInitFile_t          m_FLAC_StreamDecoderInitFile;
    FLAC_StreamDecoderProcessUntilEndOfMetadata_t m_FLAC_StreamDecoderProcessUntilEndOfMetadata;
    FLAC_StreamDecoderProcessSingle_t     m_FLAC_StreamDecoderProcessSingle;
    FLAC_StreamDecoderGetState_t          m_FLAC_StreamDecoderGetState;
    FLAC_StreamDecoderFinish_t            m_FLAC_StreamDecoderFinish;
    FLAC_StreamDecoderDelete_t            m_FLAC_StreamDecoderDelete;
    #endif
};

#endif // _FLACWRAPPER_H

#endif // HAVE_FLAC
#endif // DISABLE_TRANSCODING
