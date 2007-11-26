/***************************************************************************
 *            ConfigFile.cpp
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

#include "ConfigFile.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"
#include "DefaultConfig.h"

#include <sstream>
#include <iostream>

using namespace std;

CConfigFile::CConfigFile()
{
  m_pDoc = new CXMLDocument();
  
  m_sNetInterface = "";  
  m_nHttpPort  = 0;

  m_sLocalCharset   = "UTF-8";
  m_bUseImageMagick = false;
  m_bUseTaglib      = false;
  m_bUseLibAvFormat = false;
  
  m_bTranscodeFlac     = true;
  m_bTranscodeVorbis   = true;
  m_bTranscodeMusePack = true;
}

CConfigFile::~CConfigFile()
{
  if(m_pDoc != NULL) {
    delete m_pDoc;
  }    
}

int CConfigFile::Load(std::string p_sFileName, std::string* p_psErrorMsg)
{
  if(!m_pDoc->Load(p_sFileName)) {    
    *p_psErrorMsg = "parse error";
    return CF_PARSE_ERROR;
  }

  if(m_pDoc->RootNode() == NULL) {
    *p_psErrorMsg = "parse error";
    return CF_PARSE_ERROR;
  }
  
  string sVersion = m_pDoc->RootNode()->Attribute("version");
  if(sVersion.compare(NEEDED_CONFIGFILE_VERSION) != 0) {
    *p_psErrorMsg = "configuration deprecated";
    return CF_CONFIG_DEPRECATED;
  }
  
  CXMLNode* pTmpNode;
  int i;
  
	ReadSharedObjects();
  ReadNetworkSettings();
	ReadGlobalSettings();
		
  // content_directory
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("content_directory", false);
  if(pTmpNode != NULL) {
    for(i = 0; i < pTmpNode->ChildCount(); i++) {
      
      if(pTmpNode->ChildNode(i)->Name().compare("local_charset") == 0) {
        m_sLocalCharset = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("use_imagemagick") == 0) {
        m_bUseImageMagick = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("use_taglib") == 0) {
        m_bUseTaglib = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }      
      else if(pTmpNode->ChildNode(i)->Name().compare("use_libavformat") == 0) {
        m_bUseLibAvFormat = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }     
    }
  }
  // end content_directory

  // transcoding
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("transcoding", false);
  if(pTmpNode != NULL) {
    for(i = 0; i < pTmpNode->ChildCount(); i++) {    
      
      if(pTmpNode->ChildNode(i)->Name().compare("audio_encoder") == 0) {
        m_sAudioEncoder = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("transcode_vorbis") == 0) {
        m_bTranscodeVorbis = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("transcode_musepack") == 0) {
        m_bTranscodeMusePack = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("transcode_flac") == 0) {
        m_bTranscodeFlac = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }
	  
      else if(pTmpNode->ChildNode(i)->Name().compare("lame_libname") == 0) {
        m_sLameLibName = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("twolame_libname") == 0) {
        m_sTwoLameLibName = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("vorbis_libname") == 0) {
        m_sVorbisLibName = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("flac_libname") == 0) {
        m_sFlacLibName = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("mpc_libname") == 0) {
        m_sMpcLibName = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("faad_libname") == 0) {
        m_sFaadLibName = pTmpNode->ChildNode(i)->Value();	  
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("mp4ff_libname") == 0) {
        m_sMp4ffLibName = pTmpNode->ChildNode(i)->Value();	  
      }	  
    }
  }
  // end transcoding
  
  // device_settings
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("device_settings", false);
  if(pTmpNode != NULL) {
    SetupDeviceIdentificationMgr(pTmpNode);
  }
  // end device_settings
  
  return CF_OK;
}



void CConfigFile::ReadSharedObjects()
{
  CXMLNode* pTmpNode;
  int i;  
  
  m_lSharedDirs.clear();    
  m_lSharedITunes.clear();
  
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("shared_objects", false);
  if(!pTmpNode) {
		return;
	}
		
	for(i = 0; i < pTmpNode->ChildCount(); i++) {
		if(pTmpNode->ChildNode(i)->Name().compare("dir") == 0) {
			m_lSharedDirs.push_back(pTmpNode->ChildNode(i)->Value());
		}
		else if(pTmpNode->ChildNode(i)->Name().compare("itunes") == 0) {
			m_lSharedITunes.push_back(pTmpNode->ChildNode(i)->Value());
		}
	}
}

void CConfigFile::ReadNetworkSettings()
{
  CXMLNode* pTmpNode;
  int i;
  int j;  
  
  m_lAllowedIps.clear();
  
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("network", false);
  if(pTmpNode == NULL) {
    return;
  }
  
  for(i = 0; i < pTmpNode->ChildCount(); i++) {
      
    if(pTmpNode->ChildNode(i)->Name().compare("interface") == 0) {
      if(pTmpNode->ChildNode(i)->Value().length() > 0) {
        m_sNetInterface = pTmpNode->ChildNode(i)->Value();
      }        
    }
    else if(pTmpNode->ChildNode(i)->Name().compare("http_port") == 0) {
      if(pTmpNode->ChildNode(i)->Value().length() > 0) {
        m_nHttpPort = atoi(pTmpNode->ChildNode(i)->Value().c_str());
      } 
    }
    else if(pTmpNode->ChildNode(i)->Name().compare("allowed_ips") == 0) {
      for(j = 0; j < pTmpNode->ChildNode(i)->ChildCount(); j++) {
        if(pTmpNode->ChildNode(i)->ChildNode(j)->Name().compare("ip") == 0) {
          m_lAllowedIps.push_back(pTmpNode->ChildNode(i)->ChildNode(j)->Value());
        }
      }          
    }     
    
  }  
}

void CConfigFile::ReadGlobalSettings()
{
	CXMLNode* pTmpNode;
  
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("global_settings", false);
  if(pTmpNode == NULL) {
    return;
  }

  for(int i = 0; i < pTmpNode->ChildCount(); i++) {

    if(pTmpNode->ChildNode(i)->Name().compare("temp_dir") == 0) {
      if(pTmpNode->ChildNode(i)->Value().length() > 0) {
        m_sTempDir = pTmpNode->ChildNode(i)->Value();
      }
    }
  }  
}

void CConfigFile::SetupDeviceIdentificationMgr(CXMLNode* pDeviceSettingsNode, bool p_bDefaultInitialized)
{
  int i;
  int j;
  int k;
  CXMLNode* pDevice;
  CXMLNode* pTmp;
  CDeviceSettings* pSettings;
  
  for(i = 0; i < pDeviceSettingsNode->ChildCount(); i++) {
    
    pDevice = pDeviceSettingsNode->ChildNode(i);
    
    // "default" can't be disabled
    if(pDevice->Attribute("name").compare("default") != 0 && 
       pDevice->Attribute("enabled").compare("true") != 0) {
      continue;
    }
    
    // make sure we fully initialize "default"
    // because other devices inherit the settings
    if(pDevice->Attribute("name").compare("default") != 0 &&
       !p_bDefaultInitialized) {
      continue;
    }
    
    // default already initialized
    if(pDevice->Attribute("name").compare("default") == 0 &&
       p_bDefaultInitialized) {
      continue;
    }
    
    pSettings = CDeviceIdentificationMgr::Shared()->GetSettingsForInitialization(pDevice->Attribute("name"));
    
    // virtual device
    if(pDevice->Attribute("virtual").length() > 0) {
      pSettings->m_sVirtualFolderDevice = pDevice->Attribute("virtual");
    }
    
    // settings
    for(j = 0; j < pDevice->ChildCount(); j++) {
      pTmp = pDevice->ChildNode(j);
      
      // user_agent
      if(pTmp->Name().compare("user_agent") == 0) {        
        pSettings->m_slUserAgents.push_back(pTmp->Value());
      }
      // ip
      else if(pTmp->Name().compare("ip") == 0) {
        pSettings->m_slIPAddresses.push_back(pTmp->Value());
      }
      // playlist_style
      else if(pTmp->Name().compare("playlist_style") == 0) {
        pSettings->m_bShowPlaylistAsContainer = (pTmp->Value().compare("container") == 0);
      }
      // max_file_name_length
      else if(pTmp->Name().compare("max_file_name_length") == 0) {
        pSettings->DisplaySettings()->nMaxFileNameLength = atoi(pTmp->Value().c_str());
      }
      // show_childcount_in_title
      else if(pTmp->Name().compare("show_childcount_in_title") == 0) {
        pSettings->DisplaySettings()->bShowChildCountInTitle = (pTmp->Value().compare("true") == 0);
      }
      // xbox360
      else if(pTmp->Name().compare("xbox360") == 0) {
        pSettings->m_bXBox360Support = (pTmp->Value().compare("true") == 0);
				// xbox implies	media_receiver_registrar
				if(pSettings->m_bXBox360Support)
					pSettings->MediaServerSettings()->UseXMSMediaReceiverRegistrar = true;
      }
			// media_receiver_registrar
			else if(pTmp->Name().compare("enable_xms_media_receiver_registrar") == 0) {
        pSettings->MediaServerSettings()->UseXMSMediaReceiverRegistrar = (pTmp->Value().compare("true") == 0);
      }
      // enable_url_base
			else if(pTmp->Name().compare("enable_url_base") == 0) {
				if(pTmp->Value().compare("true") == 0)
					pSettings->MediaServerSettings()->UseURLBase = true;
				else if(pTmp->Value().compare("false") == 0)				
					pSettings->MediaServerSettings()->UseURLBase = false;
			}
			// dlna
      else if(pTmp->Name().compare("enable_dlna") == 0) {
        pSettings->m_bDLNAEnabled = (pTmp->Value().compare("true") == 0);
      }
      else if(pTmp->Name().compare("transcoding_release_delay") == 0) {
        pSettings->nDefaultReleaseDelay = atoi(pTmp->Value().c_str());
      }
      else if(pTmp->Name().compare("show_device_icon") == 0) {
        pSettings->m_bEnableDeviceIcon = (pTmp->Value().compare("true") == 0);
      }
			// file_settings
      else if(pTmp->Name().compare("file_settings") == 0) {
        
        for(k = 0; k < pTmp->ChildCount(); k++) {
          if(pTmp->ChildNode(k)->Name().compare("file") != 0)
            continue;
          
          ParseFileSettings(pTmp->ChildNode(k), pSettings);
        }
      }
      // description_values
			else if(pTmp->Name().compare("description_values") == 0) {
				ParseDescriptionValues(pTmp, pSettings);
			}
				
    }
			
    // now that we got "default" initialized let's
    // set up the other devices
    if(pDevice->Attribute("name").compare("default") == 0) {
      SetupDeviceIdentificationMgr(pDeviceSettingsNode, true);
      break;
    }
    
  }
}

OBJECT_TYPE ParseObjectType(std::string p_sObjectType)
{
  if(p_sObjectType.compare("AUDIO_ITEM") == 0) {
    return ITEM_AUDIO_ITEM;
  }
  else if(p_sObjectType.compare("AUDIO_ITEM_MUSIC_TRACK") == 0) {
    return ITEM_AUDIO_ITEM_MUSIC_TRACK;
  }
  
  else if(p_sObjectType.compare("IMAGE_ITEM") == 0) {
    return ITEM_IMAGE_ITEM;
  }
  else if(p_sObjectType.compare("IMAGE_ITEM_PHOTO") == 0) {
    return ITEM_IMAGE_ITEM_PHOTO;
  }
  
  else if(p_sObjectType.compare("VIDEO_ITEM") == 0) {
    return ITEM_VIDEO_ITEM;
  }
  else if(p_sObjectType.compare("VIDEO_ITEM_MOVIE") == 0) {
    return ITEM_VIDEO_ITEM_MOVIE;
  }
  
  else if(p_sObjectType.compare("PLAYLIST") == 0) {
    return CONTAINER_PLAYLIST_CONTAINER;
  }
  
  else 
    return OBJECT_TYPE_UNKNOWN;
}

void CConfigFile::ParseFileSettings(CXMLNode* pFileSettings, CDeviceSettings* pDevSet)
{
  int i;
  CFileSettings* pFileSet;
  CXMLNode* pTmp;

  pFileSet = pDevSet->FileSettings(ToLower(pFileSettings->Attribute("ext")));
  pFileSet->sExt = ToLower(pFileSettings->Attribute("ext"));
  
  if(pFileSettings->Attribute("extract_metadata").compare("false") == 0) {
    pFileSet->bExtractMetadata = false;
  }
  
  for(i = 0; i < pFileSettings->ChildCount(); i++) {    
    
    pTmp = pFileSettings->ChildNode(i);  
    
    if(pTmp->Name().compare("type") == 0) {
      pFileSet->nType = ParseObjectType(pTmp->Value());
    }
    else if(pTmp->Name().compare("ext") == 0) {
      pDevSet->AddExt(pFileSet, ToLower(pTmp->Value()));
    }
    else if(pTmp->Name().compare("mime_type") == 0) {
      pFileSet->sMimeType = pTmp->Value();
    }
    else if(pTmp->Name().compare("dlna") == 0) {
      pFileSet->sDLNA = pTmp->Value();
    }
    else if(pTmp->Name().compare("transcode") == 0) {
      ParseTranscodingSettings(pTmp, pFileSet);
    }
    else if(pTmp->Name().compare("convert") == 0) {
      ParseImageSettings(pTmp, pFileSet);
    }
    
  }
}

void CConfigFile::ParseTranscodingSettings(CXMLNode* pTCNode, CFileSettings* pFileSet)
{
  int i;
  CXMLNode* pTmp;
  
  // delete existing (inherited) settings
  // if transcoding disabled
  if(pTCNode->Attribute("enabled").compare("true") != 0) {
    if(pFileSet->pTranscodingSettings) {
      delete pFileSet->pTranscodingSettings;
      pFileSet->pTranscodingSettings = NULL;
      return;
    }
  }
  
  // create new settings
  if(!pFileSet->pTranscodingSettings) {
    pFileSet->pTranscodingSettings = new CTranscodingSettings();
  }
  
  bool bExt = false;
  
  // read transcoding settings
  for(i = 0; i < pTCNode->ChildCount(); i++) {
    pTmp = pTCNode->ChildNode(i);
    
    if(pTmp->Name().compare("ext") == 0) {
      pFileSet->pTranscodingSettings->sExt = pTmp->Value();
      bExt = true;
    }
    else if(pTmp->Name().compare("mime_type") == 0) {
      pFileSet->pTranscodingSettings->sMimeType = pTmp->Value();
    }
    else if(pTmp->Name().compare("dlna") == 0) {
      pFileSet->pTranscodingSettings->sDLNA = pTmp->Value();
    }
    else if(pTmp->Name().compare("http_encoding") == 0) {
      if(pTmp->Value().compare("chunked") == 0) {
        pFileSet->pTranscodingSettings->nTranscodingResponse = RESPONSE_CHUNKED;
      }
      else if(pTmp->Value().compare("stream") == 0) {
        pFileSet->pTranscodingSettings->nTranscodingResponse = RESPONSE_STREAM;
      }
    }
    else if(pTmp->Name().compare("out_params") == 0) {
      pFileSet->pTranscodingSettings->sOutParams = pTmp->Value();
    }                           
    else if(pTmp->Name().compare("encoder") == 0) {
      
      if(pTmp->Value().compare("lame") == 0)
        pFileSet->pTranscodingSettings->nEncoderType = ET_LAME;
      else if(pTmp->Value().compare("twolame") == 0)
        pFileSet->pTranscodingSettings->nEncoderType = ET_TWOLAME;
      else if(pTmp->Value().compare("wav") == 0) {
        pFileSet->pTranscodingSettings->nEncoderType = ET_WAV;
        pFileSet->pTranscodingSettings->nAudioSampleRate = 44100;
        pFileSet->pTranscodingSettings->nAudioBitRate = 176400;
      }
      else if(pTmp->Value().compare("pcm") == 0) {
        pFileSet->pTranscodingSettings->nEncoderType = ET_PCM;
        pFileSet->pTranscodingSettings->nAudioSampleRate = 44100;
        pFileSet->pTranscodingSettings->nAudioBitRate = 176400;
      }
      else
        pFileSet->pTranscodingSettings->nEncoderType = ET_NONE;
      
    }
    else if(pTmp->Name().compare("decoder") == 0) {
      
      if(pTmp->Value().compare("vorbis") == 0)
        pFileSet->pTranscodingSettings->nDecoderType = DT_OGG_VORBIS;
      else if(pTmp->Value().compare("flac") == 0)
        pFileSet->pTranscodingSettings->nDecoderType = DT_FLAC;
      else if(pTmp->Value().compare("mpc") == 0)
        pFileSet->pTranscodingSettings->nDecoderType = DT_MUSEPACK;
      else if(pTmp->Value().compare("faad") == 0)
        pFileSet->pTranscodingSettings->nDecoderType = DT_FAAD;
      else
        pFileSet->pTranscodingSettings->nDecoderType = DT_NONE;
      
    }
    else if(pTmp->Name().compare("transcoder") == 0) {
      
      if(pTmp->Value().compare("ffmpeg") == 0) {
        pFileSet->pTranscodingSettings->nTranscoderType   = TTYP_FFMPEG;        
      }
      else {
        pFileSet->pTranscodingSettings->nTranscoderType   = TTYP_NONE;            
      }
      
    }
    
    else if(pTmp->Name().compare("video_codec") == 0) {
      pFileSet->pTranscodingSettings->sVCodecCondition = pTmp->Attribute("vcodec");
      pFileSet->pTranscodingSettings->sVCodec = pTmp->Value();
    }
    else if(pTmp->Name().compare("video_bitrate") == 0) {
      pFileSet->pTranscodingSettings->nVideoBitRate = atoi(pTmp->Value().c_str());
    }
    else if(pTmp->Name().compare("audio_codec") == 0) {
      pFileSet->pTranscodingSettings->sACodecCondition = pTmp->Attribute("acodec");
      pFileSet->pTranscodingSettings->sACodec = pTmp->Value();
    }  
    else if(pTmp->Name().compare("ffmpeg_params") == 0) {      
      pFileSet->pTranscodingSettings->sFFmpegParams = pTmp->Value();
    }
    else if((pTmp->Name().compare("bitrate") == 0) || pTmp->Name().compare("audio_bitrate") == 0) {
      pFileSet->pTranscodingSettings->nAudioBitRate = atoi(pTmp->Value().c_str());
    }
    else if((pTmp->Name().compare("samplerate") == 0) || pTmp->Name().compare("audio_samplerate") == 0) {
      pFileSet->pTranscodingSettings->nAudioSampleRate = atoi(pTmp->Value().c_str());
    }
    else if(pTmp->Name().compare("lame_quality") == 0) {
      pFileSet->pTranscodingSettings->nLameQuality = atoi(pTmp->Value().c_str());
    }
    
  }
  
  
  
  // no de/encoders selected but extension available => rename file
  if((pFileSet->pTranscodingSettings->nDecoderType == DT_NONE) &&
     (pFileSet->pTranscodingSettings->nEncoderType == ET_NONE) &&
     (pFileSet->pTranscodingSettings->nTranscoderType == TTYP_NONE) && bExt) {
       
    pFileSet->pTranscodingSettings->nTranscodingType  = TT_RENAME; 
  }
  
  // de- and encoder selected => threaded de/encoder
  else if(pFileSet->pTranscodingSettings->nDecoderType != DT_NONE &&
     pFileSet->pTranscodingSettings->nEncoderType != ET_NONE) {  
    pFileSet->pTranscodingSettings->nTranscodingType  = TT_THREADED_DECODER_ENCODER;
  }
  
  // transcoder selected => threaded transcoder
  else if(pFileSet->pTranscodingSettings->nTranscoderType != TTYP_NONE) {
    pFileSet->pTranscodingSettings->nTranscodingType  = TT_THREADED_TRANSCODER;
  }
  
  else {
    pFileSet->pTranscodingSettings->nTranscodingType  = TT_NONE;
  }
  
  
}

void CConfigFile::ParseImageSettings(CXMLNode* pISNode, CFileSettings* pFileSet)
{
  int i;
  CXMLNode* pTmp;
  int j;
  CXMLNode* pChild;
  
  // delete existing (inherited) settings
  // if transcoding disabled
  if(pISNode->Attribute("enabled").compare("true") != 0) {
    if(pFileSet->pImageSettings) {
      delete pFileSet->pImageSettings;
      pFileSet->pImageSettings = NULL;
      return;
    }
  }
  
  // create new settings
  if(!pFileSet->pImageSettings) {
    pFileSet->pImageSettings = new CImageSettings();
    pFileSet->pImageSettings->bEnabled = (pISNode->Attribute("enabled").compare("true") == 0);
  }
  
  // read image settings
  for(i = 0; i < pISNode->ChildCount(); i++) {
    pTmp = pISNode->ChildNode(i);
    
    if(pTmp->Name().compare("ext") == 0) {
      pFileSet->pImageSettings->sExt = pTmp->Value();
    }
    else if(pTmp->Name().compare("mime_type") == 0) {
      pFileSet->pImageSettings->sMimeType = pTmp->Value();
    }    
    else if(pTmp->Name().compare("dcraw") == 0) {
      if(pTmp->Attribute("enabled").compare("false") != 0) {
        pFileSet->pImageSettings->bDcraw = true;        
      }
      pFileSet->pImageSettings->sDcrawParams = pTmp->Value();
    }
    /*else if(pTmp->Name().compare("scale") == 0) {
      for(j = 0; j < pTmp->ChildCount(); j++) {
        pChild = pTmp->ChildNode(j);*/
        
        else if(pTmp->Name().compare("height") == 0) {
          pFileSet->pImageSettings->nHeight = atoi(pTmp->Value().c_str());
        }
        else if(pTmp->Name().compare("width") == 0) {
          pFileSet->pImageSettings->nWidth = atoi(pTmp->Value().c_str());
        }
        else if(pTmp->Name().compare("greater") == 0) {
          pFileSet->pImageSettings->bGreater = (pTmp->Value().compare("true") == 0);
        }
        else if(pTmp->Name().compare("less") == 0) {
          pFileSet->pImageSettings->bLess = (pTmp->Value().compare("true") == 0);
        }
      //}
    //}

  }
}

void CConfigFile::ParseDescriptionValues(CXMLNode* pDescrValues, CDeviceSettings* pDevSet)
{
	for(int i = 0; i < pDescrValues->ChildCount(); i++) {
			
		// friendly_name
		if(pDescrValues->ChildNode(i)->Name().compare("friendly_name") == 0) {
			pDevSet->MediaServerSettings()->FriendlyName = 
						pDescrValues->ChildNode(i)->Value();
    }
		// manufacturer
		else if(pDescrValues->ChildNode(i)->Name().compare("manufacturer") == 0) {
			pDevSet->MediaServerSettings()->Manufacturer = 
						pDescrValues->ChildNode(i)->Value();
    }
		// manufacturer_url
		else if(pDescrValues->ChildNode(i)->Name().compare("manufacturer_url") == 0) {
			pDevSet->MediaServerSettings()->ManufacturerURL = 
						pDescrValues->ChildNode(i)->Value();
    }
		// model_name
		else if(pDescrValues->ChildNode(i)->Name().compare("model_name") == 0) {
			pDevSet->MediaServerSettings()->ModelName = 
						pDescrValues->ChildNode(i)->Value();
    }
		// model_number
		else if(pDescrValues->ChildNode(i)->Name().compare("model_number") == 0) {
			pDevSet->MediaServerSettings()->ModelNumber = 
						pDescrValues->ChildNode(i)->Value();
    }
		// model_url
		else if(pDescrValues->ChildNode(i)->Name().compare("model_url") == 0) {
			pDevSet->MediaServerSettings()->ModelURL = 
						pDescrValues->ChildNode(i)->Value();
    }		
		// model_description
		else if(pDescrValues->ChildNode(i)->Name().compare("model_description") == 0) {
			pDevSet->MediaServerSettings()->ModelDescription = 
						pDescrValues->ChildNode(i)->Value();
				
			if(pDescrValues->ChildNode(i)->Attribute("enabled").compare("true") == 0)
				pDevSet->MediaServerSettings()->UseModelDescription = true;
			else if(pDescrValues->ChildNode(i)->Attribute("enabled").compare("false") == 0)
				pDevSet->MediaServerSettings()->UseModelDescription = false;
    }
		// serial_number
		else if(pDescrValues->ChildNode(i)->Name().compare("serial_number") == 0) {
			pDevSet->MediaServerSettings()->SerialNumber = 
						pDescrValues->ChildNode(i)->Value();
				
			if(pDescrValues->ChildNode(i)->Attribute("enabled").compare("true") == 0)
				pDevSet->MediaServerSettings()->UseSerialNumber = true;
			else if(pDescrValues->ChildNode(i)->Attribute("enabled").compare("false") == 0)
				pDevSet->MediaServerSettings()->UseSerialNumber = false;
    }
		// upc
		else if(pDescrValues->ChildNode(i)->Name().compare("upc") == 0) {
			pDevSet->MediaServerSettings()->UPC = 
						pDescrValues->ChildNode(i)->Value();
				
			if(pDescrValues->ChildNode(i)->Attribute("enabled").compare("true") == 0)
				pDevSet->MediaServerSettings()->UseUPC = true;
			else if(pDescrValues->ChildNode(i)->Attribute("enabled").compare("false") == 0)
				pDevSet->MediaServerSettings()->UseUPC = false;
    }
	} // for
}

void CConfigFile::AddSharedDir(std::string p_sDirName)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  if(pTmp != NULL) {
    pTmp->AddChild("dir", p_sDirName);
    ReadSharedObjects();
    m_pDoc->Save();
  }
}

void CConfigFile::RemoveSharedDir(int p_nIdx)
{
  int i;
  int nIdx = 0;
  
  CXMLNode* pObj = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  CXMLNode* pTmp;
  if(pObj == NULL) {
    return;    
  }
  
  for(i = 0; i < pObj->ChildCount(); i++) {
    pTmp = pObj->ChildNode(i);
    if(pTmp->Name().compare("dir") == 0) {
      if(nIdx == p_nIdx) {
        pObj->RemoveChild(i);
        ReadSharedObjects();
        m_pDoc->Save();
        break;
      }      
      nIdx++;
    }    
  }
}

void CConfigFile::AddSharedITunes(std::string p_sITunesName)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  if(pTmp != NULL) {
    pTmp->AddChild("itunes", p_sITunesName); 
    ReadSharedObjects();
    m_pDoc->Save();
  }  
}

void CConfigFile::RemoveSharedITunes(int p_nIdx)
{
  int i;
  int nIdx = 0;
  
  CXMLNode* pObj = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  CXMLNode* pTmp;
  if(pObj == NULL) {
    return;    
  }
  
  for(i = 0; i < pObj->ChildCount(); i++) {
    pTmp = pObj->ChildNode(i);
    if(pTmp->Name().compare("itunes") == 0) {
      if(nIdx == p_nIdx) {
        pObj->RemoveChild(i);
        ReadSharedObjects();
        m_pDoc->Save();
        break;
      }      
      nIdx++;
    }    
  }  
}


void CConfigFile::NetInterface(std::string p_sNetInterface)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("interface", true);
  if(pTmp) {
    pTmp->Value(p_sNetInterface);
    m_pDoc->Save();
    m_sNetInterface = p_sNetInterface;
  }  
}

void CConfigFile::HttpPort(int p_nHttpPort)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("http_port", true);
  if(pTmp) {
    pTmp->Value(p_nHttpPort);
    m_pDoc->Save();  
    m_nHttpPort = p_nHttpPort;
  }  
}

void CConfigFile::LocalCharset(std::string p_sLocalCharset)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("local_charset", true);
  if(pTmp) {
    pTmp->Value(p_sLocalCharset);
    m_pDoc->Save();  
    m_sLocalCharset = p_sLocalCharset;
  } 
}


void CConfigFile::AddAllowedIp(std::string p_sIpAddress)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("allowed_ips", true);
  if(pTmp != NULL) {
    pTmp->AddChild("ip", p_sIpAddress);
    ReadNetworkSettings();
    m_pDoc->Save();
  }  
}

void CConfigFile::RemoveAllowedIp(int p_nIdx)
{
  int i;  
  int nIdx = 0;
  
  CXMLNode* pObj = m_pDoc->RootNode()->FindNodeByName("allowed_ips", true);
  CXMLNode* pTmp;
  if(pObj == NULL) {
    return;    
  }
  
  for(i = 0; i < pObj->ChildCount(); i++) {
    pTmp = pObj->ChildNode(i);
    
    if(pTmp->Name().compare("ip") == 0) {    
      if(nIdx == p_nIdx) {
        pObj->RemoveChild(i);
        ReadNetworkSettings();
        m_pDoc->Save();
        break;
      }        
    }
  }  
}


bool CConfigFile::WriteDefaultConfig(std::string p_sFileName)
{
  return WriteDefaultConfigFile(p_sFileName);
}
