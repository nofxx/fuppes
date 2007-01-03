/***************************************************************************
 *            HTTPRequestHandler.cpp
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "HTTPRequestHandler.h"
#include "../Presentation/PresentationHandler.h"
#include "../ContentDirectory/PlaylistFactory.h"
#include "../Common/UUID.h"
#include "../GENA/SubscriptionMgr.h"
#include "../SharedLog.h"
#include "../ContentDirectory/FileDetails.h"

#include <iostream>
#include <sstream>

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
  CSharedLog::Shared()->Log(L_EXTENDED, "HandleHTTPRequest() :: " + pRequest->GetRequest(), __FILE__, __LINE__);  

  string sRequest = pRequest->GetRequest();  
  bool   bResult  = false;  
  
  /* Presentation */
  if(
     ((sRequest.compare("/") == 0) || (ToLower(sRequest).compare("/index.html") == 0)) ||
     ((sRequest.length() > 14) && (ToLower(sRequest).substr(0, 14).compare("/presentation/") == 0))
    )
  {
    CPresentationHandler* pHandler = new CPresentationHandler();
    pHandler->OnReceivePresentationRequest(NULL, pRequest, pResponse);
    delete pHandler;
    bResult = true;
  }
  
  
  /* Playlist-Item */
  if((sRequest.length() > 23) && (sRequest.substr(0, 23).compare("/MediaServer/Playlists/") == 0))
  {
    #warning FIXME: get the playlist type from db
    string sObjectId = sRequest.substr(23, sRequest.length());
    CPlaylistFactory* pFact = new CPlaylistFactory();    
    string sPlaylist = pFact->BuildPlaylist(sObjectId);
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
    if(ExtractFileExt(sObjectId).compare("pls") == 0)      
      pResponse->SetContentType(CFileDetails::GetMimeType("pls", false));
    else if(ExtractFileExt(sObjectId).compare("m3u") == 0)
      pResponse->SetContentType(CFileDetails::GetMimeType("m3u", false));      
    pResponse->SetContent(sPlaylist);    

    delete pFact;
    bResult = true;
  }


  /* AudioItem, ImageItem, videoItem */
  else if(
          ((sRequest.length() > 24) && 
           ((sRequest.substr(0, 24).compare("/MediaServer/AudioItems/") == 0) ||
            (sRequest.substr(0, 24).compare("/MediaServer/ImageItems/") == 0) ||
            (sRequest.substr(0, 24).compare("/MediaServer/VideoItems/") == 0)
           )))
  {
    string sObjectId = TruncateFileExt(sRequest.substr(24, sRequest.length()));
    bResult = HandleItemRequest(sObjectId, pRequest->GetMessageType(), pResponse);    
  }
  
  /* set remaining response values */
  pResponse->SetVersion(pRequest->GetVersion());
  if(!bResult)
  {
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_404_NOT_FOUND);
    pResponse->SetContentType(MIME_TYPE_TEXT_HTML);    
    bResult = false;
  }  
  
  return bResult;
}
    
bool CHTTPRequestHandler::HandleSOAPAction(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  return false;
}

bool CHTTPRequestHandler::HandleGENAMessage(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  //return false;
  //cout << pRequest->GetMessage() << endl;
  CSubscriptionMgr* pMgr = new CSubscriptionMgr();  
  pMgr->HandleSubscription(pRequest, pResponse);
  delete pMgr;  
  
  pResponse->SetVersion(pRequest->GetVersion());
  pResponse->SetMessageType(HTTP_MESSAGE_TYPE_GENA_OK);
 
  cout << pResponse->GetHeaderAsString() << endl;
  /*string sID = GenerateUUID();
  pResponse->SetGENASubscriptionID(sID);*/
  
  
  return true;
}

bool CHTTPRequestHandler::HandleItemRequest(std::string p_sObjectId, HTTP_MESSAGE_TYPE p_nRequestType, CHTTPMessage* pResponse)
{
  std::stringstream sSql;
  OBJECT_TYPE       nObjectType;
  std::string       sExt;
  std::string       sPath;
  std::string       sMimeType;
  CContentDatabase* pDb         = new CContentDatabase();  
  bool              bResult     = false;  
  
  sSql << "select * from objects where ID = " << HexToInt(p_sObjectId) << ";";  
  pDb->Select(sSql.str());
  
  if(!pDb->Eof())
  {
    //nObjectType = (OBJECT_TYPE)atoi(pDb->GetResult()->GetValue("TYPE").c_str());
    //cout << "OBJECT_TYPE: " << nObjectType << endl;
        
    sPath = pDb->GetResult()->GetValue("PATH");    
    sExt  = ExtractFileExt(sPath);
    
    if(!FileExists(sPath))
    {
      CSharedLog::Shared()->Log(L_WARNING, "file: " + sPath + " not found", __FILE__, __LINE__);
      bResult = false;      
    }
    else
    {    
      if(CFileDetails::IsTranscodingExtension(sExt))
      {
        CSharedLog::Shared()->Log(L_EXTENDED, "transcode " + sPath, __FILE__, __LINE__);  
        #warning TODO: check if transcoding is possible
        sMimeType = CFileDetails::GetMimeType(sPath, true);
        if(p_nRequestType == HTTP_MESSAGE_TYPE_GET)
        {
          cout << "TRANSCODE" << endl;
          pResponse->TranscodeContentFromFile(sPath);
        }
        else if(p_nRequestType == HTTP_MESSAGE_TYPE_HEAD)
          pResponse->SetIsChunked(true); // mark the head response as chunked so the correct header will be build
      }
      else
      {
        sMimeType = pDb->GetResult()->GetValue("MIME_TYPE");
        pResponse->LoadContentFromFile(sPath);
      }
    
      cout << "mime type: " << sMimeType << " " << __FILE__ << " " << __LINE__ << endl;
      
      // we always set the response type to "200 OK"
      // if the message should be a "206 partial content" 
      // CHTTPServer will change the type
      pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
      pResponse->SetContentType(sMimeType);
      bResult = true;
    }
  }
  else // eof
  {
    CSharedLog::Shared()->Log(L_WARNING, "unknown object id: " + p_sObjectId , __FILE__, __LINE__);
    bResult = false;
  }
  
  delete pDb;
  return bResult;
}
