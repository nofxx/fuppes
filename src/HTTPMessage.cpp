/***************************************************************************
 *            HTTPMessage.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 
#include "HTTPMessage.h"

#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

#include "RegEx.h"
#include "UPnPActionFactory.h"

using namespace std;

CHTTPMessage::CHTTPMessage(eHTTPMessageType p_HTTPMessageType, 
													 eHTTPVersion p_HTTPVersion)
                           : CMessageBase("")
{
	m_HTTPMessageType = p_HTTPMessageType;
	m_HTTPVersion			= p_HTTPVersion;
//	m_HTTPContentType	= unknown;
  m_nBinContentLength = 0;  
}

CHTTPMessage::CHTTPMessage(eHTTPMessageType p_HTTPMessageType,													 
 													 eHTTPContentType p_HTTPContentType)
                          : CMessageBase("")
{
	m_HTTPMessageType = p_HTTPMessageType;
	m_HTTPVersion			= http_1_1;
	m_HTTPContentType = p_HTTPContentType;
  m_nBinContentLength = 0;
}

CHTTPMessage::CHTTPMessage(std::string p_sContent): CMessageBase(p_sContent)
{
	//cout << p_sContent << endl;
	m_nBinContentLength = 0;
  
	RegEx rxGET("GET +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
	if(rxGET.Search(p_sContent.c_str()))
	{
		m_HTTPMessageType = http_get;
		
		string sVersion = rxGET.Match(2);
		if(sVersion.compare("0"))		
			m_HTTPVersion = http_1_0;		
		else if(sVersion.compare("1"))		
			m_HTTPVersion = http_1_1;

		m_sRequest = rxGET.Match(1);	
		cout << "[HTTPMessage] GET " << m_sRequest << endl;
	}
	
	RegEx rxPOST("POST +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
	if(rxPOST.Search(p_sContent.c_str()))
	{
		m_HTTPMessageType = http_post;
		
		string sVersion = rxPOST.Match(2);
		if(sVersion.compare("0"))		
			m_HTTPVersion = http_1_0;		
		else if(sVersion.compare("1"))
			m_HTTPVersion = http_1_1;
		
		m_sRequest = rxPOST.Match(1);	
		cout << "[HTTPMessage] POST " << m_sRequest << endl;
    
    ParsePOSTMessage(p_sContent);
	}
}

eHTTPMessageType CHTTPMessage::GetMessageType()
{
	return m_HTTPMessageType;
}

std::string CHTTPMessage::GetRequest()
{
	return m_sRequest;
}

eHTTPContentType CHTTPMessage::GetContentType()
{
	return m_HTTPContentType;
}

void	CHTTPMessage::SetContent(std::string p_sContent)
{
	m_sContent = p_sContent;
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
    
    m_szBinContent = new char[m_nBinContentLength + 1];     
    fFile.read(m_szBinContent, m_nBinContentLength); 
    m_szBinContent[m_nBinContentLength] = '\0';    
    
    fFile.close();
  }  
	return true;
}

std::string CHTTPMessage::GetHeaderAsString()
{
	stringstream sResult;
	string sVersion;
	string sType;
	string sContentType;
	
	switch(m_HTTPVersion)
	{
		case http_1_0:
			sVersion = "HTTP/1.0";
		  break;
		case http_1_1:
			sVersion = "HTTP/1.1";
		  break;
	}
	
	switch(m_HTTPMessageType)
	{
		case http_get:
			// todo
			break;
		case http_post:
			// todo
		  break;
		case http_200_ok:
			sResult << sVersion << " " << "200 OK\r\n";
			break;
	  case http_404_not_found:
			// todo
			break;
	}
	
	sResult << "CONTENT-LENGTH: " << strlen(m_sContent.c_str()) << "\r\n";
	
	switch(m_HTTPContentType)
	{
		case text_html:
			sContentType = "text/html";
		  break;
		case text_xml:
			sContentType = "text/xml";
		  break;
		case audio_mpeg:
			sContentType = "audio/mpeg";
		  break;	
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

CUPnPAction* CHTTPMessage::GetAction()
{
  return m_pUPnPAction;
}

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
      
  RegEx rxContentLength("CONTENT-LENGTH: *(\\d+)", PCRE_CASELESS);
  if(rxContentLength.Search(p_sMessage.c_str()))
  {
    string sContentLength = rxContentLength.Match(1);
    m_nContentLength = std::atoi(sContentLength.c_str());
    //cout << "[HTTPMessage] CONTENT-LENGTH  " << m_nContentLength << endl;
  }
  
  m_sContent = p_sMessage.substr(p_sMessage.length() - m_nContentLength, m_nContentLength);
  
  CUPnPActionFactory* pFactory = new CUPnPActionFactory();
  m_pUPnPAction = pFactory->BuildActionFromString(m_sContent);
}
