/***************************************************************************
 *            TranscodingMgr.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _TRANSCODINGMGR_H
#define _TRANSCODINGMGR_H
 
#include "TranscodingCache.h" 
#include "LameWrapper.h"
#include "WrapperBase.h"
#include <string>
 
/* error codes */
typedef enum tagTRANSCODING_MGR_ERROR_CODES
{
  TRANSCODING_MGR_OK = 0,
  TRANSCODING_MGR_FILE_NOT_FOUND = 1,
  TRANSCODING_MGR_AUDIO_ENCODER_MISSING = 2,
  TRANSCODING_MGR_AUDIO_DECODER_MISSING = 3
} TRANSCODING_MGR_ERROR_CODES;



class CTranscodeSessionInfo
{
  public:
    //CHTTPMessage* m_pHTTPMessage;
    bool        m_bBreakTranscoding;
    bool        m_bIsTranscoding;  
    std::string m_sFileName;
    unsigned int* m_pnBinContentLength;
    char**        m_pszBinBuffer;
};

class CTranscodingMgr
{
  public:
    CTranscodingMgr();
    ~CTranscodingMgr();
  
    TRANSCODING_MGR_ERROR_CODES Init(CTranscodeSessionInfo* p_SessionInfo);
  
    int Transcode();
    /** Append
     *  @param p_szBinBuffer  the buffer the content should be added to
     *  @param p_nBinBufferSize the current size of the buffer
     *  @param p_nOffset the append offset
     *  @return the number of byted appended to the buffer
     */
    int Append(char** p_pszBinBuffer, unsigned int p_nBinBufferSize, unsigned int p_nOffset);
  
  private:
    CTranscodeSessionInfo* m_pSessionInfo;
    CLameWrapper* m_pLameWrapper;
    CDecoderBase* m_pDecoder;
  
    CTranscodingCacheObject* m_pCacheObject;
  
    // buffer that stores the transcoded stuff
    //char* m_sAppendBuffer; 
    // the append buffer's size
    //unsigned int m_nAppendBufferSize;
    // append count
    unsigned int m_nAppendCount;
  
    bool m_bIsTranscoding;
  
    
};

#endif /* _TRANSCODINGMGR_H */
#endif /* DISABLE_TRANSCODING */
