/***************************************************************************
 *            TranscodingCache.h
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

#ifndef _TRANSCODINGCACHE_H
#define _TRANSCODINGCACHE_H

#ifndef DISABLE_TRANSCODING
#include "../Common/Common.h"
#include "LameWrapper.h"
#include "WrapperBase.h"
#include <map>
#endif

#include <string>

class CTranscodeSessionInfo
{
  public:
    //CHTTPMessage* m_pHTTPMessage;
    bool          m_bBreakTranscoding;
    bool          m_bIsTranscoding;
    std::string   m_sInFileName;
    unsigned int* m_pnBinContentLength;
    char**        m_pszBinBuffer;
};

#ifndef DISABLE_TRANSCODING
class CTranscodingCacheObject
{
  public:
    CTranscodingCacheObject();
    ~CTranscodingCacheObject();
  
    bool Init(CTranscodeSessionInfo* pSessionInfo);
  
    //void Retain();
    //void Release();
  
    bool IsReleased();
  
    void Lock();
    void Unlock();
  
    // the buffer that stores the transcoded bytes
    char* m_szBuffer; 
    // the buffer's size
    unsigned int m_nBufferSize;
  
    bool m_bIsTranscoding;
    bool m_bBreakTranscoding;
    void* m_pTranscodingMgr;
  
    bool  m_bIsComplete;
  
    unsigned int Transcode();
    int Append(char** p_pszBinBuffer, unsigned int p_nBinBufferSize);
  
  //private:
    unsigned int m_nRefCount;
    fuppesThreadMutex  m_Mutex;
        
    CLameWrapper* m_pLameWrapper;
    CDecoderBase* m_pDecoder;    
    
    
    CTranscodeSessionInfo* m_pSessionInfo;    
    
    int nBufferLength;  
    short int* pcmout;
    
//  private:
    std::string m_sInFileName;
    fuppesThread m_TranscodeThread;
};

class CTranscodingCache
{
  protected:
		CTranscodingCache();
  
	public:
    ~CTranscodingCache();
    static CTranscodingCache* Shared();
  
  private:
    static CTranscodingCache* m_pInstance;
 
  
  public:
    CTranscodingCacheObject* GetCacheObject(std::string p_sFileName);
    void ReleaseCacheObject(CTranscodingCacheObject* pCacheObj);

  
  private:
    fuppesThreadMutex  m_Mutex;
  
    std::map<std::string, CTranscodingCacheObject*>           m_CachedObjects;
    std::map<std::string, CTranscodingCacheObject*>::iterator m_CachedObjectsIterator;  
  
  
};
#endif /* DISABLE_TRANSCODING */

#endif /* _TRANSCODINGCACHE_H */
