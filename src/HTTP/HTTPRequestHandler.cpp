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
#include "../ContentDirectory/PlaylistFactory.h"
#include "../UUID.h"
#include "../GENA/SubscriptionMgr.h"

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
  /*cout << "CHTTPRequestHandler::HandleHTTPRequest()" << endl;  */
  //cout << "Request: " << pRequest->GetRequest() << endl;
  
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
    delete pHandler;
    return true;
  }
  
  
  /* Playlist-Item */
  if((sRequest.length() > 23) && (sRequest.substr(0, 23).compare("/MediaServer/Playlists/") == 0))
  {
    string sItemObjId = sRequest.substr(23, sRequest.length());    
    CPlaylistFactory* pFact = new CPlaylistFactory();    
    string sPlaylist = pFact->BuildPlaylist(sItemObjId);
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
    if(ExtractFileExt(sItemObjId).compare("pls") == 0)
      pResponse->SetContentType(MIME_TYPE_AUDIO_X_SCPLS);
    else if(ExtractFileExt(sItemObjId).compare("m3u") == 0)
      pResponse->SetContentType(MIME_TYPE_AUDIO_X_MPGEURL);
    pResponse->SetContent(sPlaylist);    

    delete pFact;
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
  //cout << pRequest->GetMessage() << endl;
  
  CSubscriptionMgr::Shared()->HandleSubscription(pRequest, pResponse);
  
  pResponse->SetVersion(pRequest->GetVersion());
  pResponse->SetMessageType(HTTP_MESSAGE_TYPE_GENA_OK);
 
  cout << pResponse->GetHeaderAsString() << endl;
  /*string sID = GenerateUUID();
  pResponse->SetGENASubscriptionID(sID);*/
  
  
  return true;
}
