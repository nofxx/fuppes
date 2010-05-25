/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            DatabaseObject.cpp
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

#include "../Common/Common.h"
#include "DatabaseObject.h"
#include "ContentDatabase.h"
using namespace fuppes;

#include <sstream>
#include <iostream>
using namespace std;


DbObject* DbObject::createFromObjectId(unsigned int objectId, SQLQuery* qry /*= NULL*/) // static
{
  DbObject* result = NULL;

  bool tmpQry = false;
  if(!qry) {
    qry = new SQLQuery();
    tmpQry = true;
  }

  std::stringstream sql;
  sql <<
    "select * from OBJECTS where "
    "REF_ID = 0 and "
    "DEVICE is NULL and "
    "OBJECT_ID = " << objectId;

  qry->select(sql.str());
  if(!qry->eof()) {
    result = new DbObject(qry->result());
  }

  if(tmpQry)
    delete qry;
  
  return result;
}

DbObject* DbObject::createFromFileName(std::string fileName, SQLQuery* qry /*= NULL*/) // static
{
  DbObject* result = NULL;

  bool tmpQry = false;
  if(!qry) {
    qry = new SQLQuery();
    tmpQry = true;
  }

	std::string path = ExtractFilePath(fileName);
	std::string tmp;
	if(path.length() < fileName.length()) {
    tmp = fileName.substr(path.length(), fileName.length());
	}

  std::string sql = 
    "select * from OBJECTS where "
    "REF_ID = 0 and "
    "DEVICE is NULL and "
    "PATH = '" + SQLEscape(path) + "' and ";

  if(tmp.empty())
    sql += "FILE_NAME is NULL ";
	else
    sql += "FILE_NAME = '" + SQLEscape(tmp) + "' ";


  //std::cout << sql << std::endl;
  
  qry->select(sql);
  if(!qry->eof()) {
    result = new DbObject(qry->result());
  }

  
  if(tmpQry)
    delete qry;
  
  return result;
}


DbObject::DbObject()
{
  reset();
}

DbObject::DbObject(DbObject* object)
{
  m_id        = 0;
  m_objectId  = 0;
  m_parentId  = object->m_parentId;
  m_detailId  = object->m_detailId;
  m_type      = object->m_type;
  m_path      = object->m_path;
  m_oldPath   = m_path;
  m_fileName  = object->m_fileName;
  m_title     = object->m_title;
  m_md5       = object->m_md5;
  m_visible   = object->m_visible;
  
  m_refId     = object->m_refId;
  m_device    = object->m_device;
  m_vcType    = object->m_vcType;
  m_vcPath    = object->m_vcPath;
  m_vrefId    = object->m_vrefId;

  m_details   = object->m_details;
  m_changed   = false;
  m_pathChanged = false;
}
    
DbObject::DbObject(CSQLResult* result)
{
  m_id        = result->asUInt("ID");
  m_objectId  = result->asUInt("OBJECT_ID");
  m_parentId  = result->asUInt("PARENT_ID");
  m_detailId  = result->asUInt("DETAIL_ID");
  m_type      = (OBJECT_TYPE)result->asInt("TYPE");
  m_path      = result->asString("PATH");
  m_oldPath   = m_path;
  m_fileName  = result->asString("FILE_NAME");
  m_title     = result->asString("TITLE");
  m_md5       = result->asString("MD5");
  m_visible   = (result->asInt("VISIBLE") == 1 ? true : false);
  
  m_refId     = result->asUInt("REF_ID");
  m_device    = result->asString("DEVICE");
  m_vcType    = (VirtualContainerType)result->asInt("VCONTAINER_TYPE");
  m_vcPath    = result->asString("VCONTAINER_PATH");
  m_vrefId    = result->asUInt("VREF_ID");
  
  m_changed   = false;
  m_pathChanged = false;

  m_details.reset();
}

void DbObject::reset()
{
  m_id        = 0;
  m_objectId  = 0;
  m_parentId  = 0;
  m_detailId  = 0;
  m_type      = OBJECT_TYPE_UNKNOWN;
  m_refId     = 0;
  m_visible   = true;
  m_vcType    = None;
  m_vcPath    = "";
  m_vrefId    = 0;

  m_changed   = false;
  m_pathChanged = false;

  m_details.reset();
}


bool DbObject::save(SQLQuery* qry /*= NULL*/, bool createReference /*= false*/)
{
  if(!m_changed)
    return true;

  bool tmpQry = (qry == NULL);
  if(tmpQry) {
    qry = new SQLQuery();
  }

  bool ret = false;
  std::stringstream sql;

  if(m_objectId == 0) {
    m_objectId = CContentDatabase::GetObjId();
  }
  
  if(m_id > 0) { // UPDATE

    sql << "update OBJECTS set "
      "PARENT_ID = " << m_parentId << ", " <<
      "DETAIL_ID = " << m_detailId << ", " <<
      "TYPE = " << m_type << ", " <<
      "PATH = " << (m_path.empty() ? "NULL" : "'" + SQLEscape(m_path) + "'") << ", " <<
      "FILE_NAME = " << (m_fileName.empty() ? "NULL" : "'" + SQLEscape(m_fileName) + "'") << ", " <<
      "TITLE = " << (m_title.empty() ? "NULL" : "'" + SQLEscape(m_title) + "'") << ", " <<
      "MD5 = " << (m_md5.empty() ? "NULL" : "'" + SQLEscape(m_md5) + "'") << ", " <<
      "REF_ID = " << m_refId << ", " <<
      "VISIBLE = " << (m_visible ? 1 : 0) << ", " <<
      "DEVICE = " << (m_device.empty() ? "NULL" : "'" + SQLEscape(m_device) + "'") << ", " <<
      "VCONTAINER_TYPE = " << m_vcType << ", " <<
      "VCONTAINER_PATH = " << (m_vcPath.empty() ? "NULL" : "'" + SQLEscape(m_vcPath) + "'") << ", " <<
      "VREF_ID = " << m_vrefId << ", " <<
      "UPDATED_AT = " << DateTime::now().toInt() << " " <<
      "where "
      "ID = " << m_id;

    ret = qry->exec(sql.str());


    // if we update a container which path has changed
    // we have to update the path of all child objects
    if(m_pathChanged && (m_type > OBJECT_TYPE_UNKNOWN && m_type < ITEM)) {
      sql.str("");

      sql << "update OBJECTS set "
        "PATH = replace(PATH, '" << m_oldPath << "', '" << m_path << "') "
        "where "
        "PATH like '" << m_oldPath << "%'";

      ret = qry->exec(sql.str());
      
      m_pathChanged = false;
    }
    
  }
  else { // INSERT

    if(createReference) {

      sql << "select OBJECT_ID from OBJECTS where "
        "DETAIL_ID = " << m_detailId << " and "
        "DEVICE " << (m_device.empty() ? "is NULL" : ("= '" + SQLEscape(m_device) + "'")) << " and "
        "TYPE = " << m_type << " and "
        "PATH = '" << SQLEscape(m_path) << "' and " 
        "FILE_NAME = '" << SQLEscape(m_fileName) << "' and " 
        "TITLE = '" << SQLEscape(m_title) << "' and " 
        "REF_ID = 0";

      qry->select(sql.str());
      if(!qry->eof()) {
        m_refId = qry->result()->asUInt("OBJECT_ID");
      }
      sql.str("");
    }
    
    sql << "insert into OBJECTS ( " <<
      "OBJECT_ID, "
      "PARENT_ID, "
      "DETAIL_ID, "
      "TYPE, "
      "PATH, "
      "FILE_NAME, "
      "TITLE, "
      "MD5, "
      "REF_ID, "
      "DEVICE, "
      "VCONTAINER_TYPE, "
      "VCONTAINER_PATH, "
      "VREF_ID, "
      "VISIBLE, "
      "CHANGED_AT"
      ") values ( " <<
      m_objectId << ", " << 
      m_parentId << ", " << 
      m_detailId << ", " << 
      m_type << ", " <<
      (m_path.empty() ? "NULL" : "'" + SQLEscape(m_path) + "'") << ", " <<
      (m_fileName.empty() ? "NULL" : "'" + SQLEscape(m_fileName) + "'") << ", " <<
      (m_title.empty() ? "NULL" : "'" + SQLEscape(m_title) + "'") << ", " <<
      (m_md5.empty() ? "NULL" : "'" + SQLEscape(m_md5) + "'") << ", " <<
      m_refId << ", " <<
      (m_device.empty() ? "NULL" : "'" + SQLEscape(m_device) + "'") << ", " <<
      m_vcType << ", " << 
      (m_vcPath.empty() ? "NULL" : "'" + SQLEscape(m_vcPath) + "'") << ", " <<
      m_vrefId << ", " << (m_visible ? 1 : 0) << ", " <<
      DateTime::now().toInt() <<
    ")";


    //std::cout << sql.str() << std::endl;
    
    ret = (qry->insert(sql.str()) > 0);
    m_id = qry->lastInsertId();
  }

  if(tmpQry)
    delete qry;
  
  m_changed = !ret;
  return ret;
}


bool DbObject::remove()
{
  SQLQuery qry;
  std::string sql;


  // container
  if(m_type > OBJECT_TYPE_UNKNOWN && m_type < ITEM) {
  
    // delete object details
    sql = "delete from OBJECT_DETAILS where ID in ("
      "select DETAIL_ID from OBJECTS where PATH like '" + SQLEscape(m_path) + "%')";
    qry.exec(sql);

    // delete objects
    sql = "delete from OBJECTS where PATH like '" + SQLEscape(m_path) + "%'";
    qry.exec(sql);

    #warning todo delete virtual folders and ref items

  }
  else if(m_type > ITEM) {

    // delete object details
    sql = "delete from OBJECT_DETAILS where ID = ("
      "select DETAIL_ID from OBJECTS where "
      "  PATH = '" + SQLEscape(m_path) + "' and "
      "  FILE_NAME = '" + SQLEscape(m_fileName) + "' "
      ")";
    qry.exec(sql);

    // delete object
    sql = "delete from OBJECTS where "
      "  PATH = '" + SQLEscape(m_path) + "' and "
      "  FILE_NAME = '" + SQLEscape(m_fileName) + "' ";
    qry.exec(sql);
    
  }
    
  return true;
}






ObjectDetails::ObjectDetails()
{
  reset();
}

void ObjectDetails::reset()
{
  m_id = 0;
  m_a_trackNo = 0;
  m_a_samplerate = 0;
  m_a_bitrate = 0;
  m_a_album = "";
  m_a_artist = "";
  m_a_genre = "";
  m_a_composer = "";
  m_a_description = "";
  m_a_codec = "";
  m_a_channels = 0;
  m_av_duration = "";
  m_iv_width = 0;
  m_iv_height = 0;
  m_v_bitrate = 0;
  m_v_codec = "";
  m_size = 0;
  m_source = Unknown;
  m_changed = false;
}

bool ObjectDetails::save(SQLQuery* qry /*= NULL*/)
{
  if(!m_changed)
    return true;

  bool tmpQry = (qry == NULL);
  if(tmpQry) {
    qry = new SQLQuery();
  }

  bool ret = false;
  std::stringstream sql;

  if(m_id > 0) { // UPDATE
     
    sql << "update OBJECT_DETAILS set "
    "A_TRACK_NO = " << m_a_trackNo << ", " <<
    "A_SAMPLERATE = " << m_a_samplerate << ", " <<
    "A_BITRATE = " << m_a_bitrate << ", " << 

    "A_ALBUM = " << (m_a_album.empty() ? "NULL" : "'" + SQLEscape(m_a_album) + "'") << ", " <<
    "A_ARTIST = " << (m_a_artist.empty() ? "NULL" : "'" + SQLEscape(m_a_artist) + "'") << ", " <<      
    "A_GENRE = " << (m_a_genre.empty() ? "NULL" : "'" + SQLEscape(m_a_genre) + "'") << ", " <<
    "A_COMPOSER = " << (m_a_composer.empty() ? "NULL" : "'" + SQLEscape(m_a_composer) + "'") << ", " <<
    "A_DESCRIPTION = " << (m_a_description.empty() ? "NULL" : "'" + SQLEscape(m_a_description) + "'") << ", " <<
    "A_CODEC = " << (m_a_codec.empty() ? "NULL" : "'" + SQLEscape(m_a_codec) + "'") << ", " <<
    "A_CHANNELS = " << m_a_channels << ", " <<
    "AV_DURATION = " << (m_av_duration.empty() ? "NULL" : "'" + SQLEscape(m_av_duration) + "'") << ", " <<

    "IV_WIDTH = " << m_iv_width << ", " <<
    "IV_HEIGHT = " << m_iv_height << ", " <<
      
    "V_CODEC = " << (m_v_codec.empty() ? "NULL" : "'" + SQLEscape(m_v_codec) + "'") << ", " <<
    "V_BITRATE = " << m_v_bitrate << ", " << 

    "SIZE = " << m_size << ", " << 
    "SOURCE = " << m_source << " " << 

    "where "
    "ID = " << m_id;

     /*  "ALBUM_ART_ID INTEGER, "
    "ALBUM_ART_EXT TEXT, " 
      "DATE TEXT, "*/

    ret = qry->exec(sql.str());

    sql.str("");
    sql << "update OBJECTS set "
      "UPDATED_AT = " << DateTime::now().toInt() << " " <<
      "where DETAIL_ID = " << m_id;
    qry->exec(sql.str());
    
  }
  else { // INSERT
    
    sql << "insert into OBJECT_DETAILS (" <<
      "A_TRACK_NO, " <<
      "A_SAMPLERATE, " <<
      "A_BITRATE, " <<
      "A_ALBUM, " <<
      "A_ARTIST, " <<
      "A_GENRE, " <<
      "A_COMPOSER, " <<
      "A_DESCRIPTION, " <<
      "A_CODEC, " <<
      "A_CHANNELS, " <<
      "AV_DURATION, " <<
      "IV_WIDTH, " <<
      "IV_HEIGHT, " <<
      "V_CODEC, " <<
      "V_BITRATE, " <<
      "SIZE, " <<
      "SOURCE " <<
      " ) values ( " <<
      m_a_trackNo << ", " <<
      m_a_samplerate << ", " <<
      m_a_bitrate << ", " << 
      (m_a_album.empty() ? "NULL" : "'" + SQLEscape(m_a_album) + "'") << ", " <<
      (m_a_artist.empty() ? "NULL" : "'" + SQLEscape(m_a_artist) + "'") << ", " <<
      (m_a_genre.empty() ? "NULL" : "'" + SQLEscape(m_a_genre) + "'") << ", " <<
      (m_a_composer.empty() ? "NULL" : "'" + SQLEscape(m_a_composer) + "'") << ", " <<
      (m_a_description.empty() ? "NULL" : "'" + SQLEscape(m_a_description) + "'") << ", " <<
      (m_a_codec.empty() ? "NULL" : "'" + SQLEscape(m_a_codec) + "'") << ", " <<
      m_a_channels << ", " <<
      (m_av_duration.empty() ? "NULL" : "'" + SQLEscape(m_av_duration) + "'") << ", " <<
      m_iv_width << ", " <<
      m_iv_height << ", " <<
      (m_v_codec.empty() ? "NULL" : "'" + SQLEscape(m_v_codec) + "'") << ", " <<
      m_v_bitrate << ", " <<
      m_size << ", " <<
      m_source << " " <<
      " ) ";

    
  //  std::cout << sql.str() << std::endl;    
    ret = (qry->insert(sql.str()) > 0);
    m_id = qry->lastInsertId();


 //  std::cout << "DETAIL_ID: "<< m_id << std::endl;   

    /*sql.str("");
    sql << "update OBJECTS set "
      "UPDATED_AT = " << DateTime::now().toInt() << " " <<
      "where DETAIL_ID = " << m_id;
    qry->exec(sql.str()); */
  }

  if(tmpQry)
    delete qry;
  
  m_changed = !ret;
  return ret;
}




