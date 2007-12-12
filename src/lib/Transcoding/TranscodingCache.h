/***************************************************************************
 *            TranscodingCache.h
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

#ifndef _TRANSCODINGCACHE_H
#define _TRANSCODINGCACHE_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifndef DISABLE_TRANSCODING
#include "../Common/Common.h"
#include "WrapperBase.h"
#include "../DeviceSettings/DeviceSettings.h"
#include <map>
#endif

#include <string>


#ifndef DISABLE_TRANSCODING
class CTranscodingCacheObject
{
  public:
    CTranscodingCacheObject();
    ~CTranscodingCacheObject();
  
    bool Init(CTranscodeSessionInfo* pSessionInfo, CDeviceSettings* pDeviceSettings);

    bool IsReleased();
  
    void Lock();
    void Unlock();
    bool Locked() { return m_bLocked; }
  
    // the buffer that stores the transcoded bytes
    char* m_sBuffer; 
    // valid bytes in buffer
    unsigned int m_nValidBytes;
    
    // the buffer's size
    unsigned int m_nBufferSize;  
  
  
    unsigned int GetValidBytes();
  
    bool TranscodeToFile();
  
    bool  m_bIsTranscoding;
    bool  m_bBreakTranscoding;
    void* m_pTranscodingMgr;
  
    bool  m_bIsComplete;
    bool  m_bInitialized;
  
    unsigned int Transcode(CDeviceSettings* pDeviceSettings);
    //int Append(char** p_pszBinBuffer, unsigned int p_nBinBufferSize);
  
    void GetId3v1(char buffer[128]);
    bool IsMp3Encoding();
  //private:
    unsigned int m_nRefCount;
    
    unsigned int ReleaseCount()
      { return m_nReleaseCnt; }
  
    void ReleaseCount(unsigned int p_nCnt) 
      { 
        if(m_nReleaseCnt < p_nCnt) {
          m_nReleaseCnt = p_nCnt;
          m_nReleaseCntBak = m_nReleaseCnt;
        }
      }
  
    void DecReleaseCount()
      { m_nReleaseCnt--; }
  
    void ResetReleaseCount() 
      { m_nReleaseCnt = m_nReleaseCntBak; }
 
		unsigned int ReleaseCountOrig() { return m_nReleaseCntBak; }
		
    fuppesThreadMutex  m_Mutex;

    CAudioEncoderBase* m_pAudioEncoder;
    CAudioDecoderBase* m_pDecoder;    
    //CAudioDetails      m_AudioDetails;
    
    CTranscoderBase*   m_pTranscoder;
  
    //CTranscodeSessionInfo* m_pSessionInfo;          
  
    unsigned int nPcmBufferSize;  
    short int*   m_pPcmOut;
    
//  private:
    std::string m_sInFileName;
    std::string m_sOutFileName;
    fuppesThread m_TranscodeThread;
  
    bool Threaded() { return m_bThreaded; }
  
    CDeviceSettings* DeviceSettings() { return m_pDeviceSettings; }
  
   private:

    bool m_bLocked;
    
    bool m_bThreaded;
  
    unsigned int m_nReleaseCnt;
    unsigned int m_nReleaseCntBak;
  
    CDeviceSettings* m_pDeviceSettings;
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


    fuppesThreadMutex  m_Mutex; 
    std::map<std::string, CTranscodingCacheObject*>           m_CachedObjects;
    std::map<std::string, CTranscodingCacheObject*>::iterator m_CachedObjectsIterator;
  
  private:
    fuppesThread       m_ReleaseThread;
};
#endif // DISABLE_TRANSCODING
#endif // _TRANSCODINGCACHE_H
