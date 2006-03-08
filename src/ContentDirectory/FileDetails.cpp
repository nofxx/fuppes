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

  else if((sExt.compare("jpeg") == 0) || (sExt.compare("jpg") == 0))
    return ITEM_IMAGE_ITEM_PHOTO;
  
  else if((sExt.compare("mpeg") == 0) || (sExt.compare("mpg") == 0))
    return ITEM_VIDEO_ITEM_MOVIE;
  
  else if((sExt.compare("avi") == 0))
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
      return MIME_TYPE_APPLICATION_OCTETSTREAM;
  }
  
  /* image types */
  else if((sExt.compare("jpeg") == 0) || (sExt.compare("jpg") == 0))
    return MIME_TYPE_IMAGE_JPEG;
  else if(sExt.compare("bmp") == 0)
    return MIME_TYPE_IMAGE_BMP;
  else if(sExt.compare("png") == 0)
    return MIME_TYPE_IMAGE_PNG;      
  
  /* video types */
  else if((sExt.compare("mpeg") == 0) || (sExt.compare("mpg") == 0))
    return MIME_TYPE_VIDEO_MPEG;
  else if(sExt.compare("avi") == 0)
    return MIME_TYPE_VIDEO_X_MSVIDEO;
  /*else if(sExt.compare("rm") == 0)
    return MIME_TYPE_AUDIO_X_PN_REALAUDIO;*/
  
  return "unknown";  
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
