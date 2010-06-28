/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            SoapControl.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#include "SoapControl.h"

#include "ControlInterface.h"

#include <iostream>
#include <sstream>
using namespace std;


#include "../ContentDirectory/ContentDatabase.h"


SoapControl::SoapControl(std::string HTTPServerURL):
CUPnPService(FUPPES_SOAP_CONTROL, 1, HTTPServerURL)
{
}

std::string SoapControl::GetServiceDescription()
{
	return "";
}

void SoapControl::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
	FuppesCtrlAction* action = (FuppesCtrlAction*)pUPnPAction;
	
	cout << "SoapControl:: handleAction : " << pUPnPAction->GetContent() << endl;
	
	stringstream content;


	switch(action->type()) {

		case FUPPES_CTRL_DATABASE_REBUILD:
			break;
		
	}
	

	if(action->type() == FUPPES_CTRL_TEST) {
			content << 
				"<c:TestResponse xmlns:c=\"urn:fuppesControl\">"
				"<Result>test</Result>"
				"</c:TestResponse>";


		//CContentDatabase::exportData("/home/ulrich/Desktop/export.db", "/home/ulrich/Desktop/fuppes-test/Oregon/", true);		
	}

	
  if(content.str().empty()) { 
		content << 
				"<c:Error xmlns:c=\"urn:fuppesControl\">"
				"<Code>123</Code>"
				"<Message>fuppes soap control error</Message>"
				"</c:Error>";
	}
  
	string result = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
		"  <s:Body>"
		+ content.str() +
		"  </s:Body>"
		"</s:Envelope>";
	
	pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml; charset=\"utf-8\"");	
  pMessageOut->SetContent(result);

cout << result << endl;
	
/*
  else {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR, "text/xml; charset=\"utf-8\"");            

    content = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"  
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <s:Fault>"
    "      <faultcode>s:Client</faultcode>"
    "      <faultstring>UPnPError</faultstring>"
    "      <detail>"
    "        <UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">"
    "          <errorCode>401</errorCode>"
    "          <errorDescription>Invalid Action</errorDescription>"
    "        </UPnPError>"
    "      </detail>"
    "    </s:Fault>"
    "  </s:Body>"
    "</s:Envelope>";
    
    pMessageOut->SetContent(content);
	}
*/
}
