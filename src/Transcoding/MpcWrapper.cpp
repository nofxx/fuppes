/***************************************************************************
 *            MpcWrapper.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*  Some code parts are taken from the
 *  libmpcdec sources and the bmp-musepack plugin   
 *  Copyright (c) 2005, The Musepack Development Team
 *  All rights reserved. 
 */

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef DISABLE_TRANSCODING
#ifndef DISABLE_MUSEPACK

#include "MpcWrapper.h"

#include <iostream>
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

static void convertLE32to16(MPC_SAMPLE_FORMAT* sample_buffer, char *xmms_buffer, unsigned int status)
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
    do {
      xmms_buffer[n * 2 + (shift / 8)] = (unsigned char)((val >> shift) & 0xFF);
      shift += 8;
    } while (shift < m_bps);
  }
}

/* constructor */
CMpcDecoder::CMpcDecoder()
{
}

/* destructor */
CMpcDecoder::~CMpcDecoder()
{
}

bool CMpcDecoder::LoadLib()
{
  return true;
}

bool CMpcDecoder::OpenFile(std::string p_sFileName)
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
  mpc_streaminfo_init(&m_StreamInfo);
  if (mpc_streaminfo_read(&m_StreamInfo, &m_Reader) != ERROR_CODE_OK) {
      printf("Not a valid musepack file: \"%s\"\n", p_sFileName.c_str());
      return false;
  }

  /* instantiate a decoder with our file reader */
  mpc_decoder_setup(&m_Decoder, &m_Reader);
  if (!mpc_decoder_initialize(&m_Decoder, &m_StreamInfo)) {
      printf("Error initializing decoder.\n"); //, p_sFileName.c_str());
      return false;
  }  
  
  return true;
}

void CMpcDecoder::CloseFile()
{
}

long CMpcDecoder::DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize)
{
  MPC_SAMPLE_FORMAT sample_buffer[MPC_DECODER_BUFFER_LENGTH];
  
  if(p_nSize < MPC_DECODER_BUFFER_LENGTH)
  {
    CSharedLog::Shared()->Error(LOGNAME, "bufer size too small for mpc decoding");
    return -1;
  } 

  unsigned status = mpc_decoder_decode(&m_Decoder, sample_buffer, 0, 0);
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
    convertLE32to16(sample_buffer, p_PcmOut, status);
    return status;
  }
}

#endif /* DISABLE_MUSEPACK */
#endif /* DISABLE_TRANSCODING */
