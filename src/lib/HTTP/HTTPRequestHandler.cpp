/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            HTTPRequestHandler.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "../Transcoding/TranscodingMgr.h"
#include "../ControlInterface/SoapControl.h"
#include "../DLNA/DLNA.h"

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
  bool bResult = false; 
  
  pResponse->DeviceSettings(pRequest->DeviceSettings());  
  
  switch(pRequest->GetMessageType())
  {
    // HTTP request
    case HTTP_MESSAGE_TYPE_GET:
    case HTTP_MESSAGE_TYPE_HEAD:
    case HTTP_MESSAGE_TYPE_POST:
      bResult = this->HandleHTTPRequest(pRequest, pResponse);
      if(bResult)
       	Log::log(Log::http, Log::debug, __FILE__, __LINE__, "RESPONSE:\n" + pResponse->GetHeaderAsString());
      break;
      
    // SOAP
    case HTTP_MESSAGE_TYPE_POST_SOAP_ACTION:
      bResult = this->HandleSOAPAction(pRequest, pResponse);
      if(bResult)
  			Log::log(Log::soap, Log::debug, __FILE__, __LINE__, "RESPONSE:\n" + pResponse->GetMessageAsString());
      break;
      
    // GENA
    case HTTP_MESSAGE_TYPE_SUBSCRIBE:
      bResult = this->HandleGENAMessage(pRequest, pResponse);
      if(bResult)
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
    bool isImage;
    int width = 0;
    int height = 0;
    CPresentationHandler pres(m_sHTTPServerURL);
    pres.OnReceivePresentationRequest(pRequest, pResponse, isImage, width, height);

    // dlna
    if(isImage && //CPluginMgr::dlnaPlugin() != NULL &&
      //(pRequest->dlnaGetContentFeatures() == true) &&
      (pRequest->DeviceSettings()->dlnaVersion() != CMediaServerSettings::dlna_none)) {
        
      string sExt = pRequest->DeviceSettings()->extensionByMimeType(pResponse->GetContentType());
      
      std::string mimeType;
      std::string profile;
      DLNA::getImageProfile(sExt, width, height, profile, mimeType);

      std::string dlnaFeatures = CContentDirectory::buildDlnaInfo(false, profile);
      std::string dlnaMode = "Interactive";

      pResponse->dlnaContentFeatures(dlnaFeatures);
      pResponse->dlnaTransferMode(dlnaMode);        
    }
      

    
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
    if(rxUrl.search(sRequest)) {

      string sObjectId = rxUrl.match(2);
      //cout << "request EXT: " << rxUrl.match(3) << "*" << endl;

      if(rxUrl.match(1).compare("Video") == 0) {
			  bResult = handleAVItemRequest(sObjectId, pRequest, pResponse, false, rxUrl.match(3));
      }
      else if(rxUrl.match(1).compare("Audio") == 0) {
			  bResult = handleAVItemRequest(sObjectId, pRequest, pResponse, true, rxUrl.match(3));
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

bool CHTTPRequestHandler::handleAVItemRequest(std::string p_sObjectId, CHTTPMessage* pRequest, CHTTPMessage* pResponse, bool audio, std::string requestExt)
{
  std::stringstream sSql;
  //OBJECT_TYPE       nObjectType;
  std::string       sExt;
  std::string       sPath;
  std::string       sMimeType;
  string            targetExt;
  SQLQuery				  qry;
  bool              bResult = true;  
  bool              transcode = false;
  
  unsigned int      objectId = HexToInt(p_sObjectId);
  
  string sDevice = pRequest->virtualFolderLayout();
  string sql = qry.build(SQL_GET_OBJECT_DETAILS, objectId, sDevice);
  qry.select(sql);
  if(qry.eof()) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unknown object id: %s", p_sObjectId.c_str());
    return false;
  }
  
  // TODO Object Types are still on the todo list
  //nObjectType = (OBJECT_TYPE)atoi(pDb->GetResult()->asString("TYPE").c_str());
  //cout << "OBJECT_TYPE: " << nObjectType << endl;
      
  sPath = qry.result()->asString("PATH") + qry.result()->asString("FILE_NAME");
  sExt  = ExtractFileExt(sPath);
  
  if(!fuppes::File::exists(sPath)) {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "file: %s not found", sPath.c_str());
    return false;
  }


  // check for subtitles
  if(!audio && requestExt.compare("srt") == 0) {
    sPath = TruncateFileExt(sPath) + "." + requestExt;
    cout << "SUB REQUEST: " << sPath << "*" << endl;
    if(!fuppes::File::exists(sPath))
      return false;

    pResponse->LoadContentFromFile(sPath);
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
    pResponse->SetContentType("application/x-subrip");
    return true;
  }

  transcode = pRequest->DeviceSettings()->DoTranscode(sExt, qry.result()->asString("AUDIO_CODEC"), qry.result()->asString("VIDEO_CODEC"));
  sMimeType = pRequest->DeviceSettings()->MimeType(sExt, qry.result()->asString("AUDIO_CODEC"), qry.result()->asString("VIDEO_CODEC"));
  targetExt = pRequest->DeviceSettings()->Extension(sExt, qry.result()->asString("AUDIO_CODEC"), qry.result()->asString("VIDEO_CODEC"));


  if(!transcode) {
    pResponse->LoadContentFromFile(sPath);
  }  
  else {
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "transcode %s",  sPath.c_str());
 
    if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_GET) {  
      DbObject object(qry.result());
      bResult = pResponse->TranscodeContentFromFile(sPath, &object);
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
  
  

  // dlna
  if(//(CPluginMgr::dlnaPlugin() != NULL) &&
     //(pRequest->dlnaGetContentFeatures() == true) &&
     (pRequest->DeviceSettings()->dlnaVersion() != CMediaServerSettings::dlna_none)) {

    bool hasProfile = false;
    std::string profile;
    if(audio) {

      int channels = 0;
      int bitrate = 0;
      if(!transcode) {
        channels = qry.result()->asInt("A_CHANNELS");
        bitrate = qry.result()->asInt("A_BITRATE");
      }

      if(audio) {
        hasProfile = DLNA::getAudioProfile(targetExt, channels, bitrate, profile, sMimeType);
      }
      else {
        //hasProfile = CPluginMgr::dlnaPlugin()->getVideoProfile(targetExt, channels, bitrate, &profile, &sMimeType);
      }
    }
    else {
      //CPluginMgr::dlnaPlugin()->getVideoProfile();
    }

    //if(hasProfile) {
      std::string dlnaFeatures = CContentDirectory::buildDlnaInfo(transcode, profile);
      std::string dlnaMode = (transcode ? "Streaming" : "Interactive");

      pResponse->dlnaContentFeatures(dlnaFeatures);
      pResponse->dlnaTransferMode(dlnaMode);
    //}
  } // end dlna
  
  // we always set the response type to "200 OK"
  // if the message should be a "206 partial content" 
  // CHTTPServer will change the type
  pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
  pResponse->SetContentType(sMimeType);
  
  bResult = true;
    
  
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

  
  string sDevice = pRequest->virtualFolderLayout();
  if(pRequest->GetVarExists("vfolder")) {
    sDevice = pRequest->getGetVar("vfolder");
    if(sDevice == "none")
      sDevice = "";
  }

  object_id_t objectId = HexToInt(p_sObjectId);  
  DbObject* tmp = DbObject::createFromObjectId(objectId, &qry, sDevice);
  if(tmp == NULL) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unknown object id: %s", p_sObjectId.c_str());
    return false;
  }

  DbObject obj = *tmp;
  delete tmp;

/*
  bool result = false;

  // real image
  if(obj->type() >= ITEM_IMAGE_ITEM && obj->type() < ITEM_IMAGE_ITEM_MAX) {
    result = handleRealImageRequest(obj, pRequest, pResponse);
  }
  // audio file embedded image
  else if(obj->type() >= ITEM_AUDIO_ITEM && obj->type() < ITEM_AUDIO_ITEM_MAX) {
    result = handleAudioEmbeddedImageRequest(obj, pRequest, pResponse);
  }
  // video file thumbnail image
  else if(obj->type() >= ITEM_VIDEO_ITEM && obj->type() < ITEM_VIDEO_ITEM_MAX) {
    result = handleVideoImageRequest(obj, pRequest, pResponse);
  }
  else {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unsupported image request on object type %d", obj->type());
	}

  delete obj;
  return result;
  */
  

  
  
  string sql = qry.build(SQL_GET_OBJECT_DETAILS, objectId, sDevice);
  qry.select(sql);
    
  if(qry.eof()) {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unknown object id: %s", p_sObjectId.c_str());
		return false;
	}

	sPath = qry.result()->asString("PATH") + qry.result()->asString("FILE_NAME");
  sExt  = ExtractFileExt(sPath);
  sMimeType = pRequest->DeviceSettings()->MimeType(sExt);
    
	bool audioFile = false;
	bool videoFile = false;
  bool hasCached = false;
	
	//OBJECT_TYPE type = (OBJECT_TYPE)qry.result()->asInt("TYPE");
	if(obj.type() >= ITEM_IMAGE_ITEM && obj.type() < ITEM_IMAGE_ITEM_MAX) {
		//cout << "request image file " << sPath << endl;
	}
	else if(obj.type() >= ITEM_AUDIO_ITEM && obj.type() < ITEM_AUDIO_ITEM_MAX) {
		//cout << "request image from audio file " << sPath << endl;
		audioFile = true;
	}
	else if(obj.type() >= ITEM_VIDEO_ITEM && obj.type() < ITEM_VIDEO_ITEM_MAX) {
		//cout << "request image from video file " << sPath << endl;
		videoFile = true;

    if(qry.result()->asUInt("ALBUM_ART_ID") > 0) {
      sPath = PathFinder::findThumbnailsDir() + qry.result()->asString("ALBUM_ART_ID") + ".jpg";
      hasCached = fuppes::File::exists(sPath);
      sExt = "jpg";
      sMimeType = pRequest->DeviceSettings()->MimeType(sExt);
    }
	}
	else {
		CSharedLog::Log(L_EXT, __FILE__, __LINE__, "unsupported image request on object type %d", obj.type());
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
		char tmpMime[100];
		//memset(tmpMime, 0, 1);
		
		// embedded image from audio or video file
		bool transcode = true;
		if(audioFile || videoFile) {

      string plugin;
			if(audioFile) {
        plugin = "taglib";
			}
      else if(videoFile) {
        plugin = "ffmpegthumbnailer";
				transcode = false;
			}

			CMetadataPlugin* metadata = CPluginMgr::metadataPlugin(plugin);
			if(!metadata) {
				CSharedLog::Log(L_EXT, __FILE__, __LINE__, "metadata plugin %s not found", plugin.c_str());
				free(inBuffer);
				free(outBuffer);
				//free(tmpMime);
			  return false;
      }
      
      
			metadata->openFile(sPath);
			inSize = 0;
			if(!metadata->readImage(&tmpMime[0], &inBuffer, &inSize)) {
				metadata->closeFile();
				CSharedLog::Log(L_EXT, __FILE__, __LINE__, "metadata plugin %s failed to read embedded image", "taglib");
				free(inBuffer);
				free(outBuffer);
				//free(tmpMime);
		    return false;
			}
			metadata->closeFile();

      // get the mime type and the extension of the extracted file      
      sMimeType = tmpMime;
      sExt = pRequest->DeviceSettings()->extensionByMimeType(sMimeType);        
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
				//free(tmpMime);
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
				//free(tmpMime);
			  return false;
			}
		
			CFileSettings* settings = new CFileSettings(pRequest->DeviceSettings()->FileSettings(sExt));
		
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

      // todo: get the actual image dimensions
      //width = width;
      //height = height:
      sMimeType = pRequest->DeviceSettings()->MimeType(sExt);


		} else { // transcode
			pResponse->SetBinContent((char*)inBuffer, inSize);

      width = obj.details()->width();
      height = obj.details()->height();
		}

    
/*
		pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
#warning todo: set correct mime type 
    // the current mimeType variable only holds a dummy mime type set above
		pResponse->SetContentType(sMimeType);
*/
		free(inBuffer);
		free(outBuffer);
		//free(tmpMime);
		//return true;
	} // embedded audio or width|height via GET
	

  // a real image file
  else {

    width = obj.details()->width();
    height = obj.details()->height();
    
	  if(pRequest->DeviceSettings()->DoTranscode(sExt, qry.result()->asString("AUDIO_CODEC"), qry.result()->asString("VIDEO_CODEC"))) {
		  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "transcode %s",  sPath.c_str());
   
		  sMimeType = pRequest->DeviceSettings()->MimeType(sExt, qry.result()->asString("AUDIO_CODEC"), qry.result()->asString("VIDEO_CODEC"));
		  if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_GET) {          
			  DbObject object(qry.result());
			  if(!pResponse->TranscodeContentFromFile(sPath, &object)) {
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
		  sMimeType = pRequest->DeviceSettings()->MimeType(sExt, qry.result()->asString("AUDIO_CODEC"), qry.result()->asString("VIDEO_CODEC"));
		  pResponse->LoadContentFromFile(sPath);
	  }

  } // a real image file



  
  // dlna
  if(//(pRequest->dlnaGetContentFeatures() == true) &&
     (pRequest->DeviceSettings()->dlnaVersion() != CMediaServerSettings::dlna_none)) {

    std::string mimeType;
    std::string profile;
    DLNA::getImageProfile(sExt, width, height, profile, mimeType);
    sMimeType = mimeType;

    std::string dlnaFeatures = CContentDirectory::buildDlnaInfo(false, profile);
    std::string dlnaMode = "Interactive";

    pResponse->dlnaContentFeatures(dlnaFeatures);
    pResponse->dlnaTransferMode(dlnaMode);        
  }
  
	// we always set the response type to "200 OK"
	// if the message should be a "206 partial content" 
	// CHTTPServer will change the type
	pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
	pResponse->SetContentType(sMimeType);

  return true;
}

