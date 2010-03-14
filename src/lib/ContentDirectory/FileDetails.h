/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            FileDetails.h
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
 
#ifndef _FILEDETAILS_H
#define _FILEDETAILS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../../../include/fuppes_plugin_types.h"
#include "../Common/Common.h"
#include "UPnPObjectTypes.h"
#include <string>

struct FileType_t
{
  std::string sExt;
  OBJECT_TYPE nObjectType;
  std::string sMimeType;
  std::string sDLNA;
};

struct SAudioItem
{ 
  std::string sTitle;
  std::string sGenre;
	std::string sDuration;
  std::string sDescription;
  /*std::string sLongDescription;
  std::string sPublisher;
  std::string sLanguage;
  std::string sRelation;
  std::string sRights;*/
	
  std::string sArtist;
  std::string sAlbum;
  int         nOriginalTrackNumber;
  std::string sOriginalTrackNumber;
  int         nYear;
  /*std::string sPlaylist;
  std::string sStorageMedium;
  std::string sContributor;*/
  std::string sDate;
  
  std::string sACodec;
  std::string sVCodec;
  
	int nNrAudioChannels;
	int nBitrate;
	int nSampleRate;
  int nBitsPerSample;
  
  fuppes_off_t nSize;
};


struct SImageItem {
	std::string sDescription;
  std::string sLongDescription;
	std::string sPublisher;
	std::string sRating;
	std::string sRights;
	std::string sStorageMedium;
	std::string sDate;
	
	unsigned int nHeight;
	unsigned int nWidth;
};

  /*struct SPhoto {
    std::string sAlbum;
  };*/
/*upnp:longDescription upnp O
  upnp:storageMedium upnp O
  upnp:rating upnp O
  dc:description dc O
  dc:publisher dc O
  dc:date dc O
  dc:rights dc O
	
	upnp:album upnp O*/

struct SVideoItem
{
  std::string sGenre;
  std::string sDescription;
  std::string sLongDescription;
  std::string sProducer;
  std::string sRating;
  std::string sActor;
  std::string sDirector;  
  std::string sPublisher;
  std::string sLanguage;
  std::string sRelation;
	
	std::string  sDuration;
	std::string  sVCodec;
  std::string  sACodec;
  
  int   nHeight;
	int   nWidth;	
	int   nBitrate;
  
  fuppes_off_t nSize;
};
  
  /*struct sMovie
  {
    std::string sStorageMedium;
    int         nDVDRegionCode;
    std::string sChannelName;
    std::string sScheduledStartTime;
    std::string sScheduledEndTime;
  };*/

class CFileDetails
{
  protected:
    CFileDetails();
  
  public:
		~CFileDetails();
				
    static CFileDetails* Shared();
		static void deleteInstance();
		
    OBJECT_TYPE GetObjectType(std::string p_sFileName);    
    std::string GetObjectTypeAsStr(OBJECT_TYPE p_nObjectType);
    std::string GetContainerTypeAsStr(OBJECT_TYPE p_nContainerType);
  
    bool getMusicTrackDetails(std::string p_sFileName, metadata_t* metadata);
    bool GetImageDetails(std::string p_sFileName, SImageItem* pImageItem);
	  bool GetVideoDetails(std::string p_sFileName, SVideoItem* pVideoItem);
	
    bool IsSupportedFileExtension(std::string p_sFileExtension);

  private:
    static CFileDetails* m_Instance; 
};

#endif // _FILEDETAILS_H
