/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            decoder_flac.c
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#include "../../include/fuppes_plugin.h"


#ifdef __cplusplus
extern "C" {
#endif
		
#ifdef HAVE_FLAC_FILEDECODER // <= 1.1.2
#include <FLAC/file_decoder.h>
#else // >= 1.1.3
#include <FLAC/stream_decoder.h>
#endif
		
typedef struct flacData_t {
	
	ENDIANESS				outEndianess;
	
	#ifdef HAVE_FLAC_FILEDECODER
  FLAC__FileDecoder *decoder;
  #else
  FLAC__StreamDecoder *decoder;
  #endif
  
  FLAC__uint64  total_samples;
	
  int  channels;
  int  bitrate;

  char *buffer;
  int buffer_size;  
  int samplerate;
  int position;
  int duration;	
	
	char* pcm;	
	int		eof;
	int		samples_read;
	int		bytes_consumed;
	
} flacData_t;




#ifdef HAVE_FLAC_FILEDECODER
FLAC__StreamDecoderWriteStatus FLACFileDecoderWriteCallback(const FLAC__FileDecoder *decoder,
                                                            const FLAC__Frame* frame,
                                                            const FLAC__int32* const buffer[],
                                                            void* client_data)
#else
FLAC__StreamDecoderWriteStatus FLAC_StreamDecoderWriteCallback(const FLAC__StreamDecoder *decoder,
                                                              const FLAC__Frame* frame, 
                                                               const FLAC__int32* const buffer[], 
                                                               void* client_data)
#endif
{  
	flacData_t* data = (flacData_t*)client_data;	
	
  if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER) {
    data->pcm     			= (char*)buffer;    
    data->samples_read  = frame->header.blocksize;  
  }
	
	else if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER) {
		
    data->samples_read = frame->header.blocksize;
    
    int i;
    int j;
    int k = 0;    
    
    for(j = 0; j < frame->header.blocksize; j++) {
			
      for(i = 0; i < data->channels; i++) {
      
				FLAC__uint16 sample = buffer[i][j];
        
        if(data->outEndianess == E_LITTLE_ENDIAN) {          
          data->pcm[k++] = sample;
          data->pcm[k++] = sample >> 8;
        }
        else if(data->outEndianess == E_BIG_ENDIAN) {          
          data->pcm[k++] = sample >> 8;
          data->pcm[k++] = sample;
        }
      }
    }
    
    data->bytes_consumed = k;    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;    
  }
	else {
		//CSharedLog::Shared()->Log(L_DBG, "FLAC__STREAM_DECODER_WRITE_STATUS_ABORT", __FILE__, __LINE__);
		//dcd->error_occurred = true;
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}	
}

#ifdef HAVE_FLAC_FILEDECODER
void FLACFileDecoderMetadataCallback(const FLAC__FileDecoder* decoder,
                                     const FLAC__StreamMetadata* metadata,
                                     void* client_data)
#else
void FLAC_StreamDecoderMetadataCallback(const FLAC__StreamDecoder* decoder,
                                     const FLAC__StreamMetadata* metadata, 
                                     void* client_data)
#endif
{
	flacData_t* data = (flacData_t*)client_data;
	
  data->total_samples   = metadata->data.stream_info.total_samples;
  data->channels        = metadata->data.stream_info.channels;
  data->bitrate					= metadata->data.stream_info.bits_per_sample;  
  data->samplerate			= metadata->data.stream_info.sample_rate;
    
  //fuppesSleep(1);
}

#ifdef HAVE_FLAC_FILEDECODER
void FLACFileDecoderErrorCallback(const FLAC__FileDecoder* decoder,
                                  FLAC__StreamDecoderErrorStatus status,
                                  void* client_data)
#else
void FLAC_StreamDecoderErrorCallback(const FLAC__StreamDecoder* decoder, 
                                  FLAC__StreamDecoderErrorStatus status, 
                                  void* client_data)
#endif
{
  /*cout << "FLACFileDecoderErrorCallback" << endl;
  fflush(stdout);*/
}






void register_fuppes_plugin(plugin_info* plugin)
{
	strcpy(plugin->plugin_name, "FLAC");
	strcpy(plugin->plugin_author, "Ulrich Voelkel");
	plugin->plugin_type = PT_AUDIO_DECODER;
}

int fuppes_decoder_file_open(plugin_info* plugin, const char* fileName, audio_settings_t* settings)
{	
	plugin->user_data = malloc(sizeof(struct flacData_t));
	flacData_t* data = (flacData_t*)plugin->user_data;
	
	data->decoder = NULL;
  data->buffer = NULL;
  data->buffer_size = 0;
  data->channels = 0;
  data->samplerate = -1;
  data->duration = -1;
	data->eof = 0;
	data->samples_read = 0;
	data->bytes_consumed = 0;

	
	#ifdef HAVE_FLAC_FILEDECODER
	data->decoder = FLAC__file_decoder_new();
	#else	
	data->decoder = FLAC__stream_decoder_new();
	#endif
	
	#ifdef HAVE_FLAC_FILEDECODER
	if(!FLAC__file_decoder_set_write_callback(data->decoder, FLACFileDecoderWriteCallback)) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetWriteCallback" << endl;
		fuppes_decoder_file_close(plugin);
    return 1;
  }
  
  if(!FLAC__file_decoder_set_metadata_callback(data->decoder, FLACFileDecoderMetadataCallback)) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetMetadataCallback" << endl;
		fuppes_decoder_file_close(plugin);
    return 1;
  }
  
  if(!FLAC__file_decoder_set_error_callback(data->decoder, FLACFileDecoderErrorCallback)) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetErrorCallback" << endl;
		fuppes_decoder_file_close(plugin);
    return 1;
  }
  
  if(!FLAC__file_decoder_set_client_data(data->decoder, data)) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetClientData" << endl;
		fuppes_decoder_file_close(plugin);
    return 1; 
  }
  
  if(!FLAC__file_decoder_set_filename(data->decoder, filename)) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetFilename" << endl;
		fuppes_decoder_file_close(plugin);
    return 1; 
  }
  
  if(FLAC__file_decoder_init(data->decoder) != FLAC__FILE_DECODER_OK) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderInit" << endl;
		fuppes_decoder_file_close(plugin);
    return 1; 
  }
  
  if(!FLAC__file_decoder_process_until_end_of_metadata(data->decoder)) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderProcessUntilEndOfMetadata" << endl;
		fuppes_decoder_file_close(plugin);
    return 1; 
  }
  #endif // #ifdef HAVE_FLAC_FILEDECODER
  
    
  #ifndef HAVE_FLAC_FILEDECODER
  data->decoder = FLAC__stream_decoder_new();

  FLAC__stream_decoder_init_file(data->decoder, 
                               fileName, 
                               FLAC_StreamDecoderWriteCallback,
                               FLAC_StreamDecoderMetadataCallback,
                               FLAC_StreamDecoderErrorCallback, 
                               data);
  
  if(!FLAC__stream_decoder_process_until_end_of_metadata(data->decoder)) {
    //cout << "[ERROR] CFLACDecoder::OpenFile() - FLAC_StreamDecoderProcessUntilEndOfMetadata" << endl;
		plugin->log(0, __FILE__, __LINE__, "[ERROR] CFLACDecoder::OpenFile() - FLAC_StreamDecoderProcessUntilEndOfMetadata");
    fuppes_decoder_file_close(plugin);
    return 1; 
  }  
  #endif // #ifndef HAVE_FLAC_FILEDECODER	
	
	
  settings->channels   = data->channels;
  settings->bitrate    = data->bitrate;
  settings->samplerate = data->samplerate;
  //pAudioDetails->nNumPcmSamples = m_pFLACData->total_samples;
	
	data->eof = 1;
	return 0;
}

int fuppes_decoder_set_out_endianess(plugin_info* plugin, ENDIANESS endianess)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	data->outEndianess = endianess;
	return 0;
}

int fuppes_decoder_total_samples(plugin_info* plugin)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	return data->total_samples;
}

int fuppes_decoder_decode_interleaved(plugin_info* plugin, char* pcmOut, int bufferSize, int* bytesRead)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	
	if(data->eof == 0) {
		return -1;
	}
	
	data->pcm = pcmOut;	
	
	#ifdef HAVE_FLAC_FILEDECODER
  if(FLAC__file_decoder_get_state(data->decoder) == FLAC__FILE_DECODER_END_OF_FILE) {    
    FLAC__file_decoder_finish(data->decoder);
  #else
  if(FLAC__stream_decoder_get_state(data->decoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {    
    FLAC__stream_decoder_finish(data->decoder);
  #endif 
    data->eof = 0;
    return data->samples_read;
  }

  
  #ifdef HAVE_FLAC_FILEDECODER
  if(FLAC__file_decoder_process_single(data->decoder)) {
  #else
  if(FLAC__stream_decoder_process_single(data->decoder)) {
  #endif    
    *bytesRead = data->bytes_consumed;
    return data->samples_read;
  }	

	return -1;
}

void fuppes_decoder_file_close(plugin_info* plugin)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	
	if(data->decoder) {
		#ifdef HAVE_FLAC_FILEDECODER
		FLAC__file_decoder_delete(data->decoder);
		#else	
		FLAC__stream_decoder_delete(data->decoder);
		#endif
	}
	
	free(plugin->user_data);
	plugin->user_data = NULL;
}
		
void unregister_fuppes_plugin(plugin_info* info)
{
}
		
#ifdef __cplusplus
}
#endif
