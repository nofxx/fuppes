/***************************************************************************
 *            HTTPRequestHandler.cpp
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

#include "HTTPRequestHandler.h"
#include "../Presentation/PresentationHandler.h"
#include "../UUID.h"

#include <iostream>

using namespace std;

CHTTPRequestHandler::CHTTPRequestHandler()
{
}

bool CHTTPRequestHandler::HandleRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  /*cout << "CHTTPRequestHandler::HandleRequest()" << endl;    
  cout << "Type: " << pRequest->GetMessageType() << endl;  */
  
  switch(pRequest->GetMessageType())
  {
    
    /* HTTP request */
    case HTTP_MESSAGE_TYPE_GET:
    case HTTP_MESSAGE_TYPE_HEAD:
    case HTTP_MESSAGE_TYPE_POST:
      return this->HandleHTTPRequest(pRequest, pResponse);
      
    /* GENA */
    case HTTP_MESSAGE_TYPE_SUBSCRIBE:
      return this->HandleGENAMessage(pRequest, pResponse);
    
    default :
      return false;    
  }
}

bool CHTTPRequestHandler::HandleHTTPRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  /*cout << "CHTTPRequestHandler::HandleHTTPRequest()" << endl;  
  cout << "Request: " << pRequest->GetRequest() << endl;*/
  
  pResponse->SetVersion(pRequest->GetVersion());
  
  string sRequest = pRequest->GetRequest();  
  
  /* Presentation */
  if(
     ((sRequest.compare("/") == 0) || (ToLower(sRequest).compare("/index.html") == 0)) ||
     ((sRequest.length() > 14) && (ToLower(sRequest).substr(0, 14).compare("/presentation/") == 0))
    )
  {
    CPresentationHandler* pHandler = new CPresentationHandler();
    pHandler->OnReceivePresentationRequest(NULL, pRequest, pResponse);
    return true;
  }
  
  
  
  return false;
}
    
bool CHTTPRequestHandler::HandleSOAPAction(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  return false;
}

bool CHTTPRequestHandler::HandleGENAMessage(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  pResponse->SetVersion(pRequest->GetVersion());
  pResponse->SetMessageType(HTTP_MESSAGE_TYPE_GENA_OK);
 
  string sID = "uuid:" + GenerateUUID(); 
  pResponse->SetGENASubscriptionID(sID);
  
  
  return true;
}
