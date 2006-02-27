/***************************************************************************
 *            HTTPMessage.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "HTTPMessage.h"
#include "Common.h"
#include "SharedLog.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>
#include <time.h>

#ifndef DISABLE_TRANSCODING
#include "Transcoding/LameWrapper.h"
#include "Transcoding/WrapperBase.h"

  #ifndef DISABLE_VORBIS
  #include "Transcoding/VorbisWrapper.h"
  #endif
  
  #ifndef DISABLE_MUSEPACK
  #include "Transcoding/MpcWrapper.h"
  #endif

#endif

#include "RegEx.h"
#include "UPnPActionFactory.h"

/*===============================================================================
 CONSTANTS
===============================================================================*/

const std::string LOGNAME = "HTTPMessage";

/*===============================================================================
 CLASS CHTTPMessage
===============================================================================*/

/* <PUBLIC> */

fuppesThreadCallback TranscodeLoop(void *arg);
fuppesThreadMutex TranscodeMutex;

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CHTTPMessage::CHTTPMessage()
{
  /* Init */
  m_nHTTPVersion			  = HTTP_VERSION_UNKNOWN;
  m_nHTTPMessageType    = HTTP_MESSAGE_TYPE_UNKNOWN;
	m_sHTTPContentType    = "";
  m_nBinContentLength   = 0; 
  m_nBinContentPosition = 0;
  m_nContentLength      = 0;
  m_pszBinContent       = NULL;
  m_bIsChunked          = false;
  m_TranscodeThread     = (fuppesThread)NULL;
  m_pUPnPItem           = NULL;
  m_bIsBinary           = false;
  m_nRangeStart         = 0;
  m_nRangeEnd           = 0;
  m_nHTTPConnection     = HTTP_CONNECTION_UNKNOWN;
  m_pUPnPAction         = NULL;
}

CHTTPMessage::~CHTTPMessage()
{
  //cout << "CHTTPMessage::~CHTTPMessage()" << endl;
  
  /* Cleanup */
  if(m_pUPnPAction)
    delete m_pUPnPAction;
  
  if(m_pszBinContent)
    delete[] m_pszBinContent;
    
  if(m_TranscodeThread)
    fuppesThreadClose(m_TranscodeThread);

  if(m_fsFile.is_open())
    m_fsFile.close();
}

/*===============================================================================
 INIT
===============================================================================*/

void CHTTPMessage::SetMessage(HTTP_MESSAGE_TYPE nMsgType, std::string p_sContentType)
{
	CMessageBase::SetMessage("");
  
  m_nHTTPMessageType  = nMsgType;
	m_sHTTPContentType  = p_sContentType;
  m_nBinContentLength = 0;
}

bool CHTTPMessage::SetMessage(std::string p_sMessage)
{   
  CMessageBase::SetMessage(p_sMessage);  
  CSharedLog::Shared()->DebugLog(LOGNAME, p_sMessage);  

  return BuildFromString(p_sMessage);
}

void CHTTPMessage::SetBinContent(char* p_szBinContent, unsigned int p_nBinContenLength)
{ 
  m_bIsBinary = true;

  m_nBinContentLength = p_nBinContenLength;      
  m_pszBinContent     = new char[m_nBinContentLength + 1];    
  memcpy(m_pszBinContent, p_szBinContent, m_nBinContentLength);
  m_pszBinContent[m_nBinContentLength] = '\0';   
}

/*===============================================================================
 GET MESSAGE DATA
===============================================================================*/

CUPnPAction* CHTTPMessage::GetAction()
{
  //BOOL_CHK_RET_POINTER(pAction);

  if(!m_pUPnPAction)
  {
    /* Build UPnPAction */  
    CUPnPActionFactory ActionFactory;
    m_pUPnPAction = ActionFactory.BuildActionFromString(m_sContent);
  }
  return m_pUPnPAction;
}

std::string CHTTPMessage::GetHeaderAsString()
{
	stringstream sResult;
	string sVersion;
	string sType;
	string sContentType;
	
  /* Version */
	switch(m_nHTTPVersion)
	{
		case HTTP_VERSION_1_0: sVersion = "HTTP/1.0"; break;
		case HTTP_VERSION_1_1: sVersion = "HTTP/1.1"; break;
    default:               ASSERT(0);             break;
  }
	
  /* Message Type */
	switch(m_nHTTPMessageType)
	{
		case HTTP_MESSAGE_TYPE_GET:	          /* todo */			                            break;
    case HTTP_MESSAGE_TYPE_HEAD:	        /* todo */			                            break;
		case HTTP_MESSAGE_TYPE_POST:          /* todo */		                              break;
		case HTTP_MESSAGE_TYPE_200_OK:        
      sResult << sVersion << " " << "200 OK\r\n";
      break;
    case HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT:
      sResult << sVersion << " " << "206 Partial Content\r\n";
      break;    
	  case HTTP_MESSAGE_TYPE_404_NOT_FOUND:	/* todo */
	  case HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR:
      sResult << sVersion << " " << "500 Internal Server Error\r\n";
      break;
    default:
      cout << "MSG: *" << m_sMessage << "*" << endl;
      fflush(stdout);
      ASSERT(0);                                  
      break;
	}
	
  /* Content length */
  if(!m_bIsBinary)
  {
    sResult << "CONTENT-LENGTH: " << (int)strlen(m_sContent.c_str()) << "\r\n";
  }
  else
  {
    if(!m_bIsTranscoding && (m_nBinContentLength > 0))
    {
      if((m_nRangeStart > 0) || (m_nRangeEnd > 0))
      {
        if(m_nRangeEnd < m_nBinContentLength)
          sResult << "CONTENT-LENGTH: " << m_nRangeEnd - m_nRangeStart + 1<< "\r\n";
        else
          sResult << "CONTENT-LENGTH: " << m_nBinContentLength - m_nRangeStart << "\r\n";
      }
      else
        sResult << "CONTENT-LENGTH: " << m_nBinContentLength << "\r\n";
    }
    else
      sResult << "CONTENT-LENGTH: 0\r\n";
  }      

	
  /* Content type */
	/*switch(m_HTTPContentType)
	{
		case HTTP_CONTENT_TYPE_TEXT_HTML:  sContentType = "text/html";  break;
		case HTTP_CONTENT_TYPE_TEXT_XML:   sContentType = "text/xml; charset=\"utf-8\"";   break;
		case HTTP_CONTENT_TYPE_AUDIO_MPEG: sContentType = "audio/mpeg"; break;
    case HTTP_CONTENT_TYPE_IMAGE_PNG : sContentType = "image/png";  break;      
    default:                           ASSERT(0);                   break;	
	}*/

  sResult << "CONTENT-TYPE: " << m_sHTTPContentType << "\r\n";			  
  sResult << "CONNECTION: close\r\n";  
  
  if(m_nHTTPVersion == HTTP_VERSION_1_1)
    sResult << "ACCEPT-RANGES: bytes\r\n";
  
  if((m_nRangeStart > 0) || (m_nRangeEnd > 0))
  {   
    //sResult << "ETag: \"84a04256ea0bf1:3cae20\"\r\n";
    
    sResult << "CONTENT-RANGE: bytes ";
    //sResult << "Content-Range: bytes ";    
    if(m_nRangeEnd > m_nBinContentLength)
      sResult << m_nRangeStart << "-" << m_nBinContentLength - 1;
    else
      sResult << m_nRangeStart << "-" << m_nRangeEnd;
    sResult << "/" << m_nBinContentLength << "\r\n";
  }
  
  //sResult << "contentFeatures.dlna.org: \r\n";
  
	//sResult << "EXT: \r\n";
	
  /* Transfer-Encoding */
  if(m_nHTTPVersion == HTTP_VERSION_1_1)
  {
    /*if(m_bIsChunked)
    {
      sResult << "TRANSFER-ENCODING: chunked\r\n";
    }*/
  }	
  
  char   szTime[30];
  time_t tTime = time(NULL);
  strftime(szTime, 30,"%a, %d %b %Y %H:%M:%S GMT" , gmtime(&tTime));   
	sResult << "DATE: " << szTime << "\r\n";    
  sResult << "SERVER: Linux/2.6.x, UPnP/1.0, ";
  sResult << CSharedConfig::Shared()->GetAppFullname() << "/" << CSharedConfig::Shared()->GetAppVersion() << "\r\n";
  
	
	sResult << "\r\n";
	return sResult.str();
}

std::string CHTTPMessage::GetMessageAsString()
{
  stringstream sResult;
  sResult << GetHeaderAsString();
  sResult << m_sContent;
  return sResult.str();
}

unsigned int CHTTPMessage::GetBinContentChunk(char* p_sContentChunk, unsigned int p_nSize, unsigned int p_nOffset)
{
  /* read from file */
  if(m_fsFile.is_open())
  {
    /*cout << "size: " << p_nSize << " offset: " << p_nOffset << " filesize: " << m_nBinContentLength << endl;
    fflush(stdout);*/
    
    if(p_nOffset > 0)
      m_fsFile.seekg(p_nOffset, ios::beg);
    else
      p_nOffset = m_nBinContentPosition;
    
    /*unsigned int nRest = 0;
    if((p_nOffset + p_nSize) > m_nBinContentLength)    
      nRest = m_nBinContentLength - p_nOffset;
    else
      nRest = m_nBinContentLength - p_nSize; */
    
    unsigned int nRead = 0;
    if((m_nBinContentLength - p_nOffset) < p_nSize)
      nRead = m_nBinContentLength - p_nOffset;
    else
      nRead = p_nSize;
    
    /*cout << "read " << nRead << " bytes from file. offset: " << p_nOffset << endl;
    fflush(stdout);*/
    
    m_fsFile.read(p_sContentChunk, nRead);      
    m_nBinContentPosition += nRead;
    return nRead;
  }  
  
  /* read transcoded data from memory */
  else
  {    
    fuppesThreadLockMutex(&TranscodeMutex);      
    
    unsigned int nRest       = m_nBinContentLength - m_nBinContentPosition;
    unsigned int nDelayCount = 0;  
    while(m_bIsTranscoding && (nRest < p_nSize) && !m_bBreakTranscoding)
    { 
      nRest = m_nBinContentLength - m_nBinContentPosition;

      stringstream sLog;
      sLog << "we are sending faster then we can transcode!" << endl;
      sLog << "  Try     : " << (nDelayCount + 1) << endl;
      sLog << "  Lenght  : " << m_nBinContentLength << endl;
      sLog << "  Position: " << m_nBinContentPosition << endl;
      sLog << "  Size    : " << p_nSize << endl;
      sLog << "  Rest    : " << nRest << endl;
      sLog << "delaying send-process!";

      CSharedLog::Shared()->Critical(LOGNAME, sLog.str());    
      fuppesThreadUnlockMutex(&TranscodeMutex);
      fuppesSleep(100);
      fuppesThreadLockMutex(&TranscodeMutex);    
      nDelayCount++;
      
      /* if bufer is still empty after n tries
         the machine seems to be too slow. so
         we give up */
      if (nDelayCount == 100) /* 100 * 100ms = 10sec */
      {
        fuppesThreadLockMutex(&TranscodeMutex);    
        m_bBreakTranscoding = true;
        fuppesThreadUnlockMutex(&TranscodeMutex);
        return 0;
      }
    }
    
    if(nRest > p_nSize)
    {
      memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], p_nSize);    
      m_nBinContentPosition += p_nSize;
      fuppesThreadUnlockMutex(&TranscodeMutex);
      return p_nSize;
    }
    else if((nRest < p_nSize) && !m_bIsTranscoding)
    {
      memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], nRest);
      m_nBinContentPosition += nRest;
      fuppesThreadUnlockMutex(&TranscodeMutex);
      return nRest;
    }
     
    fuppesThreadUnlockMutex(&TranscodeMutex);
  
  }
  return 0;
}

/*===============================================================================
 OTHER
===============================================================================*/

bool CHTTPMessage::BuildFromString(std::string p_sMessage)
{
  m_nBinContentLength = 0;
  m_sMessage = p_sMessage;
  m_sContent = p_sMessage;  
  
  bool bResult = false;

  /*cout << "BUILD FROM STR" << endl;
  fflush(stdout);*/

  /* Message GET */
  RegEx rxGET("GET +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxGET.Search(p_sMessage.c_str()))
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_GET;

    string sVersion = rxGET.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxGET.Match(1);
    bResult = true;		
  }

  /* Message HEAD */
  RegEx rxHEAD("HEAD +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxHEAD.Search(p_sMessage.c_str()))
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_HEAD;

    string sVersion = rxHEAD.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxHEAD.Match(1);			   
    bResult = true;  
  }
   
  
  /* Message POST */
  RegEx rxPOST("POST +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxPOST.Search(p_sMessage.c_str()))
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_POST;

    string sVersion = rxPOST.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxPOST.Match(1);			

    bResult = ParsePOSTMessage(p_sMessage);
  }
  
  /* Range */
  RegEx rxRANGE("RANGE: +BYTES=(\\d+)-(\\d+)", PCRE_CASELESS);
  if(rxRANGE.Search(p_sMessage.c_str()))
  {
    m_nRangeStart = atoi(rxRANGE.Match(1));
    m_nRangeEnd   = atoi(rxRANGE.Match(2));
    
    /*cout << "RANGE-START: " << m_nRangeStart << endl;
    cout << "RANGE-END: " << m_nRangeEnd << endl;*/
  }
  
  /* CONNETION */
  RegEx rxCONNECTION("CONNECTION: +(close|keep-alive)", PCRE_CASELESS);
  if(rxCONNECTION.Search(p_sMessage.c_str()))
  {
    //cout << "!!!CONNECTION: " << rxCONNECTION.Match(1) << endl;
    std::string sConnection = ToLower(rxCONNECTION.Match(1));
    if(sConnection.compare("close") == 0)
      m_nHTTPConnection = HTTP_CONNECTION_CLOSE;
  }

  
  /*cout << "END BUILD FROM STR" << endl;
  fflush(stdout);  */
  
  return bResult;
}

bool CHTTPMessage::LoadContentFromFile(std::string p_sFileName)
{
  m_bIsChunked = true;
  m_bIsBinary  = true;
  
  //fstream fFile;    
  m_fsFile.open(p_sFileName.c_str(), ios::binary|ios::in);
  if(m_fsFile.fail() != 1)
  { 
    m_fsFile.seekg(0, ios::end); 
    m_nBinContentLength = streamoff(m_fsFile.tellg()); 
    m_fsFile.seekg(0, ios::beg);
    //fFile.close();
  } 
	
  return true;
}


bool CHTTPMessage::TranscodeContentFromFile(std::string p_sFileName)
{ 
  m_bIsChunked = true;
  m_bIsBinary  = true;
  //m_nHTTPVersion = HTTP_VERSION_1_1;  
  
  m_bBreakTranscoding = false;
  
  m_TranscodeThread = (fuppesThread)NULL;
  fuppesThreadInitMutex(&TranscodeMutex);
      
  CTranscodeSessionInfo* session = new CTranscodeSessionInfo();
  session->m_pHTTPMessage = this;
  session->m_sFileName    = p_sFileName;
  
  fuppesThreadStartArg(m_TranscodeThread, TranscodeLoop, *session); 
  m_bIsTranscoding = true;
  
  //fuppesSleep(1000); /* let the encoder work for a second */
  
  return true;
}

fuppesThreadCallback TranscodeLoop(void *arg)
{
  #ifndef DISABLE_TRANSCODING
  CTranscodeSessionInfo* pSession = (CTranscodeSessionInfo*)arg;
	  
  /* init lame encoder */
  CLameWrapper* pLameWrapper = new CLameWrapper();
  if(!pLameWrapper->LoadLib())
  {    
    delete pLameWrapper;
    delete pSession; 
    pSession->m_pHTTPMessage->m_bIsTranscoding = false;     
    fuppesThreadExit();
  }
  pLameWrapper->SetBitrate(LAME_BITRATE_320);
  pLameWrapper->Init();	

  string sExt = ExtractFileExt(pSession->m_sFileName);

  CDecoderBase* pDecoder = NULL;
  if(ToLower(sExt).compare("ogg") == 0)
  {
    #ifndef DISABLE_VORBIS
    pDecoder = new CVorbisDecoder();
    #endif
  }
  else if(ToLower(sExt).compare("mpc") == 0)
  {
    #ifndef DISABLE_MUSEPACK
    pDecoder = new CMpcDecoder();
    #endif
  }

  /* init decoder */  
  if(!pDecoder || !pDecoder->LoadLib() || !pDecoder->OpenFile(pSession->m_sFileName))
  {
    delete pDecoder;
    delete pLameWrapper;
    delete pSession;
    pSession->m_pHTTPMessage->m_bIsTranscoding = false;
    fuppesThreadExit();
  }
   
  /* begin transcode */
  long  samplesRead  = 0;    
  int   nLameRet     = 0;
  int   nAppendCount = 0;
  int   nAppendSize  = 0;
	char* sAppendBuf   = NULL; 
  #ifndef DISABLE_MUSEPACK
  int nBufferLength = 0;  
  short int* pcmout;
  CMpcDecoder* pTmp = dynamic_cast<CMpcDecoder*>(pDecoder);  
  if (pTmp)
  {
	  pcmout = new short int[MPC_DECODER_BUFFER_LENGTH * 4];
    nBufferLength = MPC_DECODER_BUFFER_LENGTH * 4;
  }
  else
  {
    pcmout = new short int[4096];    
    nBufferLength = 4096;
  }  
  #else
  short int* pcmout = new short int[4096];
  int nBufferLength = 4096;
  #endif
  
  stringstream sLog;
  sLog << "start transcoding \"" << pSession->m_sFileName << "\"";
  CSharedLog::Shared()->Log(LOGNAME, sLog.str());
    
  while(((samplesRead = pDecoder->DecodeInterleaved((char*)pcmout, nBufferLength)) >= 0) && !pSession->m_pHTTPMessage->m_bBreakTranscoding)
  {
    /* encode */
    nLameRet = pLameWrapper->EncodeInterleaved(pcmout, samplesRead);      
  
    /* append encoded mp3 frames to the append buffer */
    char* tmpBuf = NULL;
    if(sAppendBuf != NULL)
    {
      tmpBuf = new char[nAppendSize];
      memcpy(tmpBuf, sAppendBuf, nAppendSize);
      delete[] sAppendBuf;      
    }
    
    sAppendBuf = new char[nAppendSize + nLameRet];
    if(tmpBuf != NULL)
    {
      memcpy(sAppendBuf, tmpBuf, nAppendSize);
      delete[] tmpBuf;
    }
    memcpy(&sAppendBuf[nAppendSize], pLameWrapper->GetMp3Buffer(), nLameRet);
    nAppendSize += nLameRet;
    nAppendCount++;
    /* end append */
    
    
    if(nAppendCount == 100)
    {      
      /* append encoded audio to bin content buffer */
      fuppesThreadLockMutex(&TranscodeMutex);
      
      /* merge existing bin content and new buffer */
      char* tmpBuf = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nAppendSize];
      memcpy(tmpBuf, pSession->m_pHTTPMessage->m_pszBinContent, pSession->m_pHTTPMessage->m_nBinContentLength);
      memcpy(&tmpBuf[pSession->m_pHTTPMessage->m_nBinContentLength], sAppendBuf, nAppendSize);
      
      /* recreate bin content buffer */
      delete[] pSession->m_pHTTPMessage->m_pszBinContent;
      pSession->m_pHTTPMessage->m_pszBinContent = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nAppendSize];
      memcpy(pSession->m_pHTTPMessage->m_pszBinContent, tmpBuf, pSession->m_pHTTPMessage->m_nBinContentLength + nAppendSize);
            
      /* set the new content length */
      pSession->m_pHTTPMessage->m_nBinContentLength += nAppendSize;
      
      delete[] tmpBuf;
      
      /* reset append buffer an variables */
      nAppendCount = 0;
      nAppendSize  = 0;
      delete[] sAppendBuf;
      sAppendBuf = NULL;
      
      fuppesThreadUnlockMutex(&TranscodeMutex);  
      /* end append */      
    }
    
  } /* while */  
  
  
  /* delete buffers */
  if(pcmout)
    delete[] pcmout;
	
  if(sAppendBuf)
    delete[] sAppendBuf;
  
  
  /* flush and end transcoding */
  if(!pSession->m_pHTTPMessage->m_bBreakTranscoding)
  {
    /*sLog.str("");
    sLog << "done transcoding loop now flushing" << endl;
    CSharedLog::Shared()->Log(LOGNAME, sLog.str()); */
  
    /* flush mp3 */
    nLameRet = pLameWrapper->Flush();
  
    /* append encoded audio to bin content buffer */
    fuppesThreadLockMutex(&TranscodeMutex);
      
    char* tmpBuf = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nLameRet];
    if(pSession->m_pHTTPMessage->m_nBinContentLength > 0)
      memcpy(tmpBuf, pSession->m_pHTTPMessage->m_pszBinContent, pSession->m_pHTTPMessage->m_nBinContentLength);
    memcpy(&tmpBuf[pSession->m_pHTTPMessage->m_nBinContentLength], pLameWrapper->GetMp3Buffer(), nLameRet);
    
    delete[] pSession->m_pHTTPMessage->m_pszBinContent;
    pSession->m_pHTTPMessage->m_pszBinContent = new char[pSession->m_pHTTPMessage->m_nBinContentLength + nLameRet];
    memcpy(pSession->m_pHTTPMessage->m_pszBinContent, tmpBuf, pSession->m_pHTTPMessage->m_nBinContentLength + nLameRet);
       
    pSession->m_pHTTPMessage->m_nBinContentLength += nLameRet;
    delete[] tmpBuf;
    pSession->m_pHTTPMessage->m_bIsTranscoding = false;    
  
    fuppesThreadUnlockMutex(&TranscodeMutex);  
    /* end append */      
    
    sLog.str("");
    sLog << "done transcoding \"" << pSession->m_sFileName << "\"";
    CSharedLog::Shared()->Log(LOGNAME, sLog.str());  
  }
  /* break transcoding */
  /*else
  {
    cout << "break transcoding" << endl;
    fflush(stdout);
  } */   
  /* end transcode */
  
  
  /* cleanup and exit */
  pDecoder->CloseFile();  
  delete pDecoder;
  delete pLameWrapper;
  delete pSession;  
  fuppesThreadExit();
  
  #endif /* ifndef DISABLE_TRANSCODING */
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

bool CHTTPMessage::ParsePOSTMessage(std::string p_sMessage)
{
  /*POST /UPnPServices/ContentDirectory/control HTTP/1.1
    Host: 192.168.0.3:32771
    SOAPACTION: "urn:schemas-upnp-org:service:ContentDirectory:1#Browse"
    CONTENT-TYPE: text/xml ; charset="utf-8"
    Content-Length: 467*/
  
  RegEx rxSOAP("SOAPACTION: *\"(.+)\"", PCRE_CASELESS);
	if(rxSOAP.Search(p_sMessage.c_str()))
	{
    string sSOAP = rxSOAP.Match(1);
    //cout << "[HTTPMessage] SOAPACTION " << sSOAP << endl;
	}
      
  /* Content length */
  RegEx rxContentLength("CONTENT-LENGTH: *(\\d+)", PCRE_CASELESS);
  if(rxContentLength.Search(p_sMessage.c_str()))
  {
    string sContentLength = rxContentLength.Match(1);    
    m_nContentLength = std::atoi(sContentLength.c_str());
  }
  
  if((unsigned int)m_nContentLength >= p_sMessage.length())                      
    return false;
  
  m_sContent = p_sMessage.substr(p_sMessage.length() - m_nContentLength, m_nContentLength);
  
  return true;
}

/* <\PRIVATE> */
