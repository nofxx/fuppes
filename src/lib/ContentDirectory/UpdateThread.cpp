/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            UpdateThread.cpp
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

#include "UpdateThread.h"
#include "FileDetails.h"
#include "ContentDatabase.h"
#include "VirtualContainerMgr.h"
#include "../Plugins/Plugin.h"
#include "../SharedConfig.h"

#include "../../../include/fuppes_plugin.h"

using namespace fuppes;

#include <iostream>
using namespace std;



UpdateThread::UpdateThread(FileAlterationHandler* famHandler)
:Thread("UpdateThread")
{
  m_famHandler = famHandler;
}

UpdateThread::~UpdateThread()
{
  close();
}


/*

1. check for "normal" files to update (DEVICE == NULL && REF_ID == 0)
2. check for "referenced" files to update (DEVICE == NULL && REF_ID > 0)
   if we find metadata in the playlist we add this else we set the DETAIL_ID to the same as the referenced object (REF_ID)
3. check "normal" dirs for update (DEVICE == NULL)
   - check for album art
   - set container type to e.g. music.Album if it contains only audio items
 
*/



void UpdateThread::run()
{
  cout << "start update thread" << endl;

  msleep(1000);

  CDatabaseConnection* connection = CDatabase::connection(true);
  
  SQLQuery qry(connection);
  SQLQuery ins(connection);
  SQLQuery get(connection);
  DbObject* obj;
  stringstream sql;
  int count = 0;
  int sleep = 0;
  while(!stopRequested()) {

    count = 0;

    // check if a fam event occured recently
    int diff = DateTime::now().toInt() - m_famHandler->lastEventTime().toInt();
    if(diff < 5) {
      msleep(500);
      continue;
    }


    // update metadata
    sql.str("");
    sql << "select * from OBJECTS where TYPE > " << ITEM << " and (UPDATED_AT is NULL or UPDATED_AT < MODIFIED_AT) and DEVICE is NULL and REF_ID = 0";
    qry.select(sql.str());
    if(qry.eof()) {
      if(sleep < 4000)
        sleep += 500;
    }
    else {
      sleep = 500;
      //ins.connection()->startTransaction();
    }
    
    while(!qry.eof() && !stopRequested()) {


      // check if a fam event occured recently
      int diff = DateTime::now().toInt() - m_famHandler->lastEventTime().toInt();
      if(diff < 5) {
        //ins.connection()->commit();
        break;
      }
      

      
      count++;
      obj = new DbObject(qry.result());
      
      cout << "update object " << count << " of " << qry.size() << " :: " << obj->fileName() << endl;
      //cout << "OBJ: " << obj->type() << " :: " << qry.result()->asInt("TYPE") << endl;
      
      // container
      if(obj->type() < ITEM) {

      } // container
      else {

        switch(obj->type()) {

          case ITEM_IMAGE_ITEM:
          case ITEM_IMAGE_ITEM_PHOTO:
            updateImageFile(obj, &ins);
            break;

          case ITEM_AUDIO_ITEM:
          case ITEM_AUDIO_ITEM_MUSIC_TRACK: {
              bool update = (obj->detailId() > 0);
              ObjectDetails oldDetails = *obj->details();
              updateAudioFile(obj, &ins);
              if(!update)
                VirtualContainerMgr::insertAudioFile(obj);
              else
                VirtualContainerMgr::updateAudioFile(obj, &oldDetails);
            }
            break;
          case ITEM_AUDIO_ITEM_AUDIO_BROADCAST:
            break;

          case ITEM_VIDEO_ITEM:
          case ITEM_VIDEO_ITEM_MOVIE:
            updateVideoFile(obj, &ins);
            break;
          case ITEM_VIDEO_ITEM_VIDEO_BROADCAST:
            break;

          default:
            break;          
        }

      } // item



      // vfolder updates

      /*if(count % 1000 == 0) {
        ins.connection()->commit();
        ins.connection()->startTransaction();
      }*/
        

      
      delete obj;
      qry.next();
      msleep(1);
    } // while !eof (update metadata)
    //ins.connection()->commit();



    
    // check for album art and update dirs/files
    count = 0;
    sql.str("");
    DbObject* parent;
    DbObject* sibling;
    object_id_t lastPid = 0;
    
    // get all visible images that have a certain name
    sql << 
      "select * from OBJECTS where TYPE >= " << ITEM_IMAGE_ITEM << " and TYPE < " << ITEM_IMAGE_ITEM_MAX << " and " <<
      "DEVICE is NULL and REF_ID = 0 and VISIBLE = 1 and FILE_NAME in ( " <<
      CSharedConfig::getAlbumArtFiles() <<
      ") order by FILE_NAME, PARENT_ID";
   
    qry.select(sql.str());
    while(!qry.eof() && !stopRequested()) {

      // check if a fam event occured recently
      int diff = DateTime::now().toInt() - m_famHandler->lastEventTime().toInt();
      if(diff < 5) {
        break;
      }

      count++;
      obj = new DbObject(qry.result());

      cout << "ALBUM ART FILE: " << obj->fileName() << " PID: " << obj->parentId() << endl;

      if(obj->parentId() == lastPid) {
        delete obj;
        qry.next();
        continue;        
      }
            
      parent = DbObject::createFromObjectId(obj->parentId());

      if(parent->details()->albumArtId() != 0) {
        delete parent;
        delete obj;
        qry.next();
        continue; 
      }

      
      // set the parent folder album art id      
      parent->details()->setAlbumArtId(obj->objectId());
      parent->details()->setAlbumArtExt(ExtractFileExt(obj->fileName()));
      parent->details()->save();
      parent->setDetailId(parent->details()->id());
      parent->save();
      delete parent;


      // get the audio siblings and set their album art id
      sql.str("");
      sql << 
        "select * from OBJECTS where PARENT_ID = " << obj->parentId() << " and " <<
        "TYPE >= " << ITEM_AUDIO_ITEM << " and TYPE < " << ITEM_AUDIO_ITEM_MAX << " and " <<
        "DEVICE is NULL";
      cout << sql.str() << endl;
      get.select(sql.str());
      while(!get.eof()) {
        
        sibling = new DbObject(get.result());

        cout << "SIBLING: " << sibling->fileName() << endl;
        
        if(sibling->details()->albumArtId() == 0) {
          sibling->details()->setAlbumArtId(obj->objectId());
          sibling->details()->setAlbumArtExt(ExtractFileExt(obj->fileName()));
          sibling->details()->save();
          sibling->setDetailId(sibling->details()->id());
          sibling->save();
        }

        delete sibling;
        get.next();
      }
      

      

      // hide the image object
      lastPid = obj->parentId();      
      obj->setVisible(false);
      obj->save();
      obj->details()->setAlbumArtExt(ExtractFileExt(obj->fileName()));
      obj->details()->save();
      delete obj;
      qry.next();
      msleep(1);
    } // while !eof (update album art)

    


    // create video thumbnails if enabled
    count = 0;
    CMetadataPlugin* thumbnailer = CPluginMgr::metadataPlugin("ffmpegthumbnailer");
    sql.str("");
    sql << 
      "select * from OBJECTS where TYPE >= " << ITEM_VIDEO_ITEM << " and TYPE < " << ITEM_VIDEO_ITEM_MAX << " and " <<
      "DEVICE is NULL and REF_ID = 0 and " <<
      "DETAIL_ID in (select ID from OBJECT_DETAILS where ALBUM_ART_ID = 0 and ALBUM_ART_EXT is NULL);";
    qry.select(sql.str());
    while(thumbnailer && !qry.eof() && !stopRequested()) {

      // check if a fam event occured recently
      int diff = DateTime::now().toInt() - m_famHandler->lastEventTime().toInt();
      if(diff < 5) {
        break;
      }

      count++;
      obj = new DbObject(qry.result());
      string filename = obj->path() + obj->fileName();
      stringstream tmpfile;
      tmpfile << CSharedConfig::Shared()->globalSettings->GetTempDir() << obj->objectId() << ".jpg";


      cout << "create thumbnail " << count << " of " << qry.size() << " for: " << filename << endl;
      
      size_t size = 0;
	  	unsigned char* buffer = (unsigned char*)malloc(1);
	  	char* mimeType = (char*)malloc(1);
	  	memset(mimeType, 0, 1);

			thumbnailer->openFile(filename);
      bool hasImage = thumbnailer->readImage(&mimeType, &buffer, &size, 300);
  		thumbnailer->closeFile();
			

      if(hasImage) {

        cout << "HAS IMAGE: " << size << endl;
        
        fuppes::File out(tmpfile.str());
        out.open(File::Write);
        out.write((char*)buffer, size);
        out.close();


        // set the album art id to the same value as the object id
        obj->details()->setAlbumArtId(obj->objectId());
        obj->details()->setAlbumArtExt("jpg");
        obj->details()->save(&ins);
      }
      else {
        obj->details()->setAlbumArtExt("fail");
        obj->details()->save(&ins);
      }
      
      free(buffer);
			free(mimeType);

      


      delete obj;
      qry.next();
      msleep(1);
    } // while !eof (video thumbnails)
    if(thumbnailer)
      delete thumbnailer;





    //cout << "update thread sleep " << sleep << " ms" << endl;

    // let the thread sleep for a while
    // we chunk the sleep time to avoid blocking closing of this thread
    // on shutdown or database update/rebuild
    int tmp = sleep;
    do {
      if(tmp > 500) {
        msleep(500);
        tmp -= 500;
      }
      else {
        msleep(tmp);
        tmp = 0;
      }
    } while(tmp > 0 && !stopRequested());
    
  } // !stopRequested


  delete connection;
  
  cout << "exit update thread" << endl;
  
}


void UpdateThread::updateAudioFile(DbObject* obj, SQLQuery* qry)
{
  
  metadata_t metadata;
	init_metadata(&metadata);

  string fileName = obj->path() + obj->fileName();

  //cout << "UPDATE AUDIO FILE: " << fileName << endl;

  bool gotMetadata = true;
	if(!CFileDetails::Shared()->getMusicTrackDetails(fileName, &metadata)) {
		free_metadata(&metadata);
    cout << "NO METADATA" << endl;
    gotMetadata = false;
	  //return;

    obj->setUpdated();
    obj->save();
    return;
	}

  fuppes_off_t fileSize = getFileSize(fileName);

  unsigned int objectId = obj->objectId(); // CContentDatabase::GetObjId();
	unsigned int imgId = 0;
  
	string imgMimeType;
	if(metadata.has_image == 1) {
		imgId = objectId;
		imgMimeType = metadata.image_mime_type;
	}

/*
  string sDlna;

	stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(A_ARTIST, A_ALBUM, A_TRACK_NO, A_GENRE, AV_DURATION, DATE, " <<
    "A_CHANNELS, AV_BITRATE, A_SAMPLERATE, " <<
		"ALBUM_ART_ID, ALBUM_ART_EXT, SIZE, DLNA_PROFILE) " <<
		"values (" <<
		"'" << SQLEscape(metadata.artist) << "', " <<
		"'" << SQLEscape(metadata.album) << "', " <<
		metadata.track_no << ", " <<
		"'" << SQLEscape(metadata.genre) << "', " <<
		"'" << metadata.duration << "', " <<
		"'" << "" << "', " <<
		metadata.channels << ", " <<
		metadata.bitrate << ", " <<
		metadata.samplerate << ", " <<
		imgId << ", " <<
		"'" << imgMimeType << "', " <<
    fileSize << ", " <<
    "'" << sDlna << "')";
  */

  ObjectDetails details;
  details.setSize(fileSize);
  if(gotMetadata) {
    details.setArtist(metadata.artist);
    details.setAlbum(metadata.album);
    details.setTrackNo(metadata.track_no);
    details.setGenre(metadata.genre);
    details.setDurationMs(metadata.duration_ms);
    details.setAudioChannels(metadata.channels);
    details.setAudioBitrate(metadata.bitrate);
    details.setAudioSamplerate(metadata.samplerate);
  }

  string title = metadata.title;
	free_metadata(&metadata);	

  //cout << sSql.str() << endl;


  details.save(qry);
  
  obj->setTitle(title);
  obj->setDetailId(details.id());
  obj->save(qry);  
}

void UpdateThread::updateImageFile(DbObject* obj, SQLQuery* qry)
{
  string fileName = obj->path() + obj->fileName();
  cout << "UPDATE IMAGE FILE: " << fileName << endl;

  SImageItem ImageItem;
  bool gotMetadata = CFileDetails::Shared()->GetImageDetails(fileName, &ImageItem);

  if(!gotMetadata) {
    obj->setUpdated();
    obj->save();
    return;
  }
  
  string dlna;
	string mimeType;
	string ext = ExtractFileExt(fileName);
	if(CPluginMgr::dlnaPlugin()) {
		CPluginMgr::dlnaPlugin()->getImageProfile(ext, ImageItem.nWidth, ImageItem.nHeight, &dlna, &mimeType);
	}

  ObjectDetails details;
  details.setWidth(ImageItem.nWidth);
  details.setHeight(ImageItem.nHeight);
  details.save(qry);
  
  obj->setDetailId(details.id());
  obj->save(qry);
  
	/*stringstream sSql;
	sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(SIZE, IV_WIDTH, IV_HEIGHT, DATE, " <<
		"DLNA_PROFILE, DLNA_MIME_TYPE) " <<
		"values (" <<
		getFileSize(fileName) << ", " <<
		ImageItem.nWidth << ", " <<
		ImageItem.nHeight << ", " <<
   (ImageItem.sDate.empty() ? "NULL" : "'" + ImageItem.sDate + "'") << ", " <<
    "'" << dlna << "', " <<
		"'" << mimeType << "')";
	
  return qry->insert(sSql.str()); */ 
  
}


void UpdateThread::updateVideoFile(DbObject* obj, SQLQuery* qry)
{
  string fileName = obj->path() + obj->fileName();
  cout << "UPDATE VIDEO FILE: " << fileName << endl;
  
  SVideoItem VideoItem;
	bool gotMetadata = CFileDetails::Shared()->GetVideoDetails(fileName, &VideoItem);
  
  //string sDlna; // = CFileDetails::Shared()->GuessDLNAProfileId(p_sFileName);
	VideoItem.nSize = getFileSize(fileName);

  ObjectDetails details;
  details.setSize(VideoItem.nSize);
  if(gotMetadata) {
    details.setWidth(VideoItem.nWidth);
    details.setHeight(VideoItem.nHeight);
    details.setDurationMs(VideoItem.durationMs);
    details.setVideoBitrate(VideoItem.nBitrate);
    details.setAudioCodec(VideoItem.sACodec);
    details.setVideoCodec(VideoItem.sVCodec);
  }
  details.save(qry);
  
  obj->setDetailId(details.id());
  obj->save(qry);
}

