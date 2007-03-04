/***************************************************************************
 *            ContentDatabase.h
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

#ifndef _CONTENTDATABASE_H
#define _CONTENTDATABASE_H

#include <sqlite3.h>
#include <string>
#include <map>
#include <list>
#include "../Common/Common.h"

typedef enum tagOBJECT_TYPE
{
  OBJECT_TYPE_UNKNOWN = 0,
  
  ITEM_IMAGE_ITEM = 1,
    ITEM_IMAGE_ITEM_PHOTO = 100,
  
  ITEM_AUDIO_ITEM = 2,
    ITEM_AUDIO_ITEM_MUSIC_TRACK     = 200,
    ITEM_AUDIO_ITEM_AUDIO_BROADCAST = 201,
    //ITEM_AUDIO_ITEM_AUDIO_BOOK      = 202,
  
  ITEM_VIDEO_ITEM = 3,
    ITEM_VIDEO_ITEM_MOVIE            = 300,
    ITEM_VIDEO_ITEM_VIDEO_BROADCAST  = 301,
    //ITEM_VIDEO_ITEM_MUSIC_VIDEO_CLIP = 302,  
  
  //CONTAINER_PERSON = 4,
    //CONTAINER_PERSON_MUSIC_ARTIST = 400,
  
  /* "object.container.playlistContainer" and "object.item.playlistItem”
     have the same OBJECT_TYPE in the database though the type of representation
     is selected from the configuration at runtime */
  CONTAINER_PLAYLIST_CONTAINER = 5,  
  
  //CONTAINER_ALBUM = 6,
    //CONTAINER_ALBUM_MUSIC_ALBUM = 600,
    //CONTAINER_ALBUM_PHOTO_ALBUM = 601,
    
  //CONTAINER_GENRE = 7,
    //CONTAINER_GENRE_MUSIC_GENRE = 700,
    //CONTAINER_GENRE_MOVIE_GENRE = 701,
    
  //CONTAINER_STORAGE_SYSTEM = 8,
  //CONTAINER_STORAGE_VOLUME = 9,  
  CONTAINER_STORAGE_FOLDER = 10  
}OBJECT_TYPE;

class CSelectResult
{
  public:
    std::string  GetValue(std::string p_sFieldName);
    bool         IsNull(std::string p_sFieldName);
  
  //private:
    std::map<std::string, std::string> m_FieldValues;
    std::map<std::string, std::string>::iterator m_FieldValuesIterator;  
};

class CContentDatabase
{
  public:
    static CContentDatabase* Shared();  
  
    CContentDatabase(bool p_bShared = false);
    ~CContentDatabase();
  
    std::string GetLibVersion();

    bool Init(bool* p_bIsNewDB);
  
    unsigned int Insert(std::string p_sStatement);
    bool Execute(std::string p_sStatement);
    bool Select(std::string p_sStatement);
  
    bool Eof();
    CSelectResult* GetResult();
    void Next();
  
    std::list<CSelectResult*> m_ResultList;
    std::list<CSelectResult*>::iterator m_ResultListIterator;
    unsigned int  m_nRowsReturned;  
  
    void ClearResult();
    void Lock();
    void Unlock();
  
    void BuildDB();
    bool IsRebuilding() { return m_bIsRebuilding; };
  
		fuppesThreadMutex m_Mutex;
  private:    
    void DbScanDir(std::string p_sDirectory, long long int p_nParentId);
    void BuildPlaylists();
    void ParsePlaylist(CSelectResult* pResult);
    void ParseM3UPlaylist(CSelectResult* pResult);
    void ParsePLSPlaylist(CSelectResult* pResult);
  
    bool m_bIsRebuilding;
  
    static CContentDatabase* m_Instance;
		sqlite3*                 m_pDbHandle;  
    std::string              m_sDbFileName;    
    bool Open();
    void Close();
};

#endif // _CONTENTDATABASE_H
