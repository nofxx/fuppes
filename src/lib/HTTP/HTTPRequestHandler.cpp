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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "HTTPRequestHandler.h"
#include "../Presentation/PresentationHandler.h"
#include "../ContentDirectory/PlaylistFactory.h"
#include "../Common/UUID.h"
#include "../GENA/SubscriptionMgr.h"
#include "../SharedLog.h"
#include "../ContentDirectory/FileDetails.h"
#include "../ContentDirectory/VirtualContainerMgr.h"

#include <iostream>
#include <sstream>

using namespace std;

CHTTPRequestHandler::CHTTPRequestHandler(std::string p_sHTTPServerURL)
{
  m_sHTTPServerURL = p_sHTTPServerURL;
}

bool CHTTPRequestHandler::HandleRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  /*cout << "CHTTPRequestHandler::HandleRequest()" << endl;    
  cout << "Type: " << pRequest->GetMessageType() << endl;  */
  bool bResult; 
  
  pResponse->DeviceSettings(pRequest->DeviceSettings());  
  
  switch(pRequest->GetMessageType())
  {
    // HTTP request
    case HTTP_MESSAGE_TYPE_GET:
    case HTTP_MESSAGE_TYPE_HEAD:
    case HTTP_MESSAGE_TYPE_POST:
      bResult = this->HandleHTTPRequest(pRequest, pResponse);
      break;
      
    // SOAP
    case HTTP_MESSAGE_TYPE_POST_SOAP_ACTION:
      bResult = this->HandleSOAPAction(pRequest, pResponse);
      break;
      
    // GENA
    case HTTP_MESSAGE_TYPE_SUBSCRIBE:
      bResult = this->HandleGENAMessage(pRequest, pResponse);
      break;
    
    default :
      bResult = false;    
      break;
  }
  
  return bResult;  
}

bool CHTTPRequestHandler::HandleHTTPRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  CSharedLog::Shared()->Log(L_EXT, "HandleHTTPRequest() :: " + pRequest->GetRequest(), __FILE__, __LINE__);  

  string sRequest = pRequest->GetRequest();  
  bool   bResult  = false;  
  
  // set version
  pResponse->SetVersion(pRequest->GetVersion());
  
  
  // ContentDirectory description
  if(sRequest.compare("/UPnPServices/ContentDirectory/description.xml") == 0) {
    pResponse->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");
    
    CContentDirectory* pDir = new CContentDirectory(m_sHTTPServerURL);
    pResponse->SetContent(pDir->GetServiceDescription());
    delete pDir;
    return true;
  }

  //ConnectionManager description
  if(sRequest.compare("/UPnPServices/ConnectionManager/description.xml") == 0) {
    pResponse->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");
      
    CConnectionManager* pMgr = new CConnectionManager(m_sHTTPServerURL);
    pResponse->SetContent(pMgr->GetServiceDescription());
    delete pMgr;
    return true;
  }
  
  
  // Presentation
  if(
     ((sRequest.compare("/") == 0) || (ToLower(sRequest).compare("/index.html") == 0)) ||
     ((sRequest.length() > 14) && (ToLower(sRequest).substr(0, 14).compare("/presentation/") == 0))
    )
  {
    CPresentationHandler* pHandler = new CPresentationHandler();
    pHandler->OnReceivePresentationRequest(pRequest, pResponse);
    delete pHandler;    
    return true;
  }
  
  
  /* Playlist-Item */
  if((sRequest.length() > 23) && (sRequest.substr(0, 23).compare("/MediaServer/Playlists/") == 0))
  {
    #warning FIXME: get the playlist type from db
    string sObjectId = sRequest.substr(23, sRequest.length());
    CPlaylistFactory* pFact = new CPlaylistFactory();    
    string sPlaylist = pFact->BuildPlaylist(sObjectId);
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);    
    string sExt = ExtractFileExt(sObjectId);
    pResponse->SetContentType(pRequest->DeviceSettings()->MimeType(sExt));
    //pResponse->SetContentType(CFileDetails::GetMimeType(sObjectId));    
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
    bResult = HandleItemRequest(sObjectId, pRequest, pResponse);    
  }
  
  /* set 404 */
  if(!bResult) {
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_404_NOT_FOUND);
    pResponse->SetContentType(MIME_TYPE_TEXT_HTML);    
    bResult = false;
  }  
  
  return bResult;
}
    
bool CHTTPRequestHandler::HandleSOAPAction(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{  
  // get UPnP action
  CUPnPAction* pAction = NULL;
  pAction = pRequest->GetAction();
  if(!pAction) {
    return false;
  }

  // set version
  pResponse->SetVersion(pRequest->GetVersion());
  
  // handle UPnP action
  bool bRet = true;
  CContentDirectory* pDir = NULL; //new CContentDirectory(m_sHTTPServerURL);
  CConnectionManager* pMgr = NULL; //new CConnectionManager(m_sHTTPServerURL);;
  CXMSMediaReceiverRegistrar* pXMS = NULL; //new CXMSMediaReceiverRegistrar();
  
  switch(pAction->GetTargetDeviceType())
  {
    case UPNP_SERVICE_CONTENT_DIRECTORY:
      pDir = new CContentDirectory(m_sHTTPServerURL);
      pDir->HandleUPnPAction(pAction, pResponse);
      //cout << "HTTPREQHND: delete CDIR" << endl; fflush(stdout);
      delete pDir;
      //cout << "HTTPREQHND: CDIR DELETED" << endl; fflush(stdout);      
      break;
    case UPNP_SERVICE_CONNECTION_MANAGER:
      pMgr = new CConnectionManager(m_sHTTPServerURL);
      pMgr->HandleUPnPAction(pAction, pResponse);
      delete pMgr;
      break;    
		case UPNP_SERVICE_X_MS_MEDIA_RECEIVER_REGISTRAR:
      pXMS = new CXMSMediaReceiverRegistrar();
      pXMS->HandleUPnPAction(pAction, pResponse);
      delete pXMS;
      break;
    default:
      bRet = false;
      break;
  }
  
  return bRet;
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
 
  //cout << pResponse->GetHeaderAsString() << endl;
  /*string sID = GenerateUUID();
  pResponse->SetGENASubscriptionID(sID);*/
  
  
  return true;
}

bool CHTTPRequestHandler::HandleItemRequest(std::string p_sObjectId, CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  std::stringstream sSql;
  OBJECT_TYPE       nObjectType;
  std::string       sExt;
  std::string       sPath;
  std::string       sMimeType;
  CContentDatabase* pDb         = new CContentDatabase();  
  bool              bResult     = true;  
  
  
  string sDevice = " and o.DEVICE is NULL ";
  if(CVirtualContainerMgr::Shared()->IsVirtualContainer(HexToInt(p_sObjectId), pRequest->DeviceSettings()->VirtualFolderDevice()))
    sDevice = " and o.DEVICE = '" + pRequest->DeviceSettings()->VirtualFolderDevice() + "' ";
  
  sSql << 
    "select " <<
    "  * " <<
    "from " <<
    "  OBJECTS o " <<
    "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  o.OBJECT_ID = " << HexToInt(p_sObjectId) << sDevice;
  
  pDb->Select(sSql.str());
  
  if(!pDb->Eof())
  {
    //nObjectType = (OBJECT_TYPE)atoi(pDb->GetResult()->GetValue("TYPE").c_str());
    //cout << "OBJECT_TYPE: " << nObjectType << endl;
        
    sPath = pDb->GetResult()->GetValue("PATH");    
    sExt  = ExtractFileExt(sPath);
    
    if(!FileExists(sPath)) {
      CSharedLog::Shared()->Log(L_EXT, "file: " + sPath + " not found", __FILE__, __LINE__);
      bResult = false;
    }
    else
    {      
      if(pRequest->DeviceSettings()->DoTranscode(sExt, pDb->GetResult()->GetValue("A_CODEC"), pDb->GetResult()->GetValue("V_CODEC"))) {
        CSharedLog::Shared()->Log(L_EXT, "transcode " + sPath, __FILE__, __LINE__);        
     
        sMimeType = pRequest->DeviceSettings()->MimeType(sExt, pDb->GetResult()->GetValue("A_CODEC"), pDb->GetResult()->GetValue("V_CODEC"));
        if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_GET) {          
          //
          SAudioItem trackDetails;
          if(!pDb->GetResult()->IsNull("TITLE")) {
            trackDetails.sTitle = pDb->GetResult()->GetValue("TITLE");
          }
          
          if(!pDb->GetResult()->IsNull("A_ARTIST")) {
            trackDetails.sArtist = pDb->GetResult()->GetValue("A_ARTIST");
          }
          
          if(!pDb->GetResult()->IsNull("A_ALBUM")) {
            trackDetails.sAlbum  = pDb->GetResult()->GetValue("A_ALBUM");
          }
          
          if(!pDb->GetResult()->IsNull("A_GENRE")) {
            trackDetails.sGenre = pDb->GetResult()->GetValue("A_GENRE");
          }
          
          if(!pDb->GetResult()->IsNull("A_TRACK_NO")) {
            trackDetails.sOriginalTrackNumber = pDb->GetResult()->GetValue("A_TRACK_NO");
          }          
          
          if(!pDb->GetResult()->IsNull("A_CODEC")) {
            trackDetails.sACodec = pDb->GetResult()->GetValue("A_CODEC");
          }
          if(!pDb->GetResult()->IsNull("V_CODEC")) {
            trackDetails.sVCodec = pDb->GetResult()->GetValue("V_CODEC");
          }
          
          bResult = pResponse->TranscodeContentFromFile(sPath, trackDetails);
        }
        else if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_HEAD) {
          // mark the head response as chunked so
          // the correct header will be build
          pResponse->SetIsBinary(true);
          bResult = true;
						
          if(pRequest->DeviceSettings()->TranscodingHTTPResponse(sExt) == RESPONSE_CHUNKED) {
            pResponse->SetTransferEncoding(HTTP_TRANSFER_ENCODING_CHUNKED);
          }
          else if(pRequest->DeviceSettings()->TranscodingHTTPResponse(sExt) == RESPONSE_STREAM) {
            pResponse->SetTransferEncoding(HTTP_TRANSFER_ENCODING_NONE);
          }
        }
      }
      else {
        sMimeType = pRequest->DeviceSettings()->MimeType(sExt, pDb->GetResult()->GetValue("A_CODEC"), pDb->GetResult()->GetValue("V_CODEC"));
        pResponse->LoadContentFromFile(sPath);
      }      
      
      // we always set the response type to "200 OK"
      // if the message should be a "206 partial content" 
      // CHTTPServer will change the type
      pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
      pResponse->SetContentType(sMimeType);
      //bResult = true;
    }
  }
  else // eof
  {
    CSharedLog::Shared()->Log(L_EXT, "unknown object id: " + p_sObjectId , __FILE__, __LINE__);
    bResult = false;
  }
  
  delete pDb;
  return bResult;
}
