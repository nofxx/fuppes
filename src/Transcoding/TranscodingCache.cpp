/***************************************************************************
 *            TranscodingCache.cpp
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

#include "TranscodingCache.h"

#include "LameWrapper.h"
#include "../Common/Common.h"

#ifndef DISABLE_VORBIS
#include "VorbisWrapper.h"
#endif

#ifndef DISABLE_MUSEPACK
#include "MpcWrapper.h"
#endif

#ifndef DISABLE_FLAC
#include "FlacWrapper.h"
#endif

#include <iostream>

using namespace std;

fuppesThreadCallback TranscodeThread(void *arg);

CTranscodingCacheObject::CTranscodingCacheObject()
{
  m_nRefCount       = 0;    
  m_szBuffer        = NULL;
  m_nBufferSize     = 0;  
  m_bIsTranscoding  = false;  
  m_TranscodeThread = (fuppesThread)NULL; 
  m_pLameWrapper    = NULL;
  m_pDecoder        = NULL;  
  m_bIsComplete     = false;
  m_pSessionInfo    = NULL;
  m_bBreakTranscoding = false;  
  
  fuppesThreadInitMutex(&m_Mutex);
}

CTranscodingCacheObject::~CTranscodingCacheObject()
{  
  /* break transcoding */
  if(m_bIsTranscoding)
  {
    m_bBreakTranscoding = true;
    fuppesThreadClose(m_TranscodeThread);
  }
  
  fuppesThreadDestroyMutex(&m_Mutex);  
  free(m_szBuffer);
  
  if(pcmout)
    delete[] pcmout;
}

bool CTranscodingCacheObject::Init(CTranscodeSessionInfo* pSessionInfo)
{
  CSharedLog::Shared()->Log(L_EXTENDED, "Init " + pSessionInfo->m_sInFileName, __FILE__, __LINE__);  
  m_pSessionInfo = pSessionInfo;  
  
  /* initialize LAME */
  if(!m_pLameWrapper)
  {
    m_pLameWrapper = new CLameWrapper();  
    if(!m_pLameWrapper->LoadLib())
    {      
      delete m_pLameWrapper;
      m_pLameWrapper = NULL;
      return false;
    }
    
    m_pLameWrapper->SetBitrate(LAME_BITRATE_320);
    m_pLameWrapper->Init();
  }  
  
  /* init decoder */
  if(!m_pDecoder)
  {  
    /* select decoder */
    std::string sExt = ToLower(ExtractFileExt(m_pSessionInfo->m_sInFileName));  
    #ifndef DISABLE_VORBIS
    if(sExt.compare("ogg") == 0)        
      m_pDecoder = new CVorbisDecoder();
    #endif  
    
    #ifndef DISABLE_MUSEPACK
    if(sExt.compare("mpc") == 0)      
      m_pDecoder = new CMpcDecoder();
    #endif 
    
    #ifndef DISABLE_FLAC    
    if(sExt.compare("flac") == 0)    
      m_pDecoder = new CFLACDecoder();   
    #endif  
    
    if(!m_pDecoder || !m_pDecoder->LoadLib() || !m_pDecoder->OpenFile(m_pSessionInfo->m_sInFileName))
    {
      delete m_pDecoder;
      m_pDecoder = NULL;      
      return false;
    }

    /* create pcm buffer */    
    #ifndef DISABLE_MUSEPACK  
    CMpcDecoder* pTmp = dynamic_cast<CMpcDecoder*>(m_pDecoder);  
    if (pTmp)
    {
      pcmout = new short int[MPC_DECODER_BUFFER_LENGTH * 4];
      nBufferLength = MPC_DECODER_BUFFER_LENGTH * 4;
    }
    else
    {
      pcmout = new short int[32768];  // 4096
      nBufferLength = 32768;
    }
    #else
    pcmout = new short int[32768];
    nBufferLength = 32768;
    #endif
  }    

  return true;
}

bool CTranscodingCacheObject::IsReleased()
{
  return (m_nRefCount == 0);
}

void CTranscodingCacheObject::Lock()
{
  fuppesThreadLockMutex(&m_Mutex);
}

void CTranscodingCacheObject::Unlock()
{
  fuppesThreadUnlockMutex(&m_Mutex);
}

unsigned int CTranscodingCacheObject::Transcode()
{  
  /* start new transcoding thread */
  if(!m_TranscodeThread && !m_bIsComplete)
  {   
    m_bIsTranscoding = true;
    fuppesThreadStartArg(m_TranscodeThread, TranscodeThread, *this);
    while(m_bIsTranscoding && (m_nBufferSize == 0))
      fuppesSleep(100);
    
    return m_nBufferSize;
  }
  
  /* transcoding is already running */
  if(m_bIsTranscoding)
  {    
    unsigned int nSize = m_nBufferSize;
    while(m_bIsTranscoding && (nSize == m_nBufferSize))
      fuppesSleep(100);
    if(m_bIsTranscoding)
      return m_nBufferSize;
  }
  
  /* object is already transcoded completely */  
  if(m_bIsComplete)    
    return m_nBufferSize;
}

int CTranscodingCacheObject::Append(char** p_pszBinBuffer, unsigned int p_nBinBufferSize)
{
  *p_pszBinBuffer = (char*)realloc(*p_pszBinBuffer, sizeof(char)*(m_nBufferSize));
  memcpy(*p_pszBinBuffer, m_szBuffer, m_nBufferSize);
    
  return m_nBufferSize;  
}

fuppesThreadCallback TranscodeThread(void *arg)
{  
  CTranscodingCacheObject* pCacheObj = (CTranscodingCacheObject*)arg;     
  CSharedLog::Shared()->Log(L_EXTENDED, "TranscodeThread :: " + pCacheObj->m_pSessionInfo->m_sInFileName, __FILE__, __LINE__);   
  
  long  samplesRead  = 0;    
  int   nLameRet     = 0;  
  unsigned int nAppendCount = 0;
  char* szTmpBuff = NULL;
  unsigned int nTmpSize = 0;
  
  /* Transcoding loop */
  while(((samplesRead = pCacheObj->m_pDecoder->DecodeInterleaved((char*)pCacheObj->pcmout, pCacheObj->nBufferLength)) >= 0) && !pCacheObj->m_bBreakTranscoding)
  {    
    /* encode */
    nLameRet = pCacheObj->m_pLameWrapper->EncodeInterleaved(pCacheObj->pcmout, samplesRead);
    if(nLameRet == 0)
      continue;
    
    /* (re-)allocate temporary buffer ... */
    if(!szTmpBuff)
      szTmpBuff = (char*)malloc(nLameRet * sizeof(char));
    else
      szTmpBuff = (char*)realloc(szTmpBuff, (nTmpSize + nLameRet) * sizeof(char));
    
    /* ... store encoded frames ... */
    memcpy(&szTmpBuff[nTmpSize], pCacheObj->m_pLameWrapper->GetMp3Buffer(), nLameRet);    
    
    /* ... and set new temporary buffer size */
    nTmpSize += nLameRet;
    
    nAppendCount++;
    
    
    /* append frames to the cache object's buffer */
    if((nAppendCount % 200) == 0)
    {      
      pCacheObj->Lock();
      
      if(!pCacheObj->m_szBuffer)
        pCacheObj->m_szBuffer = (char*)malloc(nTmpSize * sizeof(char));
      else
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->m_nBufferSize + nTmpSize) * sizeof(char));
      
      memcpy(&pCacheObj->m_szBuffer[pCacheObj->m_nBufferSize], szTmpBuff, nTmpSize);
      
      pCacheObj->m_nBufferSize += nTmpSize;
      
      pCacheObj->Unlock();

      free(szTmpBuff);
      szTmpBuff = NULL;
      nTmpSize = 0;
      nAppendCount = 0;
    }
    
  }
  
  /* transcoding loop exited */
  if(!pCacheObj->m_bBreakTranscoding)
  {
    /* append remaining frames */
    pCacheObj->Lock();
    
    if(!pCacheObj->m_szBuffer)
        pCacheObj->m_szBuffer = (char*)malloc(nTmpSize * sizeof(char));
      else
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->m_nBufferSize + nTmpSize) * sizeof(char));
      
    memcpy(&pCacheObj->m_szBuffer[pCacheObj->m_nBufferSize], szTmpBuff, nTmpSize);      
    pCacheObj->m_nBufferSize += nTmpSize;
    
    pCacheObj->Unlock();


    /* flush and end transcoding */

    /* flush mp3 */
    nTmpSize = pCacheObj->m_pLameWrapper->Flush();
    pCacheObj->Lock();
    
    if(!pCacheObj->m_szBuffer)
        pCacheObj->m_szBuffer = (char*)malloc(nTmpSize * sizeof(char));
      else
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->m_nBufferSize + nTmpSize) * sizeof(char));
    
    memcpy(&pCacheObj->m_szBuffer[pCacheObj->m_nBufferSize], pCacheObj->m_pLameWrapper->GetMp3Buffer(), nTmpSize);
    pCacheObj->m_nBufferSize += nTmpSize;
    
    pCacheObj->m_bIsComplete    = true;
    pCacheObj->m_bIsTranscoding = false;
      
    pCacheObj->Unlock();
  }
  
  /* delete temporary buffer */
  if(szTmpBuff)
    free(szTmpBuff);
  
  fuppesThreadExit();  
}



CTranscodingCache* CTranscodingCache::m_pInstance = 0;

CTranscodingCache* CTranscodingCache::Shared()
{
	if (m_pInstance == 0)
		m_pInstance = new CTranscodingCache();
	return m_pInstance;
}

CTranscodingCache::CTranscodingCache()
{
  fuppesThreadInitMutex(&m_Mutex); 
}

CTranscodingCache::~CTranscodingCache()
{
  fuppesThreadDestroyMutex(&m_Mutex);
}


CTranscodingCacheObject* CTranscodingCache::GetCacheObject(std::string p_sFileName)
{
  fuppesThreadLockMutex(&m_Mutex);
  
  CTranscodingCacheObject* pResult = NULL;  
  
  /* check if object exists */
  pResult = m_CachedObjects[p_sFileName];  
  if(!pResult)      
  {
    pResult = new CTranscodingCacheObject();    
    m_CachedObjects[p_sFileName] = pResult;
    pResult->m_sInFileName = p_sFileName;
  } 
    
  pResult->m_nRefCount++;
  
  fuppesThreadUnlockMutex(&m_Mutex);  
  
  return pResult;
}

void CTranscodingCache::ReleaseCacheObject(CTranscodingCacheObject* pCacheObj)
{
  fuppesThreadLockMutex(&m_Mutex);  
    
  pCacheObj->m_nRefCount--;  
  if(pCacheObj->m_nRefCount == 0)
  {  
    m_CachedObjectsIterator = m_CachedObjects.find(pCacheObj->m_sInFileName);
    if(m_CachedObjectsIterator != m_CachedObjects.end())
    {
      m_CachedObjects.erase(m_CachedObjectsIterator);
      delete pCacheObj;
    } 
  }

  fuppesThreadUnlockMutex(&m_Mutex);  
}

#endif // DISABLE_TRANSCODING
