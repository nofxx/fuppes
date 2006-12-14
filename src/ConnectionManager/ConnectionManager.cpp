/***************************************************************************
 *            ConnectionManager.cpp
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
 
#include "ConnectionManager.h"
#include "ConnectionManagerDescription.cpp"

using namespace std;

CConnectionManager::CConnectionManager(std::string p_sHTTPServerURL):
CUPnPService(UPNP_DEVICE_TYPE_CONNECTION_MANAGER, p_sHTTPServerURL)
{
}


std::string CConnectionManager::GetServiceDescription()
{  
  return sConnectionManagerDescription;
}

void CConnectionManager::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
  #warning todo
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
