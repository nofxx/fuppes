/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            XMSMediaReceiverRegistrar.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "XMSMediaReceiverRegistrar.h"
#include "XMSMediaReceiverRegistrarDescription.cpp"

CXMSMediaReceiverRegistrar::CXMSMediaReceiverRegistrar(std::string HTTPServerURL):
CUPnPService(UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR, HTTPServerURL)
{
}

CXMSMediaReceiverRegistrar::~CXMSMediaReceiverRegistrar()
{
}

std::string CXMSMediaReceiverRegistrar::GetServiceDescription()
{
  return sXMSMediaReceiverRegistrarDescription;
}
		
void CXMSMediaReceiverRegistrar::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
  string sContent = "";

  if(pUPnPAction->GetActionType() == UPNP_IS_AUTHORIZED) {
	  sContent =
  		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
			"  <s:Body>"
			"    <u:IsAuthorizedResponse xmlns:u=\"urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1\">"
			"      <Result>1</Result>"
			"    </u:IsAuthorizedResponse>"
			"  </s:Body>"
			"</s:Envelope>";    

/* twonky 5 sends the following to WMP 11
   <?xml version="1.0" encoding="utf-8" ?><SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><m:IsAuthorizedResponse xmlns:m="urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1"><Result xmlns:dt="urn:schemas-microsoft-com:datatypes" dt:dt="int">1</Result></m:IsAuthorizedResponse></SOAP-ENV:Body></SOAP-ENV:Envelope>
*/
    
	}
	else if(pUPnPAction->GetActionType() == UPNP_IS_VALIDATED) {
	  sContent =
  		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		  "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
			"  <s:Body>"
			"    <u:IsValidatedResponse xmlns:u=\"urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1\">"
			"      <Result>1</Result>"
			"    </u:IsValidatedResponse>"
			"  </s:Body>"
			"</s:Envelope>";

/* twonky 5 <> wmp 11
<?xml version="1.0" encoding="utf-8" ?><s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><s:Body><m:IsValidatedResponse xmlns:m="urn:microsoft.com:service:X_MS_MediaReceiverRegistrar:1"><Result>1</Result></m:IsValidatedResponse></s:Body></s:Envelope>
*/
    
	}

	else if(pUPnPAction->GetActionType() == UPNP_REGISTER_DEVICE) {
	  sContent =
  		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>"
      "<SOAP-ENV:Envelope xmlns:SOAP-ENV=\"http://schemas.xmlsoap.org/soap/envelope/\" SOAP-ENV:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
      "  <SOAP-ENV:Body>"
      "    <SOAP-ENV:Fault>"
      "      <faultcode>SOAP-ENV:Client</faultcode>"
      "      <faultstring>UPnPError</faultstring>"
      "      <detail>"
      "        <u:UPnPError xmlns:u=\"urn:schemas-upnp-org:control-1-0\">"
      "          <u:errorCode>501</u:errorCode>"
      "          <u:errorDescription>Action Failed</u:errorDescription>"
      "        </u:UPnPError>"
      "      </detail>"
      "    </SOAP-ENV:Fault>"
      "  </SOAP-ENV:Body>"
      "</SOAP-ENV:Envelope>";
/* twonky 5 <> wmp 11
<?xml version="1.0" encoding="utf-8" ?><SOAP-ENV:Envelope xmlns:SOAP-ENV="http://schemas.xmlsoap.org/soap/envelope/" SOAP-ENV:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/"><SOAP-ENV:Body><SOAP-ENV:Fault><faultcode>SOAP-ENV:Client</faultcode><faultstring>UPnPError</faultstring><detail><u:UPnPError xmlns:u="urn:schemas-upnp-org:control-1-0"><u:errorCode>501</u:errorCode><u:errorDescription>Action Failed</u:errorDescription></u:UPnPError></detail></SOAP-ENV:Fault></SOAP-ENV:Body></SOAP-ENV:Envelope>
*/
	}
  
  if(!sContent.empty())
  {    
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml; charset=\"utf-8\"");
    pMessageOut->SetContent(sContent);
  }
  else
  {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR, "text/xml; charset=\"utf-8\"");            

    sContent = 
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
    
    pMessageOut->SetContent(sContent);
	}
}
