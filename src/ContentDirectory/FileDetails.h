/***************************************************************************
 *            FileDetails.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
  std::string sDecoderName;
  std::string sEncoderName;
};

struct SAudioItem
{ 
  std::string sTitle;
  std::string sGenre;
  std::string sDescription;
  std::string sLongDescription;
  std::string sPublisher;
  std::string sLanguage;
  std::string sRelation;
  std::string sRights;
};

  struct SMusicTrack
  {
    SAudioItem  mAudioItem;
    std::string sArtist;
    std::string sAlbum;
    int         nOriginalTrackNumber;
    std::string sPlaylist;
    std::string sStorageMedium;
    std::string sContributor;
    std::string sDate;
  };

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
    std::string GetMimeType(std::string p_sFileName, bool p_bTranscodingMimeType);
  
    SMusicTrack GetMusicTrackDetails(std::string p_sFileName, std::string* p_sResult);
  
    static bool IsTranscodingExtension(std::string p_sExt);
    static std::string GetTargetMimeType(std::string p_sExt);
    static std::string GetTargetExtension(std::string p_sExt);
  
  private:
    static CFileDetails* m_Instance; 
    
};

#endif /* _FILEDETAILS_H */
