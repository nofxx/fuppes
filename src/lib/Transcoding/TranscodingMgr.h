/***************************************************************************
 *            TranscodingMgr.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006, 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifndef _TRANSCODINGMGR_H
#define _TRANSCODINGMGR_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "WrapperBase.h"
#include <string>

class CTranscodingMgr
{
  public:
    static CTranscodingMgr* Shared();
  
    bool IsTranscodingAvailable();
  
    bool IsLameAvailable();
    bool IsTwoLameAvailable();
  
    bool IsVorbisAvailabe();
    bool IsFlacAvailable();
    bool IsMusePackAvailabe();
  
    //bool IsTranscodingExtension(std::string p_sFileExt);
    //TRANSCODING_TYPE GetTranscodingType(CDeviceSettings* pDeviceSettings);
  
    void SetDoTranscodeVorbis(bool p_bDoTranscodeVorbis);
    bool GetDoTranscodeVorbis() { return m_bTranscodeVorbis; }
    
    void SetDoTranscodeFlac(bool p_bDoTranscodeFlac);
    bool GetDoTranscodeFlac() { return m_bTranscodeFlac; }
    
    void SetDoTranscodeMusePack(bool p_bDoTranscodeMusePack);
    bool GetDoTranscodeMusePack() { return m_bTranscodeMusePack; }
    
    void SetDoUseLame(bool p_bDoUseLame);
    bool GetDoUseLame() { return m_bUseLame; }
    
    
  
    void PrintTranscodingSettings(std::string* p_sHTMLVersion = NULL);
    
    CAudioEncoderBase* CreateAudioEncoder(ENCODER_TYPE p_nEncoderType);
  
    CAudioDecoderBase* CreateAudioDecoder(DECODER_TYPE p_nDecoderType, unsigned int* p_nBufferSize);
  
    CTranscoderBase* CreateTranscoder(TRANSCODER_TYPE p_nTranscoderType);
  
  private:
    CTranscodingMgr();
    ~CTranscodingMgr();
  
    static CTranscodingMgr* m_Instance;
  
    bool m_bLameAvailable;
    bool m_bTwoLameAvailable;
    bool m_bVorbisAvailable;
    bool m_bFlacAvailable;
    bool m_bMusePackAvailable;
  
    bool m_bTranscodeVorbis;
    bool m_bTranscodeFlac;
    bool m_bTranscodeMusePack;
  
    bool m_bUseLame;
};

#endif // _TRANSCODINGMGR_H
