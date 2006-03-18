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

FLAC__StreamDecoderWriteStatus FLACFileDecoderWriteCallback(const FLAC__FileDecoder *decoder,
                                                            const FLAC__Frame* frame,
                                                            const FLAC__int32* const buffer[],
                                                            void *client_data)
{
  
}

void FLACFileDecoderMetadataCallback(const FLAC__StreamDecoder *decoder,
                                     const FLAC__StreamMetadata *metadata,
                                     void *client_data)
{
  
}

void FLACFileDecoderErrorCallback(const FLAC__StreamDecoder *decoder,
                                  FLAC__StreamDecoderErrorStatus status,
                                  void *client_data)
{

}


CFLACDecoder::CFLACDecoder()
{
  m_pFLACFileDecoder = NULL;
}

CFLACDecoder::~CFLACDecoder()
{
  if(m_pFLACFileDecoder)
    CloseFile();
}

bool CFLACDecoder::LoadLib()
{
  #ifdef WIN32  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening mpcdec.dll");
  m_LibHandle = FuppesLoadLibrary("mpcdec.dll");
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

  /* FLAC__bool FLAC__file_decoder_set_error_callback(FLAC__FileDecoder *decoder, FLAC__FileDecoderErrorCallback value) */
  typedef FLAC__bool (*FLACFileDecoderSetErrorCallback_t)(FLAC__FileDecoder*, FLAC__FileDecoderErrorCallback);
  
  /* FLAC__bool FLAC__file_decoder_set_client_data(FLAC__FileDecoder *decoder, void *value) */
  typedef FLAC__bool (*FLACFileDecoderSetClientData_t)(FLAC__FileDecoder*, void*);
  
  /* FLAC__FileDecoderState FLAC__file_decoder_init(FLAC__FileDecoder *decoder) */
  typedef FLAC__FileDecoderState (*FLACFileDecoderInit_t)(FLAC__FileDecoder*);
   
  /* FLAC__bool	FLAC__file_decoder_process_single(FLAC__FileDecoder *decoder) */
  typedef FLAC__bool (*FLACFileDecoderProcessSingle_t)(FLAC__FileDecoder*);
  
  /* FLAC__bool	FLAC__file_decoder_finish(FLAC__FileDecoder *decoder) */
  typedef FLAC__bool (*FLACFileDecoderFinish_t)(FLAC__FileDecoder*);  
  
  
  return true;
}

bool CFLACDecoder::OpenFile(std::string p_sFileName)
{
  m_pFLACFileDecoder = m_FLACFileDecoderNew();
}

void CFLACDecoder::CloseFile()
{
  m_FLACFileDecoderDelete(m_pFLACFileDecoder);
}

long CFLACDecoder::DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize)
{
}

#endif /* DISABLE_FLAC */
#endif /* DISABLE_TRANSCODING */
