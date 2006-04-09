/***************************************************************************
 *            FlacWrapper.h
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
#ifndef DISABLE_FLAC
 
#ifndef _FLACWRAPPER_H
#define _FLACWRAPPER_H

#include "WrapperBase.h"
#include <FLAC/file_decoder.h>

#ifdef __cplusplus
extern "C"
{
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
  
  /* FLAC__bool	FLAC__file_decoder_finish(FLAC__FileDecoder *decoder) */
  typedef FLAC__bool (*FLACFileDecoderFinish_t)(FLAC__FileDecoder*);  
}
#endif

typedef struct flac_data_s {
  FLAC__FileDecoder *decoder;
  
  FLAC__uint64  total_samples;
  unsigned int  channels;
  unsigned int  bits_per_sample;
  
  char *buffer;
  int buffer_size;  
  int sample_rate;
  long position;
  long duration;
} flac_data_t;




class CFLACDecoder: public CDecoderBase
{
  public:
    CFLACDecoder();
    virtual ~CFLACDecoder();
  
    bool LoadLib();
    bool OpenFile(std::string p_sFileName);
    void CloseFile();
    long DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize);

    char* m_pPcmOut;
    long  m_nBytesReturned;
  
    flac_data_t* m_pFLACData;
  
  private:
    fuppesLibHandle  m_LibHandle;  
  
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
    FLACFileDecoderFinish_t               m_FLACFileDecoderFinish;
};

#endif /* _FLACWRAPPER_H */

#endif /* DISABLE_FLAC */
#endif /* DISABLE_TRANSCODING */
