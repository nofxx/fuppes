/***************************************************************************
 *            FileDetails.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "FileDetails.h"

#include "../Common/Common.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Transcoding/TranscodingMgr.h"

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifdef HAVE_TAGLIB
#include <fileref.h>
#include <tstring.h>
#include <tfile.h>
#include <tag.h>
#endif

#ifdef HAVE_IMAGEMAGICK
#include <Magick++/Image.h> 
#endif

#include <sstream>
#include <iostream>


/** default file settings */
struct FileType_t FileTypes[] = 
{
  /* audio types */
  {"mp3" , ITEM_AUDIO_ITEM_MUSIC_TRACK, "audio/mpeg"},
  {"ogg" , ITEM_AUDIO_ITEM_MUSIC_TRACK, "application/octet-stream"},
  {"mpc" , ITEM_AUDIO_ITEM_MUSIC_TRACK, "application/octet-stream"},
  {"flac", ITEM_AUDIO_ITEM_MUSIC_TRACK, "audio/x-flac"},  
  
  /* image types */
  {"jpeg", ITEM_IMAGE_ITEM_PHOTO, "image/jpeg"},
  {"jpg" , ITEM_IMAGE_ITEM_PHOTO, "image/jpeg"},
  {"bmp" , ITEM_IMAGE_ITEM_PHOTO, "image/bmp"},
  {"png" , ITEM_IMAGE_ITEM_PHOTO, "image/png"},
  {"gif" , ITEM_IMAGE_ITEM_PHOTO, "image/gif"},
  
  /* video types */
  {"mpeg", ITEM_VIDEO_ITEM_MOVIE, "video/mpeg"},
  {"mpg" , ITEM_VIDEO_ITEM_MOVIE, "video/mpeg"},
  {"avi" , ITEM_VIDEO_ITEM_MOVIE, "video/x-msvideo"},
  {"wmv" , ITEM_VIDEO_ITEM_MOVIE, "video/x-ms-wmv"},
  {"vob" , ITEM_VIDEO_ITEM_MOVIE, "video/x-ms-vob"},
  {"vdr" , ITEM_VIDEO_ITEM_MOVIE, "application/x-extension-vdr"},
  
  /* playlist types */
  {"m3u", CONTAINER_PLAYLIST_CONTAINER, "audio/x-mpegurl"},
  {"pls", CONTAINER_PLAYLIST_CONTAINER, "audio/x-scpls"},
  
  /* empty entry to mark the list's end */
  {"", OBJECT_TYPE_UNKNOWN, ""}
};


struct TranscodingSetting_t TranscodingSettings[] =
{
  /* audio */
  {"ogg" , "mp3", "audio/mpeg", ITEM_AUDIO_ITEM_MUSIC_TRACK},
  {"mpc" , "mp3", "audio/mpeg", ITEM_AUDIO_ITEM_MUSIC_TRACK},
  {"flac", "mp3", "audio/mpeg", ITEM_AUDIO_ITEM_MUSIC_TRACK},
  
  /* video */
  {"vdr", "vob", "video/x-ms-vob", ITEM_VIDEO_ITEM_MOVIE},
  
  /* empty entry to mark the list's end */
  {"", "", "", OBJECT_TYPE_UNKNOWN}
};


using namespace std;

CFileDetails* CFileDetails::m_Instance = 0;

CFileDetails* CFileDetails::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CFileDetails();
	return m_Instance;
}

CFileDetails::CFileDetails()
{
}

OBJECT_TYPE CFileDetails::GetObjectType(std::string p_sFileName)
{
  string sExt = ToLower(ExtractFileExt(p_sFileName));
  struct FileType_t* pType;   
  
  pType = FileTypes;
  while(!pType->sExt.empty())
  {
    if(pType->sExt.compare(sExt) == 0)
    {
      return pType->nObjectType;
      break;
    } 
    
    pType++;
  }

  CSharedLog::Shared()->Log(L_WARNING, "unhandled file extension: " + sExt, __FILE__, __LINE__);  
  return OBJECT_TYPE_UNKNOWN;  
}

std::string CFileDetails::GetMimeType(std::string p_sFileName, bool p_bTranscodingMimeType)
{
  string sExt = ToLower(ExtractFileExt(p_sFileName));
  struct FileType_t* pType; 
  struct TranscodingSetting_t* pTranscoding;  
  
  pType = FileTypes;
  while(!pType->sExt.empty())
  {
    if(pType->sExt.compare(sExt) == 0)
    {
      
      // check for transcoding settings
      if(p_bTranscodingMimeType && CTranscodingMgr::Shared()->IsTranscodingExtension(sExt))
      {
        pTranscoding = TranscodingSettings;
        while(!pTranscoding->sExt.empty())
        {
          if(pTranscoding->sExt.compare(sExt) == 0)                      
            return pTranscoding->sTargetMimeType;
          
          pTranscoding++;
        }
      }
      
      // return default mime type
      return pType->sMimeType;      
    } // if
    
    pType++;
  } // while !sExt.empty
  
  
  CSharedLog::Shared()->Log(L_EXTENDED_WARN, "unhandled file extension: " + sExt, __FILE__, __LINE__);  
  return "";
}

std::string CFileDetails::GetObjectTypeAsString(unsigned int p_nObjectType)
{
  switch(p_nObjectType)
  {
    case OBJECT_TYPE_UNKNOWN :
      return "unknown";
    
    case ITEM_IMAGE_ITEM :
      return "imageItem";    
    case ITEM_IMAGE_ITEM_PHOTO :
      return "imageItem.photo";
  
    case ITEM_AUDIO_ITEM :
      return "audioItem";
    case ITEM_AUDIO_ITEM_MUSIC_TRACK :
      return "audioItem.musicTrack";
    case ITEM_AUDIO_ITEM_AUDIO_BROADCAST :
      return "audioItem.audioBroadcast";
    //ITEM_AUDIO_ITEM_AUDIO_BOOK      = 202,*/
  
    case ITEM_VIDEO_ITEM :
      return "videoItem";
    case ITEM_VIDEO_ITEM_MOVIE :
      return "videoItem.movie";
    case ITEM_VIDEO_ITEM_VIDEO_BROADCAST :
      return "videoItem.videoBroadcast";
    //ITEM_VIDEO_ITEM_MUSIC_VIDEO_CLIP = 302,  
  
    /*CONTAINER_PERSON = 4,
      CONTAINER_PERSON_MUSIC_ARTIST = 400,*/
    
    case CONTAINER_PLAYLIST_CONTAINER :
      return "container.playlistContainer";
    
    /*CONTAINER_ALBUM = 6,
    
      CONTAINER_ALBUM_MUSIC_ALBUM = 600,
      CONTAINER_ALBUM_PHOTO_ALBUM = 601,
      
    CONTAINER_GENRE = 7,
      CONTAINER_GENRE_MUSIC_GENRE = 700,
      CONTAINER_GENRE_MOVIE_GENRE = 701,
      
    CONTAINER_STORAGE_SYSTEM = 8,
    CONTAINER_STORAGE_VOLUME = 9, */
    case CONTAINER_STORAGE_FOLDER :
      return "container";
    
    default :
      return "CFileDetails::GetObjectTypeAsString() :: unhandled type (please send a bugreport)";
  }
}

bool CFileDetails::IsTranscodingExtension(std::string p_sExt)
{
  TranscodingSetting_t* pTranscoding;
  pTranscoding = TranscodingSettings;
  while(!pTranscoding->sExt.empty())
  {
    if(pTranscoding->sExt.compare(p_sExt) == 0)    
      return CTranscodingMgr::Shared()->IsTranscodingExtension(p_sExt);
    
    pTranscoding++;
  }
  return false;
}

std::string CFileDetails::GetTargetExtension(std::string p_sExt)
{
  string sResult = p_sExt;
  
  TranscodingSetting_t* pTranscoding;
  pTranscoding = TranscodingSettings;
  while(!pTranscoding->sExt.empty())
  {
    if(pTranscoding->sExt.compare(p_sExt) == 0)
    {
      sResult = pTranscoding->sTargetExt;
      break;
    }    
    pTranscoding++;
  }
  
  return sResult;
}

bool CFileDetails::GetMusicTrackDetails(std::string p_sFileName, SMusicTrack* pMusicTrack)
{
  #ifdef HAVE_TAGLIB  
  TagLib::FileRef pFile(p_sFileName.c_str());
  
	if (pFile.isNull()) {
	  CSharedLog::Shared()->Log(L_EXTENDED_ERR, "taglib error", __FILE__, __LINE__);
		return false;
	}
	
	TagLib::String sTmp;
	uint nTmp;
	
	// title
	sTmp = pFile.tag()->title();
  pMusicTrack->mAudioItem.sTitle = sTmp.to8Bit(true);  
  
	//string duration = "0:00:00.00";	
	long length = pFile.audioProperties()->length();
  stringstream s; 
	s << length/(60*60) << ":" << length/60 << ":" << (length - length/60);
  pMusicTrack->mAudioItem.sDuration = s.str();
	
	// artist
  sTmp = pFile.tag()->artist();
  pMusicTrack->sArtist = sTmp.to8Bit(true);  
    
  // album
  sTmp = pFile.tag()->album();
  pMusicTrack->sAlbum = sTmp.to8Bit(true);  
  
  // genre
	sTmp = pFile.tag()->genre();
  pMusicTrack->mAudioItem.sGenre = sTmp.to8Bit(true);   

  // description/comment
	sTmp = pFile.tag()->comment();
	pMusicTrack->mAudioItem.sDescription = sTmp.to8Bit(true);

  // track no.
	nTmp = pFile.tag()->track();
	pMusicTrack->nOriginalTrackNumber = nTmp;

  // date/year
	nTmp = pFile.tag()->year();
  stringstream sDate;
  sDate << nTmp;
  pMusicTrack->sDate = sDate.str();

  return true;
  #else
	return false;
	#endif
}

bool CFileDetails::GetImageDetails(std::string p_sFileName, SImageItem* pImageItem)
{
  #ifdef HAVE_IMAGEMAGICK
	cout << "GET IMAGE DETAILS" << endl;
	Magick::Image img;
	img.read(p_sFileName);
	
	unsigned int nDepth = img.depth();
	cout << "Depth: " << nDepth << endl;
	
	
	
/*	size	Geometry	void	const Geometry &geometry_	
	
	width


unsigned int width_

height*/
	
	#else
	return false;
	#endif
}

