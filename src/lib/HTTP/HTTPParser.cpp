/***************************************************************************
 *            HTTPParser.cpp
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
 
#include "HTTPParser.h"
#include "../Common/RegEx.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"

bool CHTTPParser::Parse(CHTTPMessage* pMessage)
{
  m_pMessage = pMessage;

  /* detect message type and HTTP version */
  std::string sType;
  int nVersion;
  
  RegEx rxRequest("([GET|HEAD|POST|SUBSCRIBE|UNSUBSCRIBE|NOTIFY]+) +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  RegEx rxResponse("HTTP/1\\.([1|0]) +(\\d+) +(.+)", PCRE_CASELESS);
 	
	// it's a request
  if(rxRequest.Search(pMessage->GetHeader().c_str())) { 
    sType    = rxRequest.Match(1);
    nVersion = atoi(rxRequest.Match(3));
  }
	// it's a response
  else if(rxResponse.Search(pMessage->GetHeader().c_str())) {
		sType    = rxResponse.Match(2);
		nVersion = atoi(rxResponse.Match(1));
	}
	else {    
		return false;
	}
  
	// set version
	if(nVersion == 0)
	  pMessage->SetVersion(HTTP_VERSION_1_0);
	else if(nVersion == 1)
	  pMessage->SetVersion(HTTP_VERSION_1_1);
	else {    
	  return false;
  }
    
  // set message type
  sType = ToUpper(sType);
	
  // GET
  if(sType.compare("GET") == 0) {
	  pMessage->SetMessageType(HTTP_MESSAGE_TYPE_GET);
	}
  
	// HEAD
  else if(sType.compare("HEAD") == 0) {
	  pMessage->SetMessageType(HTTP_MESSAGE_TYPE_HEAD);
  }
	
  // POST
	else if(sType.compare("POST") == 0) {
	  pMessage->SetMessageType(HTTP_MESSAGE_TYPE_POST);
  }

  /* SUBSCRIBE|UNSUBSCRIBE */
  
  /* NOTIFY */
  
  // 200 OK
	else if(sType.compare("200") == 0) {
	  pMessage->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
	}
	
	// 403 FORBIDDEN
	else if(sType.compare("403") == 0) {
	  pMessage->SetMessageType(HTTP_MESSAGE_TYPE_403_FORBIDDEN);
	}
	
	// 404 NOT FOUND
	else if(sType.compare("404") == 0) {
	  pMessage->SetMessageType(HTTP_MESSAGE_TYPE_404_NOT_FOUND);
	}
  
	
	ParseCommonValues();  
	CDeviceIdentificationMgr::Shared()->IdentifyDevice(pMessage); 
  return true;
}

void CHTTPParser::ParseCommonValues()
{
  RegEx rxUserAgent("USER-AGENT: *(.*)\r\n", PCRE_CASELESS);
	if(rxUserAgent.Search(m_pMessage->GetHeader().c_str())) {
		m_pMessage->m_sUserAgent = rxUserAgent.Match(1);
	}
}

void CHTTPParser::ConvertURLEncodeContentToPlain(CHTTPMessage* pMessage)
{
  m_pMessage = pMessage;
 
  string sContentType;
	string sBoundary;
	
	RegEx rxContentType("CONTENT-TYPE: *([text/plain|application/x\\-www\\-form\\-urlencoded]+)(.*)", PCRE_CASELESS);
	if(rxContentType.Search(pMessage->GetHeader().c_str())) {
	  sContentType = rxContentType.Match(1);
		if(rxContentType.SubStrings() == 3)
  		sBoundary = rxContentType.Match(2);
		
		if((ToLower(sContentType).compare("text/plain") == 0) && (ToLower(sBoundary).find("boundary") == std::string::npos))
		  return;
	}
	else {
	  return;
	}
	
	string sPost = m_pMessage->GetContent();
	string sPart;
	stringstream sVars;

  RegEx rxValue("([\\w|_|-|\\d]+)=(.*)");
	
	while(sPost.length() > 0)
	{
	  if(sPost.find("&") != std::string::npos) {
		  sPart = sPost.substr(0, sPost.find("&"));
		  sPost = sPost.substr(sPost.find("&") + 1, sPost.length());
		}
		else {
		  sPart = sPost;
	    sPost = "";
		}
			
		if(rxValue.Search(sPart.c_str())) {
			sVars << rxValue.Match(1) << "=" << URLEncodeValueToPlain(rxValue.Match(2)) << "\r\n";
		}
		else {
			sVars << sPart << "\r\n";
		}	
	}
	
  m_pMessage->SetContent(sVars.str());
}

std::string CHTTPParser::URLEncodeValueToPlain(std::string p_sValue)
{
  string sResult = p_sValue;
	string sChar;
	string sMatch;
	char cChar;
		
	RegEx rxChar("%([A-F|0-9]{2})\\+*");
	
	if(rxChar.Search(p_sValue.c_str())) {
	  sResult = "";
		while(true) 
		{
		  sMatch = rxChar.Match(0);
		  sChar  = rxChar.Match(1);
		  
			cChar = HexToInt(sChar);
						
			sResult += p_sValue.substr(0, p_sValue.find(sMatch));
			sResult += cChar;
			
			if(strcmp(&sMatch[sMatch.length() - 1], "+") == 0) // +
			  sResult += " ";
			
			p_sValue = p_sValue.substr(p_sValue.find(sMatch) + sMatch.length(), p_sValue.length());	
		
		  if(rxChar.Search(p_sValue.c_str()))
			  continue;
				
			if(p_sValue.length() > 0) 
			  sResult += p_sValue;
			break;		
		} // while bLoop
	}	
	
	return sResult;
}
