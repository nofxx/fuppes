/***************************************************************************
 *            MpcWrapper.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

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
 
#ifndef _MPCWRAPPER_H
#define _MPCWRAPPER_H

#include <mpcdec/mpcdec.h>
#include <dlfcn.h>
#include <string>

#include "WrapperBase.h"

#ifdef __cplusplus
extern "C"
{

}
#endif

typedef struct reader_data_t {
    FILE *file;
    long size;
    mpc_bool_t seekable;
} reader_data;

class CMpcDecoder: public CDecoderBase
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
    
    
};

#endif /* _MPCWRAPPER_H */

#endif /* DISABLE_MUSEPACK */
#endif /* DISABLE_TRANSCODING */
