/***************************************************************************
 *            FlacWrapper.cpp
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

#include "FlacWrapper.h"

const std::string LOGNAME = "FLACDecoder";

#include <sstream>
#include <iostream>
#include "../Common/Common.h"

using namespace std;

FLAC__StreamDecoderWriteStatus FLACFileDecoderWriteCallback(const FLAC__FileDecoder *decoder,
                                                            const FLAC__Frame* frame,
                                                            const FLAC__int32* const buffer[],
                                                            void *client_data)
{  
  if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_FRAME_NUMBER)
  {
    ((CFLACDecoder*)client_data)->m_pPcmOut        = (char*)buffer;    
    ((CFLACDecoder*)client_data)->m_nBytesReturned = frame->header.blocksize;    
  }
	else if (frame->header.number_type == FLAC__FRAME_NUMBER_TYPE_SAMPLE_NUMBER)
  {
    ((CFLACDecoder*)client_data)->m_nBytesReturned = frame->header.blocksize;   
    
    int i;
    int j;
    int k = 0;    
    
    for(j = 0; j < frame->header.blocksize; j++) 
    {
      for(i = 0; i < ((CFLACDecoder*)client_data)->m_pFLACData->channels; i++) 
      {
        /*int sample;
        sample = buffer[i][j];
        ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = sample && 0xFF;
        ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = sample >> 8;*/
      
        // flac fix from Qball Cow 2006-12-06
        FLAC__uint16 sample = buffer[i][j];
        unsigned char *uc = (unsigned char *)&sample;
      
        ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = *(uc++);
        ((CFLACDecoder*)client_data)->m_pPcmOut[k++] = *(uc);          
      }
    }
    
    return FLAC__STREAM_DECODER_WRITE_STATUS_CONTINUE;    
  }
	else 
  {
		CSharedLog::Shared()->Log(LOG_ERROR, "FLAC__STREAM_DECODER_WRITE_STATUS_ABORT", __FILE__, __LINE__);
		//dcd->error_occurred = true;
		return FLAC__STREAM_DECODER_WRITE_STATUS_ABORT;
	}	
}

void FLACFileDecoderMetadataCallback(const FLAC__FileDecoder *decoder,
                                     const FLAC__StreamMetadata *metadata,
                                     void *client_data)
{
  ((CFLACDecoder*)client_data)->m_pFLACData->total_samples   = metadata->data.stream_info.total_samples;
  ((CFLACDecoder*)client_data)->m_pFLACData->channels        = metadata->data.stream_info.channels;
  ((CFLACDecoder*)client_data)->m_pFLACData->bits_per_sample = metadata->data.stream_info.bits_per_sample;  
  
  fuppesSleep(1);
}

void FLACFileDecoderErrorCallback(const FLAC__FileDecoder *decoder,
                                  FLAC__StreamDecoderErrorStatus status,
                                  void *client_data)
{
  /*cout << "FLACFileDecoderErrorCallback" << endl;
  fflush(stdout);*/
}

CFLACDecoder::CFLACDecoder()
{
  m_pFLACFileDecoder = NULL;
  
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
  
  if(m_pFLACFileDecoder)
    CloseFile();
  
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
}

bool CFLACDecoder::LoadLib()
{  
  #ifdef WIN32  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening libFLAC.dll");
  m_LibHandle = FuppesLoadLibrary("libFLAC.dll");
  #else
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening libFLAC.so");
  m_LibHandle = FuppesLoadLibrary("libFLAC.so");   
  #endif
  if(!m_LibHandle)
  {
    std::stringstream sLog;
    sLog << "cannot open library";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }   
   
  m_FLACFileDecoderNew = (FLACFileDecoderNew_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_new");  
  if(!m_FLACFileDecoderNew)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_new'");
    return false;
  }    
  
  m_FLACFileDecoderDelete = (FLACFileDecoderDelete_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_delete");
  if(!m_FLACFileDecoderDelete)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_delete'");
    return false;  
  }

  m_FLACFileDecoderSetFilename = (FLACFileDecoderSetFilename_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_filename");
  if(!m_FLACFileDecoderSetFilename)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_filename'");
    return false;    
  }
  
  m_FLACFileDecoderSetWriteCallback = (FLACFileDecoderSetWriteCallback_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_write_callback");
  if(!m_FLACFileDecoderSetWriteCallback)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_write_callback'");
    return false;      
  }

  m_FLACFileDecoderSetMetadataCallback = (FLACFileDecoderSetMetadataCallback_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_metadata_callback");
  if(!m_FLACFileDecoderSetMetadataCallback)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_metadata_callback'");
    return false;
  }

  m_FLACFileDecoderSetErrorCallback = (FLACFileDecoderSetErrorCallback_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_error_callback");
  if(!m_FLACFileDecoderSetErrorCallback)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_error_callback'");
    return false;
  }
  
  m_FLACFileDecoderSetClientData = (FLACFileDecoderSetClientData_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_set_client_data");
  if(!m_FLACFileDecoderSetClientData)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_set_client_data'");
    return false;
  }

  m_FLACFileDecoderInit = (FLACFileDecoderInit_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_init");
  if(!m_FLACFileDecoderInit)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_init'");
    return false;
  }
  
  m_FLACFileDecoderProcessUntilEndOfMetadata = (FLACFileDecoderProcessUntilEndOfMetadata_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_process_until_end_of_metadata");
  if(!m_FLACFileDecoderProcessUntilEndOfMetadata)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_process_until_end_of_metadata'");
    return false;
  }
  
  m_FLACFileDecoderProcessSingle = (FLACFileDecoderProcessSingle_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_process_single");
  if(!m_FLACFileDecoderProcessSingle)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_process_single'");
    return false;
  }
  
  
  m_FLACFileDecoderGetState = (FLACFileDecoderGetState_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_get_state");
  if(!m_FLACFileDecoderGetState)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_get_state'");
    return false;
  }
  
  
  m_FLACFileDecoderFinish = (FLACFileDecoderFinish_t)FuppesGetProcAddress(m_LibHandle, "FLAC__file_decoder_finish");
  if(!m_FLACFileDecoderFinish)
  {
    CSharedLog::Shared()->Warning(LOGNAME, "cannot load symbol 'FLAC__file_decoder_finish'");
    return false;
  }
  
  return true;
}

bool CFLACDecoder::OpenFile(std::string p_sFileName)
{
  m_pFLACFileDecoder = m_FLACFileDecoderNew();
  
  if(!m_FLACFileDecoderSetWriteCallback(m_pFLACFileDecoder, FLACFileDecoderWriteCallback))
  {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetWriteCallback" << endl;
    return false;
  }
  
  if(!m_FLACFileDecoderSetMetadataCallback(m_pFLACFileDecoder, FLACFileDecoderMetadataCallback))
  {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetMetadataCallback" << endl;
    return false;
  }
  
  if(!m_FLACFileDecoderSetErrorCallback(m_pFLACFileDecoder, FLACFileDecoderErrorCallback))
  {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetErrorCallback" << endl;
    return false;
  }
  
  if(!m_FLACFileDecoderSetClientData(m_pFLACFileDecoder, this))
  {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetClientData" << endl;
    return false; 
  }
  
  if(!m_FLACFileDecoderSetFilename(m_pFLACFileDecoder, p_sFileName.c_str()))
  {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderSetFilename" << endl;
    return false; 
  }
  
  if(m_FLACFileDecoderInit(m_pFLACFileDecoder) != FLAC__FILE_DECODER_OK)
  {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderInit" << endl;
    return false; 
  }
  
  if(!m_FLACFileDecoderProcessUntilEndOfMetadata(m_pFLACFileDecoder))
  {
    cout << "[ERROR] CFLACDecoder::OpenFile() - FLACFileDecoderProcessUntilEndOfMetadata" << endl;
    return false; 
  }
  
  m_bEOF = false;
  return true;
}

void CFLACDecoder::CloseFile()
{
  if(m_FLACFileDecoderDelete)
    m_FLACFileDecoderDelete(m_pFLACFileDecoder);
}

long CFLACDecoder::DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize)
{
  m_pPcmOut = p_PcmOut;
    
  if(m_bEOF)
    return -1;
  
  if (m_FLACFileDecoderGetState(m_pFLACFileDecoder) == FLAC__FILE_DECODER_END_OF_FILE)
  {    
    m_FLACFileDecoderFinish(m_pFLACFileDecoder);
    m_bEOF = true;
    return m_nBytesReturned;        
  }
  
  if(m_FLACFileDecoderProcessSingle(m_pFLACFileDecoder))  
    return m_nBytesReturned;  
  else
    return -1;
}

#endif /* DISABLE_FLAC */
#endif /* DISABLE_TRANSCODING */
