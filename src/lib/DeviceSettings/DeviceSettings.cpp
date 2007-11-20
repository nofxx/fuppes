/***************************************************************************
 *            DeviceSettings.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "DeviceSettings.h"
#include "../Common/RegEx.h"
#include <iostream>

#define DEFAULT_RELEASE_DELAY 4

using namespace std;

CImageSettings::CImageSettings()
{ 
  bEnabled = true;
  
  nHeight = 0;
  nWidth = 0;
  bGreater = false;
  bLess = false;
  
  bDcraw = false;
}

CImageSettings::CImageSettings(CImageSettings* pImageSettings)
{
  bEnabled = pImageSettings->bEnabled;
  
  sExt = pImageSettings->sExt;
  sMimeType = pImageSettings->sMimeType;
  
  nHeight = pImageSettings->nHeight;
  nWidth  = pImageSettings->nWidth;
  bGreater = pImageSettings->bGreater;
  bLess = pImageSettings->bLess;
  
  bDcraw = pImageSettings->bDcraw;
  sDcrawParams = pImageSettings->sDcrawParams;
}

CTranscodingSettings::CTranscodingSettings() 
{
  bEnabled              = true;
  nTranscodingResponse  = RESPONSE_STREAM;
  nTranscodingType      = TT_NONE;
  nTranscoderType       = TTYP_NONE;
  nDecoderType          = DT_NONE;
  nEncoderType          = ET_NONE;

  nAudioBitRate         = 0;
  nAudioSampleRate      = 0;
  nVideoBitRate         = 0;
  nReleaseDelay         = -1; 
  nLameQuality          = -1;
  
  sACodec = "copy";
  sVCodec = "copy";
}

CTranscodingSettings::CTranscodingSettings(CTranscodingSettings* pTranscodingSettings)
{
  bEnabled = pTranscodingSettings->bEnabled;
  nTranscodingResponse = pTranscodingSettings->nTranscodingResponse;
  
  nTranscodingType  = pTranscodingSettings->nTranscodingType;
  nTranscoderType   = pTranscodingSettings->nTranscoderType;
  nDecoderType      = pTranscodingSettings->nDecoderType;
  nEncoderType      = pTranscodingSettings->nEncoderType;

  nAudioBitRate       = pTranscodingSettings->nAudioBitRate;
  nAudioSampleRate    = pTranscodingSettings->nAudioSampleRate;
  nVideoBitRate       = pTranscodingSettings->nVideoBitRate;
  nReleaseDelay = pTranscodingSettings->nReleaseDelay;
  nLameQuality  = pTranscodingSettings->nLameQuality;
  
  sExt        = pTranscodingSettings->sExt;
  sDLNA       = pTranscodingSettings->sDLNA;
  sMimeType   = pTranscodingSettings->sMimeType;
  
  sACodec           = pTranscodingSettings->sACodec;
  sVCodec           = pTranscodingSettings->sVCodec;
  sACodecCondition  = pTranscodingSettings->sACodecCondition;
  sVCodecCondition  = pTranscodingSettings->sVCodecCondition;  
}

bool CTranscodingSettings::DoTranscode(std::string p_sACodec, std::string p_sVCodec)
{
  if(!bEnabled || nTranscodingType == TT_NONE || nTranscodingType == TT_RENAME) {
    return false;
  }

  if(sACodecCondition.empty() && sVCodecCondition.empty()) {    
    return true;
  }  

  if(sACodecCondition.substr(0, 1).compare(" ") != 0) {
    sACodecCondition = " " + sACodecCondition + " ";
  }
  
  if(sVCodecCondition.substr(0, 1).compare(" ") != 0) {
    sVCodecCondition = " " + sVCodecCondition + " ";
  }
  
  string sReg = "[ |,]" + p_sACodec + "[ |,]";
  RegEx rxACodec(sReg.c_str(), PCRE_CASELESS);
  if(rxACodec.Search(sACodecCondition.c_str())) {       
    return true;
  }

  sReg = "[ |,]" + p_sVCodec + "[ |,]";  
  RegEx rxVCodec(sReg.c_str(), PCRE_CASELESS);
  if(rxVCodec.Search(sVCodecCondition.c_str())) {            
    return true;
  }  

  return false;
}

std::string  CTranscodingSettings::AudioCodec(std::string p_sACodec)
{
  //if(p_sACodec.empty()) {
    return sACodec;
  //}
  
  /*size_t pos = sACodecCondition.find(p_sACodec);
  if(pos != string::npos) {
    return sACodec;
  }
  else {
    return "copy";
  }*/
}

std::string  CTranscodingSettings::VideoCodec(std::string p_sVCodec)
{
  if(p_sVCodec.empty()) {
    return sVCodec;
  }
    
  if(DoTranscode("", p_sVCodec)) {
    return sVCodec;
  }
  else {
    return "copy";
  }
}


CFileSettings::CFileSettings() 
{
  pTranscodingSettings  = NULL;
  pImageSettings        = NULL;
  bEnabled              = true; 
  bExtractMetadata      = true;
}

CFileSettings::CFileSettings(CFileSettings* pFileSettings)
{
  pTranscodingSettings = NULL;
  pImageSettings = NULL;
  
  if(pFileSettings->pTranscodingSettings) {
    pTranscodingSettings = new CTranscodingSettings(pFileSettings->pTranscodingSettings);
  }
  else if(pFileSettings->pImageSettings) {
    pImageSettings = new CImageSettings(pFileSettings->pImageSettings);
  }  
  
  bEnabled  = pFileSettings->bEnabled;
  sMimeType = pFileSettings->sMimeType;
  sDLNA     = pFileSettings->sDLNA;
  nType     = pFileSettings->nType;
  
  bExtractMetadata = pFileSettings->bExtractMetadata;
}

std::string CFileSettings::ObjectTypeAsStr() 
{
  switch(nType) {
    case OBJECT_TYPE_UNKNOWN :
      return "unknown";
    
    case ITEM_IMAGE_ITEM :
      return "object.item.imageItem";    
    case ITEM_IMAGE_ITEM_PHOTO :
      return "object.item.imageItem.photo";
  
    case ITEM_AUDIO_ITEM :
      return "object.item.audioItem";
    case ITEM_AUDIO_ITEM_MUSIC_TRACK :
      return "object.item.audioItem.musicTrack";
    case ITEM_AUDIO_ITEM_AUDIO_BROADCAST :
      return "object.item.audioItem.audioBroadcast";
    //ITEM_AUDIO_ITEM_AUDIO_BOOK      = 202,*/
  
    case ITEM_VIDEO_ITEM :
      return "object.item.videoItem";
    case ITEM_VIDEO_ITEM_MOVIE :
      return "object.item.videoItem.movie";
    case ITEM_VIDEO_ITEM_VIDEO_BROADCAST :
      return "object.item.videoItem.videoBroadcast";
    //ITEM_VIDEO_ITEM_MUSIC_VIDEO_CLIP = 302,  
  
    /*CONTAINER_PERSON = 4,*/
    case CONTAINER_PERSON_MUSIC_ARTIST :
      return "object.container.person.musicArtist";
    
    case CONTAINER_PLAYLIST_CONTAINER :
      return "object.container.playlistContainer";
    
    /*CONTAINER_ALBUM = 6, */
    
		case CONTAINER_ALBUM_MUSIC_ALBUM :
		  return "object.container.album.musicAlbum";
			
    case CONTAINER_ALBUM_PHOTO_ALBUM :
		  return "object.container.album.photoAlbum";
    
    case CONTAINER_GENRE :
      return "object.container.genre";
    case CONTAINER_GENRE_MUSIC_GENRE :
      return "object.container.genre.musicGenre";    
    /*  CONTAINER_GENRE_MOVIE_GENRE = 701,
      
    CONTAINER_STORAGE_SYSTEM = 8,
    CONTAINER_STORAGE_VOLUME = 9, */
    case CONTAINER_STORAGE_FOLDER :
      return "object.container.storageFolder";
    
    default:
      return "unknown";
  }
}

std::string CFileSettings::MimeType(std::string p_sACodec, std::string p_sVCodec)
{
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    if(pTranscodingSettings->DoTranscode(p_sACodec, p_sVCodec) || 
       (pTranscodingSettings->TranscodingType() == TT_RENAME && !pTranscodingSettings->MimeType().empty())) {      
        return pTranscodingSettings->MimeType();
    }
    else
      return sMimeType;
  }
  else if(pImageSettings && pImageSettings->Enabled()) {
    if(!pImageSettings->MimeType().empty()) {
      return pImageSettings->MimeType();
    }
    else {
      return sMimeType;
    }
  }
  else { 
    return sMimeType;
  }
}

std::string CFileSettings::DLNA()
{
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->DLNA();
  }
  else { 
    return sDLNA;
  }
}

unsigned int  CFileSettings::TargetAudioSampleRate()
{
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->AudioSampleRate();
  }
  else {
    return 0;
  }
}

unsigned int  CFileSettings::TargetAudioBitRate()
{
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->AudioBitRate();
  }
  else {
    return 0;
  }
}

std::string CFileSettings::Extension(std::string p_sACodec, std::string p_sVCodec)
{
  // if transcoding is enabled it MUST have an extension
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    if(pTranscodingSettings->DoTranscode(p_sACodec, p_sVCodec) ||
       (pTranscodingSettings->TranscodingType() == TT_RENAME && !pTranscodingSettings->Extension().empty())) {
    
      /*if(pTranscodingSettings->VideoCodec(p_sVCodec).compare("copy") == 0)
        return sExt;
      else*/
        return pTranscodingSettings->Extension();
      
    }
    else
       return sExt;
  }
  // image settings MAY have an extension
  else if(pImageSettings && pImageSettings->Enabled()) {
    if(!pImageSettings->Extension().empty()) {
      return pImageSettings->Extension();
    }
    else {
      return sExt;
    }
  }
  else {
    return sExt;
  }
}

TRANSCODING_HTTP_RESPONSE CFileSettings::TranscodingHTTPResponse()
{ 
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->TranscodingHTTPResponse();
  }
  else {
    return RESPONSE_CHUNKED;
  }
}

int CFileSettings::ReleaseDelay()
{ 
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->ReleaseDelay();
  }
  else {
    return -1;
  }
}

CDeviceSettings::CDeviceSettings(std::string p_sDeviceName)
{
  m_sDeviceName = p_sDeviceName;
  m_sVirtualFolderDevice = "default";

  m_bShowPlaylistAsContainer = false;
	m_bXBox360Support					 = false;
	m_bDLNAEnabled             = false; 
  m_bEnableDeviceIcon        = false;

  m_DisplaySettings.bShowChildCountInTitle = false;
  m_DisplaySettings.nMaxFileNameLength     = 0;

  nDefaultReleaseDelay = DEFAULT_RELEASE_DELAY;

	m_MediaServerSettings.FriendlyName = "FUPPES %v (%h)";
	m_MediaServerSettings.Manufacturer = "Ulrich Voelkel";
	m_MediaServerSettings.ManufacturerURL = "http://www.ulrich-voelkel.de"; 	
	m_MediaServerSettings.ModelName = "Free UPnP Entertainment Service %v";
	m_MediaServerSettings.ModelNumber = "%v";
	m_MediaServerSettings.ModelURL = "http://fuppes.ulrich-voelkel.de";
	m_MediaServerSettings.ModelDescription = "Free UPnP Media Server licensed under the terms of the GPL";
	m_MediaServerSettings.UseModelDescription = true;
	m_MediaServerSettings.SerialNumber = "0123456789";
	m_MediaServerSettings.UseSerialNumber = true;
	m_MediaServerSettings.UseUPC = false;
	m_MediaServerSettings.UseDLNA = false;
		
	m_MediaServerSettings.UseURLBase = true;
	m_MediaServerSettings.UseXMSMediaReceiverRegistrar = false;
}

CDeviceSettings::CDeviceSettings(std::string p_sDeviceName, CDeviceSettings* pSettings)
{
  m_sDeviceName = p_sDeviceName;
  m_sVirtualFolderDevice = pSettings->m_sVirtualFolderDevice;
  
  m_bShowPlaylistAsContainer = pSettings->m_bShowPlaylistAsContainer;
  m_bXBox360Support          = pSettings->m_bXBox360Support;
  m_bDLNAEnabled             = pSettings->m_bDLNAEnabled;
  m_bEnableDeviceIcon        = pSettings->m_bEnableDeviceIcon;
  
  /*m_ImageSettings.bResize    = pSettings->m_ImageSettings.bResize;
  m_ImageSettings.bResizeIfLarger = pSettings->m_ImageSettings.bResizeIfLarger;
  m_ImageSettings.nWidth  = pSettings->m_ImageSettings.nWidth;
  m_ImageSettings.nHeight = pSettings->m_ImageSettings.nWidth;*/
  
  m_DisplaySettings.bShowChildCountInTitle = pSettings->m_DisplaySettings.bShowChildCountInTitle;
  m_DisplaySettings.nMaxFileNameLength     = pSettings->m_DisplaySettings.nMaxFileNameLength; 
  
  nDefaultReleaseDelay = pSettings->nDefaultReleaseDelay;
  
	m_MediaServerSettings.FriendlyName = pSettings->MediaServerSettings()->FriendlyName;
	m_MediaServerSettings.Manufacturer = pSettings->MediaServerSettings()->Manufacturer;
	m_MediaServerSettings.ManufacturerURL = pSettings->MediaServerSettings()->ManufacturerURL;
	m_MediaServerSettings.ModelName = pSettings->MediaServerSettings()->ModelName;
	m_MediaServerSettings.ModelNumber = pSettings->MediaServerSettings()->ModelNumber;
	m_MediaServerSettings.ModelURL = pSettings->MediaServerSettings()->ModelURL;
	m_MediaServerSettings.ModelDescription = pSettings->MediaServerSettings()->ModelDescription;
	m_MediaServerSettings.UseModelDescription = pSettings->MediaServerSettings()->UseModelDescription;
	m_MediaServerSettings.SerialNumber = pSettings->MediaServerSettings()->SerialNumber;
	m_MediaServerSettings.UseSerialNumber = pSettings->MediaServerSettings()->UseSerialNumber;
	m_MediaServerSettings.UPC = pSettings->MediaServerSettings()->UPC;
	m_MediaServerSettings.UseUPC = pSettings->MediaServerSettings()->UseUPC;
	m_MediaServerSettings.UseDLNA = pSettings->MediaServerSettings()->UseDLNA;	
	
	m_MediaServerSettings.UseURLBase = pSettings->MediaServerSettings()->UseURLBase;
	m_MediaServerSettings.UseXMSMediaReceiverRegistrar = pSettings->MediaServerSettings()->UseXMSMediaReceiverRegistrar;
		
  for(pSettings->m_FileSettingsIterator = pSettings->m_FileSettings.begin();
      pSettings->m_FileSettingsIterator != pSettings->m_FileSettings.end();
      pSettings->m_FileSettingsIterator++) {
    
    m_FileSettingsIterator = m_FileSettings.find(pSettings->m_FileSettingsIterator->first);
    
    if(m_FileSettingsIterator != m_FileSettings.end()) {
      m_FileSettings[pSettings->m_FileSettingsIterator->first] = m_FileSettingsIterator->second;
    }
    else {    
      m_FileSettings[pSettings->m_FileSettingsIterator->first] =
          new CFileSettings(pSettings->m_FileSettingsIterator->second);
    }
  }  
}

bool CDeviceSettings::HasUserAgent(std::string p_sUserAgent)
{
  bool   bResult = false;
	string sUserAgent;
	RegEx* pRxUserAgent;
	std::list<std::string>::const_iterator it;
	
	for(it = m_slUserAgents.begin(); it != m_slUserAgents.end(); it++) {
		sUserAgent = *it;
	
		pRxUserAgent = new RegEx(sUserAgent.c_str(), PCRE_CASELESS);
		if(pRxUserAgent->Search(p_sUserAgent.c_str())) {
			bResult = true;
			delete pRxUserAgent;
			break;
		}
		
		delete pRxUserAgent;
	}

	return bResult;
}

bool CDeviceSettings::HasIP(std::string p_sIPAddress)
{
  string sIP;
	std::list<std::string>::const_iterator it;
	for(it = m_slIPAddresses.begin(); it != m_slIPAddresses.end(); it++) {
	  sIP = *it;
		if(sIP.compare(p_sIPAddress) == 0)
		  return true;
	}
	return false;
}

CFileSettings* CDeviceSettings::FileSettings(std::string p_sExt) {
  
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  
  if(m_FileSettingsIterator == m_FileSettings.end()) {
    m_FileSettings[p_sExt] = new CFileSettings();
  }
  
  return m_FileSettings[p_sExt];  
}

void CDeviceSettings::AddExt(CFileSettings* pFileSettings, std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    return;
  }
  m_FileSettings[p_sExt] = pFileSettings;
}


std::string CDeviceSettings::ObjectTypeAsStr(std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    return m_FileSettingsIterator->second->ObjectTypeAsStr();
  }
  else {
    return "unkown";
  }
}

OBJECT_TYPE CDeviceSettings::ObjectType(std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    return m_FileSettingsIterator->second->ObjectType();
  }
  else {
    return OBJECT_TYPE_UNKNOWN;
  }
}


bool CDeviceSettings::DoTranscode(std::string p_sExt, std::string p_sACodec, std::string p_sVCodec)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    if(
       (m_FileSettingsIterator->second->pTranscodingSettings &&
        m_FileSettingsIterator->second->pTranscodingSettings->Enabled() &&
        (!m_FileSettingsIterator->second->pTranscodingSettings->DecoderType() != DT_NONE ||
         !m_FileSettingsIterator->second->pTranscodingSettings->EncoderType() != ET_NONE ||
         !m_FileSettingsIterator->second->pTranscodingSettings->TranscoderType() != TTYP_NONE)
        )) {
      
      if(!p_sACodec.empty() || !p_sVCodec.empty()) {
        return m_FileSettingsIterator->second->pTranscodingSettings->DoTranscode(p_sACodec, p_sVCodec);
      }
      else {
        return true;
      }
    }
    else if (m_FileSettingsIterator->second->pImageSettings &&
        m_FileSettingsIterator->second->pImageSettings->Enabled()) {
      return true;
    }
    else {
      return false;
    }
  }
  else {
    return false;
  }
}


TRANSCODING_TYPE CDeviceSettings::GetTranscodingType(std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    if(m_FileSettingsIterator->second->pTranscodingSettings &&
       m_FileSettingsIterator->second->pTranscodingSettings->Enabled()) {
         
       CTranscodingSettings* pTranscodingSettings = m_FileSettingsIterator->second->pTranscodingSettings;

       if(pTranscodingSettings->TranscoderType() != TTYP_NONE) {
         return TT_THREADED_TRANSCODER;
       }
       else if(!pTranscodingSettings->DecoderType() != DT_NONE && !pTranscodingSettings->EncoderType() != ET_NONE) {
         return TT_THREADED_DECODER_ENCODER;
       }
       else {
         return TT_NONE;
       }         
    }
    else if(m_FileSettingsIterator->second->pImageSettings &&
            m_FileSettingsIterator->second->pImageSettings->Enabled()) {
      return TT_TRANSCODER;
    }
    else {
      return TT_NONE;
    }
  }
  else {
    return TT_NONE;
  }
}

TRANSCODER_TYPE CDeviceSettings::GetTranscoderType(std::string p_sExt, std::string p_sACodec, std::string p_sVCodec)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    if(m_FileSettingsIterator->second->pTranscodingSettings &&
       m_FileSettingsIterator->second->pTranscodingSettings->Enabled()) {

       CTranscodingSettings* pTranscodingSettings = m_FileSettingsIterator->second->pTranscodingSettings;

       return pTranscodingSettings->TranscoderType();   
    }
    else if(m_FileSettingsIterator->second->pImageSettings &&
            m_FileSettingsIterator->second->pImageSettings->Enabled()) {
      return TTYP_IMAGE_MAGICK;
    }
    else {
      return TTYP_NONE;
    }
  }
  else {
    return TTYP_NONE;
  }
}

DECODER_TYPE CDeviceSettings::GetDecoderType(std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    if(m_FileSettingsIterator->second->pTranscodingSettings &&
       m_FileSettingsIterator->second->pTranscodingSettings->Enabled()) {

       CTranscodingSettings* pTranscodingSettings = m_FileSettingsIterator->second->pTranscodingSettings;

       return pTranscodingSettings->DecoderType();
    }   
    else {
      return DT_NONE;
    }
  }
  else {
    return DT_NONE;
  }
}

ENCODER_TYPE CDeviceSettings::GetEncoderType(std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    if(m_FileSettingsIterator->second->pTranscodingSettings &&
       m_FileSettingsIterator->second->pTranscodingSettings->Enabled()) {

       CTranscodingSettings* pTranscodingSettings = m_FileSettingsIterator->second->pTranscodingSettings;
       return pTranscodingSettings->EncoderType();
    }   
    else {
      return ET_NONE;
    }
  }
  else {
    return ET_NONE;
  }
}



std::string CDeviceSettings::MimeType(std::string p_sExt, std::string p_sACodec, std::string p_sVCodec)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->MimeType(p_sACodec, p_sVCodec);
  else  
    return "";
}

std::string CDeviceSettings::DLNA(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->DLNA();
  else  
    return "";
}

unsigned int CDeviceSettings::TargetAudioSampleRate(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->TargetAudioSampleRate();
  else  
    return 0;
}

unsigned int CDeviceSettings::TargetAudioBitRate(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->TargetAudioBitRate();
  else  
    return 0;
}

bool CDeviceSettings::Exists(std::string p_sExt)
{
  FileSettingsIterator_t  iter;  
  iter = m_FileSettings.find(p_sExt);  
  return (iter != m_FileSettings.end());
}

std::string CDeviceSettings::Extension(std::string p_sExt, std::string p_sACodec, std::string p_sVCodec)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->Extension(p_sACodec, p_sVCodec);
  else  
    return p_sExt;
}

TRANSCODING_HTTP_RESPONSE CDeviceSettings::TranscodingHTTPResponse(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->TranscodingHTTPResponse();
  else  
    return RESPONSE_CHUNKED;
}

int CDeviceSettings::ReleaseDelay(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end()) {
    if(iter->second->ReleaseDelay() >= 0) {
      return iter->second->ReleaseDelay();
    }
  }
  
  return nDefaultReleaseDelay;
}
