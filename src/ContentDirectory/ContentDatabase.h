/***************************************************************************
 *            ContentDatabase.h
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

#ifndef _CONTENTDATABASE_H
#define _CONTENTDATABASE_H

#include <sqlite3.h>

typedef enum tagOBJECT_TYPE
{
  
  ITEM_IMAGE_ITEM = 1,
    ITEM_IMAGE_ITEM_PHOTO = 100,
  
  ITEM_AUDIO_ITEM = 2,
    ITEM_AUDIO_ITEM_MUSIC_TRACK     = 200,
    ITEM_AUDIO_ITEM_AUDIO_BROADCAST = 201,
    ITEM_AUDIO_ITEM_AUDIO_BOOK      = 202,
  
  ITEM_VIDEO_ITEM = 3,
    ITEM_VIDEO_ITEM_MOVIE = 300,
    ITEM_VIDEO_ITEM_VIDEO_BROADCAST = 301,
    ITEM_VIDEO_ITEM_MUSIC_VIDEO_CLIP = 302,  
  
  CONTAINER_PERSON = 4,
    CONTAINER_PERSON_MUSIC_ARTIST = 400,
  
  CONTAINER_PLAYLIST_CONTAINER = 5,
  
  CONATINER_ALBUM = 6,
  
    CONTAINER_ALBUM_MUSIC_ALBUM = 600,
    CONTAINER_ALBUM_PHOTO_ALBUM = 601,
    
  CONTAINER_GENRE = 7,
    CONTAINER_GENRE_MUSIC_GENRE = 700,
    CONTAINER_GENRE_MOVIE_GENRE = 701,
    
  CONTAINER_STORAGE_SYSTEM = 8,
  CONTAINER_STORAGE_VOLUME = 9,  
  CONTAINER_STORAGE_FOLDER = 10  
}OBJECT_TYPE;


class CContentDatabase
{
  public:
    CContentDatabase();
    ~CContentDatabase();
  
  private:
    sqlite3 *m_pDbHandle;    
};

#endif /* _CONTENTDATABASE_H */
