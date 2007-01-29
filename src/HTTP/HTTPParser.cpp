/***************************************************************************
 *            HTTPParser.cpp
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
 
#include "HTTPParser.h"
#include "../Common/RegEx.h"

bool CHTTPParser::Parse(CHTTPMessage* pMessage)
{
  /* find out the message type and HTTP version */
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
	else
	  return false;
    
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
  
  return true;
}
