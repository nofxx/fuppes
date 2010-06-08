/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            HTTPRequestHandler.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "HTTPRequestHandler.h"
#include "../Presentation/PresentationHandler.h"
#include "../ContentDirectory/PlaylistFactory.h"
#include "../Common/UUID.h"
#include "../Common/File.h"
#include "../Common/RegEx.h"
#include "../GENA/SubscriptionMgr.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"
#include "../ContentDirectory/FileDetails.h"
#include "../ContentDirectory/DatabaseConnection.h"
#include "../Plugins/Plugin.h"
#include "../Transcoding/TranscodingMgr.h"
#include "../ControlInterface/SoapControl.h"

//#include <iostream>
#include <sstream>
using namespace std;
using namespace fuppes;

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
			Log::log(Log::soap, Log::debug, __FILE__, __LINE__, "RESPONSE:\n" + pResponse->GetMessageAsString());
      break;
      
    // GENA
    case HTTP_MESSAGE_TYPE_SUBSCRIBE:
      bResult = this->HandleGENAMessage(pRequest, pResponse);
			Log::log(Log::gena, Log::debug, __FILE__, __LINE__, "RESPONSE:\n" + pResponse->GetMessageAsString());
			break;
    
    default :
      bResult = false;    
      break;
  }
  
  return bResult;  
}

bool CHTTPRequestHandler::HandleHTTPRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
	Log::log(Log::http, Log::debug, __FILE__, __LINE__, "REQUEST:\n" + pRequest->GetMessage());
	
  string sRequest = pRequest->GetRequest();  
  bool   bResult  = false;  
  
  // set version
  pResponse->SetVersion(pRequest->GetVersion());
  
  
  // ContentDirectory description
  if(sRequest.compare("/UPnPServices/ContentDirectory/description.xml") == 0) {
    pResponse->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");
    
    CContentDirectory dir(m_sHTTPServerURL);
    pResponse->SetContent(dir.GetServiceDescription());
    return true;
  }

  // ConnectionManager description
  if(sRequest.compare("/UPnPServices/ConnectionManager/description.xml") == 0) {
    pResponse->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");

    CConnectionManager mgr(m_sHTTPServerURL);
    pResponse->SetContent(mgr.GetServiceDescription());
    return true;
  }

	// XMSMediaReceiverRegistrar description
  if(sRequest.compare("/UPnPServices/XMSMediaReceiverRegistrar/description.xml") == 0) {
    pResponse->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");

    CXMSMediaReceiverRegistrar reg(m_sHTTPServerURL);
    pResponse->SetContent(reg.GetServiceDescription());
    return true;
  }
  
  // Presentation
  if(
     ((sRequest.compare("/") == 0) || (ToLower(sRequest).compare("/index.html") == 0)) ||
     ((sRequest.length() > 14) && (ToLower(sRequest).substr(0, 14).compare("/presentation/") == 0))
    )
  {
		CPresentationPlugin* pres = CPluginMgr::presentationPlugin();
    if(pres && pres->handleRequest(pRequest, pResponse)) {
   		return true;
		}

    CPresentationHandler* pHandler = new CPresentationHandler(m_sHTTPServerURL);
    pHandler->OnReceivePresentationRequest(pRequest, pResponse);
    delete pHandler;    
    return true;
  }
  
  
  /* Playlist-Item */
  if((sRequest.length() > 23) && (sRequest.substr(0, 23).compare("/MediaServer/Playlists/") == 0))
  {
    #warning FIXME: get the playlist type from db
    string sObjectId = sRequest.substr(23, sRequest.length());
    string sExt = ExtractFileExt(sObjectId);
    sObjectId = TruncateFileExt(sObjectId);

    switch(pRequest->DeviceSettings()->playlistStyle()) {
      case CDeviceSettings::container:
      case CDeviceSettings::file:
        break;
      case CDeviceSettings::pls:
        sExt = "pls";
        break;
      case CDeviceSettings::m3u:
        sExt = "m3u";
        break;
      case CDeviceSettings::wpl:
        sExt = "wpl";
        break;
      case CDeviceSettings::xspf:
        sExt = "xspf";
        break;
    }
    
    PlaylistFactory factory(m_sHTTPServerURL);
    string sPlaylist = factory.BuildPlaylist(sObjectId, sExt);
    
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
    string mimeType = pRequest->DeviceSettings()->MimeType(sExt);
    pResponse->SetContentType(mimeType);
    //pResponse->SetContentType(CFileDetails::GetMimeType(sObjectId));    
    pResponse->SetContent(sPlaylist); 


    bResult = true;
  }


  /* AudioItem, ImageItem, videoItem */
  else {

    RegEx rxUrl("/(Audio|Video|Image)Items/([0-9|A-F|a-f]+)/*[\\w|%20|-]*\\.(\\w+)");
    if(!rxUrl.search(sRequest)) {
      cout << "malformed url: " << sRequest << endl;
    }
    else {

      string sObjectId = rxUrl.match(2);
      // cout << rxUrl.match(3) << endl;

      if((rxUrl.match(1).compare("Audio") == 0) || 
         (rxUrl.match(1).compare("Video") == 0)) {

			  bResult = handleItemRequest(sObjectId, pRequest, pResponse);
      }
      else if(rxUrl.match(1).compare("Image") == 0) {

  			bResult = handleImageRequest(sObjectId, pRequest, pResponse);    
      }
      
    }
 
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
	Log::log(Log::soap, Log::debug, __FILE__, __LINE__, "REQUEST:\n" + pRequest->GetMessage());
	
  // get UPnP action
  CUPnPAction* pAction = pRequest->GetAction();
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
  SoapControl* soapCtrl = NULL;
  
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
      pXMS = new CXMSMediaReceiverRegistrar(m_sHTTPServerURL);
      pXMS->HandleUPnPAction(pAction, pResponse);
      delete pXMS;
      break;
		case FUPPES_SOAP_CONTROL:
      soapCtrl = new SoapControl(m_sHTTPServerURL);
      soapCtrl->HandleUPnPAction(pAction, pResponse);
      delete soapCtrl;
      break;
    default:
      bRet = false;
      break;
  }
  
  return bRet;
}

bool CHTTPRequestHandler::HandleGENAMessage(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
	Log::log(Log::gena, Log::debug, __FILE__, __LINE__, "REQUEST:\n" + pRequest->GetMessage());
  
  CSubscriptionMgr::HandleSubscription(pRequest, pResponse);
  pResponse->SetVersion(pRequest->GetVersion());
  pResponse->SetMessageType(HTTP_MESSAGE_TYPE_GENA_OK);  
  return true;
}

bool CHTTPRequestHandler::handleItemRequest(std::string p_sObjectId, CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  std::stringstream sSql;
  //OBJECT_TYPE       nObjectType;
  std::string       sExt;
  std::string       sPath;
  std::string       sMimeType;
  SQLQuery				  qry;
  bool              bResult = true;  
  

  unsigned int      objectId = HexToInt(p_sObjectId);
  
  string sDevice = pRequest->virtualFolderLayout();
  string sql = qry.build(SQL_GET_OBJECT_DETAILS, objectId, sDevice);
  qry.select(sql);
  if(!qry.eof()) {
    // TODO Object Types are still on the todo list
    //nObjectType = (OBJECT_TYPE)atoi(pDb->GetResult()->asString("TYPE").c_str());
    //cout << "OBJECT_TYPE: " << nObjectType << endl;
        
    sPath = qry.result()->asString("PATH") + qry.result()->asString("FILE_NAME");
    sExt  = ExtractFileExt(sPath);
    
    if(!fuppes::File::exists(sPath)) {
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, "file: %s not found", sPath.c_str());
      bResult = false;
    }
    else
    {      
      if(pRequest->DeviceSettings()->DoTranscode(sExt, qry.result()->asString("A_CODEC"), qry.result()->asString("V_CODEC"))) {
        CSharedLog::Log(L_EXT, __FILE__, __LINE__, "transcode %s",  sPath.c_str());
     
        sMimeType = pRequest->DeviceSettings()->MimeType(sExt, qry.result()->asString("A_CODEC"), qry.result()->asString("V_CODEC"));
        if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_GET) {          
          //
          SAudioItem trackDetails;
          if(!qry.result()->isNull("TITLE")) {
            trackDetails.sTitle = qry.result()->asString("TITLE");
          }
          
          if(!qry.result()->isNull("A_ARTIST")) {
            trackDetails.sArtist = qry.result()->asString("A_ARTIST");
          }
          
          if(!qry.result()->isNull("A_ALBUM")) {
            trackDetails.sAlbum  = qry.result()->asString("A_ALBUM");
          }
          
          if(!qry.result()->isNull("A_GENRE")) {
            trackDetails.sGenre = qry.result()->asString("A_GENRE");
          }
          
          if(!qry.result()->isNull("A_TRACK_NO")) {
            trackDetails.sOriginalTrackNumber = qry.result()->asString("A_TRACK_NO");
          }          
          
          if(!qry.result()->isNull("A_CODEC")) {
            trackDetails.sACodec = qry.result()->asString("A_CODEC");
          }
          if(!qry.result()->isNull("V_CODEC")) {
            trackDetails.sVCodec = qry.result()->asString("V_CODEC");
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
        sMimeType = pRequest->DeviceSettings()->MimeType(sExt, qry.result()->asString("A_CODEC"), qry.result()->asString("V_CODEC"));
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
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unknown object id: %s", p_sObjectId.c_str());
    bResult = false;
  }
  
  return bResult;
}

bool CHTTPRequestHandler::handleImageRequest(std::string p_sObjectId, CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
	/*cout << "image request" << endl;
	cout << pRequest->getVarAsInt("width") << "x" << pRequest->getVarAsInt("height") << endl;*/
	
  std::stringstream sSql;
  //OBJECT_TYPE       nObjectType;
  std::string       sExt;
  std::string       sPath;
  std::string       sMimeType;
  SQLQuery  				qry;

  unsigned int objectId = HexToInt(p_sObjectId);
  
  string sDevice = pRequest->virtualFolderLayout();
  if(pRequest->GetVarExists("vfolder")) {
    sDevice = pRequest->getGetVar("vfolder");
    if(sDevice == "none")
      sDevice = "";
  }
  string sql = qry.build(SQL_GET_OBJECT_DETAILS, objectId, sDevice);
  qry.select(sql);
    
  if(qry.eof()) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unknown object id: %s", p_sObjectId.c_str());
		return false;
	}

	sPath = qry.result()->asString("PATH") + qry.result()->asString("FILE_NAME");
  sExt  = ExtractFileExt(sPath);	
	bool audioFile = false;
	bool videoFile = false;
  bool hasCached = false;
	
	OBJECT_TYPE type = (OBJECT_TYPE)qry.result()->asInt("TYPE");
	if(type >= ITEM_IMAGE_ITEM && type < ITEM_IMAGE_ITEM_MAX) {
		//cout << "request image file " << sPath << endl;
	}
	else if(type >= ITEM_AUDIO_ITEM && type < ITEM_AUDIO_ITEM_MAX) {
		//cout << "request image from audio file " << sPath << endl;
		audioFile = true;
	}
	else if(type >= ITEM_VIDEO_ITEM && type < ITEM_VIDEO_ITEM_MAX) {
		//cout << "request image from video file " << sPath << endl;
		videoFile = true;

    if(qry.result()->asUInt("ALBUM_ART_ID") > 0) {
      hasCached = fuppes::File::exists(CSharedConfig::Shared()->globalSettings->GetTempDir() + qry.result()->asString("ALBUM_ART_ID") + ".jpg");
      sPath = CSharedConfig::Shared()->globalSettings->GetTempDir() + qry.result()->asString("ALBUM_ART_ID") + ".jpg";
      sExt = "jpg";
    }
	}
	else {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unsupported image request on object type %d", type);
		return false;
	}
    
  if(!fuppes::File::exists(sPath)) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "file: %s not found", sPath.c_str());
    return false;
  }
	//cout << "image request: " << sPath << endl;

	
	int width = pRequest->getVarAsInt("width");
	int height = pRequest->getVarAsInt("height");
	/*int less = pRequest->getVarAsInt("less");
	int greater = pRequest->getVarAsInt("greater");*/
	
	// transcode | scale request via GET
	// and/or embedded image from audio file
	if((width > 0 || height > 0 || audioFile || videoFile) && !hasCached) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "GET transcode %s - %dx%d",  sPath.c_str(), width, height);
		
		size_t inSize = 0;
		size_t outSize = 0;
		unsigned char* inBuffer = (unsigned char*)malloc(1);
		unsigned char* outBuffer = (unsigned char*)malloc(1);
		char* mimeType = (char*)malloc(1);
		memset(mimeType, 0, 1);
		
		// embedded image from audio file
		bool transcode = true;
		if(audioFile || videoFile) {

			CMetadataPlugin* metadata = NULL;
			if(audioFile) {
				metadata = CPluginMgr::metadataPlugin("taglib");
				if(!metadata) {
					CSharedLog::Log(L_EXT, __FILE__, __LINE__, "metadata plugin %s not found", "taglib");
					free(inBuffer);
					free(outBuffer);
					free(mimeType);
				  return false;
				}
			}
      else if(videoFile) {
				metadata = CPluginMgr::metadataPlugin("ffmpegthumbnailer");
				if(!metadata) {
					CSharedLog::Log(L_EXT, __FILE__, __LINE__, "metadata plugin %s not found", "ffmpegthumbnailer");
					free(inBuffer);
					free(outBuffer);
					free(mimeType);
				  return false;
        }
				transcode = false;
			}
			
			metadata->openFile(sPath);
			inSize = 0;
			if(!metadata->readImage(&mimeType, &inBuffer, &inSize)) {
				metadata->closeFile();
				CSharedLog::Log(L_EXT, __FILE__, __LINE__, "metadata plugin %s failed to read embedded image", "taglib");
				free(inBuffer);
				free(outBuffer);
				free(mimeType);
		    return false;
			}
			metadata->closeFile();
			delete metadata;
		} // embedded image



    // an actual image file or a cached image is requested
		else {

      if(hasCached) {
				transcode = false;
      }
      
			std::fstream fsImg;
			fsImg.open(sPath.c_str(), ios::binary|ios::in);
		  if(fsImg.fail() == 1) {
				CSharedLog::Log(L_EXT, __FILE__, __LINE__, "failed to load image file %s", sPath.c_str());
				free(inBuffer);
				free(outBuffer);
				free(mimeType);
			  return false;
			}
			fsImg.seekg(0, ios::end); 
  		inSize = streamoff(fsImg.tellg()); 
  		fsImg.seekg(0, ios::beg);
			inBuffer = (unsigned char*)realloc(inBuffer, inSize);
			fsImg.read((char*)inBuffer, inSize);
			fsImg.close();
		} // image file

		if(transcode) {		
			CTranscoderBase* transcoder = CPluginMgr::transcoderPlugin("magickWand");
			if(transcoder == NULL) {
				CSharedLog::Log(L_EXT, __FILE__, __LINE__, "image magick transcoder not available");
				free(inBuffer);
				free(outBuffer);
				free(mimeType);
			  return false;
			}
		
			CFileSettings* settings = new CFileSettings(pRequest->DeviceSettings()->FileSettings("jpeg"));
		
			// TODO fixme. Robert: What are we supposed to be fixing?
			if(!settings->pImageSettings) {
				settings->pImageSettings = new CImageSettings();
			}
			settings->pImageSettings->nHeight = height;
			settings->pImageSettings->nWidth = width;
			settings->pImageSettings->bGreater = true;
			settings->pImageSettings->bLess = true;
		
			transcoder->TranscodeMem(settings,
															 (const unsigned char**)&inBuffer, inSize, &outBuffer, &outSize);
			delete settings;
			delete transcoder;

			pResponse->SetBinContent((char*)outBuffer, outSize);
		} else { // transcode
			pResponse->SetBinContent((char*)inBuffer, inSize);
		}
		

		pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
#warning todo: set correct mime type 
    // the current mimeType variable only holds a dummy mime type set above
		pResponse->SetContentType(mimeType);
		
		free(inBuffer);
		free(outBuffer);
		free(mimeType);
		return true;
	} // embedded audio or width|height via GET
	
	
	if(pRequest->DeviceSettings()->DoTranscode(sExt, qry.result()->asString("A_CODEC"), qry.result()->asString("V_CODEC"))) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "transcode %s",  sPath.c_str());
 
		sMimeType = pRequest->DeviceSettings()->MimeType(sExt, qry.result()->asString("A_CODEC"), qry.result()->asString("V_CODEC"));
		if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_GET) {          
			SAudioItem trackDetails;
			if(!pResponse->TranscodeContentFromFile(sPath, trackDetails)) {
				return false;
			}
		}
		else if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_HEAD) {
			// mark the head response as chunked so
			// the correct header will be build
			pResponse->SetIsBinary(true);
				
			if(pRequest->DeviceSettings()->TranscodingHTTPResponse(sExt) == RESPONSE_CHUNKED) {
				pResponse->SetTransferEncoding(HTTP_TRANSFER_ENCODING_CHUNKED);
			}
			else if(pRequest->DeviceSettings()->TranscodingHTTPResponse(sExt) == RESPONSE_STREAM) {
				pResponse->SetTransferEncoding(HTTP_TRANSFER_ENCODING_NONE);
			}
		}
	}
	else {
		sMimeType = pRequest->DeviceSettings()->MimeType(sExt, qry.result()->asString("A_CODEC"), qry.result()->asString("V_CODEC"));
		pResponse->LoadContentFromFile(sPath);
	}
	
	// we always set the response type to "200 OK"
	// if the message should be a "206 partial content" 
	// CHTTPServer will change the type
	pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
	pResponse->SetContentType(sMimeType);

  return true;
}
