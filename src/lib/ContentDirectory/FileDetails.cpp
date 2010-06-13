/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            FileDetails.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "FileDetails.h"

#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Transcoding/TranscodingMgr.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"

#include "../Plugins/Plugin.h"

#include <sstream>
#include <iostream>

using namespace std;

CFileDetails* CFileDetails::m_Instance = 0;

CFileDetails* CFileDetails::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CFileDetails();
	return m_Instance;
}

void CFileDetails::deleteInstance() // static
{
  if(m_Instance == 0)
    return;
  delete m_Instance;
  m_Instance = NULL;
}

CFileDetails::CFileDetails()
{
}

CFileDetails::~CFileDetails()
{
}

OBJECT_TYPE CFileDetails::GetObjectType(std::string p_sFileName)
{
  string sExt = ExtractFileExt(p_sFileName);
  return CDeviceIdentificationMgr::Shared()->DefaultDevice()->ObjectType(sExt);
}

std::string CFileDetails::GetObjectTypeAsStr(OBJECT_TYPE p_nObjectType)
{
  if(p_nObjectType < ITEM) {
    return GetContainerTypeAsStr(p_nObjectType);
  }
  
  switch(p_nObjectType) {
    
    case ITEM:
      return "object.item";      
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
  
    case ITEM_VIDEO_ITEM :
      return "object.item.videoItem";
    case ITEM_VIDEO_ITEM_MOVIE :
      return "object.item.videoItem.movie";
    case ITEM_VIDEO_ITEM_VIDEO_BROADCAST :
      return "object.item.videoItem.videoBroadcast";
      
    default:
      return "unknown";
  }
}

std::string CFileDetails::GetContainerTypeAsStr(OBJECT_TYPE p_nContainerType)
{
  switch(p_nContainerType) {
    
    //CONTAINER_PERSON = 4,
    case CONTAINER_PERSON_MUSIC_ARTIST :
      return "object.container.person.musicArtist";
    
    case CONTAINER_PLAYLIST_CONTAINER :
      return "object.container.playlistContainer";
    
    //CONTAINER_ALBUM = 6, 
    
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

bool CFileDetails::IsSupportedFileExtension(std::string p_sFileExtension)
{
  p_sFileExtension = ToLower(p_sFileExtension);
  return CDeviceIdentificationMgr::Shared()->DefaultDevice()->Exists(p_sFileExtension);
}

bool CFileDetails::getMusicTrackDetails(std::string p_sFileName, AudioItem* audioItem) // static
{
  string sExt = ExtractFileExt(p_sFileName);
  if(!CDeviceIdentificationMgr::Shared()->DefaultDevice()->FileSettings(sExt)->ExtractMetadata())
    return false;
	
	CMetadataPlugin* audio = NULL;

	/*if(sExt.compare("m4a") == 0 || sExt.compare("mp4") == 0) {
		audio = CPluginMgr::metadataPlugin("mp4v2");
	}
	else {*/
		audio = CPluginMgr::metadataPlugin("taglib");
	//}

	//audio = CPluginMgr::metadataPlugin("libavformat");
	
	//metadata_t metadata;
	if(!audio)
    return false;

  bool result = false;
  if(audio->openFile(p_sFileName)) {
    result = audio->readData(audioItem->metadata());
	  audio->closeFile(); 
	}
  delete audio;  
	return result;	
}

bool CFileDetails::getImageDetails(std::string p_sFileName, ImageItem* imageItem) // static
{
	if(!CPluginMgr::hasMetadataPlugin("exiv2") && 
	 	 !CPluginMgr::hasMetadataPlugin("magickWand") &&
     !CPluginMgr::hasMetadataPlugin("simage"))
		return false;
	
	string sExt = ExtractFileExt(p_sFileName);  
  if(!CDeviceIdentificationMgr::Shared()->DefaultDevice()->FileSettings(sExt)->ExtractMetadata())
    return false;
	
	CMetadataPlugin* image;
  bool result = false;

  std::string plugins[] = {"exiv2, magickWand", "simage", ""};      

  for(int i = 0; plugins[i].length() > 0; i++) {

    image = CPluginMgr::metadataPlugin(plugins[i]);
    if(!image)
      continue;

    if(image->openFile(p_sFileName)) {
		  result = image->readData(imageItem->metadata());
      image->closeFile();
	  }
    delete image;
    image = NULL;

    if(result)
      return true;
  }


	return false;
}

bool CFileDetails::getVideoDetails(std::string p_sFileName, VideoItem* videoItem) // static
{
	string sExt = ExtractFileExt(p_sFileName);  
  if(!CDeviceIdentificationMgr::Shared()->DefaultDevice()->FileSettings(sExt)->ExtractMetadata())
    return false;
  
	CMetadataPlugin* video = CPluginMgr::metadataPlugin("libavformat");
	if(!video) {
		return false;
	}

	bool result = false;
	if(video->openFile(p_sFileName)) {
		result = video->readData(videoItem->metadata());
		video->closeFile();
	}
	delete video;	

	return result;
}
