/***************************************************************************
 *            ConnectionManager.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "ConnectionManager.h"
#include "ConnectionManagerDescription.cpp"

using namespace std;

CConnectionManagerCore* CConnectionManagerCore::m_instance = 0;


void CConnectionManagerCore::init()
{
	if(m_instance == 0) {
		m_instance = new CConnectionManagerCore();
	}
}




CConnectionManager::CConnectionManager(std::string p_sHTTPServerURL):
	CUPnPService(UPNP_SERVICE_CONNECTION_MANAGER, p_sHTTPServerURL)
{
}

std::string CConnectionManager::GetServiceDescription()
{  
  return sConnectionManagerDescription;
}

void CConnectionManager::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
	CM_ERROR ret = ERR_INVALID_ACTION;
	string   res;
	
	switch(pUPnPAction->GetActionType()) {
		case CMA_GET_PROTOCOL_INFO:
			ret = getProtocolInfo(pUPnPAction, &res);
			break;
		case CMA_PREPARE_FOR_CONNECTION:
			ret = prepareForConnection(pUPnPAction, &res);
			break;
		case CMA_CONNECTION_COMPLETE:
			ret = connectionComplete(pUPnPAction, &res);
			break;
		case CMA_GET_CURRENT_CONNECTION_IDS:
			ret = getCurrentConnectionIds(pUPnPAction, &res);
			break;
		case CMA_GET_CURRENT_CONNECTION_INFO:
			ret = getCurrentConnectionInfo(pUPnPAction, &res);
			break;
	}

	if(ret == ERR_NONE) {
		pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml; charset=\"utf-8\"");
		pMessageOut->SetContent(res);
	}
	else {
		pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR, "text/xml; charset=\"utf-8\"");
		std::string sContent = 
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

CM_ERROR CConnectionManager::getProtocolInfo(CUPnPAction* /*action*/, std::string* /*result*/)
{
	return ERR_INVALID_ACTION;
}

CM_ERROR CConnectionManager::prepareForConnection(CUPnPAction* /*action*/, std::string* /*result*/)
{
	return ERR_INVALID_ACTION;
}

CM_ERROR CConnectionManager::connectionComplete(CUPnPAction* /*action*/, std::string* /*result*/)
{
	return ERR_INVALID_ACTION;
}

CM_ERROR CConnectionManager::getCurrentConnectionIds(CUPnPAction* /*action*/, std::string* /*result*/)
{
	return ERR_INVALID_ACTION;
}

CM_ERROR CConnectionManager::getCurrentConnectionInfo(CUPnPAction* /*action*/, std::string* /*result*/)
{
	return ERR_INVALID_ACTION;
}
