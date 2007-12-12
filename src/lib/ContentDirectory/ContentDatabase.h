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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
#ifndef _CONTENTDATABASE_H
#define _CONTENTDATABASE_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <sqlite3.h>
#include <string>
#include <map>
#include <list>
#include "../Common/Common.h"
#include "FileAlterationMonitor.h"

typedef enum tagOBJECT_TYPE
{
  OBJECT_TYPE_UNKNOWN = 0,  
  //ITEM = 1,
  
  
  CONTAINER_STORAGE_FOLDER = 1,
  
  //CONTAINER_PERSON = 10,
    CONTAINER_PERSON_MUSIC_ARTIST = 11,
    
  /* "object.container.playlistContainer" and "object.item.playlistItem”
     have the same OBJECT_TYPE in the database though the type of representation
     is selected from the configuration at runtime */
  CONTAINER_PLAYLIST_CONTAINER = 20,
  
  CONTAINER_ALBUM = 30,
    CONTAINER_ALBUM_MUSIC_ALBUM = 31,
    CONTAINER_ALBUM_PHOTO_ALBUM = 32,
    
  CONTAINER_GENRE = 40,
    CONTAINER_GENRE_MUSIC_GENRE = 41,
    //CONTAINER_GENRE_MOVIE_GENRE = 42,
    
  /*CONTAINER_CHANNEL_GROUP = 50,
    CONTAINER_CHANNEL_GROUP_AUDIO_CHANNEL_GROUP = 51,
    CONTAINER_CHANNEL_GROUP_VIDEO_CHANNEL_GROUP = 52,*/
  
  //CONTAINER_EPG_CONTAINER = 60,
  
  /*CONTAINER_STORAGE_SYSTEM = 70,
  CONTAINER_STORAGE_VOLUME = 80,
  CONTAINER_BOOKMARK_FOLDER = 90*/  
  
  
  ITEM = 100,  
  
  ITEM_IMAGE_ITEM = 110,
    ITEM_IMAGE_ITEM_PHOTO = 111,
  
  ITEM_AUDIO_ITEM = 120,
    ITEM_AUDIO_ITEM_MUSIC_TRACK     = 121,
    ITEM_AUDIO_ITEM_AUDIO_BROADCAST = 122,
    //ITEM_AUDIO_ITEM_AUDIO_BOOK      = 123,
  
  ITEM_VIDEO_ITEM = 130,
    ITEM_VIDEO_ITEM_MOVIE            = 131,
    ITEM_VIDEO_ITEM_VIDEO_BROADCAST  = 132,
    //ITEM_VIDEO_ITEM_MUSIC_VIDEO_CLIP = 133,
    
  ITEM_TEXT_ITEM = 140//,
  /*ITEM_BOOKMARK_ITEM = 150,
  
  ITEM_EPG_ITEM = 160,
    ITEM_EPG_ITEM_AUDIO_PROGRAM = 161,
    ITEM_EPG_ITEM_VIDEO_PROGRAM = 162,*/     

  
}OBJECT_TYPE;

class CSelectResult
{
  public:
    std::string  GetValue(std::string p_sFieldName);
    bool         IsNull(std::string p_sFieldName);
    unsigned int GetValueAsUInt(std::string p_sFieldName);
  
  //private:
    std::map<std::string, std::string> m_FieldValues;
    std::map<std::string, std::string>::iterator m_FieldValuesIterator;  
};

class CContentDatabase: public IFileAlterationMonitor
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


    void BeginTransaction();
    void Commit();    
    
    void RebuildDB();
    void UpdateDB();
    void AddNew();
    void RemoveMissing();
    bool IsRebuilding();// { return m_bIsRebuilding; };
	
    unsigned int GetObjId();
  
		fuppesThreadMutex m_Mutex;
  
    void FamEvent(FAM_EVENT_TYPE p_nEventType, std::string p_sPath);
  private:    
    void BuildDB();
    
		void Lock();
    void Unlock();
    void ClearResult();
  
	  fuppesThread m_RebuildThread;
	
    //bool m_bIsRebuilding;
		bool m_bShared;
    static CContentDatabase* m_Instance;
    int m_nLockCount;
  
    CFileAlterationMonitor* m_pFileAlterationMonitor;
  
		sqlite3*      m_pDbHandle;  
    std::string   m_sDbFileName;
    bool          m_bInTransaction;
    bool Open();
    void Close();
};

#endif // _CONTENTDATABASE_H
