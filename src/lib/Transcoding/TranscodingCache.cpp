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
#include "../ContentDirectory/FileDetails.h"

#include <iostream>
#include <fstream>

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
  m_pTranscoder     = NULL;
  m_bIsComplete     = false;
  //m_pSessionInfo    = NULL;
  m_bBreakTranscoding = false;  
  m_bInitialized    = false;
  
  m_bThreaded       = false;
  m_nReleaseCnt     = 0;
  m_nReleaseCntBak  = 0;
  
  fuppesThreadInitMutex(&m_Mutex);
}

CTranscodingCacheObject::~CTranscodingCacheObject()
{
  // break transcoding
  if(m_bIsTranscoding) {                    
    m_bBreakTranscoding = true;    
  }
  
  if(m_TranscodeThread) {
    if(TranscodeToFile()) {
      fuppesThreadCancel(m_TranscodeThread);
    }
    fuppesThreadClose(m_TranscodeThread);        
  }
    
  fuppesThreadDestroyMutex(&m_Mutex);  
  free(m_szBuffer);
  
  if(m_pPcmOut)
    delete[] m_pPcmOut;
    
  delete m_pAudioEncoder;
  delete m_pDecoder;
  
  delete m_pTranscoder;
}

bool CTranscodingCacheObject::Init(CTranscodeSessionInfo* pSessionInfo, CDeviceSettings* pDeviceSettings)
{
  std::string sExt = ToLower(ExtractFileExt(pSessionInfo->m_sInFileName));
  
  ReleaseCount(pDeviceSettings->ReleaseDelay(sExt));
  
  if(CTranscodingMgr::Shared()->GetTranscodingType(sExt) == TT_THREADED_TRANSCODER ||
     CTranscodingMgr::Shared()->GetTranscodingType(sExt) == TT_TRANSCODER) {
    
    if(m_bInitialized) {
      return true;
    }     
    
    m_pTranscoder = CTranscodingMgr::Shared()->CreateTranscoder(sExt);
    m_bInitialized = true;
        
    m_bThreaded = m_pTranscoder->Threaded();
        
    return true;
  }
  
       
       
  
  
  if(m_bInitialized) {
    //m_pSessionInfo = pSessionInfo;
    if(!m_bIsComplete) {
      pSessionInfo->m_nGuessContentLength = m_pAudioEncoder->GuessContentLength(m_pDecoder->GuessPcmLength());
    }
    else {
      pSessionInfo->m_nGuessContentLength = m_nBufferSize;
    }
    return true;
  }
  
  
  CSharedLog::Shared()->Log(L_EXTENDED, "Init " + pSessionInfo->m_sInFileName, __FILE__, __LINE__);  
  //mm_pSessionInfo = pSessionInfo;  
    
  
  // init decoder
  if(!m_pDecoder)
  {  
    // create decoder
    nBufferLength = 32768;    
    
    m_pDecoder = CTranscodingMgr::Shared()->CreateAudioDecoder(sExt, &nBufferLength);
    
    // init decoder
    if(!m_pDecoder || !m_pDecoder->LoadLib() || !m_pDecoder->OpenFile(pSessionInfo->m_sInFileName, &m_AudioDetails)) {
      delete m_pDecoder;
      m_pDecoder = NULL;     
      
      cout << "error initializing audio decoder" << endl;
      return false;
    }

    // create pcm buffer
    m_pPcmOut = new short int[nBufferLength];
  }  
  
  
  // init encoder
  if(!m_pAudioEncoder) {
    
    m_pAudioEncoder = CTranscodingMgr::Shared()->CreateAudioEncoder(pDeviceSettings->Extension(sExt)); //(CFileDetails::Shared()->GetTargetExtension(sExt));
    if(!m_pAudioEncoder->LoadLib()) {      
      delete m_pAudioEncoder;
      m_pAudioEncoder = NULL;
      
      cout << "error initializing audio encoder" << endl;      
      return false;
    }
    
    #warning todo: set encoder bitrate from config
    //m_pLameWrapper->SetBitrate(LAME_BITRATE_320);
    m_pAudioEncoder->SetAudioDetails(&m_AudioDetails);
    m_pAudioEncoder->SetSessionInfo(pSessionInfo);
    //m_pAudioEncoder->Init();
    
         
    pSessionInfo->m_nGuessContentLength = m_pAudioEncoder->GuessContentLength(m_pDecoder->GuessPcmLength());
    //cout << "guess content: " << pSessionInfo->m_nGuessContentLength << endl;
  }  
  
  m_bThreaded = true;
  m_bInitialized = true;

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

unsigned int CTranscodingCacheObject::GetBufferSize()
{
  if(m_pTranscoder != NULL) {
    
    if(m_bIsComplete && m_nBufferSize > 0) {
      return m_nBufferSize;
    }
    
    unsigned int nFileSize = 0;
    std::fstream fsFile;
      
    fsFile.open(m_sOutFileName.c_str(), ios::binary|ios::in);
    if(fsFile.fail() != 1) { 
      fsFile.seekg(0, ios::end); 
      nFileSize = streamoff(fsFile.tellg()); 
      fsFile.seekg(0, ios::beg);
      fsFile.close();
    }
    else {
      cout << __FILE__ << " error opening " << m_sOutFileName << endl;
    }
    
    if(m_bIsComplete) {
      m_nBufferSize = nFileSize;
    }
    
    return nFileSize;
  }
  else {
    return m_nBufferSize;
  }
}

bool CTranscodingCacheObject::TranscodeToFile()
{
  if(m_pTranscoder != NULL) {
    return true;
  }
  else {
    return false;
  }        
}

unsigned int CTranscodingCacheObject::Transcode()
{  
  
  if(!m_bThreaded) {
    m_sOutFileName = "/tmp/fuppes.jpg";
    m_pTranscoder->Transcode(string(""), m_sInFileName, string(""), &m_sOutFileName);
    
    m_bIsComplete    = true;
    m_bIsTranscoding = false;
    return GetBufferSize();
  }
  
  
  /* start new transcoding thread */
  if(!m_TranscodeThread && !m_bIsComplete)
  {
    m_bIsTranscoding = true;
    fuppesThreadStartArg(m_TranscodeThread, TranscodeThread, *this);
    while(m_bIsTranscoding && (GetBufferSize() == 0))
      fuppesSleep(100);
    
    return GetBufferSize();
  }
  
  /* transcoding is already running */
  if(m_bIsTranscoding)
  {    
    unsigned int nSize = GetBufferSize();
    while(m_bIsTranscoding && (nSize == GetBufferSize()))
      fuppesSleep(100);
    if(m_bIsTranscoding)
      return GetBufferSize();
  }
  
  /* object is already transcoded completely */  
  if(m_bIsComplete) 
    return GetBufferSize();

	return 0;
}

int CTranscodingCacheObject::Append(char** p_pszBinBuffer, unsigned int p_nBinBufferSize)
{
  Lock();   
  cout << __FILE__ <<" :: append: " << m_nBufferSize << " bytes" << endl;
  *p_pszBinBuffer = (char*)realloc(*p_pszBinBuffer, sizeof(char)*(m_nBufferSize));  
  memcpy(*p_pszBinBuffer, m_szBuffer, m_nBufferSize);    
  Unlock();
    
  return m_nBufferSize;  
}

fuppesThreadCallback TranscodeThread(void *arg)
{  
  CTranscodingCacheObject* pCacheObj = (CTranscodingCacheObject*)arg;     
  //CSharedLog::Shared()->Log(L_EXTENDED, "TranscodeThread :: " + pCacheObj->m_pSessionInfo->m_sInFileName, __FILE__, __LINE__);   
  
  
  if(pCacheObj->m_pTranscoder != NULL) {
    pCacheObj->m_sOutFileName = "/tmp/fuppes.mpg";
    
    pCacheObj->m_pTranscoder->Transcode(string(""), pCacheObj->m_sInFileName, string(""), &pCacheObj->m_sOutFileName);    
    
    pCacheObj->Lock();
    pCacheObj->m_bIsComplete = true;
    pCacheObj->Unlock();   
    
    fuppesThreadExit();
    return 0;
  }
  
  
  
  
  long  samplesRead  = 0;    
  int   nLameRet     = 0;  
  unsigned int nAppendCount = 0;
  char* szTmpBuff = NULL;
  unsigned int nTmpSize = 0;
  int   nBytesRead = 0;
  
  pCacheObj->m_pAudioEncoder->Init();
  
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
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->GetBufferSize() + nTmpSize) * sizeof(char));
      
      memcpy(&pCacheObj->m_szBuffer[pCacheObj->GetBufferSize()], szTmpBuff, nTmpSize);
      
      //cout << "buffer size: " << pCacheObj->m_nBufferSize << " + " << nTmpSize << " = "; fflush(stdout);
      pCacheObj->SetBufferSize(pCacheObj->GetBufferSize()+ nTmpSize);
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
       
  //cout << "transcoding loop exited. now flushing" << endl;
 
    
    /* append remaining frames */
    pCacheObj->Lock();
    
    if(!pCacheObj->m_szBuffer)
        pCacheObj->m_szBuffer = (char*)malloc(nTmpSize * sizeof(char));
      else
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->GetBufferSize() + nTmpSize) * sizeof(char));
      
    memcpy(&pCacheObj->m_szBuffer[pCacheObj->GetBufferSize()], szTmpBuff, nTmpSize);      
    pCacheObj->SetBufferSize(pCacheObj->GetBufferSize()+ nTmpSize);
    
    pCacheObj->Unlock();


    /* flush and end transcoding */

    /* flush mp3 */
    nTmpSize = pCacheObj->m_pAudioEncoder->Flush();
    pCacheObj->Lock();
    
    if(!pCacheObj->m_szBuffer)
        pCacheObj->m_szBuffer = (char*)malloc(nTmpSize * sizeof(char));
      else
        pCacheObj->m_szBuffer = (char*)realloc(pCacheObj->m_szBuffer, (pCacheObj->GetBufferSize() + nTmpSize) * sizeof(char));
    
    memcpy(&pCacheObj->m_szBuffer[pCacheObj->GetBufferSize()], pCacheObj->m_pAudioEncoder->GetEncodedBuffer(), nTmpSize);
    pCacheObj->SetBufferSize(pCacheObj->GetBufferSize() + nTmpSize);

    pCacheObj->m_bIsComplete = true;          
    pCacheObj->Unlock();    
  }

  //cout << "flushed" << endl;
  
  pCacheObj->Lock();
  pCacheObj->m_bIsTranscoding = false;
  //pCacheObj->m_pSessionInfo->m_bIsTranscoding = false;
  pCacheObj->Unlock();
  
  /* delete temporary buffer */
  if(szTmpBuff)
    free(szTmpBuff);
  
  /*stringstream sLog;
  sLog << "transcoding \"" << pCacheObj->m_pSessionInfo->m_sInFileName << "\" done. (" << pCacheObj->m_nBufferSize << " bytes)";  
  CSharedLog::Shared()->Log(L_DEBUG, sLog.str(), __FILE__, __LINE__); */  
  
  //cout << "transcoding done. (" << pCacheObj->m_nBufferSize << " bytes)" << endl;
  
  fuppesThreadExit();  
}



fuppesThreadCallback ReleaseLoop(void* arg);

CTranscodingCache* CTranscodingCache::m_pInstance = 0;

CTranscodingCache* CTranscodingCache::Shared()
{
	if (m_pInstance == 0)
		m_pInstance = new CTranscodingCache();
	return m_pInstance;
}

CTranscodingCache::CTranscodingCache()
{
  m_ReleaseThread = (fuppesThread)NULL;
  fuppesThreadInitMutex(&m_Mutex); 
}

CTranscodingCache::~CTranscodingCache()
{
  if(m_ReleaseThread) {
    fuppesThreadCancel(m_ReleaseThread);
    fuppesThreadClose(m_ReleaseThread);
  }
  
  fuppesThreadDestroyMutex(&m_Mutex);
}


CTranscodingCacheObject* CTranscodingCache::GetCacheObject(std::string p_sFileName)
{
  fuppesThreadLockMutex(&m_Mutex);
  
  CTranscodingCacheObject* pResult = NULL;  
  
  /* check if object exists */
  pResult = m_CachedObjects[p_sFileName];  
  if(!pResult) {
    pResult = new CTranscodingCacheObject();    
    m_CachedObjects[p_sFileName] = pResult;
    pResult->m_sInFileName = p_sFileName;
    
    cout << "new transcoding obj: " << p_sFileName << endl;
  }
  else {
    cout << "existing transcoding obj: " << p_sFileName << endl;
    cout << "size: " << pResult->GetBufferSize() << endl;
    cout << "ref count: " << pResult->m_nRefCount << endl;
    if(pResult->m_bIsTranscoding) {
      cout << "transcoding running" << endl;
    }
    else {
      if (pResult->m_bIsComplete) {
        cout << "complete" << endl;
      }
      else {
        cout << "error :: obj incomplete but not transcoding" << endl;
      }
    }
  }
  cout << endl;
  
  pResult->m_nRefCount++;
  pResult->ResetReleaseCount();
  //pResult->m_nReleaseCnt = RELEASE_DELAY;
  
  fuppesThreadUnlockMutex(&m_Mutex);  
  
  return pResult;
}

void CTranscodingCache::ReleaseCacheObject(CTranscodingCacheObject* pCacheObj)
{
  fuppesThreadLockMutex(&m_Mutex);  
 
  if(!m_ReleaseThread) {
    fuppesThreadStart(m_ReleaseThread, ReleaseLoop);
  }
  
  cout << __FILE__ << " release : " << pCacheObj->m_sInFileName << endl;
  cout << "ref count: " << pCacheObj->m_nRefCount << endl;
  
  pCacheObj->m_nRefCount--;
  
  #warning todo: pause transcoding if ref count == 0
  
  fuppesThreadUnlockMutex(&m_Mutex);  
}

fuppesThreadCallback ReleaseLoop(void* arg)
{
  CTranscodingCache* pCache = (CTranscodingCache*)arg;
  CTranscodingCacheObject* pCacheObj;
  std::map<std::string, CTranscodingCacheObject*>::iterator TmpIterator; 
  
  while(true) {
    fuppesSleep(1000);
    
    fuppesThreadLockMutex(&pCache->m_Mutex);  
   
    for(pCache->m_CachedObjectsIterator = pCache->m_CachedObjects.begin();
        pCache->m_CachedObjectsIterator != pCache->m_CachedObjects.end();
        )
    {
      pCacheObj = (*pCache->m_CachedObjectsIterator).second;
      
      if((pCacheObj->m_nRefCount == 0) && 
         (pCacheObj->ReleaseCount() == 0)) {
        TmpIterator = pCache->m_CachedObjectsIterator;
        TmpIterator++;
        
        cout << "free: " << pCacheObj->m_sInFileName << endl;
           
        pCache->m_CachedObjects.erase(pCache->m_CachedObjectsIterator);
        delete pCacheObj;
        pCache->m_CachedObjectsIterator = TmpIterator;
      }
      else if(pCacheObj->m_nRefCount == 0) {
        pCacheObj->DecReleaseCount();
        pCache->m_CachedObjectsIterator++;
      }
      else {
        pCache->m_CachedObjectsIterator++;
      }
    }
    
    fuppesThreadUnlockMutex(&pCache->m_Mutex);   
  }
  
  fuppesThreadExit();
}

#endif // DISABLE_TRANSCODING
