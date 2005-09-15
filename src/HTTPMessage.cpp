/***************************************************************************
 *            HTTPMessage.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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

#include <iostream>
#include <sstream>
#include <fstream>

#include <math.h>
#include <vorbis/codec.h>
#include <vorbis/vorbisfile.h>
#include <lame/lame.h>

#include "RegEx.h"
#include "UPnPActionFactory.h"

using namespace std;

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
short int pcmout[4096];

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CHTTPMessage::CHTTPMessage()
{
  cout << "NEW" << endl;
  fflush(stdout);  
  
  /* Init */
  m_HTTPVersion			  = HTTP_VERSION_1_1;
  m_HTTPMessageType   = HTTP_MESSAGE_TYPE_UNKNOWN;
	m_HTTPContentType   = HTTP_CONTENT_TYPE_UNKNOWN;
  m_nBinContentLength = 0; 
  m_nBinContentPosition = 0;
  m_nContentLength    = 0;
  m_pszBinContent     = NULL;
  m_bIsChunked        = false;  
}

CHTTPMessage::~CHTTPMessage()
{
  cout << "DELETE" << endl;
  fflush(stdout);
  /* Cleanup */
  /* uv 2005-08-05 :: memory allocated with new[] 
     has to be freed with delete[] instead of delete */
  //SAFE_DELETE(m_pszBinContent);
  if(m_pszBinContent)
    delete[] m_pszBinContent;
  
  if(m_TranscodeThread)
    fuppesThreadClose(m_TranscodeThread, 0);
}

/*===============================================================================
 INIT
===============================================================================*/

void CHTTPMessage::SetMessage(HTTP_MESSAGE_TYPE nMsgType, HTTP_VERSION nVersion)
{
  CMessageBase::SetMessage("");
  
  m_HTTPMessageType   = nMsgType;
  m_HTTPVersion			  = nVersion;
  m_HTTPContentType   = HTTP_CONTENT_TYPE_UNKNOWN;
  m_nBinContentLength = 0;
}

void CHTTPMessage::SetMessage(HTTP_MESSAGE_TYPE nMsgType, HTTP_CONTENT_TYPE nCtntType)
{
	CMessageBase::SetMessage("");
  
  m_HTTPMessageType   = nMsgType;
	m_HTTPVersion			  = HTTP_VERSION_1_1;
	m_HTTPContentType   = nCtntType;
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
  m_nBinContentLength = p_nBinContenLength;      
  m_pszBinContent     = new char[m_nBinContentLength + 1];    
  memcpy(m_pszBinContent, p_szBinContent, m_nBinContentLength);
  m_pszBinContent[m_nBinContentLength] = '\0';   
}

/*===============================================================================
 GET MESSAGE DATA
===============================================================================*/

bool CHTTPMessage::GetAction(CUPnPBrowse* pBrowse)
{
  BOOL_CHK_RET_POINTER(pBrowse);

  /* Build UPnPAction */
  CUPnPActionFactory ActionFactory;
  return ActionFactory.BuildActionFromString(m_sContent, pBrowse);
}

std::string CHTTPMessage::GetHeaderAsString()
{
	stringstream sResult;
	string sVersion;
	string sType;
	string sContentType;
	
  /* Version */
	switch(m_HTTPVersion)
	{
		case HTTP_VERSION_1_0: sVersion = "HTTP/1.0"; break;
		case HTTP_VERSION_1_1: sVersion = "HTTP/1.1"; break;
    default:               ASSERT(0);             break;
  }
	
  /* Message Type */
	switch(m_HTTPMessageType)
	{
		case HTTP_MESSAGE_TYPE_GET:	          /* todo */			                            break;
		case HTTP_MESSAGE_TYPE_POST:          /* todo */		                              break;
		case HTTP_MESSAGE_TYPE_200_OK:        sResult << sVersion << " " << "200 OK\r\n"; break;
	  case HTTP_MESSAGE_TYPE_404_NOT_FOUND:	/* todo */			                            break;
    default:                              ASSERT(0);                                  break;
	}
	
  /* Content length */
  if(!m_bIsChunked)
  {
    if(m_nBinContentLength > 0)
      sResult << "CONTENT-LENGTH: " << m_nBinContentLength << "\r\n";
    else
      sResult << "CONTENT-LENGTH: " << (int)strlen(m_sContent.c_str()) << "\r\n";
  }
  else
    sResult << "CONTENT-LENGTH: 0\r\n";
	
  /* Content type */
	switch(m_HTTPContentType)
	{
		case HTTP_CONTENT_TYPE_TEXT_HTML:  sContentType = "text/html";  break;
		case HTTP_CONTENT_TYPE_TEXT_XML:   sContentType = "text/xml";   break;
		case HTTP_CONTENT_TYPE_AUDIO_MPEG: sContentType = "audio/mpeg"; break;
    case HTTP_CONTENT_TYPE_IMAGE_PNG : sContentType = "image/png";  break;      
    default:                           ASSERT(0);                   break;	
	}	
	sResult << "CONTENT-TYPE: " << sContentType << "\r\n";
  
  
  /* Transfer-Encoding */
  if(m_HTTPVersion == HTTP_VERSION_1_1)
  {
    /*if(m_bIsChunked)
    {
      sResult << "TRANSFER-ENCODING: chunked\r\n";
    }*/
  }
  
  
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
  fuppesThreadLockMutex(&TranscodeMutex);  
  
  //cout << "pos: " << m_nBinContentPosition << endl << "ofs: " << p_nOffset << endl << "size: " << m_nBinContentLength << endl;
  
  unsigned int nRest = m_nBinContentLength - m_nBinContentPosition;  
  if(nRest > p_nSize)
  {
    memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], p_nSize);    
    m_nBinContentPosition += p_nSize;
    fuppesThreadUnlockMutex(&TranscodeMutex);
    return p_nSize;
  }
  else if((nRest < p_nSize) && m_bIsTranscoding)
  {
    fuppesThreadUnlockMutex(&TranscodeMutex);
    cout << "[critical] we are sending faster then we can transcode" << endl;
    cout << "           delaying send-process for a second" << endl;
    fflush(stdout);
    upnpSleep(1);
    fuppesThreadLockMutex(&TranscodeMutex);
    memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], nRest);
    m_nBinContentPosition += nRest;
    fuppesThreadUnlockMutex(&TranscodeMutex);
    return nRest;    
    fuppesThreadUnlockMutex(&TranscodeMutex);
  }
  else if((nRest < p_nSize) && !m_bIsTranscoding)
  {
    memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], nRest);
    m_nBinContentPosition += nRest;
    fuppesThreadUnlockMutex(&TranscodeMutex);
    return nRest;
  }
   
  fuppesThreadUnlockMutex(&TranscodeMutex);
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

  /* Message GET */
  RegEx rxGET("GET +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxGET.Search(p_sMessage.c_str()))
  {
    m_HTTPMessageType = HTTP_MESSAGE_TYPE_GET;

    string sVersion = rxGET.Match(2);
    if(sVersion.compare("0"))		
      m_HTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1"))		
      m_HTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxGET.Match(1);
    return true;    
  }

  /* Message POST */
  RegEx rxPOST("POST +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxPOST.Search(p_sMessage.c_str()))
  {
    m_HTTPMessageType = HTTP_MESSAGE_TYPE_POST;

    string sVersion = rxPOST.Match(2);
    if(sVersion.compare("0"))		
      m_HTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1"))
      m_HTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxPOST.Match(1);			

    ParsePOSTMessage(p_sMessage);
    return true;
  }
  
   /* SUBSCRIBE publisher path HTTP/1.1
HOST: publisher host:publisher port
CALLBACK: <delivery URL>
NT: upnp:event
TIMEOUT: Second-requested subscription duration*/
  
  /* SUBSCRIBE */
  RegEx rxSUBSCRIBE("SUBSCRIBE +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  
  return false;
}

bool CHTTPMessage::LoadContentFromFile(std::string p_sFileName)
{
  fstream fFile;    
  fFile.open(p_sFileName.c_str(), ios::binary|ios::in);
  if(fFile.fail() != 1)
  { 
    fFile.seekg(0, ios::end); 
    m_nBinContentLength = streamoff(fFile.tellg()); 
    fFile.seekg(0, ios::beg);

    m_pszBinContent = new char[m_nBinContentLength + 1];     
    fFile.read(m_pszBinContent, m_nBinContentLength); 
    m_pszBinContent[m_nBinContentLength] = '\0';    

    fFile.close();
  }
  
  return true;
}

bool CHTTPMessage::TranscodeContentFromFile(std::string p_sFileName)
{ 
  m_bIsChunked  = true;
  m_HTTPVersion = HTTP_VERSION_1_1;  
  
  m_bBreakTranscoding = false;
  
  m_TranscodeThread = (fuppesThread)NULL;
  fuppesThreadInitMutex(&TranscodeMutex);
      
  CTranscodeSessionInfo* session = new CTranscodeSessionInfo();
  session->m_pHTTPMessage = this;
  session->m_sFileName    = p_sFileName;
  
  fuppesThreadStartArg(m_TranscodeThread, TranscodeLoop, *session); 
  m_bIsTranscoding = true;
  
  upnpSleep(1); /* let the encoder work for a second */
  
  return true;
}

fuppesThreadCallback TranscodeLoop(void *arg)
{
  CTranscodeSessionInfo* pSession = (CTranscodeSessionInfo*)arg;
  
  stringstream sLog;
  sLog << "start transcoding \"" << pSession->m_sFileName << "\"" << endl;
  CSharedLog::Shared()->Log(LOGNAME, sLog.str());
  
  
  unsigned char mp3buffer[LAME_MAXMP3BUFFER];	
	lame_global_flags *gf;
	if((gf = lame_init()) == NULL)
	  printf("erroer initializing lame");
	
	lame_set_compression_ratio(gf, 3); // default 11 = 128, 9 = 160, 7 = 192, 5 = 256, 3 = 320
	lame_init_params(gf);	
	
  int nAppendCount = 0;
  int nAppendSize = 0;
	char* sAppendBuf = NULL;
  
	/*printf(get_lame_version ());
	printf(get_lame_url ());
	lame_print_config(gf);*/
	
   /* determine endianness (clever trick courtesy of Nicholas Devillard,
    * (http://www.eso.org/~ndevilla/endian/) */
   int testvar = 1, endian;
   if(*(char *)&testvar)
      endian = 0;  // little endian
   else
      endian = 1;  // big endian	
	
  OggVorbis_File vf;	
  FILE* ogg_file;
  if ((ogg_file = fopen(pSession->m_sFileName.c_str(), "r")) == NULL)
    fprintf(stderr, "Cannot open %s\n", pSession->m_sFileName.c_str()); 
	
  if(ov_open(ogg_file, &vf, NULL, 0) < 0) {
      fprintf(stderr,"Input does not appear to be an Ogg bitstream.\n");      
  }	 

  vorbis_info *vi=ov_info(&vf,-1);
   /*
    char **ptr=ov_comment(&vf,-1)->user_comments;
    while(*ptr){
      fprintf(stderr,"%s\n",*ptr);
      ++ptr;
    }
    fprintf(stderr,"\nBitstream is %d channel, %ldHz\n",vi->channels,vi->rate);
    fprintf(stderr,"\nDecoded length: %ld samples\n",
            (long)ov_pcm_total(&vf,-1));
    fprintf(stderr,"Encoded by: %s\n\n",ov_comment(&vf,-1)->vendor);  */
  

    /* begin transcode */

  long samplesRead = 0;
  long bytesRead   = 0;
  int  bitstream   = 0;
  int  lameret     = 0;	
	
  do {
    bytesRead = ov_read(&vf, (char*)pcmout, sizeof(pcmout), endian, 2, 1, &bitstream);
    if (bytesRead == 0) {
      
    } else if (bytesRead < 0) {
      /* error in the stream.  Not a problem, just reporting it in
	 case we (the app) cares.  In this case, we don't. */
    } else {

	
    //cout << "bytesRead: " << bytesRead << endl;
    samplesRead = bytesRead / vi->channels / sizeof(short int);
    //cout << "samplesRead: " << samplesRead << endl;	
	
    lameret = lame_encode_buffer_interleaved(gf, pcmout, samplesRead, mp3buffer, LAME_MAXMP3BUFFER);

      
          
    char* tmpBuf = NULL;
    if(sAppendBuf != NULL)
    {
      tmpBuf = new char[nAppendSize];
      memcpy(tmpBuf, sAppendBuf, nAppendSize);
      delete[] sAppendBuf;
    }
    
    sAppendBuf = new char[nAppendSize + lameret];
    if(tmpBuf != NULL)
    {
      memcpy(sAppendBuf, tmpBuf, nAppendSize);
      delete[] tmpBuf;
    }
    memcpy(&sAppendBuf[nAppendSize], mp3buffer, lameret);
    nAppendSize += lameret;
    nAppendCount++;
    
    
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
    
    }
  } while (bytesRead != 0 && bitstream == 0 && !pSession->m_pHTTPMessage->m_bBreakTranscoding);
	
  if(!pSession->m_pHTTPMessage->m_bBreakTranscoding)
  {
    sLog.str("");
    sLog << "done transcoding loop now flushing" << endl;
    CSharedLog::Shared()->Log(LOGNAME, sLog.str());    
  
    lameret = lame_encode_flush(gf, mp3buffer, LAME_MAXMP3BUFFER);
  
    /* append encoded audio to bin content buffer */
    fuppesThreadLockMutex(&TranscodeMutex);
    
    /*cout << "appending buffer" << endl;    
    cout << pSession->m_pHTTPMessage->m_nBinContentLength << endl;
    fflush(stdout);*/
              
    char* tmpBuf = new char[pSession->m_pHTTPMessage->m_nBinContentLength + lameret];
    if(pSession->m_pHTTPMessage->m_nBinContentLength > 0)
      memcpy(tmpBuf, pSession->m_pHTTPMessage->m_pszBinContent, pSession->m_pHTTPMessage->m_nBinContentLength);
    memcpy(&tmpBuf[pSession->m_pHTTPMessage->m_nBinContentLength], mp3buffer, lameret);
    
    delete[] pSession->m_pHTTPMessage->m_pszBinContent;
    pSession->m_pHTTPMessage->m_pszBinContent = new char[pSession->m_pHTTPMessage->m_nBinContentLength + lameret];
    memcpy(pSession->m_pHTTPMessage->m_pszBinContent, tmpBuf, pSession->m_pHTTPMessage->m_nBinContentLength + lameret);
       
    pSession->m_pHTTPMessage->m_nBinContentLength += lameret;

    delete[] tmpBuf;
      
    /*cout << "done appending buffer" << endl;
    cout << pSession->m_pHTTPMessage->m_nBinContentLength << endl;
    fflush(stdout);*/

    pSession->m_pHTTPMessage->m_bIsTranscoding = false;    
  
    fuppesThreadUnlockMutex(&TranscodeMutex);  
    /* end append */      
    
    sLog.str("");
    sLog << "done transcoding \"" << pSession->m_sFileName << "\"" << endl;
    CSharedLog::Shared()->Log(LOGNAME, sLog.str());  
  }
  else
  {
    cout << "break transcoding" << endl;
    fflush(stdout);
  }
  
  ov_clear(&vf);  
  /* end transcode */
  
  delete pSession;
  
  fuppesThreadExit(NULL);
  return 0;
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

void CHTTPMessage::ParsePOSTMessage(std::string p_sMessage)
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
    //cout << "[HTTPMessage] CONTENT-LENGTH  " << m_nContentLength << endl;
  }
  
  m_sContent = p_sMessage.substr(p_sMessage.length() - m_nContentLength, m_nContentLength);
  
}

/* <\PRIVATE> */
