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

#ifdef HAVE_LAME
#include "LameWrapper.h"
#endif

#include <iostream>
#include <fstream>

using namespace std;

fuppesThreadCallback TranscodeThread(void *arg);

CTranscodingCacheObject::CTranscodingCacheObject()
{
  m_nRefCount       = 0;    
  m_sBuffer         = NULL;
  m_pPcmOut         = NULL;
  m_nBufferSize     = 0;
  m_nValidBytes     = 0;
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
  
  m_bLocked = false;
  
  
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
  if(m_sBuffer) {
    free(m_sBuffer);
  }
  
  if(m_pPcmOut)
    delete[] m_pPcmOut;
    
  delete m_pAudioEncoder;
  delete m_pDecoder;
  
  delete m_pTranscoder;
}

bool CTranscodingCacheObject::Init(CTranscodeSessionInfo* pSessionInfo, CDeviceSettings* pDeviceSettings)
{
  std::string sExt = ExtractFileExt(pSessionInfo->m_sInFileName);
  
  ReleaseCount(pDeviceSettings->ReleaseDelay(sExt));
  
  if(pDeviceSettings->GetTranscodingType(sExt) == TT_THREADED_TRANSCODER ||
     pDeviceSettings->GetTranscodingType(sExt) == TT_TRANSCODER) {
       
    if(m_bInitialized) {
      return true;
    }     
       
    m_pDeviceSettings = pDeviceSettings;    
    m_pTranscoder = CTranscodingMgr::Shared()->CreateTranscoder(pDeviceSettings->GetTranscoderType(sExt));
       
    if(!m_pTranscoder) {
      return false;
    }
     
    m_pTranscoder->Init(pSessionInfo->sACodec, pSessionInfo->sVCodec);
       
    m_bInitialized = true;        
    m_bThreaded = m_pTranscoder->Threaded();
    return true;
  }
  
       
       
  
  
  if(m_bInitialized) {
    //m_pSessionInfo = pSessionInfo;
    if(!m_bIsComplete) {
      pSessionInfo->m_nGuessContentLength = m_pAudioEncoder->GuessContentLength(m_pDecoder->NumPcmSamples());
    }
    else {
      pSessionInfo->m_nGuessContentLength = m_nBufferSize;
    }
    return true;
  }
  
  
  CSharedLog::Shared()->Log(L_EXT, "Init " + pSessionInfo->m_sInFileName, __FILE__, __LINE__);  
  m_pDeviceSettings = pDeviceSettings;
    
  
  CAudioDetails AudioDetails;
  // init decoder
  if(!m_pDecoder)
  {  
    // create decoder
    nPcmBufferSize = 32768;    
    
    m_pDecoder = CTranscodingMgr::Shared()->CreateAudioDecoder(pDeviceSettings->GetDecoderType(sExt), &nPcmBufferSize);
    
    // init decoder    
    if(!m_pDecoder || !m_pDecoder->LoadLib() || !m_pDecoder->OpenFile(pSessionInfo->m_sInFileName, &AudioDetails)) {
      delete m_pDecoder;
      m_pDecoder = NULL;     
      
      cout << "error initializing audio decoder" << endl;
      return false;
    }

    // create pcm buffer
    m_pPcmOut = new short int[nPcmBufferSize];
  }  
  
  
  // init encoder
  if(!m_pAudioEncoder) {
    
    m_pAudioEncoder = CTranscodingMgr::Shared()->CreateAudioEncoder(pDeviceSettings->GetEncoderType(sExt));
    if(!m_pAudioEncoder->LoadLib()) {      
      delete m_pAudioEncoder;
      m_pAudioEncoder = NULL;
      
      cout << "error initializing audio encoder" << endl;  
      return false;
    }
    
    m_pAudioEncoder->SetAudioDetails(&AudioDetails);
    m_pAudioEncoder->SetTranscodingSettings(pDeviceSettings->FileSettings(sExt)->pTranscodingSettings);    
    m_pAudioEncoder->SetSessionInfo(pSessionInfo);   
         
    pSessionInfo->m_nGuessContentLength = m_pAudioEncoder->GuessContentLength(m_pDecoder->NumPcmSamples());
    //cout << "guess content: " << pSessionInfo->m_nGuessContentLength << endl;
    
    // make sure decoder delivers correct endianess
    // lame needs machine dependent endianess
    // wav and pcm encoder currently support only litte-endian
    if(m_pDecoder->OutEndianess() != m_pAudioEncoder->InEndianess()) {
      m_pDecoder->SetOutputEndianness(m_pAudioEncoder->InEndianess());
    }
  }  
  
  m_bThreaded = true;
  m_bInitialized = true;

  
  if(pSessionInfo->m_nGuessContentLength > 0) {    
    m_nBufferSize = pSessionInfo->m_nGuessContentLength;  
    m_sBuffer = (char*)malloc(m_nBufferSize * sizeof(char*));
    if(!m_sBuffer) {      
      m_nBufferSize = 0;
    }
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
  m_bLocked = true;
}

void CTranscodingCacheObject::Unlock()
{
  m_bLocked = false;
  fuppesThreadUnlockMutex(&m_Mutex);
}

unsigned int CTranscodingCacheObject::GetValidBytes()
{
  if(m_pTranscoder != NULL) {
    
    if(m_bIsComplete && m_nValidBytes > 0) {
      return m_nValidBytes;
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
    
    if(m_bIsComplete) {
      m_nValidBytes = nFileSize;
    }

    return nFileSize;
  }
  else {    
    return m_nValidBytes;
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

unsigned int CTranscodingCacheObject::Transcode(CDeviceSettings* pDeviceSettings)
{  
  
  if(!m_bThreaded) {
    std::string sExt = ExtractFileExt(m_sInFileName);
    m_pTranscoder->Transcode(pDeviceSettings->FileSettings(sExt), m_sInFileName, &m_sOutFileName);
    
    m_bIsComplete    = true;
    m_bIsTranscoding = false;
    return GetValidBytes();
  }
  
  
  /* start new transcoding thread */
  if(!m_TranscodeThread && !m_bIsComplete)
  {
    m_bIsTranscoding = true;
    fuppesThreadStartArg(m_TranscodeThread, TranscodeThread, *this);
    while(m_bIsTranscoding && (GetValidBytes() == 0))
      fuppesSleep(100);
    
    return GetValidBytes();
  }
  
  /* transcoding is already running */
  if(m_bIsTranscoding)
  {    
    unsigned int nSize = GetValidBytes();
    while(m_bIsTranscoding && (nSize == GetValidBytes()))
      fuppesSleep(100);
    if(m_bIsTranscoding)
      return GetValidBytes();
  }
  
  /* object is already transcoded completely */  
  if(m_bIsComplete) 
    return GetValidBytes();

	return 0;
}

fuppesThreadCallback TranscodeThread(void *arg)
{  
  CTranscodingCacheObject* pCacheObj = (CTranscodingCacheObject*)arg;  
  
  // threaded transcoder
  if(pCacheObj->m_pTranscoder != NULL) {   
    
    std::string sExt = ExtractFileExt(pCacheObj->m_sInFileName);    
    pCacheObj->m_pTranscoder->Transcode(pCacheObj->DeviceSettings()->FileSettings(sExt), pCacheObj->m_sInFileName, &pCacheObj->m_sOutFileName);    
    
    pCacheObj->Lock();
    pCacheObj->m_bIsComplete = true;
    pCacheObj->Unlock();   
    
    fuppesThreadExit();
    return 0;
  }  
  
  // threaded de-/encoder 
  long          samplesRead     = 0;    
  int           nEncRet         = 0;  
  unsigned int  nAppendCount    = 0;
  char*         szTmpBuff       = NULL;
  unsigned int  nTmpBuffSize    = 0;
  unsigned int  nTmpValidBytes  = 0;
    
  int           nBytesConsumed  = 0;
  
  #define APPEND_BUFFER_SIZE 65536 // 64 kb
  
  szTmpBuff    = (char*)malloc(APPEND_BUFFER_SIZE * sizeof(char*));
  nTmpBuffSize = APPEND_BUFFER_SIZE;
  
  pCacheObj->m_pAudioEncoder->Init();
    
  // transcode loop
  while(((samplesRead = pCacheObj->m_pDecoder->DecodeInterleaved((char*)pCacheObj->m_pPcmOut, pCacheObj->nPcmBufferSize, &nBytesConsumed)) >= 0) && !pCacheObj->m_bBreakTranscoding)
  {
    // encode
    nEncRet = pCacheObj->m_pAudioEncoder->EncodeInterleaved(pCacheObj->m_pPcmOut, samplesRead, nBytesConsumed);
    nBytesConsumed = 0;
        
    // reallocate temporary buffer ...
    if((nTmpValidBytes + nEncRet) > nTmpBuffSize) {
      szTmpBuff = (char*)realloc(szTmpBuff, (nTmpBuffSize + nEncRet) * sizeof(char));
      nTmpBuffSize += nEncRet;
    }
    
    // ... store encoded frames ...
    memcpy(&szTmpBuff[nTmpValidBytes], pCacheObj->m_pAudioEncoder->GetEncodedBuffer(), nEncRet);    
    
    // ... and set new temporary valid bytes count
    nTmpValidBytes += nEncRet;
    
    nAppendCount++;  
    
    
    // append frames to the cache-object's buffer
    if(((nAppendCount % 50) == 0) || (nTmpBuffSize >= APPEND_BUFFER_SIZE)) {      
      
      if(pCacheObj->Locked())
        continue;
      
      pCacheObj->Lock();
      
      // create new buffer
      if(!pCacheObj->m_sBuffer) {        
        pCacheObj->m_sBuffer = (char*)malloc(nTmpValidBytes * sizeof(char));        
        pCacheObj->m_nBufferSize = nTmpValidBytes;        
      }
      // enlarge buffer if neccessary
      else if(pCacheObj->m_nBufferSize < (pCacheObj->m_nValidBytes + nTmpValidBytes)) {        
        pCacheObj->m_sBuffer = (char*)realloc(pCacheObj->m_sBuffer, (pCacheObj->m_nValidBytes + nTmpValidBytes) * sizeof(char));        
        pCacheObj->m_nBufferSize = pCacheObj->m_nValidBytes + nTmpValidBytes;        
      }
      
      // copy data
      memcpy(&pCacheObj->m_sBuffer[pCacheObj->m_nValidBytes], szTmpBuff, nTmpValidBytes);
      pCacheObj->m_nValidBytes += nTmpValidBytes;
                       
      pCacheObj->Unlock();

      // reset valid bytes count
      nTmpValidBytes = 0;
    }
    
  } // while decode
  
  // transcoding loop exited
  if(!pCacheObj->m_bBreakTranscoding)
  {
    // append remaining frames
    if(nTmpValidBytes > 0) {
      pCacheObj->Lock();
      
      if(!pCacheObj->m_sBuffer) {
        pCacheObj->m_sBuffer = (char*)malloc(nTmpValidBytes * sizeof(char));
        pCacheObj->m_nBufferSize = nTmpValidBytes;
      }
      else if ((pCacheObj->m_nValidBytes + nTmpValidBytes) > pCacheObj->m_nBufferSize) {
        pCacheObj->m_sBuffer = (char*)realloc(pCacheObj->m_sBuffer, (pCacheObj->m_nValidBytes + nTmpValidBytes) * sizeof(char));
        pCacheObj->m_nBufferSize = pCacheObj->m_nValidBytes + nTmpValidBytes;
      }
        
      memcpy(&pCacheObj->m_sBuffer[pCacheObj->m_nValidBytes], szTmpBuff, nTmpValidBytes);      
      pCacheObj->m_nValidBytes += nTmpValidBytes;
      nTmpValidBytes = 0;
      
      pCacheObj->Unlock();
    }

    // flush and end transcoding

    // flush mp3
    nTmpValidBytes = pCacheObj->m_pAudioEncoder->Flush();
    pCacheObj->Lock();
    
    if(!pCacheObj->m_sBuffer) {
      pCacheObj->m_sBuffer = (char*)malloc(nTmpValidBytes * sizeof(char));
      pCacheObj->m_nBufferSize = nTmpValidBytes;
    }
    else if ((pCacheObj->m_nValidBytes + nTmpValidBytes) > pCacheObj->m_nBufferSize) {
      pCacheObj->m_sBuffer = (char*)realloc(pCacheObj->m_sBuffer, (pCacheObj->m_nValidBytes + nTmpValidBytes) * sizeof(char));
      pCacheObj->m_nBufferSize = pCacheObj->m_nValidBytes + nTmpValidBytes;
    }
    
    memcpy(&pCacheObj->m_sBuffer[pCacheObj->m_nValidBytes], pCacheObj->m_pAudioEncoder->GetEncodedBuffer(), nTmpValidBytes);
    pCacheObj->m_nValidBytes += nTmpValidBytes;

    pCacheObj->m_bIsComplete = true;          
    pCacheObj->Unlock();    
  }
    
  pCacheObj->Lock();
  pCacheObj->m_bIsTranscoding = false;  
  pCacheObj->Unlock();
  
  // delete temporary buffer
  if(szTmpBuff)
    free(szTmpBuff);  
  
  fuppesThreadExit();  
  return 0;
}


void CTranscodingCacheObject::GetId3v1(char buffer[128])
{ 
  #ifdef HAVE_LAME
  CLameWrapper* pLame = dynamic_cast<CLameWrapper*>(m_pAudioEncoder);
  if(!pLame) {
  #endif
    const string sFakeMp3Tail = 
          "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
          "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
          "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
          "qqqqqqqqqqqqqqqqqqo="; 
  
    string sResult = Base64Decode(sFakeMp3Tail);
    memcpy(buffer, sFakeMp3Tail.c_str(), 128);
  #ifdef HAVE_LAME
  }
  else {
    pLame->GetMp3Tail(buffer);
  }
  #endif
}

bool CTranscodingCacheObject::IsMp3Encoding()
{
  #ifdef HAVE_LAME
  CLameWrapper* pLame = dynamic_cast<CLameWrapper*>(m_pAudioEncoder);
  if(!pLame) {
    return false;
  }
  else {  
    return true;
  }
  #else
  return false;
  #endif
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
  }
  
  pResult->m_nRefCount++;
  pResult->ResetReleaseCount();
  
  fuppesThreadUnlockMutex(&m_Mutex);  
  
  return pResult;
}

void CTranscodingCache::ReleaseCacheObject(CTranscodingCacheObject* pCacheObj)
{
  fuppesThreadLockMutex(&m_Mutex);  
 
  if(!m_ReleaseThread) {
    fuppesThreadStart(m_ReleaseThread, ReleaseLoop);
  }

	stringstream sLog;
	sLog << "release object \"" << pCacheObj->m_sInFileName << "\"" << endl <<
				"ref count: " << pCacheObj->m_nRefCount << endl <<
				"delay: " << pCacheObj->ReleaseCountOrig();
		
	CSharedLog::Shared()->Log(L_EXT, sLog.str(), __FILE__, __LINE__);
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

				stringstream sLog;
				sLog << "delete object \"" << pCacheObj->m_sInFileName << "\"" << endl <<
				"delay: " << pCacheObj->ReleaseCountOrig();
		
				CSharedLog::Shared()->Log(L_EXT, sLog.str(), __FILE__, __LINE__);						 
						 
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
