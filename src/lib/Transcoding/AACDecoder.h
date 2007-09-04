/***************************************************************************
 *            AACDecoder.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#ifdef  HAVE_FAAD
 
#ifndef _AACDECODER_H
#define _AACDECODER_H

#include "WrapperBase.h"
#include <faad.h>

#ifdef __cplusplus
extern "C"
{
  
}
#endif // __cplusplus


class CAACDecoder: public CAudioDecoderBase
{
  public:
    CAACDecoder();
    virtual ~CAACDecoder();
  
    bool LoadLib();
    bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails);
    void CloseFile();
    long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead);

    unsigned int NumPcmSamples();
};

#endif // _AACDECODER_H
#endif // HAVE_FAAD
#endif // DISABLE_TRANSCODING
