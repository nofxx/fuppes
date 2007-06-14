/***************************************************************************
 *            TranscodingCache.cpp
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
 
#ifndef DISABLE_TRANSCODING

#include "TranscodingCache.h"
#include "TranscodingMgr.h"

#include "../Common/Common.h"
#include "../SharedLog.h"

#include <iostream>

using namespace std;

fuppesThreadCallback TranscodeThread(void *arg);

CTranscodingCacheObject::CTranscodingCacheObject()
{
  m_nRefCount       = 0;    
  m_szBuffer        = NULL;
  m_pPcmOut         = NULL;
  m_nBufferSize     = 0;  
  m_bIsTranscoding  = false;  
  m_TranscodeThread = (fuppesThread)NULL; 
  m_pAudioEncoder   = NULL;
  m_pDecoder        = NULL;  
  m_bIsComplete     = false;
  m_pSessionInfo    = NULL;
  m_bBreakTranscoding = false;  
  
  fuppesThreadInitMutex(&m_Mutex);
}

CTranscodingCacheObject::~CTranscodingCacheObject()
{
  // break transcoding
  if(m_bIsTranscoding) {                    
    m_bBreakTranscoding = true;
  }
  
  if(m_TranscodeThread) {
    fuppesThreadClose(m_TranscodeThread);                        
  }
    
  fuppesThreadDestroyMutex(&m_Mutex);  
  free(m_szBuffer);
  
  if(m_pPcmOut)
    delete[] m_pPcmOut;
    
  delete m_pAudioEncoder;
  delete m_pDecoder;
}

bool CTranscodingCacheObject::Init(CTranscodeSessionInfo* pSessionInfo)
{
  CSharedLog::Shared()->Log(L_EXTENDED, "Init " + pSessionInfo->m_sInFileName, __FILE__, __LINE__);  
  m_pSessionInfo = pSessionInfo;  
  
  
  // init decoder
  if(!m_pDecoder)
  {  
    // create decoder
    nBufferLength = 32768;
    std::string sExt = ToLower(ExtractFileExt(m_pSessionInfo->m_sInFileName));  
    
    m_pDecoder = CTranscodingMgr::Shared()->CreateAudioDecoder(sExt, &nBufferLength);
    
    // init decoder
    if(!m_pDecoder || !m_pDecoder->LoadLib() || !m_pDecoder->OpenFile(m_pSessionInfo->m_sInFileName, &m_AudioDetails)) {
      delete m_pDecoder;
      m_pDecoder = NULL;      
      return false;
    }

    // create pcm buffer
    m_pPcmOut = new short int[nBufferLength];
  }  
  
  
  // init encoder
  if(!m_pAudioEncoder) {
    
    m_pAudioEncoder = CTranscodingMgr::Shared()->CreateAudioEncoder("mp3");
    if(!m_pAudioEncoder->LoadLib()) {      
      delete m_pAudioEncoder;
      m_pAudioEncoder = NULL;
      return false;
    }
    
    #warning todo: set encoder bitrate from config
    //m_pLameWrapper->SetBitrate(LAME_BITRATE_320);
    m_pAudioEncoder->SetAudioDetails(&m_AudioDetails);
    m_pAudioEncoder->Init();
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

	return 0;
}

int CTranscodingCacheObject::Append(char** p_pszBinBuffer, unsigned int p_nBinBufferSize)
{
  Lock();   
  *p_pszBinBuffer = (char*)realloc(*p_pszBinBuffer, sizeof(char)*(m_nBufferSize));  
  memcpy(*p_pszBinBuffer, m_szBuffer, m_nBufferSize);    
  Unlock();
    
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
  int   nBytesRead = 0;
  
  /* Transcoding loop */
  while(((samplesRead = pCacheObj->m_pDecoder->DecodeInterleaved((char*)pCacheObj->m_pPcmOut, pCacheObj->nBufferLength, &nBytesRead)) >= 0) && !pCacheObj->m_bBreakTranscoding)
  {    
    /* encode */
    nLameRet = pCacheObj->m_pAudioEncoder->EncodeInterleaved(pCacheObj->m_pPcmOut, samplesRead, nBytesRead);
    if(nLameRet == 0)
      continue;
    
    /* (re-)allocate temporary buffer ... */
    if(!szTmpBuff)
      szTmpBuff = (char*)malloc(nLameRet * sizeof(char));
    else
      szTmpBuff = (char*)realloc(szTmpBuff, (nTmpSize + nLameRet) * sizeof(char));
    
    /* ... store encoded frames ... */
    memcpy(&szTmpBuff[nTmpSize], pCacheObj->m_pAudioEncoder->GetEncodedBuffer(), nLameRet);    
    
    /* ... and set new temporary buffer size */
    nTmpSize += nLameRet;
    
    nAppendCount++;
    
    #define APPEND_BUFFER_SIZE 65536 // 64 kb
    
    /* append frames to the cache-object's buffer */
    if(((nAppendCount % 50) == 0) || (nTmpSize >= APPEND_BUFFER_SIZE))
    {      
      pCacheObj->Lock();
      
      if(!pCacheObj->m_szBuffer)
        pCacheObj->m_szBuffer = (char*)malloc(nTmpSize * sizeof(char));
      else
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->m_nBufferSize + nTmpSize) * sizeof(char));
      
      memcpy(&pCacheObj->m_szBuffer[pCacheObj->m_nBufferSize], szTmpBuff, nTmpSize);
      
      //cout << "buffer size: " << pCacheObj->m_nBufferSize << " + " << nTmpSize << " = "; fflush(stdout);
      pCacheObj->m_nBufferSize += nTmpSize;
      //cout << pCacheObj->m_nBufferSize << endl; fflush(stdout);
            
      pCacheObj->Unlock();

      free(szTmpBuff);
      szTmpBuff = NULL;
      nTmpSize = 0;
      //nAppendCount = 0;
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
    nTmpSize = pCacheObj->m_pAudioEncoder->Flush();
    pCacheObj->Lock();
    
    if(!pCacheObj->m_szBuffer)
        pCacheObj->m_szBuffer = (char*)malloc(nTmpSize * sizeof(char));
      else
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->m_nBufferSize + nTmpSize) * sizeof(char));
    
    memcpy(&pCacheObj->m_szBuffer[pCacheObj->m_nBufferSize], pCacheObj->m_pAudioEncoder->GetEncodedBuffer(), nTmpSize);
    pCacheObj->m_nBufferSize += nTmpSize;
    
    pCacheObj->m_bIsComplete = true;          
    pCacheObj->Unlock();    
  }

  pCacheObj->Lock();
  pCacheObj->m_bIsTranscoding = false;
  pCacheObj->m_pSessionInfo->m_bIsTranscoding = false;
  pCacheObj->Unlock();
  
  /* delete temporary buffer */
  if(szTmpBuff)
    free(szTmpBuff);
  
  stringstream sLog;
  sLog << "transcoding \"" << pCacheObj->m_pSessionInfo->m_sInFileName << "\" done. (" << pCacheObj->m_nBufferSize << " bytes)";  
  CSharedLog::Shared()->Log(L_DEBUG, sLog.str(), __FILE__, __LINE__);  
  
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
    if(m_CachedObjectsIterator != m_CachedObjects.end()) {
      CSharedLog::Shared()->Log(L_DEBUG, "delete cache object: " + pCacheObj->m_sInFileName, __FILE__, __LINE__);
      m_CachedObjects.erase(m_CachedObjectsIterator);
      delete pCacheObj;
    } 
  }

  fuppesThreadUnlockMutex(&m_Mutex);  
}

#endif // DISABLE_TRANSCODING
