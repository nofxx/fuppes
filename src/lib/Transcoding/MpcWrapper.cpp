/***************************************************************************
 *            MpcWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*  Some code parts are taken from the
 *  libmpcdec sources and the bmp-musepack plugin   
 *  Copyright (c) 2005, The Musepack Development Team
 *  All rights reserved. 
 */

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
#include "MpcWrapper.h"
#ifdef HAVE_MUSEPACK

#include "../SharedConfig.h"
#include <iostream>
#include <sstream>
using namespace std;

const std::string LOGNAME = "MpcDecoder";

/* callback functions for the mpc_reader */
mpc_int32_t
read_impl(void *data, void *ptr, mpc_int32_t size)
{
    reader_data *d = (reader_data *) data;
    return fread(ptr, 1, size, d->file);
}

mpc_bool_t
seek_impl(void *data, mpc_int32_t offset)
{
    reader_data *d = (reader_data *) data;
    return d->seekable ? !fseek(d->file, offset, SEEK_SET) : false;
}

mpc_int32_t
tell_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return ftell(d->file);
}

mpc_int32_t
get_size_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return d->size;
}

mpc_bool_t
canseek_impl(void *data)
{
    reader_data *d = (reader_data *) data;
    return d->seekable;
}

#ifdef MPC_FIXED_POINT
static int shift_signed(MPC_SAMPLE_FORMAT val, int shift)
{
  if (shift > 0)
    val <<= shift;
  else if (shift < 0)  
    val >>= -shift;
  return (int)val;
}
#endif

void CMpcDecoder::convertLE32to16(MPC_SAMPLE_FORMAT* sample_buffer, char *xmms_buffer, unsigned int status, unsigned int* nBytesConsumed)
{
  unsigned int m_bps = 16; //output on 16 bits
  unsigned n;
  int clip_min = -1 << (m_bps - 1),
  clip_max = (1 << (m_bps - 1)) - 1, float_scale = 1 << (m_bps - 1);
  
  for (n = 0; n < 2 * status; n++) 
  {
    int val;
    
    #ifdef MPC_FIXED_POINT
    val = shift_signed(sample_buffer[n],
    m_bps - MPC_FIXED_POINT_SCALE_SHIFT);  
    #else
    val = (int)(sample_buffer[n] * float_scale);  
    #endif
  
    if (val < clip_min)
      val = clip_min;
    else if (val > clip_max)  
      val = clip_max;

    unsigned shift = 0;
    if(m_nOutEndianess == E_BIG_ENDIAN) {
      shift = 8;
    }
    
    unsigned offset = 0;
    do {
      //xmms_buffer[n * 2 + (shift / 8)] = (unsigned char)((val >> shift) & 0xFF);
      
      if(m_nOutEndianess == E_LITTLE_ENDIAN) {
        xmms_buffer[n * 2 + offset] = (unsigned char)((val >> shift) & 0xFF);
      }
      else if(m_nOutEndianess == E_BIG_ENDIAN) {
        if(offset % 2 == 0) 
          xmms_buffer[n * 2 + offset] = (unsigned char)((val >> shift) & 0xFF);
        else
          xmms_buffer[n * 2 + offset] = (unsigned char)(val & 0xFF);
      }
      
      shift += 8;
      offset++;
    } while (shift < m_bps);
  }
  
  *nBytesConsumed = n;
}

/* constructor */
CMpcDecoder::CMpcDecoder()
{
}

/* destructor */
CMpcDecoder::~CMpcDecoder()
{
  if(m_LibHandle)
    FuppesCloseLibrary(m_LibHandle);
}

bool CMpcDecoder::LoadLib()
{
  #ifdef WIN32
  std::string sLibName = "libmpcdec-5.dll";
  #else  
  std::string sLibName = "libmpcdec.so";
  #endif
  
  if(!CSharedConfig::Shared()->MpcLibName().empty()) {
    sLibName = CSharedConfig::Shared()->MpcLibName();
  }  
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "try opening " + sLibName);
  m_LibHandle = FuppesLoadLibrary(sLibName);  
  
  
  if(!m_LibHandle)
  {
    stringstream sLog;
    sLog << "cannot open library";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }   
   
  m_MpcStreaminfoInit = (MpcStreaminfoInit_t)FuppesGetProcAddress(m_LibHandle, "mpc_streaminfo_init");
  if(!m_MpcStreaminfoInit)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'mpc_streaminfo_init'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }  
  
  m_MpcStreaminfoRead = (MpcStreaminfoRead_t)FuppesGetProcAddress(m_LibHandle, "mpc_streaminfo_read");
  if(!m_MpcStreaminfoRead)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'mpc_streaminfo_read'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  }  
  
  m_MpcStreaminfoGetLengthSamples = (MpcStreaminfoGetLengthSamples_t)FuppesGetProcAddress(m_LibHandle, "mpc_streaminfo_get_length_samples");
  if(!m_MpcStreaminfoGetLengthSamples) {
    CSharedLog::Shared()->Log(L_EXT, "cannot load symbol 'mpc_streaminfo_get_length_samples'", __FILE__, __LINE__);
  }  
  
  m_MpcDecoderSetup = (MpcDecoderSetup_t)FuppesGetProcAddress(m_LibHandle, "mpc_decoder_setup");
  if(!m_MpcDecoderSetup)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'mpc_decoder_setup'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
  
  m_MpcDecoderInitialize = (MpcDecoderInitialize_t)FuppesGetProcAddress(m_LibHandle, "mpc_decoder_initialize");
  if(!m_MpcDecoderInitialize)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'mpc_decoder_initialize'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
  
  m_MpcDecoderDecode = (MpcDecoderDecode_t)FuppesGetProcAddress(m_LibHandle, "mpc_decoder_decode");
  if(!m_MpcDecoderDecode)
  {
    stringstream sLog;
    sLog << "cannot load symbol 'mpc_decoder_decode'";
    CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
    return false;
  } 
  
  return true;
}

bool CMpcDecoder::OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails)
{
  FILE *input = fopen(p_sFileName.c_str(), "rb");
  if(input == 0) 
  {
    printf("Error opening input file: \"%s\"\n", p_sFileName.c_str());
    return false;
  }

  /* initialize our reader_data tag the reader will carry around with it */  
  m_ReaderData.file = input;
  m_ReaderData.seekable = true;
  fseek(m_ReaderData.file, 0, SEEK_END);
  m_ReaderData.size = ftell(m_ReaderData.file);
  fseek(m_ReaderData.file, 0, SEEK_SET);

  /* set up an mpc_reader linked to our function implementations */  
  m_Reader.read = read_impl;
  m_Reader.seek = seek_impl;
  m_Reader.tell = tell_impl;
  m_Reader.get_size = get_size_impl;
  m_Reader.canseek = canseek_impl;
  m_Reader.data = &m_ReaderData;

  /* read file's streaminfo data */  
  m_MpcStreaminfoInit(&m_StreamInfo);
  if (m_MpcStreaminfoRead(&m_StreamInfo, &m_Reader) != ERROR_CODE_OK) {
      printf("Not a valid musepack file: \"%s\"\n", p_sFileName.c_str());
      return false;
  }

  /* instantiate a decoder with our file reader */
  m_MpcDecoderSetup(&m_Decoder, &m_Reader);
  if (!m_MpcDecoderInitialize(&m_Decoder, &m_StreamInfo)) {
      printf("Error initializing decoder.\n"); //, p_sFileName.c_str());
      return false;
  }  
  
  return true;
}

void CMpcDecoder::CloseFile()
{
}

long CMpcDecoder::DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{
  MPC_SAMPLE_FORMAT sample_buffer[MPC_DECODER_BUFFER_LENGTH];
  
  if(p_nBufferSize < MPC_DECODER_BUFFER_LENGTH)
  {
    CSharedLog::Shared()->Error(LOGNAME, "bufer size too small for mpc decoding");
    return -1;
  } 

  unsigned status = m_MpcDecoderDecode(&m_Decoder, sample_buffer, 0, 0);
  if (status == (unsigned)(-1)) 
  {
    //decode error
    printf("Error decoding file.\n");
    return -1;
  }
  else if (status == 0)   // EOF
  {
    return -1;    
  }
  else                    // status>0
  {
    #warning todo: bytes read
    unsigned int nBytesConsumed = 0;
    *p_nBytesRead = nBytesConsumed;
    convertLE32to16(sample_buffer, p_PcmOut, status, &nBytesConsumed);
    return status;
  }
}

unsigned int CMpcDecoder::NumPcmSamples()
{
  if(m_MpcStreaminfoGetLengthSamples) {
    return m_MpcStreaminfoGetLengthSamples(&m_StreamInfo);
  }
  else {
    return 0;
  }
}

#endif // HAVE_MUSEPACK
#endif // DISABLE_TRANSCODING
