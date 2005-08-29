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

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CHTTPMessage::CHTTPMessage()
{
  /* Init */
  m_HTTPVersion			  = HTTP_VERSION_1_1;
  m_HTTPMessageType   = HTTP_MESSAGE_TYPE_UNKNOWN;
	m_HTTPContentType   = HTTP_CONTENT_TYPE_UNKNOWN;
  m_nBinContentLength = 0; 
  m_nContentLength    = 0;
  m_pszBinContent     = NULL;
  m_bIsChunked        = false;
}

CHTTPMessage::~CHTTPMessage()
{
  /* Cleanup */
  /* uv 2005-08-05 :: memory allocated with new[] 
     has to be freed with delete[] instead of delete */
  //SAFE_DELETE(m_pszBinContent);
  if(m_pszBinContent)
    delete[] m_pszBinContent;
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
	sResult << "CONTENT-LENGTH: " << (int)strlen(m_sContent.c_str()) << "\r\n";
	
  /* Content type */
	switch(m_HTTPContentType)
	{
		case HTTP_CONTENT_TYPE_TEXT_HTML:  sContentType = "text/html";  break;
		case HTTP_CONTENT_TYPE_TEXT_XML:   sContentType = "text/xml";   break;
		case HTTP_CONTENT_TYPE_AUDIO_MPEG: sContentType = "audio/mpeg"; break;
    case HTTP_CONTENT_TYPE_IMAGE_PNG : sContentType = "image/png";  break;      
    default:                           ASSERT(0);                   break;	
	}
	
	sResult << "CONTENT-TYPE: " << sContentType << "\r\n\r\n";
	return sResult.str();
}

std::string CHTTPMessage::GetMessageAsString()
{
  stringstream sResult;
  sResult << GetHeaderAsString();
  sResult << m_sContent;
  return sResult.str();
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
  }
  
  return true;
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
