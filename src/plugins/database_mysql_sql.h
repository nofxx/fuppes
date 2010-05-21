/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            database_mysql_sql.h
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
 
 #include "../../include/fuppes_db_connection_plugin.h"

/*

CREATE TABLE FUPPES_DB_INFO ( VERSION INTEGER NOT NULL )
 
 CREATE TABLE OBJECTS ( 
	  ID INTEGER PRIMARY KEY AUTO_INCREMENT,
	  OBJECT_ID BIGINT NOT NULL, 
	  PARENT_ID BIGINT NOT NULL, 
	  DETAIL_ID BIGINT NOT NULL, 
	  TYPE INTEGER NOT NULL, 
	  PATH TEXT NOT NULL, 
	  FILE_NAME TEXT DEFAULT NULL, 
	  TITLE TEXT DEFAULT NULL, 
	  MD5 TEXT DEFAULT NULL, 
	  MIME_TYPE TEXT DEFAULT NULL, 
	  REF_ID BIGINT DEFAULT 0,
    DEVICE TEXT DEFAULT NULL, 
    VCONTAINER_TYPE INTEGER DEFAULT 0,
    VCONTAINER_PATH TEXT DEFAULT NULL,
 	  VREF_ID BIGINT DEFAULT 0,
	  VISIBLE INTEGER DEFAULT 1, 
    CHANGED_AT INTEGER,
    UPDATED_AT INTEGER )
 ENGINE=MyISAM  DEFAULT CHARSET=utf8;


CREATE TABLE OBJECT_DETAILS ( 
    ID INTEGER PRIMARY KEY AUTO_INCREMENT, 
    AV_BITRATE INTEGER, 
	  AV_DURATION TEXT, 
	  A_ALBUM TEXT, 
	  A_ARTIST TEXT, 
	  A_CHANNELS INTEGER, 
	  A_DESCRIPTION TEXT, 
	  A_GENRE TEXT, 
	  A_COMPOSER TEXT, 
	  A_SAMPLERATE INTEGER, 
	  A_TRACK_NO INTEGER, 
	  DATE TEXT, 
	  IV_HEIGHT INTEGER, 
	  IV_WIDTH INTEGER, 
    A_CODEC TEXT, 
    V_CODEC TEXT, 
	  ALBUM_ART_ID INTEGER, 
	  ALBUM_ART_EXT TEXT, 
    SIZE BIGINT DEFAULT 0, 
	  DLNA_PROFILE TEXT DEFAULT NULL,
	  DLNA_MIME_TYPE TEXT DEFAULT NULL, 
    SOURCE INT DEFAULT 0
) DEFAULT CHARSET=utf8;
 
 */


static const fuppes_sql mysql_sql[] = {

  {SQL_UNKNOWN, ""},
  
};

 
 
 
 
 
