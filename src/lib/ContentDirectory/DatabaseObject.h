/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            DatabaseObject.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifndef _DATABASEOBJECT_H
#define _DATABASEOBJECT_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "DatabaseConnection.h"
#include "UPnPObjectTypes.h"
#include "FileDetails.h"

namespace fuppes
{



/*
ID
PUBLISHER TEXT, "
DATE TEXT, "
DESCRIPTION TEXT, "
LONG_DESCRIPTION TEXT, "
AV_GENRE TEXT, "
AV_LANGUAGE TEXT, "
AV_ARTIST TEXT, "
AV_ALBUM TEXT, "
AV_CONTRIBUTOR TEXT, "
AV_PRODUCER TEXT, "
A_TRACK_NUMBER UNSIGNED INTEGER, "
V_ACTORS TEXT, "
V_DIRECTOR TEXT, "
V_SERIES_TITLE TEXT, "
V_PROGRAM_TITLE TEXT, "
V_EPISODE_NR UNSIGNED INTEGER, "
V_EPISODE_COUNT UNSIGNED INTEGER, "
SIZE BIGINT DEFAULT 0, "
AV_DURATION INTEGER, "
A_CHANNELS INTEGER, "
A_BITRATE INTEGER, "
A_BITS_PER_SAMPLE INTEGER, "
A_SAMPLERATE INTEGER, "
IV_WIDTH INTEGER, "
IV_HEIGHT INTEGER, "
IV_COLOR_DEPTH INTEGER, "
A_YEAR INTEGER, "
A_COMPOSER TEXT, "
AUDIO_CODEC TEXT, "
VIDEO_CODEC TEXT, "
V_BITRATE INTEGER, "
ALBUM_ART_ID INTEGER, "
ALBUM_ART_EXT TEXT,
MIME_TYPE TEXT
SOURCE          // metadata source (file = the file itself, playlist = from a playlist, itunes = from itunes db)
*/

class DbObject;

class ObjectDetails
{

  friend class DbObject;

  public:
    enum DetailSource {
      Unknown   = 0,
      File      = 1,
      Playlist  = 2,
      itunes    = 3
    };

    ObjectDetails();
    void reset();
    
    ObjectDetails& operator=(const ObjectDetails& details) {

      m_id = 0;
      m_a_trackNumber = details.m_a_trackNumber;
      m_a_samplerate = details.m_a_samplerate;
      m_a_bitrate = details.m_a_bitrate;
      m_av_album = details.m_av_album;
      m_av_artist = details.m_av_artist;
      m_av_genre = details.m_av_genre;
      m_a_composer = details.m_a_composer;
      m_description = details.m_description;
      m_a_codec = details.m_a_codec;
      m_a_channels = details.m_a_channels;
      m_av_duration = details.m_av_duration;
      m_iv_width = details.m_iv_width;
      m_iv_height = details.m_iv_height;
      m_v_bitrate = details.m_v_bitrate;      
      m_v_codec = details.m_v_codec;
      m_albumArtId = details.m_albumArtId;
      m_albumArtExt = details.m_albumArtExt;
      m_size = details.m_size;
      m_source = details.m_source;
      m_streamMimeType = details.m_streamMimeType;

      m_changed = true;
      
	  	return *this;
	  }

    ObjectDetails& operator=(AudioItem& audioItem);
    ObjectDetails& operator=(ImageItem& imageItem);
    ObjectDetails& operator=(VideoItem& videoItem);

    object_id_t   id() { return m_id; }
    int           trackNumber() { return m_a_trackNumber; }
    int           audioSamplerate() { return m_a_samplerate; }
    int           audioBitrate() { return m_a_bitrate; }
    std::string   album() { return m_av_album; }
    std::string   artist() { return m_av_artist; }
    std::string   genre() { return m_av_genre; }
    std::string   composer() { return m_a_composer; }
    std::string   description() { return m_description; }
    std::string   audioCodec() { return m_a_codec; }
    int           audioChannels() { return m_a_channels; }
    unsigned int  durationMs() { return m_av_duration; }
    int           width() { return m_iv_width; }
    int           height() { return m_iv_height; }
    int           videoBitrate() { return m_v_bitrate; }
    std::string   videoCodec() { return m_v_codec; }
    object_id_t   albumArtId() { return m_albumArtId; }
    std::string   albumArtExt() { return m_albumArtExt; }
    fuppes_off_t  size() { return m_size; }
    DetailSource  source() { return m_source; }
    std::string   streamMimeType() { return m_streamMimeType; }   // for audio/video streams


    void setTrackNumber(int trackNumber) {
      if(m_a_trackNumber != trackNumber) {
        m_a_trackNumber = trackNumber;
        m_changed = true;
      }
    }

    void setAudioSamplerate(int samplerate) {
      if(m_a_samplerate != samplerate) {
        m_a_samplerate = samplerate;
        m_changed = true;
      }
    }
    
    void setAudioBitrate(int bitrate) {
      if(m_a_bitrate != bitrate) {
        m_a_bitrate = bitrate;
        m_changed = true;
      }
    }

    void setAlbum(std::string album) {
      if(m_av_album != album) {
        m_av_album = album;
        m_changed = true;
      }
    }

    void setArtist(std::string artist) {
      if(m_av_artist != artist) {
        m_av_artist = artist;
        m_changed = true;
      }
    }

    void setGenre(std::string genre) {
      if(m_av_genre != genre) {
        m_av_genre = genre;
        m_changed = true;
      }
    }

    void setComposer(std::string composer) {
      if(m_a_composer != composer) {
        m_a_composer = composer;
        m_changed = true;
      }
    }

    void setDescription(std::string description) {
      if(m_description != description) {
        m_description = description;
        m_changed = true;
      }
    }

    void setAudioCodec(std::string codec) {
      if(m_a_codec != codec) {
        m_a_codec = codec;
        m_changed = true;
      }
    }

    void setAudioChannels(int channels) {
      if(m_a_channels != channels) {
        m_a_channels = channels;
        m_changed = true;
      }
    }

    void setDurationMs(unsigned int duration) {
      if(m_av_duration != duration) {
        m_av_duration = duration;
        m_changed = true;
      }
    }

    void setWidth(int width) {
      if(m_iv_width != width) {
        m_iv_width = width;
        m_changed = true;
      }
    }

    void setHeight(int height) {
      if(m_iv_height != height) {
        m_iv_height = height;
        m_changed = true;
      }
    }

    void setVideoBitrate(int bitrate) {
      if(m_v_bitrate != bitrate) {
        m_v_bitrate = bitrate;
        m_changed = true;
      }
    }

    void setVideoCodec(std::string codec) {
      if(m_v_codec != codec) {
        m_v_codec = codec;
        m_changed = true;
      }
    }

    void setAlbumArtId(object_id_t albumArtId) {
      if(m_albumArtId != albumArtId) {
        m_albumArtId = albumArtId;
        m_changed = true;
      }
    }

    void setAlbumArtExt(std::string albumArtExt) {
      if(m_albumArtExt != albumArtExt) {
        m_albumArtExt = albumArtExt;
        m_changed = true;
      }
    }
    
    void setSize(fuppes_off_t size) {
      if(m_size != size) {
        m_size = size;
        m_changed = true;
      }
    }

    void setSource(DetailSource source) {
      if(m_source != source) {
        m_source = source;
        m_changed = true;
      }
    }

    void setStreamMimeType(std::string streamMimeType) {
      if(m_streamMimeType != streamMimeType) {
        m_streamMimeType = streamMimeType;
        m_changed = true;
      }
    }

    bool load(object_id_t detailId, SQLQuery* qry = NULL);
    bool save(SQLQuery* qry = NULL);
    
  private:
    object_id_t     m_id;
    int             m_a_trackNumber;
    int             m_a_samplerate;
    int             m_a_bitrate;
    std::string     m_av_album;
    std::string     m_av_artist;
    std::string     m_av_genre;
    std::string     m_a_composer;
    std::string     m_description;
    std::string     m_a_codec;
    int             m_a_channels;
    unsigned int    m_av_duration;
    int             m_iv_width;
    int             m_iv_height;
    int             m_v_bitrate;
    std::string     m_v_codec;
    object_id_t     m_albumArtId;
    std::string     m_albumArtExt;
    fuppes_off_t    m_size;
    DetailSource    m_source;
    std::string     m_streamMimeType;

    std::string     m_publisher;
    std::string     m_date;
    std::string     m_longDescription;
    std::string     m_av_language;
    std::string     m_av_contributor;
    std::string     m_av_producer;
    std::string     m_v_actors;
    std::string     m_v_director;
    std::string     m_v_seriesTitle;
    std::string     m_v_programTitle;
    unsigned int    m_v_episodeNumber;
    unsigned int    m_v_episodeCount;
    unsigned int    m_a_bitsPerSample;
    unsigned int    m_iv_colorDepth;
    int             m_a_year;

    
    bool            m_changed;
};

  

  
/*

ID              :: unique id
OBJECT_ID       :: object id (unique by OBJECT_ID and DEVICE)
PARENT_ID       :: the object's parent id
DETAIL_ID       :: the detail id (references OBJECT_DETAILS.ID)
TYPE            :: the object type
PATH            :: the absolute path to the file (without the filename)
FILE_NAME       :: the filename
TITLE           :: the object's title
MD5             ::
REF_ID          :: id of the referenced object (e.g. a playlist item that points to a object that is also available via normal folders)
VISIBLE         :: object visiblility (0 = false, 1 = true)
MODIFIED_AT     :: last time file was modified (unix timestamp)
UPDATED_AT      :: last time file was checked by fuppes (unix timestamp)

DEVICE          :: the name of the virtual folder layout
VCONTAINER_TYPE :: the type of the virtual container (e.g. genre, album, artist)
VCONTAINER_PATH :: the full virtual path this object is part of ( e.g. folder | genre | artist)
VREF_ID         :: id of the referenced "original" object (the file with DEVICE == NULL and REF_ID == NULL)
 
*/

class DbObject
{
  public:    
    enum VirtualContainerType {
      None      = 0,
      Folder    = 1,
      Split     = 2,
      Filter    = 4,
      Genre     = 8,
      Artist    = 16,
      Composer  = 32,
      Album     = 64
    };
  
    static DbObject* createFromObjectId(object_id_t objectId, SQLQuery* qry = NULL, std::string layout = "");
    static DbObject* createFromFileName(std::string fileName, SQLQuery* qry = NULL, std::string layout = "");
    
    object_id_t             objectId() { return m_objectId; }
    object_id_t             parentId() { return m_parentId; }
    object_id_t             detailId() { return m_detailId; }
    OBJECT_TYPE             type() { return m_type; }
    std::string             path() { return m_path; }
    std::string             fileName() { return m_fileName; }
    std::string             title() { return m_title; }
    std::string             md5() { return m_md5; }
    object_id_t             refId() { return m_refId; }
    std::string             device() { return m_device; }
    VirtualContainerType    vcType() { return m_vcType; }
    std::string             vcPath() { return m_vcPath; }
    bool                    visible() { return m_visible; }
    time_t                  lastModified() { return m_lastModified; }
    time_t                  lastUpdated() { return m_lastUpdated; }

    // set the object id. if not set an object id will be set when saving
    void  setObjectId(object_id_t objectId) { 
      if(m_objectId != objectId) {
        m_objectId = objectId; 
        m_changed = true; 
      }
    }
    void  setParentId(object_id_t parentId) { 
      if(m_parentId != parentId) {
        m_parentId = parentId; 
        m_changed = true; 
      }
    }
    void  setDetailId(object_id_t detailId) { 
      if(m_detailId != detailId) {
        m_detailId = detailId; 
        m_changed = true; 
      }
    }
    void  setType(OBJECT_TYPE type) { 
      if(m_type != type) {
        m_type = type; 
        m_changed = true; 
      }
    }
    void setPath(std::string path) {
      if(m_path != path) {
        m_path = path;
        m_changed = true;
        m_pathChanged = true;
      }
    }
    void setFileName(std::string fileName) {
      if(m_fileName != fileName) {
        m_fileName = fileName;
        m_changed = true;
      }
    }
    void setTitle(std::string title) {
      if(m_title != title) {
        m_title = title;
        m_changed = true;
      }
    }

    void setRefId(object_id_t refId) {
      if(m_refId != refId) {
        m_refId = refId;
        m_changed = true;
      }
    }
    
    void setVisible(bool visible) {
      if(m_visible != visible) {
        m_visible = visible;
        m_changed = true;
      }
    }


    void setDevice(std::string device) {
      if(m_device != device) {
        m_device = device;
        m_changed = true;
      }
    }

    void setVirtualContainerType(VirtualContainerType type) {
      if(m_vcType != type) {
        m_vcType = type;
        m_changed = true;
      }
    }

    void setVirtualContainerPath(std::string path) {
      if(m_vcPath != path) {
        m_vcPath = path;
        m_changed = true;
      }
    }

    void setVirtualRefId(object_id_t vrefId) {
      if(m_vrefId != vrefId) {
        m_vrefId = vrefId;
        m_changed = true;
      }
    }

    void setLastModified(time_t lastModified) {
      if(m_lastModified != lastModified) {
        m_lastModified = lastModified;
        m_changed = true;
        m_lastModifiedChanged = true;
      }
    }
    
    void setUpdated() {
      m_changed = true;
    }


    ObjectDetails*  details() {

      // check if the object has details and load them if necessary
      if(m_detailId != 0 && m_details.m_id == 0) {
        m_details.load(m_detailId);
      }

      return &m_details;
    }
    
    
    DbObject();
    DbObject(DbObject* object);
    DbObject(CSQLResult* result);

    void reset();
    /**
     * if qry is NULL save will use it's own query instance
     * if createReference is false the object is inserted with REF_ID NULL if ref id is not excplicitly set
     * if create reference is true the function checks if there is an object with the same values and REF_ID NULL and references it
     * create reference only works for new objects that will be inserted and not updated
     */
    bool save(SQLQuery* qry = NULL, bool createReference = false);

    bool remove();

    static std::string toString(DbObject* object, bool details = false);
    
  private:
    unsigned int            m_id;
    object_id_t             m_objectId;
    object_id_t             m_parentId;
    object_id_t             m_detailId;
    OBJECT_TYPE             m_type;
    std::string             m_path;
    std::string             m_fileName;
    std::string             m_title;
    std::string             m_md5;
    object_id_t             m_refId;
    std::string             m_device;
    bool                    m_visible;
    VirtualContainerType    m_vcType;
    std::string             m_vcPath;
    object_id_t             m_vrefId;
    time_t                  m_lastModified;
    time_t                  m_lastUpdated;
    
    bool                    m_changed;
    bool                    m_pathChanged;
    std::string             m_oldPath;
    bool                    m_lastModifiedChanged;
    
    ObjectDetails           m_details;
};


}


#endif // _DATABASEOBJECT_H
