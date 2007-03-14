/***************************************************************************
 *            FileDetails.h
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
 
#ifndef _FILEDETAILS_H
#define _FILEDETAILS_H

#include "ContentDatabase.h"
#include <string>

struct FileType_t
{
  std::string sExt;
  OBJECT_TYPE nObjectType;
  std::string sMimeType;
};

struct TranscodingSetting_t
{
  std::string sExt;  
  std::string sTargetExt;  
  std::string sTargetMimeType;
  OBJECT_TYPE nTargetObjectType;  
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
	
	int nNrAudioChannels;
	int nBitrate;
	int nSampleRate;
};

  struct SMusicTrack
  {
    SAudioItem  mAudioItem;
    std::string sArtist;
    std::string sAlbum;
    int         nOriginalTrackNumber;
    /*std::string sPlaylist;
    std::string sStorageMedium;
    std::string sContributor;*/
    std::string sDate;
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
	int nHeight;
	int nWidth;
	unsigned int nSize;
	int nBitrate;
};
  
  struct sMovie
  {
    std::string sStorageMedium;
    int         nDVDRegionCode;
    std::string sChannelName;
    std::string sScheduledStartTime;
    std::string sScheduledEndTime;
  };

class CFileDetails
{
  protected:
    CFileDetails();
  
  public:
    static CFileDetails* Shared();
    OBJECT_TYPE GetObjectType(std::string p_sFileName);
    std::string GetObjectTypeAsString(unsigned int p_nObjectType);
    static std::string GetMimeType(std::string p_sFileName, bool p_bTranscodingMimeType);
  
    bool GetMusicTrackDetails(std::string p_sFileName, SMusicTrack* pMusicTrack);
    bool GetImageDetails(std::string p_sFileName, SImageItem* pImageItem);
	  bool GetVideoDetails(std::string p_sFileName, SVideoItem* pVideoItem);
	
    bool IsSupportedFileExtension(std::string p_sFileExtension);
    static bool IsTranscodingExtension(std::string p_sExt);
    static std::string GetTargetExtension(std::string p_sExt);
  
  private:
    static CFileDetails* m_Instance; 
    
};

#endif /* _FILEDETAILS_H */
