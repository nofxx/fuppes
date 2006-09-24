/***************************************************************************
 *            FileDetails.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "../Common.h"
#include "../SharedConfig.h"
/* taglib
#include <fileref.h>
#include <tstring.h>
#include <tag.h>*/
#include <sstream>

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
  
  if((sExt.compare("mp3") == 0) || (sExt.compare("ogg") == 0) ||
     (sExt.compare("mpc") == 0) || (sExt.compare("flac") == 0))
    return ITEM_AUDIO_ITEM_MUSIC_TRACK;

  else if((sExt.compare("jpeg") == 0) || (sExt.compare("jpg") == 0) ||
          (sExt.compare("bmp") == 0) || (sExt.compare("png") == 0) ||
          (sExt.compare("gif") == 0))
    return ITEM_IMAGE_ITEM_PHOTO;
  
  else if((sExt.compare("mpeg") == 0) || (sExt.compare("mpg") == 0))
    return ITEM_VIDEO_ITEM_MOVIE;
  
  else if((sExt.compare("avi") == 0))
    return ITEM_VIDEO_ITEM_MOVIE;
  
  else if((sExt.compare("wmv") == 0))
    return ITEM_VIDEO_ITEM_MOVIE;

  else if((sExt.compare("vdr") == 0))
    return ITEM_VIDEO_ITEM_MOVIE;    
  else if((sExt.compare("vob") == 0))
    return ITEM_VIDEO_ITEM_MOVIE; 
  
  /*else if((sExt.compare("rm") == 0))
    return ITEM_VIDEO_ITEM_MOVIE;*/
  
  else  
    return OBJECT_TYPE_UNKNOWN;  
}

std::string CFileDetails::GetMimeType(std::string p_sFileName)
{
  string sExt = ToLower(ExtractFileExt(p_sFileName));

  /* audio types */
  if (sExt.compare("mp3") == 0)
    return MIME_TYPE_AUDIO_MPEG;  
  else if((sExt.compare("ogg") == 0) || (sExt.compare("mpc") == 0) || (sExt.compare("flac") == 0))
  {
    if(CSharedConfig::Shared()->IsTranscodingExtension(sExt))
      return MIME_TYPE_AUDIO_MPEG;
    else
    {
      if((sExt.compare("ogg") == 0) || (sExt.compare("mpc") == 0))
        return MIME_TYPE_APPLICATION_OCTETSTREAM;
      else if(sExt.compare("flac") == 0)
        return MIME_TYPE_AUDIO_X_FLAC;
    }
  }
  
  /* image types */
  else if((sExt.compare("jpeg") == 0) || (sExt.compare("jpg") == 0))
    return MIME_TYPE_IMAGE_JPEG;
  else if(sExt.compare("bmp") == 0)
    return MIME_TYPE_IMAGE_BMP;
  else if(sExt.compare("png") == 0)
    return MIME_TYPE_IMAGE_PNG; 
  else if(sExt.compare("gif") == 0)
    return MIME_TYPE_IMAGE_GIF;
  
  /* video types */
  else if((sExt.compare("mpeg") == 0) || (sExt.compare("mpg") == 0))
    return MIME_TYPE_VIDEO_MPEG;
  else if(sExt.compare("avi") == 0)
    return MIME_TYPE_VIDEO_X_MSVIDEO;
  else if(sExt.compare("wmv") == 0)
    return MIME_TYPE_VIDEO_X_MS_WMV;  
  /*else if(sExt.compare("rm") == 0)
    return MIME_TYPE_AUDIO_X_PN_REALAUDIO;*/
  else if(sExt.compare("vdr") == 0)
    return MIME_TYPE_VIDEOS_X_MS_VOB;
  else if(sExt.compare("vob") == 0)
    return MIME_TYPE_VIDEOS_X_MS_VOB;  
  
  return "unknown";  
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
    /*ITEM_AUDIO_ITEM_AUDIO_BROADCAST = 201,
    ITEM_AUDIO_ITEM_AUDIO_BOOK      = 202,*/
  
    case ITEM_VIDEO_ITEM :
      return "videoItem";
    case ITEM_VIDEO_ITEM_MOVIE :
      return "videoItem.movie";
    case ITEM_VIDEO_ITEM_VIDEO_BROADCAST :
      return "videoItem.videoBroadcast";
    //ITEM_VIDEO_ITEM_MUSIC_VIDEO_CLIP = 302,  
  
    /*CONTAINER_PERSON = 4,
      CONTAINER_PERSON_MUSIC_ARTIST = 400,
    
    CONTAINER_PLAYLIST_CONTAINER = 5,
    
    CONTAINER_ALBUM = 6,
    
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

/*SMusicTrack CFileDetails::GetMusicTrackDetails(std::string p_sFileName)
{
  cout << "GetMusicTrackDetails" << endl;
  
  SMusicTrack Result;
  
  TagLib::FileRef pFile(p_sFileName.c_str());
  
  TagLib::String sTmp = pFile.tag()->title();
  cout << "Title: " << sTmp.to8Bit() << endl;
  Result.mAudioItem.sTitle = sTmp.to8Bit();  
  
  sTmp = pFile.tag()->artist();
  cout << "Artist: " << sTmp.to8Bit() << endl;
  Result.sArtist = sTmp.to8Bit();  
  
  sTmp = pFile.tag()->album();
  cout << "Album: " << sTmp.to8Bit() << endl;
  Result.sAlbum = sTmp.to8Bit();  
  
  sTmp = pFile.tag()->comment();
  cout << "Comment: " << sTmp.to8Bit() << endl;
  Result.mAudioItem.sLongDescription = sTmp.to8Bit();    
  
  sTmp = pFile.tag()->genre();
  cout << "Genre: " << sTmp.to8Bit() << endl;
  Result.mAudioItem.sGenre = sTmp.to8Bit();      

  uint nTmp = pFile.tag()->year();
  cout << "Year: " << nTmp << endl;
  stringstream sDate;
  sDate << nTmp;
  Result.sDate = sDate.str();
  
  nTmp = pFile.tag()->track();
  cout << "Track: " << nTmp << endl;  
  Result.nOriginalTrackNumber = nTmp;
  
  return Result;
}*/
