/***************************************************************************
 *            MpcWrapper.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif
 
#ifndef DISABLE_TRANSCODING
#ifdef  HAVE_MUSEPACK
 
#ifndef _MPCWRAPPER_H
#define _MPCWRAPPER_H

#include <mpcdec/mpcdec.h>
#include <string>

#include "WrapperBase.h"

#ifdef __cplusplus
extern "C"
{
  /* void mpc_streaminfo_init (mpc_streaminfo* si);
     Initializes a streaminfo structure. */
  typedef void (*MpcStreaminfoInit_t)(mpc_streaminfo*);
  
  /* mpc_int32_t mpc_streaminfo_read(mpc_streaminfo* si, mpc_reader* r);
     Reads streaminfo header from the mpc stream supplied by r. */
  typedef mpc_int32_t (*MpcStreaminfoRead_t)(mpc_streaminfo*, mpc_reader*);
    
  /* void mpc_decoder_setup(mpc_decoder* d, mpc_reader* r);
     Sets up decoder library. Call this first when preparing to decode an mpc stream. */
  typedef void (*MpcDecoderSetup_t)(mpc_decoder*, mpc_reader*);
  
  /* mpc_bool_t mpc_decoder_initialize(mpc_decoder* d, mpc_streaminfo* si);
     Initializes mpc decoder with the supplied stream info parameters.
     Call this next after calling mpc_decoder_setup. */
  typedef mpc_bool_t (*MpcDecoderInitialize_t)(mpc_decoder*, mpc_streaminfo*);

  /* mpc_uint32_t mpc_decoder_decode(mpc_decoder* d, MPC_SAMPLE_FORMAT*	buffer,
                                     mpc_uint32_t* vbr_update_acc, mpc_uint32_t* vbr_update_bits);
     Actually reads data from previously initialized stream.
     Call this iteratively to decode the mpc stream. */
  typedef mpc_uint32_t (*MpcDecoderDecode_t)(mpc_decoder*, MPC_SAMPLE_FORMAT*, mpc_uint32_t*, mpc_uint32_t*);  
}
#endif

typedef struct reader_data_t {
    FILE *file;
    long size;
    mpc_bool_t seekable;
} reader_data;

class CMpcDecoder: public CAudioDecoderBase
{
  public:
    CMpcDecoder();
    virtual ~CMpcDecoder();
  
    bool LoadLib();  
  
    bool OpenFile(std::string p_sFileName);
    void CloseFile();
    long DecodeInterleaved(char* p_PcmOut, unsigned int p_nSize);
  
  private:
    reader_data    m_ReaderData;   
    mpc_decoder    m_Decoder;
    mpc_reader     m_Reader;
    mpc_streaminfo m_StreamInfo;
    
    fuppesLibHandle m_LibHandle;
  
    MpcStreaminfoInit_t    m_MpcStreaminfoInit;
    MpcStreaminfoRead_t    m_MpcStreaminfoRead;
    MpcDecoderSetup_t      m_MpcDecoderSetup;
    MpcDecoderInitialize_t m_MpcDecoderInitialize;
    MpcDecoderDecode_t     m_MpcDecoderDecode;    
};

#endif // _MPCWRAPPER_H

#endif // HAVE_MUSEPACK
#endif // DISABLE_TRANSCODING
