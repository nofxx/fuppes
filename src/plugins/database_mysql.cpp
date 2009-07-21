/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            core_mysql.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "../../include/fuppes_plugin.h"
#include "../../include/fuppes_db_connection_plugin.h"

#include <string>
#include <sstream>
#include <iostream>

#include <mysql.h>

// http://dev.mysql.com/doc/refman/5.1/en/c.html


class CMySQLConnection: public CDatabaseConnection
{
	public:
		CMySQLConnection();
		~CMySQLConnection();

		virtual bool startTransaction();
		virtual bool commit();
		virtual void rollback();
		
	private:
		bool				open(const CConnectionParams params);
		CSQLQuery*	query();

		MYSQL				m_handle;
};

class CMySQLQuery: public CSQLQuery
{
	friend class CMySQLConnection;
	
	public:
		~CMySQLQuery() {
			clear();
		}

		virtual bool select(const std::string sql);
		virtual bool exec(const std::string sql);
		virtual off_t insert(const std::string sql);
		
		bool eof() { return (m_ResultListIterator == m_ResultList.end()); }
		void next() {
		  if(m_ResultListIterator != m_ResultList.end()) {
		    m_ResultListIterator++;
		  }
		}
		CSQLResult* result() { return *m_ResultListIterator; }	
		
		off_t lastInsertId() { return m_lastInsertId; }
		
		void clear() {
			for(m_ResultListIterator = m_ResultList.begin(); m_ResultListIterator != m_ResultList.end();) {
				if(m_ResultList.empty())
				  break;
				
				CSQLResult* pResult = *m_ResultListIterator;
				std::list<CSQLResult*>::iterator tmpIt = m_ResultListIterator;          
				++tmpIt;
				m_ResultList.erase(m_ResultListIterator);
				m_ResultListIterator = tmpIt;
				delete pResult;
			} 
		
			m_ResultList.clear();
			m_rowsReturned = 0;
		}
		
		CDatabaseConnection* connection() { return m_connection; }
		
	private:
		CMySQLQuery(CDatabaseConnection* connection, MYSQL* handle);		
		MYSQL* m_handle;
		CDatabaseConnection* m_connection;		
		
		off_t		m_lastInsertId;
		
		std::list<CSQLResult*> m_ResultList;
    std::list<CSQLResult*>::iterator m_ResultListIterator;
		off_t m_rowsReturned;
};


class CMySQLResult: public CSQLResult
{	
	friend class CMySQLQuery;
	
  public:
    bool isNull(std::string fieldName){			
			std::string sValue = asString(fieldName);
			if((sValue.length() == 0) || (sValue.compare("NULL") == 0))
				return true;
			else
				return false;
		}

    std::string	asString(std::string fieldName) {
		  return m_FieldValues[fieldName];
		}

		unsigned int asUInt(std::string fieldName) {	
			if(!isNull(fieldName)) {
				return strtoul(asString(fieldName).c_str(), NULL, 0);
			}
			return 0;
		}
			
		int asInt(std::string fieldName) {
			if(!isNull(fieldName)) {
  			return atoi(asString(fieldName).c_str());
		  }
			return 0;
		}
		
		CSQLResult* clone() {
			
			CMySQLResult* result = new CMySQLResult();			
			for(m_FieldValuesIterator = m_FieldValues.begin();
					m_FieldValuesIterator != m_FieldValues.end();
					m_FieldValuesIterator++) {

				result->m_FieldValues[m_FieldValuesIterator->first] = m_FieldValuesIterator->second;
			}			
			return result;
		}

  private:
    std::map<std::string, std::string> m_FieldValues;
    std::map<std::string, std::string>::iterator m_FieldValuesIterator;  
};




CMySQLConnection::CMySQLConnection()
{
	mysql_init(&m_handle);
}

CMySQLConnection::~CMySQLConnection()
{
	mysql_close(&m_handle);
}

bool CMySQLConnection::open(const CConnectionParams params)
{
	if(!mysql_real_connect(&m_handle, 
												 params.hostname.c_str(),
												 params.username.c_str(),
												 params.password.c_str(),
												 params.dbname.c_str(), 0, NULL, 0)) {
    fprintf(stderr, "Failed to connect to database: Error: %s\n", mysql_error(&m_handle));
		return false;
	}
	
	return true;
}

CSQLQuery* CMySQLConnection::query()
{	
	return new CMySQLQuery(this, &m_handle);
}

bool CMySQLConnection::startTransaction()
{
	return false;
}

bool CMySQLConnection::commit()
{
	return false;
}

void CMySQLConnection::rollback()
{
}


CMySQLQuery::CMySQLQuery(CDatabaseConnection* connection, MYSQL* handle)
{
	m_handle = handle;
	m_connection = connection;
}

bool CMySQLQuery::select(const std::string sql)
{
	clear();
	
	if(mysql_query(m_handle, sql.c_str()) != 0) {
		std::cout << "mysql: query error " << mysql_error(m_handle) << std::endl;
		return false;
	}
	
	MYSQL_RES* res;
	MYSQL_ROW row;
	MYSQL_FIELD *fields;
	unsigned int num_fields;
	unsigned int i;
	
	res = mysql_store_result(m_handle);	
	num_fields = mysql_num_fields(res);
	fields = mysql_fetch_fields(res);
	
	CMySQLResult* pResult;
	while((row = mysql_fetch_row(res))) {
		
		pResult = new CMySQLResult();		
		for(i = 0; i < num_fields; i++)	{

			if(!row[i]) {			
				pResult->m_FieldValues[std::string(fields[i].name)] = "NULL";	
				continue;
			}
			
			std::string value;
			if(IS_NUM(fields[i].type)) {
				std::stringstream str;
				str << row[i];
				value = str.str();
				str.str("");
			}
			else {
				value = row[i];
			}
			pResult->m_FieldValues[std::string(fields[i].name)] = value;

		}
		m_ResultList.push_back(pResult);
	}
  m_ResultListIterator = m_ResultList.begin();
	mysql_free_result(res);
	
	return true;
}

bool CMySQLQuery::exec(const std::string sql)
{
	
	if(mysql_query(m_handle, sql.c_str()) != 0) {
		std::cout << "mysql: exec error : " << sql << std::endl;
		return false;
	}
		
	return true;
}

off_t CMySQLQuery::insert(const std::string sql)
{
	if(mysql_query(m_handle, sql.c_str()) != 0) {
		std::cout << "mysql: insert error : " << sql << std::endl;
		return 0;
  }

	return mysql_insert_id(m_handle);
}



#ifdef __cplusplus
extern "C" {
#endif

void register_fuppes_plugin(plugin_info* plugin)
{
	strcpy(plugin->plugin_name, "mysql");
	strcpy(plugin->plugin_author, "Ulrich Voelkel");
	plugin->plugin_type = PT_DATABASE_CONNECTION;
	
	if(mysql_library_init(0, NULL, NULL) != 0) {
    fprintf(stderr, "could not initialize MySQL library\n");
  }
}

CDatabaseConnection* fuppes_plugin_create_db_connection(plugin_info* plugin __attribute__((unused)))
{
	return new CMySQLConnection();
}

void unregister_fuppes_plugin(plugin_info* plugin __attribute__((unused)))
{
  mysql_library_end();
}
	
#ifdef __cplusplus
}
#endif

/*
CREATE TABLE OBJECTS(
	ID INTEGER AUTO_INCREMENT ,
	OBJECT_ID INTEGER NOT NULL ,
	DETAIL_ID INTEGER DEFAULT NULL ,
	TYPE INTEGER NOT NULL ,
	DEVICE TEXT DEFAULT NULL ,
	PATH TEXT NOT NULL ,
	FILE_NAME TEXT DEFAULT NULL ,
	TITLE TEXT DEFAULT NULL ,
	MD5 TEXT DEFAULT NULL ,
	MIME_TYPE TEXT DEFAULT NULL ,
	REF_ID INTEGER DEFAULT NULL ,
	PRIMARY KEY ( ID )
);


CREATE TABLE OBJECT_DETAILS (
	ID INTEGER AUTO_INCREMENT,
	AV_BITRATE INTEGER, 
	AV_DURATION TEXT, 
	A_ALBUM TEXT, 
	A_ARTIST TEXT, 
	A_CHANNELS INTEGER, 
	A_DESCRIPTION TEXT, 
	A_GENRE TEXT, 
	A_SAMPLERATE INTEGER, 
	A_TRACK_NO INTEGER, 
	DATE TEXT, 
	IV_HEIGHT INTEGER, 
	IV_WIDTH INTEGER, 
	A_CODEC TEXT, 
	V_CODEC TEXT, 
	ALBUM_ART_ID INTEGER, 
	ALBUM_ART_MIME_TYPE TEXT, 
	SIZE INTEGER DEFAULT 0, 
	DLNA_PROFILE TEXT DEFAULT NULL,
	DLNA_MIME_TYPE TEXT DEFAULT NULL,
	PRIMARY KEY ( ID )
);
 
CREATE TABLE MAP_OBJECTS (
	ID INTEGER AUTO_INCREMENT, 
	OBJECT_ID INTEGER NOT NULL, 
	PARENT_ID INTEGER NOT NULL, 
	DEVICE TEXT,
	PRIMARY KEY ( ID )
);
 
*/