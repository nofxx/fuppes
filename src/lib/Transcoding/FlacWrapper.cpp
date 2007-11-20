/***************************************************************************
 *            FlacWrapper.cpp
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

#ifndef DISABLE_TRANSCODING
#include "FlacWrapper.h"
#ifdef HAVE_FLAC

const std::string LOGNAME = "FLACDecoder";

#include <sstream>
#include <iostream>
#include "../Common/Common.h"
#include "../SharedConfig.h"

using namespace std;



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
  if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER)
  {
    ((CFLACDecoder*)client_data)->m_pPcmOut       = (char*)buffer;    
    ((CFLACDecoder*)client_data)->m_nSamplesRead  = frame->header.blocksize;    
  }
	else if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER)
  {
    ((CFLACDecoder*)client_data)->m_nSamplesRead  = frame->header.blocksize;   
    
    int i;
    int j;
    int k = 0;    
    
    for(j = 0; j < frame->header.blocksize; j++) 
    {
      for(i = 0; i < ((CFLACDecoder*)client_data)->m_pFLACData->channels; i++) 
      {
        FLAC__uint16 sample = buffer[i][j];
        
        if(((CFLACDecoder*)client_data)->OutEndianess() == E_LITTLE_ENDIAN) {          
          ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = sample;
          ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = sample >> 8;
        }
        else if(((CFLACDecoder*)client_data)->OutEndianess() == E_BIG_ENDIAN) {          
          ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = sample >> 8;
          ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = sample;
        }
      }
    }
    
    ((CFLACDecoder*)client_data)->m_nBytesConsumed = k;
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;    
  }
	else 
  {
		CSharedLog::Shared()->Log(L_DBG, "FLAC__STREAM_DECODER_WRITE_STATUS_ABORT", __FILE__, __LINE__);
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
  ((CFLACDecoder*)client_data)->m_pFLACData->total_samples   = metadata->data.stream_info.total_samples;
  ((CFLACDecoder*)client_data)->m_pFLACData->channels        = metadata->data.stream_info.channels;
  ((CFLACDecoder*)client_data)->m_pFLACData->bits_per_sample = metadata->data.stream_info.bits_per_sample;  
  ((CFLACDecoder*)client_data)->m_pFLACData->sample_rate     = metadata->data.stream_info.sample_rate;  
  
  
  fuppesSleep(1);
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



CFLACDecoder::CFLACDecoder():CAudioDecoderBase()
{
  #ifdef HAVE_FLAC_FILEDECODER
  m_pFLACFileDecoder = NULL;
  #else
  m_pFLAC_StreamDecoder = NULL;
  #endif  
  
  m_pFLACData = (flac_data_t*)malloc(sizeof(flac_data_t));  
  
  m_pFLACData->buffer = NULL;
  m_pFLACData->buffer_size = 0;
  m_pFLACData->channels = 0;
  m_pFLACData->sample_rate = -1;
  m_pFLACData->duration = -1;
}

CFLACDecoder::~CFLACDecoder()
{
  free(m_pFLACData);
  
  #ifdef HAVE_FLAC_FILEDECODER
  if(m_pFLACFileDecoder) {
  #else
  if(m_pFLAC_StreamDecoder) {
  #endif
    CloseFile();
  }
  
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
}

bool CFLACDecoder::LoadLib()
{ 
  #ifdef WIN32
  std::string sLibName = "libFLAC-8.dll";
  #else  
  std::string sLibName = "libFLAC.so";
  #endif
  
  if(!CSharedConfig::Shared()->FlacLibName().empty()) {
    sLibName = CSharedConfig::Shared()->FlacLibName();
  }  
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening " + sLibName);
  m_LibHandle = FuppesLoadLibrary(sLibName);
  
  if(!m_LibHandle) {
    std::stringstream sLog;
    sLog << "cannot open library";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }   
  
  #ifdef HAVE_FLAC_FILEDECODER
  m_FLACFileDecoderNew = (FLACFileDecoderNew_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_new");  
  if(!m_FLACFileDecoderNew) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_new'");
    return false;
  }    
  
  m_FLACFileDecoderDelete = (FLACFileDecoderDelete_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_delete");
  if(!m_FLACFileDecoderDelete) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_delete'");
    return false;  
  }

  m_FLACFileDecoderSetFilename = (FLACFileDecoderSetFilename_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_filename");
  if(!m_FLACFileDecoderSetFilename) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_filename'");
    return false;    
  }
  
  m_FLACFileDecoderSetWriteCallback = (FLACFileDecoderSetWriteCallback_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_write_callback");
  if(!m_FLACFileDecoderSetWriteCallback) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_write_callback'");
    return false;      
  }

  m_FLACFileDecoderSetMetadataCallback = (FLACFileDecoderSetMetadataCallback_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_metadata_callback");
  if(!m_FLACFileDecoderSetMetadataCallback) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_metadata_callback'");
    return false;
  }

  m_FLACFileDecoderSetErrorCallback = (FLACFileDecoderSetErrorCallback_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_error_callback");
  if(!m_FLACFileDecoderSetErrorCallback) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_error_callback'");
    return false;
  }
  
  m_FLACFileDecoderSetClientData = (FLACFileDecoderSetClientData_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_client_data");
  if(!m_FLACFileDecoderSetClientData) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_client_data'");
    return false;
  }

  m_FLACFileDecoderInit = (FLACFileDecoderInit_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_init");
  if(!m_FLACFileDecoderInit) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_init'");
    return false;
  }
  
  m_FLACFileDecoderProcessUntilEndOfMetadata = (FLACFileDecoderProcessUntilEndOfMetadata_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_process_until_end_of_metadata");
  if(!m_FLACFileDecoderProcessUntilEndOfMetadata) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_process_until_end_of_metadata'");
    return false;
  }
  
  m_FLACFileDecoderProcessSingle = (FLACFileDecoderProcessSingle_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_process_single");
  if(!m_FLACFileDecoderProcessSingle) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_process_single'");
    return false;
  }
    
  m_FLACFileDecoderGetState = (FLACFileDecoderGetState_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_get_state");
  if(!m_FLACFileDecoderGetState) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_get_state'");
    return false;
  }
    
  m_FLACFileDecoderFinish = (FLACFileDecoderFinish_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_finish");
  if(!m_FLACFileDecoderFinish) {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_finish'");
    return false;
  }
  #endif // #ifdef HAVE_FLAC_FILEDECODER
  
  
  #ifndef HAVE_FLAC_FILEDECODER
  m_FLAC_StreamDecoderNew = (FLAC_StreamDecoderNew_t)FuppesGetProcAddress(m_LibHandle, "FLAC__stream_decoder_new");  
  if(!m_FLAC_StreamDecoderNew) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'FLAC__stream_decoder_new'", __FILE__, __LINE__);
    return false;
  }    
  
  m_FLAC_StreamDecoderInitFile = (FLAC_StreamDecoderInitFile_t)FuppesGetProcAddress(m_LibHandle, "FLAC__stream_decoder_init_file");  
  if(!m_FLAC_StreamDecoderInitFile) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'FLAC__stream_decoder_init_file'", __FILE__, __LINE__);
    return false;
  }    
  
  m_FLAC_StreamDecoderProcessUntilEndOfMetadata = (FLAC_StreamDecoderProcessUntilEndOfMetadata_t)FuppesGetProcAddress(m_LibHandle, "FLAC__stream_decoder_process_until_end_of_metadata");  
  if(!m_FLAC_StreamDecoderProcessUntilEndOfMetadata) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'FLAC__stream_decoder_process_until_end_of_metadata'", __FILE__, __LINE__);
    return false;
  }    
  
  m_FLAC_StreamDecoderProcessSingle = (FLAC_StreamDecoderProcessSingle_t)FuppesGetProcAddress(m_LibHandle, "FLAC__stream_decoder_process_single");  
  if(!m_FLAC_StreamDecoderProcessSingle) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'FLAC__stream_decoder_process_single'", __FILE__, __LINE__);
    return false;
  }    
  
  m_FLAC_StreamDecoderGetState = (FLAC_StreamDecoderGetState_t)FuppesGetProcAddress(m_LibHandle, "FLAC__stream_decoder_get_state");  
  if(!m_FLAC_StreamDecoderGetState) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'FLAC__stream_decoder_get_state'", __FILE__, __LINE__);
    return false;
  }    
  
  m_FLAC_StreamDecoderFinish = (FLAC_StreamDecoderFinish_t)FuppesGetProcAddress(m_LibHandle, "FLAC__stream_decoder_finish");  
  if(!m_FLAC_StreamDecoderFinish) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'FLAC__stream_decoder_finish'", __FILE__, __LINE__);
    return false;
  }    
  
  m_FLAC_StreamDecoderDelete = (FLAC_StreamDecoderDelete_t)FuppesGetProcAddress(m_LibHandle, "FLAC__stream_decoder_delete");  
  if(!m_FLAC_StreamDecoderDelete) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'FLAC__stream_decoder_delete'", __FILE__, __LINE__);
    return false;
  }    
  #endif // #ifndef HAVE_FLAC_FILEDECODER
  
  
  
  
  
  return true;
}

bool CFLACDecoder::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{
  #ifdef HAVE_FLAC_FILEDECODER
  m_pFLACFileDecoder = m_FLACFileDecoderNew();
  
  if(!m_FLACFileDecoderSetWriteCallback(m_pFLACFileDecoder, FLACFileDecoderWriteCallback)) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetWriteCallback" << endl;
    return false;
  }
  
  if(!m_FLACFileDecoderSetMetadataCallback(m_pFLACFileDecoder, FLACFileDecoderMetadataCallback)) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetMetadataCallback" << endl;
    return false;
  }
  
  if(!m_FLACFileDecoderSetErrorCallback(m_pFLACFileDecoder, FLACFileDecoderErrorCallback)) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetErrorCallback" << endl;
    return false;
  }
  
  if(!m_FLACFileDecoderSetClientData(m_pFLACFileDecoder, this)) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetClientData" << endl;
    return false; 
  }
  
  if(!m_FLACFileDecoderSetFilename(m_pFLACFileDecoder, p_sFileName.c_str())) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetFilename" << endl;
    return false; 
  }
  
  if(m_FLACFileDecoderInit(m_pFLACFileDecoder) != FLAC__FILE_DECODER_OK) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderInit" << endl;
    return false; 
  }
  
  if(!m_FLACFileDecoderProcessUntilEndOfMetadata(m_pFLACFileDecoder)) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderProcessUntilEndOfMetadata" << endl;
    return false; 
  }
  #endif // #ifdef HAVE_FLAC_FILEDECODER
  
    
  #ifndef HAVE_FLAC_FILEDECODER
  m_pFLAC_StreamDecoder = m_FLAC_StreamDecoderNew();

  m_FLAC_StreamDecoderInitFile(m_pFLAC_StreamDecoder, 
                               p_sFileName.c_str(), 
                               FLAC_StreamDecoderWriteCallback,
                               FLAC_StreamDecoderMetadataCallback,
                               FLAC_StreamDecoderErrorCallback, 
                               this);
  
  if(!m_FLAC_StreamDecoderProcessUntilEndOfMetadata(m_pFLAC_StreamDecoder)) {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLAC_StreamDecoderProcessUntilEndOfMetadata" << endl;
    return false; 
  }  
  #endif // #ifndef HAVE_FLAC_FILEDECODER
  
    
  pAudioDetails->nNumChannels   = m_pFLACData->channels;
  pAudioDetails->nBitRate       = m_pFLACData->bits_per_sample;    
  pAudioDetails->nNumPcmSamples = m_pFLACData->total_samples;
  pAudioDetails->nSampleRate    = m_pFLACData->sample_rate;
  
  m_bEOF = false;
  return true;
}

void CFLACDecoder::CloseFile()
{
  #ifdef HAVE_FLAC_FILEDECODER
  if(m_FLACFileDecoderDelete)
    m_FLACFileDecoderDelete(m_pFLACFileDecoder);
  #else
  if(m_FLAC_StreamDecoderDelete)
    m_FLAC_StreamDecoderDelete(m_pFLAC_StreamDecoder);
  #endif
}

long CFLACDecoder::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{
  m_pPcmOut = p_PcmOut;
    
  if(m_bEOF)
    return -1;
  
  #ifdef HAVE_FLAC_FILEDECODER
  if(m_FLACFileDecoderGetState(m_pFLACFileDecoder) == FLAC__FILE_DECODER_END_OF_FILE) {    
    m_FLACFileDecoderFinish(m_pFLACFileDecoder);
    m_bEOF = true;
    return m_nSamplesRead;        
  }
  #else
  if(m_FLAC_StreamDecoderGetState(m_pFLAC_StreamDecoder) == FLAC__STREAM_DECODER_END_OF_STREAM) {    
    m_FLAC_StreamDecoderFinish(m_pFLAC_StreamDecoder);
    m_bEOF = true;
    return m_nSamplesRead;
  }  
  #endif 
  
  #ifdef HAVE_FLAC_FILEDECODER
  if(m_FLACFileDecoderProcessSingle(m_pFLACFileDecoder)) {
  #else
  if(m_FLAC_StreamDecoderProcessSingle(m_pFLAC_StreamDecoder)) {
  #endif
    
  #warning todo: bytes read  
    
    *p_nBytesRead = m_nBytesConsumed;
    
    return m_nSamplesRead;
  }
  else
    return -1;
}

unsigned int CFLACDecoder::NumPcmSamples()
{
  //cout << "FLAC :: guess pcm length" << m_pFLACData->total_samples << endl;
  return m_pFLACData->total_samples;
}
  
#endif // HAVE_FLAC
#endif // DISABLE_TRANSCODING
