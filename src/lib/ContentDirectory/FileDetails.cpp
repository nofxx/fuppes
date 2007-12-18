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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "FileDetails.h"

#include "../Common/Common.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Transcoding/TranscodingMgr.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"

#ifdef HAVE_TAGLIB
#include <fileref.h>
#include <tfile.h>
#include <tag.h>
#endif

#ifdef HAVE_IMAGEMAGICK
//#include <Magick++.h>
#include <wand/magick-wand.h>
#endif

#ifdef HAVE_LIBAVFORMAT
extern "C"
{ 
  #include <avformat.h>
  #include <avcodec.h>
}
#endif

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

CFileDetails::CFileDetails()
{
	#ifdef DLNA_SUPPORT
	m_dlna = dlna_init();
  dlna_register_all_media_profiles(m_dlna);
	#endif
}

CFileDetails::~CFileDetails()
{
	#ifdef DLNA_SUPPORT
	dlna_uninit(m_dlna);
	#endif
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


bool CFileDetails::GetMusicTrackDetails(std::string p_sFileName, SAudioItem* pMusicTrack)
{
  #ifdef HAVE_TAGLIB  
  string sExt = ExtractFileExt(p_sFileName);  
  if(!CDeviceIdentificationMgr::Shared()->DefaultDevice()->FileSettings(sExt)->ExtractMetadata())
    return false;  
  
  TagLib::FileRef pFile(p_sFileName.c_str());
  
	if (pFile.isNull()) {
	  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "taglib error");
		return false;
	}
	
	TagLib::String sTmp;
	unsigned int nTmp;
	
	// title
	sTmp = pFile.tag()->title();
  pMusicTrack->sTitle = TrimWhiteSpace(sTmp.to8Bit(true));  
  
	// duration	
	long length = pFile.audioProperties()->length();  
  int hours, mins, secs;
    
  secs  = length % 60;
  length /= 60;
  mins  = length % 60;
  hours = length / 60;  

  char szDuration[12];
	sprintf(szDuration, "%02d:%02d:%02d.00", hours, mins, secs);
	szDuration[11] = '\0';  
  pMusicTrack->sDuration = szDuration;  
	
	// channels
	pMusicTrack->nNrAudioChannels = pFile.audioProperties()->channels();
	
	// bitrate
	pMusicTrack->nBitrate = (pFile.audioProperties()->bitrate() * 1024);
	
  pMusicTrack->nBitsPerSample = 0;
  
	// samplerate
	pMusicTrack->nSampleRate = pFile.audioProperties()->sampleRate();
	
  // size
  pMusicTrack->nSize = pFile.file()->length();
     
	// artist
  sTmp = pFile.tag()->artist();
  pMusicTrack->sArtist = TrimWhiteSpace(sTmp.to8Bit(true));  
  if(pMusicTrack->sArtist.empty()) {
    pMusicTrack->sArtist = "unknown";
  }   
    
  // album
  sTmp = pFile.tag()->album();
  pMusicTrack->sAlbum = TrimWhiteSpace(sTmp.to8Bit(true));  
  if(pMusicTrack->sAlbum.empty()) {
    pMusicTrack->sAlbum = "unknown";
  }  
  
  // genre
	sTmp = pFile.tag()->genre();
  pMusicTrack->sGenre = TrimWhiteSpace(sTmp.to8Bit(true));   
  if(pMusicTrack->sGenre.empty()) {
    pMusicTrack->sGenre = "unknown";
  }

  // description/comment
	sTmp = pFile.tag()->comment();
	pMusicTrack->sDescription = TrimWhiteSpace(sTmp.to8Bit(true));

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
  string sExt = ExtractFileExt(p_sFileName);  
  if(!CDeviceIdentificationMgr::Shared()->DefaultDevice()->FileSettings(sExt)->ExtractMetadata())
    return false;
  
	MagickBooleanType status;
  MagickWand* magick_wand;
  
  magick_wand = NewMagickWand();	
  status = MagickReadImage(magick_wand, p_sFileName.c_str());	
	
	if (status == MagickFalse) {
	
		ExceptionType severity; 
		char* description;
		description = MagickGetException(magick_wand, &severity);
		(void)fprintf(stderr,"%s %s %lu %s\n", GetMagickModule(), description);
		description = (char*)MagickRelinquishMemory(description);	
	
		magick_wand = DestroyMagickWand(magick_wand);
		return false;
	}
		
	pImageItem->nWidth  = MagickGetImageWidth(magick_wand);
	pImageItem->nHeight = MagickGetImageHeight(magick_wand);
	
	magick_wand = DestroyMagickWand(magick_wand);
	return true;
	
	/*Magick::Image image;	
  try {		
    image.read(p_sFileName);
  }
	catch(...) {  //Magick::Exception &ex) {
		cout << __FILE__ << " " << __LINE__ << " :: ex" << endl << endl;
    return false;
	}*/
	
  /*catch(Magick::WarningCorruptImage &ex) {
    cout << "WARNING: image \"" << p_sFileName << "\" corrupt" << endl;
    cout << ex.what() << endl << endl;
    return false;
  }
  catch(exception &ex) {
    cout << __FILE__ << " " << __LINE__ << " :: " << ex.what() << endl << endl;
    return false;
  }*/
  	
  /*pImageItem->nWidth  = image.baseColumns();
  pImageItem->nHeight = image.baseRows();*/
	
	return true;
	#else
	return false;
	#endif
}

bool CFileDetails::GetVideoDetails(std::string p_sFileName, SVideoItem* pVideoItem)
{
  #ifdef HAVE_LIBAVFORMAT
	string sExt = ExtractFileExt(p_sFileName);  
  if(!CDeviceIdentificationMgr::Shared()->DefaultDevice()->FileSettings(sExt)->ExtractMetadata())
    return false;  
  
	AVFormatContext *pFormatCtx;
	
	if(av_open_input_file(&pFormatCtx, p_sFileName.c_str(), NULL, 0, NULL) != 0) {
	  cout << "error open av file: " << p_sFileName << endl;
		return false;
	}
		
	if(av_find_stream_info(pFormatCtx) < 0) {
	  cout << "error reading stream info: " << p_sFileName << endl;
		av_close_input_file(pFormatCtx);
	  return false;
	}
		
	// duration
	if(pFormatCtx->duration != AV_NOPTS_VALUE) {
  	//cout << pFormatCtx->duration << endl;
 
	  int hours, mins, secs, us;
		secs = pFormatCtx->duration / AV_TIME_BASE;
		us   = pFormatCtx->duration % AV_TIME_BASE;
		mins = secs / 60;
		secs %= 60;
		hours = mins / 60;
		mins %= 60;
	
		char szDuration[12];
	  sprintf(szDuration, "%02d:%02d:%02d.%02d", hours, mins, secs, (10 * us) / AV_TIME_BASE);
	  szDuration[11] = '\0';
		
	  pVideoItem->sDuration = szDuration;
	}
	else {
	  pVideoItem->sDuration = "NULL";
	}
	
	// bitrate
	if(pFormatCtx->bit_rate)
  	pVideoItem->nBitrate = pFormatCtx->bit_rate / 8;
	else
	  pVideoItem->nBitrate = 0;

  // filesize  
	pVideoItem->nSize = pFormatCtx->file_size;	
  
	char* codec_name;
	char buf1[32];
	
	for(int i = 0; i < pFormatCtx->nb_streams; i++) 
  {
	  AVStream* pStream = pFormatCtx->streams[i];
	  AVCodec* pCodec;
				
		pCodec = avcodec_find_decoder(pStream->codec->codec_id);
		if(pCodec)
		{			
			codec_name = (char*)pCodec->name;
         if (pStream->codec->codec_id == CODEC_ID_MP3) {
             if (pStream->codec->sub_id == 2)
                 codec_name = "mp2";
             else if (pStream->codec->sub_id == 1)
                 codec_name = "mp1";
         }
     } else if (pStream->codec->codec_id == CODEC_ID_MPEG2TS) {
         // fake mpeg2 transport stream codec (currently not registered) 
         codec_name = "mpeg2ts";
     } else if (pStream->codec->codec_name[0] != '\0') {
         codec_name = pStream->codec->codec_name;
     } else {
         // output avi tags 
         if(   isprint(pStream->codec->codec_tag&0xFF) && isprint((pStream->codec->codec_tag>>8)&0xFF)
            && isprint((pStream->codec->codec_tag>>16)&0xFF) && isprint((pStream->codec->codec_tag>>24)&0xFF)){
             snprintf(buf1, sizeof(buf1), "%c%c%c%c / 0x%04X",
                      pStream->codec->codec_tag & 0xff,
                      (pStream->codec->codec_tag >> 8) & 0xff,
                      (pStream->codec->codec_tag >> 16) & 0xff,
                      (pStream->codec->codec_tag >> 24) & 0xff,
                       pStream->codec->codec_tag);
         } else {
             snprintf(buf1, sizeof(buf1), "0x%04x", pStream->codec->codec_tag);
         }
         codec_name = buf1;
     }		
		
		
			switch(pStream->codec->codec_type)
			{
			  case CODEC_TYPE_VIDEO:
					pVideoItem->nWidth  = pStream->codec->width;
					pVideoItem->nHeight = pStream->codec->height;        
          pVideoItem->sVCodec = codec_name;
					break;
			  case CODEC_TYPE_AUDIO:
          pVideoItem->sACodec = codec_name;
          
					break;
				case CODEC_TYPE_DATA:
					break;
				case CODEC_TYPE_SUBTITLE:
					break;
				default:
					break;
			}
		
	}
		
	av_close_input_file(pFormatCtx);
	
	return true;
	#else
	return false;
	#endif
}

std::string CFileDetails::GuessDLNAProfileId(std::string p_sFileName)
{
  #ifdef DLNA_SUPPORT
  string sResult;
  dlna_profile_t *p;
  p = dlna_guess_media_profile(m_dlna, p_sFileName.c_str());
  if(p) {
    sResult = p->id;
  }  
  return sResult;
  #else
  return "";
  #endif
}

/*std::string CFileDetails::GetDLNAString(std::string p_sFileName)
{		
	string sResult = "";	
	dlna_profile_t *p;
  dlna_org_flags_t flags;

  flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
    DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
    DLNA_ORG_FLAG_CONNECTION_STALL |
    DLNA_ORG_FLAG_DLNA_V15;		
		
	p = dlna_guess_media_profile(dlna, p_sFileName.c_str());
  if(p) {
    char *protocol_info;
  */  
    /*printf ("ID: %s\n", p->id);
    printf ("MIME: %s\n", p->mime);
    printf ("Label: %s\n", p->label);
    printf ("Class: %d\n", p->class);
    printf ("UPnP Object Item: %s\n", dlna_profile_upnp_object_item(p));*/
/*
    protocol_info = dlna_write_protocol_info(DLNA_PROTOCOL_INFO_TYPE_HTTP,
                                             DLNA_ORG_PLAY_SPEED_NORMAL,
                                             DLNA_ORG_CONVERSION_NONE,
                                             DLNA_ORG_OPERATION_RANGE,
                                             flags, p);
    
		//printf ("Protocol Info: %s\n", protocol_info);
		sResult = protocol_info;
    free (protocol_info);
  }
			
	return sResult;
	#endif
}*/
