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
  bEnabled = true;
  nTranscodingResponse = RESPONSE_CHUNKED;
  nBitRate = 128; //0;
  nSampleRate = 44100; //0;
  nReleaseDelay = -1;
}

CTranscodingSettings::CTranscodingSettings(CTranscodingSettings* pTranscodingSettings)
{
  bEnabled = pTranscodingSettings->bEnabled;
  nTranscodingResponse = pTranscodingSettings->nTranscodingResponse;
  
  nBitRate      = pTranscodingSettings->nBitRate;
  nSampleRate   = pTranscodingSettings->nSampleRate;
  nReleaseDelay = pTranscodingSettings->nReleaseDelay;
  
  sDecoder      = pTranscodingSettings->sDecoder;
  sEncoder      = pTranscodingSettings->sEncoder;
  sTranscoder   = pTranscodingSettings->sTranscoder;
  
  sExt        = pTranscodingSettings->sExt;
  sDLNA       = pTranscodingSettings->sDLNA;
  sMimeType   = pTranscodingSettings->sMimeType;
}


CFileSettings::CFileSettings() 
{
  pTranscodingSettings = NULL;
  pImageSettings       = NULL;
  bEnabled = true;
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
      return "object.item.videoItem"; //.movie";
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

std::string CFileSettings::MimeType()
{
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->MimeType();
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

unsigned int  CFileSettings::TargetSampleRate()
{
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->SampleRate();
  }
  else {
    return 0;
  }
}

unsigned int  CFileSettings::TargetBitRate()
{
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->BitRate();
  }
  else {
    return 0;
  }
}

std::string CFileSettings::Extension()
{
  // if transcoding is enabled it MUST have an extension
  if(pTranscodingSettings && pTranscodingSettings->Enabled()) {
    return pTranscodingSettings->Extension();
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
	//m_nTranscodingResponse     = RESPONSE_CHUNKED;
    
  m_DisplaySettings.bShowChildCountInTitle = false;
  m_DisplaySettings.nMaxFileNameLength     = 0;
  
  nDefaultReleaseDelay = DEFAULT_RELEASE_DELAY;
}

CDeviceSettings::CDeviceSettings(std::string p_sDeviceName, CDeviceSettings* pSettings)
{
  m_sDeviceName = p_sDeviceName;
  m_sVirtualFolderDevice = pSettings->m_sVirtualFolderDevice;
  
  m_bShowPlaylistAsContainer = pSettings->m_bShowPlaylistAsContainer;
  m_bXBox360Support          = pSettings->m_bXBox360Support;
  m_bDLNAEnabled             = pSettings->m_bDLNAEnabled;
  //m_nTranscodingResponse     = pSettings->m_nTranscodingResponse;
  
  /*m_ImageSettings.bResize    = pSettings->m_ImageSettings.bResize;
  m_ImageSettings.bResizeIfLarger = pSettings->m_ImageSettings.bResizeIfLarger;
  m_ImageSettings.nWidth  = pSettings->m_ImageSettings.nWidth;
  m_ImageSettings.nHeight = pSettings->m_ImageSettings.nWidth;*/
  
  m_DisplaySettings.bShowChildCountInTitle = pSettings->m_DisplaySettings.bShowChildCountInTitle;
  m_DisplaySettings.nMaxFileNameLength     = pSettings->m_DisplaySettings.nMaxFileNameLength; 
  
  
  nDefaultReleaseDelay = pSettings->nDefaultReleaseDelay;
  
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


bool CDeviceSettings::DoTranscode(std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    if(
       (m_FileSettingsIterator->second->pTranscodingSettings &&
        m_FileSettingsIterator->second->pTranscodingSettings->Enabled() &&
        (!m_FileSettingsIterator->second->pTranscodingSettings->sDecoder.empty() ||
         !m_FileSettingsIterator->second->pTranscodingSettings->sEncoder.empty() ||
         !m_FileSettingsIterator->second->pTranscodingSettings->sTranscoder.empty())
        ) ||
       
       (m_FileSettingsIterator->second->pImageSettings &&
        m_FileSettingsIterator->second->pImageSettings->Enabled())) {
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
         
       if(!pTranscodingSettings->sTranscoder.empty()) {
         return TT_THREADED_TRANSCODER;
       }
       else if(!pTranscodingSettings->sDecoder.empty() && !pTranscodingSettings->sEncoder.empty()) {
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

TRANSCODER_TYPE CDeviceSettings::GetTranscoderType(std::string p_sExt)
{
  m_FileSettingsIterator = m_FileSettings.find(p_sExt);
  if(m_FileSettingsIterator != m_FileSettings.end()) {
    if(m_FileSettingsIterator->second->pTranscodingSettings &&
       m_FileSettingsIterator->second->pTranscodingSettings->Enabled()) {

       CTranscodingSettings* pTranscodingSettings = m_FileSettingsIterator->second->pTranscodingSettings;

       if(!pTranscodingSettings->sTranscoder.empty() && 
          pTranscodingSettings->sTranscoder.compare("ffmpeg") == 0) {
         return TTYP_FFMPEG;
       }
       else {
         return TTYP_NONE;
       }         
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

       if(!pTranscodingSettings->sDecoder.empty()) {
        
         if(pTranscodingSettings->sDecoder.compare("vorbis") == 0) {
           return DT_OGG_VORBIS;
         }
         else {
           return DT_NONE;
         }
         
       }
       else {
         return DT_NONE;
       }         
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

       if(!pTranscodingSettings->sEncoder.empty()) {
        
         if(pTranscodingSettings->sEncoder.compare("lame") == 0) {
           return ET_LAME;
         }
         else {
           return ET_NONE;
         }
         
       }
       else {
         return ET_NONE;
       }         
    }   
    else {
      return ET_NONE;
    }
  }
  else {
    return ET_NONE;
  }
}



std::string CDeviceSettings::MimeType(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->MimeType();
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

unsigned int CDeviceSettings::TargetSampleRate(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->TargetSampleRate();
  else  
    return 0;
}

unsigned int CDeviceSettings::TargetBitRate(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->TargetBitRate();
  else  
    return 0;
}

bool CDeviceSettings::Exists(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  return (iter != m_FileSettings.end());
}

std::string CDeviceSettings::Extension(std::string p_sExt)
{
  FileSettingsIterator_t  iter;
  
  iter = m_FileSettings.find(p_sExt);
  if(iter != m_FileSettings.end())
    return iter->second->Extension();
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
