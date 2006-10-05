/***************************************************************************
 *            TranscodingMgr.cpp
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
 
#include "TranscodingMgr.h" 
#include "LameWrapper.h"
#include "../Common.h"

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

CTranscodingMgr::CTranscodingMgr()
{  
  m_pLameWrapper = NULL;
  m_pDecoder     = NULL;
  
  m_sAppendBuffer     = NULL;    
  m_nAppendBufferSize = 0;
  m_nAppendCount      = 0;
  
  m_bIsTranscoding = false;
}
 
CTranscodingMgr::~CTranscodingMgr()
{
  if(m_pLameWrapper)
    delete m_pLameWrapper;
  
  if(m_pDecoder)
    delete m_pDecoder;
}

TRANSCODING_MGR_ERROR_CODES CTranscodingMgr::Init(CTranscodeSessionInfo* p_SessionInfo)
{
  m_pSessionInfo = p_SessionInfo;
  
  if(!FileExists(m_pSessionInfo->m_sFileName))
    return TRANSCODING_MGR_FILE_NOT_FOUND;
 
  // init lame
  m_pLameWrapper = new CLameWrapper();  
  if(!m_pLameWrapper->LoadLib())    
    return TRANSCODING_MGR_AUDIO_ENCODER_MISSING;
  
  m_pLameWrapper->SetBitrate(LAME_BITRATE_320);
  m_pLameWrapper->Init();	

  
  // select decoder
  std::string sExt = ToLower(ExtractFileExt(m_pSessionInfo->m_sFileName));  
  if(sExt.compare("ogg") == 0)  
    #ifndef DISABLE_VORBIS
    m_pDecoder = new CVorbisDecoder();
    #endif  
  else if(sExt.compare("mpc") == 0)  
    #ifndef DISABLE_MUSEPACK
    m_pDecoder = new CMpcDecoder();
    #endif 
  else if(sExt.compare("flac") == 0)
    #ifndef DISABLE_FLAC    
    m_pDecoder = new CFLACDecoder();   
    #endif
  
  if(!m_pDecoder || !m_pDecoder->LoadLib() || !m_pDecoder->OpenFile(m_pSessionInfo->m_sFileName))
    return TRANSCODING_MGR_AUDIO_DECODER_MISSING;  
  
  
  return TRANSCODING_MGR_OK;
}

int CTranscodingMgr::Transcode()
{  
  /* begin transcode */
  long  samplesRead  = 0;    
  int   nLameRet     = 0;  
	
  m_bIsTranscoding = true;
  
  cout << "CTranscodingMgr::Transcode" << endl;
  fflush(stdout);
  
  #ifndef DISABLE_MUSEPACK
  int nBufferLength = 0;  
  short int* pcmout;
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
  short int* pcmout = new short int[32768];
  int nBufferLength = 32768;
  #endif
  
  /*stringstream sLog;
  sLog << "start transcoding \"" << pSession->m_sFileName << "\"";
  CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());*/
    
  m_nAppendCount = 0;
  m_nAppendBufferSize = 0;
  
   cout << "CTranscodingMgr::Transcode :: start transcoding " << nBufferLength << endl;
  fflush(stdout);
  
  while(((samplesRead = m_pDecoder->DecodeInterleaved((char*)pcmout, nBufferLength)) >= 0) && !m_pSessionInfo->m_bBreakTranscoding)
  {
    
    //cout << "CTranscodingMgr::Transcode :: got frames decoded: " << samplesRead << endl;
    
    /* encode */
    nLameRet = m_pLameWrapper->EncodeInterleaved(pcmout, samplesRead);      
  
    
    /*cout << m_nAppendCount + 1 << " CTranscodingMgr::Transcode :: got frames encoded: " << nLameRet << " append: " << m_nAppendBufferSize << endl;
    fflush(stdout);*/
    
    /* append encoded mp3 frames to the append buffer */
    char* tmpBuf = NULL;
    if(m_sAppendBuffer != NULL)
    {
      tmpBuf = new char[m_nAppendBufferSize];
      memcpy(tmpBuf, m_sAppendBuffer, m_nAppendBufferSize);
      delete[] m_sAppendBuffer;      
    }
    
    m_sAppendBuffer = new char[m_nAppendBufferSize + nLameRet];
    if(tmpBuf != NULL)
    {
      memcpy(m_sAppendBuffer, tmpBuf, m_nAppendBufferSize);
      delete[] tmpBuf;
    }
    memcpy(&m_sAppendBuffer[m_nAppendBufferSize], m_pLameWrapper->GetMp3Buffer(), nLameRet);
    m_nAppendBufferSize += nLameRet;
    m_nAppendCount++;
    /* end append */
    
    
    if(m_nAppendCount == 100)
    { 
     cout << "CTranscodingMgr::Transcode :: return transcoding " << m_nAppendBufferSize << endl;
  fflush(stdout);
      return m_nAppendBufferSize;         
    }
    
  } /* while */  
  
  
  /* delete buffers */
  if(pcmout)
    delete[] pcmout;
	
  /*if(m_sAppendBuffer)
    delete[] m_sAppendBuffer;*/
  
  
  
  
  /* flush and end transcoding */
  if(!m_pSessionInfo->m_bBreakTranscoding)
  {
    /*sLog.str("");
    cout << "done transcoding loop now flushing" << endl;
    CSharedLog::Shared()->Log(LOGNAME, sLog.str()); */
    cout << "done transcoding loop now flushing" << endl;
    fflush(stdout);
    
    /* flush mp3 */
    nLameRet = m_pLameWrapper->Flush();
  
    /* append encoded mp3 frames to the append buffer */
    char* tmpBuf = NULL;
    if(m_sAppendBuffer != NULL)
    {
      tmpBuf = new char[m_nAppendBufferSize];
      memcpy(tmpBuf, m_sAppendBuffer, m_nAppendBufferSize);
      delete[] m_sAppendBuffer;      
    }
    
    m_sAppendBuffer = new char[m_nAppendBufferSize + nLameRet];
    if(tmpBuf != NULL)
    {
      memcpy(m_sAppendBuffer, tmpBuf, m_nAppendBufferSize);
      delete[] tmpBuf;
    }
    memcpy(&m_sAppendBuffer[m_nAppendBufferSize], m_pLameWrapper->GetMp3Buffer(), nLameRet);
    m_nAppendBufferSize = nLameRet;
    
    m_bIsTranscoding = false;    
    return m_nAppendBufferSize;
    
    /* append encoded audio to bin content buffer */
 //   fuppesThreadLockMutex(&TranscodeMutex);
      
    /*char* tmpBuf = new char[m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength + nLameRet];
    if(m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength > 0)
      memcpy(tmpBuf, m_pSessionInfo->m_pHTTPMessage->m_pszBinContent, m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength);
    memcpy(&tmpBuf[m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength], m_pLameWrapper->GetMp3Buffer(), nLameRet);
    
    delete[] m_pSessionInfo->m_pHTTPMessage->m_pszBinContent;
    m_pSessionInfo->m_pHTTPMessage->m_pszBinContent = new char[m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength + nLameRet];
    memcpy(m_pSessionInfo->m_pHTTPMessage->m_pszBinContent, tmpBuf, m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength + nLameRet);
       
    m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength += nLameRet;
    delete[] tmpBuf; */
        
  
//    fuppesThreadUnlockMutex(&TranscodeMutex);  
    /* end append */      
    
    /*sLog.str("");
    sLog << "done transcoding \"" << pSession->m_sFileName << "\"";
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());  */
  }  
  
  
  return 0;
}

int CTranscodingMgr::Append(char** p_pszBinBuffer, unsigned int p_nBinBufferSize)
{
  
 cout << "CTranscodingMgr::Append : " << m_nAppendBufferSize << " bytes" << endl;
  
  /* append encoded audio to bin content buffer */
//      fuppesThreadLockMutex(&TranscodeMutex);
  
  /* merge existing bin content and new buffer */
  char* tmpBuf = new char[p_nBinBufferSize + m_nAppendBufferSize];
  memcpy(tmpBuf, *p_pszBinBuffer, p_nBinBufferSize);
  memcpy(&tmpBuf[p_nBinBufferSize], m_sAppendBuffer, m_nAppendBufferSize);
  
  /* recreate bin content buffer */
  //delete[] p_szBinBuffer;
  
  cout << "addr: " << &*p_pszBinBuffer << endl;
  
  /*if(p_nBinBufferSize == 0)
    *p_pszBinBuffer = (char*)malloc(sizeof(char) * m_nAppendBufferSize);
  else*/
  *p_pszBinBuffer = (char*)realloc(*p_pszBinBuffer, sizeof(char)*(p_nBinBufferSize + m_nAppendBufferSize));
  //p_szBinBuffer = new char[p_nBinBufferSize + m_nAppendBufferSize];
  memcpy(*p_pszBinBuffer, tmpBuf, p_nBinBufferSize + m_nAppendBufferSize);
  
cout << "addr: " << &*p_pszBinBuffer << endl;
  
  /* set the new content length */
  //m_pSessionInfo->m_pHTTPMessage->m_nBinContentLength += m_nAppendBufferSize;
  
  delete[] tmpBuf;
  
  int nResult = m_nAppendBufferSize;
  
  /* reset append buffer and variables */
  m_nAppendCount = 0;
  m_nAppendBufferSize  = 0;
  delete[] m_sAppendBuffer;
  m_sAppendBuffer = NULL;
  
//   fuppesThreadUnlockMutex(&TranscodeMutex);  
  /* end append */    
  
  
  if(!m_bIsTranscoding)
  {
    //m_pSessionInfo->m_pHTTPMessage->m_bIsTranscoding = false;    
    m_pSessionInfo->m_bIsTranscoding = false;    
  }
   
  cout << "CTranscodingMgr::Append :: return " << nResult << endl;
  return nResult;
}

#endif /* DISABLE_TRANSCODING */
