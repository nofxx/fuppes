/***************************************************************************
 *            HTTPMessage.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "HTTPMessage.h"
#include "../Common/Common.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"
#include "HTTPParser.h"

#include <iostream>
#include <sstream>
#include <time.h>

#include "../Common/RegEx.h"
#include "../UPnPActions/UPnPActionFactory.h"

const std::string LOGNAME = "HTTPMessage";

CHTTPMessage::CHTTPMessage()
{
  // Init
  m_nHTTPVersion			  = HTTP_VERSION_UNKNOWN;
  m_nHTTPMessageType    = HTTP_MESSAGE_TYPE_UNKNOWN;
	m_sHTTPContentType    = "";
  m_nBinContentLength   = 0; 
  m_nBinContentPosition = 0;
  m_nContentLength      = 0;
  m_pszBinContent       = NULL;
 // m_TranscodeThread     = (fuppesThread)NULL;  
  m_bIsBinary           = false;
  m_nRangeStart         = 0;
  m_nRangeEnd           = 0;
  m_nHTTPConnection     = HTTP_CONNECTION_UNKNOWN;
  m_pUPnPAction         = NULL;
	m_pDeviceSettings			= NULL;
  m_pTranscodingSessionInfo = NULL;
  #ifndef DISABLE_TRANSCODING
  m_pTranscodingCacheObj = NULL;
  #endif
  m_nTransferEncoding    = HTTP_TRANSFER_ENCODING_NONE;
  
  //fuppesThreadInitMutex(&TranscodeMutex);
}

CHTTPMessage::~CHTTPMessage()
{
  // Cleanup
  if(m_pUPnPAction)
    delete m_pUPnPAction;
  
  if(m_pszBinContent)
    free(m_pszBinContent);
    
  /*if(m_TranscodeThread) {
    cout << "~CHTTPMessage :: close transcode thread" << endl; fflush(stdout);
    fuppesThreadClose(m_TranscodeThread);
  }*/

  //fuppesThreadDestroyMutex(&TranscodeMutex);

  #ifndef DISABLE_TRANSCODING
  if(m_pTranscodingCacheObj) {
    CTranscodingCache::Shared()->ReleaseCacheObject(m_pTranscodingCacheObj);
  }  
  #endif

  if(m_pTranscodingSessionInfo) {    
    //m_pTranscodingSessionInfo->m_pszBinBuffer = NULL;
    delete m_pTranscodingSessionInfo;
  }
  
  if(m_fsFile.is_open())
    m_fsFile.close();
}

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
  //CSharedLog::Shared()->DebugLog(LOGNAME, p_sMessage);  

  CHTTPParser Parser;
  Parser.Parse(this);
  
  return BuildFromString(p_sMessage);
}

bool CHTTPMessage::SetHeader(std::string p_sHeader)
{
  // header is already set
  if(m_sHeader.length() > 0)
    return true;
  
  CMessageBase::SetHeader(p_sHeader);  
  
  CHTTPParser Parser;
  return Parser.Parse(this);
}

void CHTTPMessage::SetBinContent(char* p_szBinContent, unsigned int p_nBinContenLength)
{ 
  m_bIsBinary = true;

  m_nBinContentLength = p_nBinContenLength;      
  m_pszBinContent     = (char*)malloc(sizeof(char) * (m_nBinContentLength + 1));    
  memcpy(m_pszBinContent, p_szBinContent, m_nBinContentLength);
  m_pszBinContent[m_nBinContentLength] = '\0';   
}

CUPnPAction* CHTTPMessage::GetAction()
{
  if(!m_pUPnPAction) {                
    // Build UPnPAction 
    CUPnPActionFactory ActionFactory;
    m_pUPnPAction = ActionFactory.BuildActionFromString(m_sContent, m_pDeviceSettings);
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
      sResult << sVersion << " 200 OK\r\n";
      break;
    case HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT:
      sResult << sVersion << " 206 Partial Content\r\n";
      break;    
    case HTTP_MESSAGE_TYPE_403_FORBIDDEN:
      sResult << sVersion << " 403 Forbidden\r\n";
      break;
	  case HTTP_MESSAGE_TYPE_404_NOT_FOUND:
      sResult << sVersion << " 404 Not Found\r\n";
      break;
	  case HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR:
      sResult << sVersion << " " << "500 Internal Server Error\r\n";
      break;
    
    /* GENA */
    case HTTP_MESSAGE_TYPE_GENA_OK:
      sResult << sVersion << " 200 OK\r\n";
      break;    
    
    default:
      CSharedLog::Shared()->Log(L_ERROR, "GetHeaderAsString() :: unhandled message type", __FILE__, __LINE__);      
      break;
	}		  
  
  
  if(m_nHTTPMessageType != HTTP_MESSAGE_TYPE_GENA_OK)
  {
    /* Content Type */
    sResult << "Content-Type: " << m_sHTTPContentType << "\r\n";
    
    /* Content length */
    
    // if it's a non binary file give the length of m_sContent
    if(!m_bIsBinary) {
      sResult << "Content-Length: " << (int)strlen(m_sContent.c_str()) << "\r\n";
    }
    // otherwise calc length
    else
    { 
      // transcoding responses don't contain content length
      if(!this->IsTranscoding() && 
         (m_nBinContentLength > 0) &&
         (m_nTransferEncoding != HTTP_TRANSFER_ENCODING_CHUNKED))
      {      
        // ranges
        if((m_nRangeStart > 0) || (m_nRangeEnd > 0))
        {
          if(m_nRangeEnd < m_nBinContentLength) {
            sResult << "Content-Length: " << m_nRangeEnd - m_nRangeStart + 1 << "\r\n";
            sResult << "Content-Range: bytes " << m_nRangeStart << "-" << m_nRangeEnd << "/" << m_nBinContentLength << "\r\n";
          }
          else {
            sResult << "Content-Length: " << m_nBinContentLength - m_nRangeStart << "\r\n";
            sResult << "Content-Range: bytes " << m_nRangeStart << "-" << m_nBinContentLength - 1 << "/" << m_nBinContentLength << "\r\n";
          }
        }
        // complete
        else {
          sResult << "Content-Length: " << m_nBinContentLength << "\r\n";
        }
      }
      // transcoding
      else if(this->IsTranscoding() && (m_nTransferEncoding != HTTP_TRANSFER_ENCODING_CHUNKED)) {
        
        if((m_nRangeStart > 0) || (m_nRangeEnd > 0)) {
          
          // id3 v1 request
          if((m_nRangeEnd - m_nRangeStart) == 127) {            
            sResult << "Content-Length: " << m_pTranscodingSessionInfo->m_nGuessContentLength - m_nRangeStart << "\r\n";
            sResult << "Content-Range: bytes " << m_nRangeStart << "-" << m_pTranscodingSessionInfo->m_nGuessContentLength - 1 << "/" << m_pTranscodingSessionInfo->m_nGuessContentLength << "\r\n";
          }
          else if(m_nRangeEnd < m_pTranscodingSessionInfo->m_nGuessContentLength) {
            sResult << "Content-Length: " << m_nRangeEnd - m_nRangeStart + 1 << "\r\n";
            sResult << "Content-Range: bytes " << m_nRangeStart << "-" << m_nRangeEnd << "/" << m_pTranscodingSessionInfo->m_nGuessContentLength << "\r\n";
          }
          else {
            sResult << "Content-Length: " << m_pTranscodingSessionInfo->m_nGuessContentLength - m_nRangeStart << "\r\n";
            sResult << "Content-Range: bytes " << m_nRangeStart << "-" << m_pTranscodingSessionInfo->m_nGuessContentLength - 1 << "/" << m_pTranscodingSessionInfo->m_nGuessContentLength << "\r\n";
          }  
          
          
        }
        else {
                    
          if(m_pTranscodingSessionInfo->m_nGuessContentLength > 0) {
            sResult << "Content-Length: " << m_pTranscodingSessionInfo->m_nGuessContentLength << "\r\n";
          }         
          
        }
        
        
      }
      
    } // if(m_bIsBinary)
    /* end Content length */        
    
    
    switch(m_nTransferEncoding) {
      case HTTP_TRANSFER_ENCODING_NONE:
        break;
      case HTTP_TRANSFER_ENCODING_CHUNKED:
        sResult << "Transfer-Encoding: chunked\r\n";
        break;      
    }
    
    // Accept-Ranges
   if(!this->IsTranscoding() && (m_nTransferEncoding != HTTP_TRANSFER_ENCODING_CHUNKED)) {
      sResult << "Accept-Ranges: bytes\r\n";
    }
    else {
      sResult << "Accept-Ranges: none\r\n";
    }

    
    /* Connection */
    sResult << "Connection: close\r\n";    
	
    // date
    char   szTime[30];
    time_t tTime = time(NULL);
    strftime(szTime, 30,"%a, %d %b %Y %H:%M:%S GMT" , gmtime(&tTime));   
  	sResult << "DATE: " << szTime << "\r\n";	
	
	  /* dlna */
    sResult << "contentFeatures.dlna.org: \r\n";
    sResult << "EXT:\r\n";    
  }
  
  
  /* GENA header information */
  else if(m_nHTTPMessageType == HTTP_MESSAGE_TYPE_GENA_OK)
  {
    // subscription or renew    
    if(m_sGENASubscriptionID.length() > 0) { 
      sResult << "SID: uuid:" << m_sGENASubscriptionID << "\r\n";
      sResult << "Timeout: Second-" << 180 << "\r\n";
    }
  }
  
 	// server signature
  sResult << 
    "Server: " << CSharedConfig::Shared()->GetOSName() << "/" << CSharedConfig::Shared()->GetOSVersion() << ", " <<
    "UPnP/1.0, " << CSharedConfig::Shared()->GetAppFullname() << "/" << CSharedConfig::Shared()->GetAppVersion() << "\r\n";  
	
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

unsigned int CHTTPMessage::GetBinContentLength()
{
  if(m_pTranscodingSessionInfo) {    
    return m_pTranscodingSessionInfo->m_nGuessContentLength;      
  }
  else {
    return m_nBinContentLength;
  }
}

unsigned int CHTTPMessage::GetBinContentChunk(char* p_sContentChunk, unsigned int p_nSize, unsigned int p_nOffset)
{
  /* read from file */
  if(m_pTranscodingSessionInfo == NULL && m_fsFile.is_open())
  {
    //cout << "size: " << p_nSize << " offset: " << p_nOffset << " filesize: " << m_nBinContentLength << " position: " << m_nBinContentPosition << endl;
    /*fflush(stdout);*/
    
    if((p_nOffset > 0) && (p_nOffset != m_nBinContentPosition))
      m_fsFile.seekg(p_nOffset, ios::beg);
    /*else
      p_nOffset = m_nBinContentPosition;*/
    
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
    
    //cout << "read " << nRead << " bytes from file. offset: " << p_nOffset << endl;
    /*fflush(stdout);*/
    
    m_fsFile.read(p_sContentChunk, nRead);      
    m_nBinContentPosition += nRead;
    return nRead;
  }  
  
  /* read (transcoded) data from memory */
  else
  {    
    
    // id3v1 request
    if(m_pTranscodingSessionInfo) {

      if(m_pTranscodingSessionInfo->m_nGuessContentLength > 0) {
      if(((m_pTranscodingSessionInfo->m_nGuessContentLength - p_nOffset) == 127) ||
         ((m_pTranscodingSessionInfo->m_nGuessContentLength - p_nOffset) == 128)) {       
           
        const string sFakeMp3Tail = 
          "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
          "qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq"    
          "qqqqqqqqqqqqqqqqqqo=";    
    
        string sBinFake = Base64Decode(sFakeMp3Tail);      
        memcpy(p_sContentChunk, sBinFake.c_str(), m_pTranscodingSessionInfo->m_nGuessContentLength - p_nOffset);      
        return m_pTranscodingSessionInfo->m_nGuessContentLength - p_nOffset;      
      }
      }
    }
    
    
    unsigned int nRest = 0;
    unsigned int nDelayCount = 0;  
    bool bTranscode = (m_pTranscodingSessionInfo != NULL);

    
    #ifndef DISABLE_TRANSCODING
    if(bTranscode) {
      
      if(p_nOffset > 0 && p_nOffset != m_nBinContentPosition) {
        
        // offset groesser als verfuegbare daten
        if(p_nOffset > m_pTranscodingCacheObj->GetBufferSize() && m_pTranscodingCacheObj->m_bIsComplete)  {          
          cout << "error offset too high" << endl;
          return 0;
        }
        else {
          m_nBinContentPosition = p_nOffset;
        }
      }
      
      //if(m_pTranscodingCacheObj->GetBufferSize() > m_nBinContentPosition) {
        
        nRest = m_pTranscodingCacheObj->GetBufferSize() - m_nBinContentPosition; 
        
        cout << "buffer: " << m_pTranscodingCacheObj->GetBufferSize() << endl;
        cout << "pos: " << m_nBinContentPosition << endl;
      
        if(m_pTranscodingCacheObj->GetBufferSize() > m_nBinContentPosition) {
          cout << "calc rest: " << m_pTranscodingCacheObj->GetBufferSize() - m_nBinContentPosition << endl;
        }
        else {
          cout << "rest == 0" << endl;
        }
      
        cout << "rest: " << nRest << endl << endl;
        
      
      /*}
      else {
        nRest = 0;
      } */     
    }
    else
      nRest = m_nBinContentLength - m_nBinContentPosition;
    #else
      nRest = m_nBinContentLength - m_nBinContentPosition;
    #endif
    
    
    cout << "rest 2: " << nRest << endl << endl;
    
    #ifndef DISABLE_TRANSCODING
    while(bTranscode && !m_pTranscodingCacheObj->m_bIsComplete && (nRest < p_nSize) && !m_pTranscodingSessionInfo->m_bBreakTranscoding)
    { 
      //if(m_pTranscodingCacheObj->GetBufferSize() > m_nBinContentPosition) {
        nRest = m_pTranscodingCacheObj->GetBufferSize() - m_nBinContentPosition; 
      /*}
      else {
        nRest = 0;
      }*/
           
      if(m_pTranscodingCacheObj->GetBufferSize() > m_nBinContentPosition) {
        cout << "calc rest 3: " << m_pTranscodingCacheObj->GetBufferSize() - m_nBinContentPosition << endl;
      }
      else {
        cout << "rest 3 == 0" << endl;
      }      
      
      cout << "rest 3: " << nRest << endl << endl;  
      
      

      stringstream sLog;
      sLog << "we are sending faster then we can transcode!" << endl;
      sLog << "  try     : " << (nDelayCount + 1) << "/20" << endl;
      sLog << "  length  : " << m_pTranscodingCacheObj->GetBufferSize() << endl;
      sLog << "  position: " << m_nBinContentPosition << endl;
      sLog << "  size    : " << p_nSize << endl;
      sLog << "  rest    : " << nRest << endl;
      sLog << "delaying send-process!";

      //cout << sLog.str() << endl;
      
      CSharedLog::Shared()->Critical(LOGNAME, sLog.str()); 
                                   
      fuppesSleep(500);
                              
      nDelayCount++;
      
      /* if bufer is still empty after 10 seconds
         the machine seems to be too slow. so
         we give up */
      if (nDelayCount == 20) { // 20 * 500ms = 10sec
        BreakTranscoding();                                  
        return 0;
      }
    }
    
    cout << "rest 4: " << nRest << endl << endl;      
    
    
    if(bTranscode) {
      nRest = m_pTranscodingCacheObj->GetBufferSize() - m_nBinContentPosition;
      
      if(!m_pTranscodingCacheObj->TranscodeToFile()) {
        m_pTranscodingCacheObj->Append(&m_pszBinContent, 0);
      }
    }
    else {
      nRest = m_nBinContentLength - m_nBinContentPosition;         
    }
    #else
    nRest = m_nBinContentLength - m_nBinContentPosition;         
    #endif
    
    
    cout << "offset: " << p_nOffset << " pos: " << m_nBinContentPosition << endl;
    cout << "rest: " << nRest << " size: " << p_nSize << endl;
    if(bTranscode) {
      cout << "buffer size: " << m_pTranscodingCacheObj->GetBufferSize() << endl << endl;
    }
    
    if(nRest >= p_nSize) {
             
      if(bTranscode && m_pTranscodingCacheObj->TranscodeToFile()) {
        
        fstream fsTmp;        
        fsTmp.open(m_pTranscodingCacheObj->m_sOutFileName.c_str(), ios::binary|ios::in);
        if(m_fsFile.fail() != 1) { 
          fsTmp.seekg(m_nBinContentPosition, ios::beg);   
          fsTmp.read(p_sContentChunk, p_nSize);
          fsTmp.close();

          m_nBinContentPosition += p_nSize;
          return p_nSize;
        }
        else {
          return 0;
        }
      }
      else {      
        memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], p_nSize);    
        m_nBinContentPosition += p_nSize;
        return p_nSize;
      }
    }
    else if(nRest < p_nSize) {
      
      if(bTranscode && m_pTranscodingCacheObj->TranscodeToFile()) {
        fstream fsTmp;        
        fsTmp.open(m_pTranscodingCacheObj->m_sOutFileName.c_str(), ios::binary|ios::in);
        if(m_fsFile.fail() != 1) { 
          fsTmp.seekg(m_nBinContentPosition, ios::beg);   
          fsTmp.read(p_sContentChunk, nRest);
          fsTmp.close();
          
          m_nBinContentPosition += nRest;
          return nRest;
        }
        else {
          return 0;
        }
      }
      else if(!this->IsTranscoding()) {
        memcpy(p_sContentChunk, &m_pszBinContent[m_nBinContentPosition], nRest);
        m_nBinContentPosition += nRest;      
        return nRest;
      }
    }
  
  }  
  return 0;
}


bool CHTTPMessage::PostVarExists(std::string p_sPostVarName)
{
  if (m_nHTTPMessageType != HTTP_MESSAGE_TYPE_POST)
   return false;
  
  stringstream sExpr;
  sExpr << p_sPostVarName << "=";
  
  RegEx rxPost(sExpr.str().c_str(), PCRE_CASELESS);
  if (rxPost.Search(m_sContent.c_str()))  
    return true;
  else
    return false;    
}

std::string CHTTPMessage::GetPostVar(std::string p_sPostVarName)
{
  if (m_nHTTPMessageType != HTTP_MESSAGE_TYPE_POST)
   return "";
  
  stringstream sExpr;
  sExpr << p_sPostVarName << "=(.*)";
  
  string sResult = "";
  RegEx rxPost(sExpr.str().c_str(), PCRE_CASELESS);
  if (rxPost.Search(m_sContent.c_str()))
  { 
    if(rxPost.SubStrings() == 2)
      sResult = rxPost.Match(1);
    
    /* remove trailing carriage return */
    if((sResult.length() > 0) && (sResult[sResult.length() - 1] == '\r'))
      sResult = sResult.substr(0, sResult.length() - 1);    
  }
  
  return sResult;
}


bool CHTTPMessage::BuildFromString(std::string p_sMessage)
{
  m_nBinContentLength = 0;
  m_sMessage = p_sMessage;
  //m_sContent = p_sMessage;  
  
  bool bResult = false;

  /*cout << "CHTTPMessage::BUILD FROM STR" << endl;
  cout << p_sMessage << endl;
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
    //m_nHTTPMessageType = HTTP_MESSAGE_TYPE_POST;

    string sVersion = rxPOST.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxPOST.Match(1);			

    bResult = ParsePOSTMessage(p_sMessage);
  }
  
  /* Message SUBSCRIBE */
  RegEx rxSUBSCRIBE("[SUBSCRIBE|UNSUBSCRIBE]+ +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  if(rxSUBSCRIBE.Search(p_sMessage.c_str()))
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_SUBSCRIBE;     
    
    string sVersion = rxSUBSCRIBE.Match(2);
    if(sVersion.compare("0") == 0)		
      m_nHTTPVersion = HTTP_VERSION_1_0;		
    else if(sVersion.compare("1") == 0)
      m_nHTTPVersion = HTTP_VERSION_1_1;

    m_sRequest = rxSUBSCRIBE.Match(1);
                                 
    bResult = ParseSUBSCRIBEMessage(p_sMessage);       
  }
  
  /* Range */
  RegEx rxRANGE("RANGE: +BYTES=(\\d*)(-\\d*)", PCRE_CASELESS);
  if(rxRANGE.Search(p_sMessage.c_str()))
  {
    string sMatch1 = rxRANGE.Match(1);    
    string sMatch2;
    if(rxRANGE.SubStrings() > 2)
     sMatch2 = rxRANGE.Match(2);
     
    //cout << "MATCH 1: " << sMatch1 << " SUBSTR:" << sMatch1.substr(0,1) << endl;
    if(sMatch1.substr(0,1) != "-")
      m_nRangeStart = atoi(rxRANGE.Match(1));      
    else
      m_nRangeStart = 0;
    
    m_nRangeEnd = 0;
    
    string sSub;
    if(sMatch1.substr(0, 1) == "-")
    {
      sSub = sMatch1.substr(1, sMatch1.length());
      m_nRangeEnd   = atoi(sSub.c_str());
    }
    else if(rxRANGE.SubStrings() > 2)
    {
      sSub = sMatch2.substr(1, sMatch2.length());
      m_nRangeEnd   = atoi(sSub.c_str());
    }
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
  fflush(stdout);*/
  
  return bResult;
}

bool CHTTPMessage::LoadContentFromFile(std::string p_sFileName)
{
  //m_bIsChunked = true;
  m_bIsBinary  = true;  
  bool bResult = false;
  
  //m_nTransferEncoding = HTTP_TRANSFER_ENCODING_CHUNKED;
  
  //fstream fFile;    
  m_fsFile.open(p_sFileName.c_str(), ios::binary|ios::in);
  if(m_fsFile.fail() != 1)
  { 
    m_fsFile.seekg(0, ios::end); 
    m_nBinContentLength = streamoff(m_fsFile.tellg()); 
    m_fsFile.seekg(0, ios::beg);
    bResult = true;
    
    //cout << "file " << p_sFileName.c_str() << " opened" << endl << "  Length: " << m_nBinContentLength << endl; 
  } 
  else
  {
    //cout << "error opening " << p_sFileName.c_str() << endl;      
    bResult = false;
  }
	
  return bResult;
}


bool CHTTPMessage::TranscodeContentFromFile(std::string p_sFileName, SMusicTrack p_sTrackDetails)
{ 
  #ifdef DISABLE_TRANSCODING
  return false;
  #else
  
  CSharedLog::Shared()->Log(L_EXTENDED, "TranscodeContentFromFile :: " + p_sFileName, __FILE__, __LINE__);
  
  //m_bIsChunked = true;
  m_bIsBinary  = true;  
    
  if(m_pTranscodingSessionInfo) {
    //m_pTranscodingSessionInfo->m_pszBinBuffer = NULL;
    delete m_pTranscodingSessionInfo;
    
    CTranscodingCache::Shared()->ReleaseCacheObject(m_pTranscodingCacheObj);
    m_pTranscodingCacheObj = NULL;
  }  
  
  m_pTranscodingSessionInfo = new CTranscodeSessionInfo();
  m_pTranscodingSessionInfo->m_bBreakTranscoding   = false;
  m_pTranscodingSessionInfo->m_bIsTranscoding      = true;
  m_pTranscodingSessionInfo->m_sInFileName         = p_sFileName;  
  /*m_pTranscodingSessionInfo->m_pnBinContentLength  = &m_nBinContentLength;
  m_pTranscodingSessionInfo->m_pszBinBuffer        = &m_pszBinContent;*/
  m_pTranscodingSessionInfo->m_nGuessContentLength = 0;
  
  m_pTranscodingSessionInfo->m_sTitle = p_sTrackDetails.mAudioItem.sTitle;
  m_pTranscodingSessionInfo->m_sArtist = p_sTrackDetails.sArtist;
  m_pTranscodingSessionInfo->m_sAlbum  = p_sTrackDetails.sAlbum;
  m_pTranscodingSessionInfo->m_sGenre  = p_sTrackDetails.mAudioItem.sGenre;
  m_pTranscodingSessionInfo->m_sOriginalTrackNumber = p_sTrackDetails.sOriginalTrackNumber;
    
  
  m_pTranscodingCacheObj = CTranscodingCache::Shared()->GetCacheObject(m_pTranscodingSessionInfo->m_sInFileName);
  m_pTranscodingCacheObj->Init(m_pTranscodingSessionInfo, DeviceSettings());
  m_pTranscodingCacheObj->Transcode();

  if(m_pTranscodingCacheObj->Threaded() && 
     (m_pTranscodingSessionInfo->m_nGuessContentLength == 0 ||
      DeviceSettings()->TranscodingHTTPResponse(ExtractFileExt(p_sFileName)) == RESPONSE_CHUNKED)) {
    m_nTransferEncoding = HTTP_TRANSFER_ENCODING_CHUNKED;
  }
  
  return true;
  #endif
}

bool CHTTPMessage::IsTranscoding()
{ 
  #ifndef DISABLE_TRANSCODING
  if(m_pTranscodingCacheObj)
    return m_pTranscodingCacheObj->m_bIsTranscoding;
  else
  #endif
    return false;
}

void CHTTPMessage::BreakTranscoding()
{
  if(m_pTranscodingSessionInfo) {                            
    m_pTranscodingSessionInfo->m_bBreakTranscoding = true;   
  }
}



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
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_POST_SOAP_ACTION;
	}
  else
  {
    m_nHTTPMessageType = HTTP_MESSAGE_TYPE_POST;
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
  
  //m_sContent = p_sMessage.substr(p_sMessage.length() - m_nContentLength, m_nContentLength);
  
  return true;
}

bool CHTTPMessage::ParseSUBSCRIBEMessage(std::string p_sMessage)
{
/*
SUBSCRIBE /UPnPServices/ConnectionManager/event/ HTTP/1.1
HOST: 192.168.0.3:58444
CALLBACK: <http://192.168.0.3:49152/>
NT: upnp:event
TIMEOUT: Second-1801


SUBSCRIBE /UPnPServices/ConnectionManager/event/ HTTP/1.1
TIMEOUT: Second-15
HOST: 192.168.0.3:1050
CALLBACK: <http://192.168.0.3:42577>
NT: upnp:event
Content-Length: 0

*/     

  //cout << "PARSE SUBSCRIBE" << endl;

  RegEx rxCallBack("CALLBACK: *(.+)", PCRE_CASELESS);
  if(rxCallBack.Search(p_sMessage.c_str()))
  {
    m_sGENACallBack = rxCallBack.Match(1);
  }

  RegEx rxNT("NT: *(.+)", PCRE_CASELESS);
  if(rxNT.Search(p_sMessage.c_str()))
  {
    m_sGENANT = rxNT.Match(1);
  }

  // Header information: Timeout (GENA - Request)
  //std::string        m_sGENATimeout;  
  // Header information: Subscription-ID (GENA - Request & Response)
  //std::string        m_sGENASubscriptionID;    
  
  
  return true;     
}
