/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            database_sqlite3.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "database_sqlite3_sql.h"

#include <string>
#include <sstream>
#include <iostream>

#include <sqlite3.h>

#ifdef WIN32
#include <windows.h>
#endif

class CSQLiteConnection: public CDatabaseConnection
{
	public:
		CSQLiteConnection(plugin_info* plugin);
		~CSQLiteConnection();

		bool startTransaction();
		bool commit();
		void rollback();

		const char* getStatement(fuppes_sql_no number) {

//std::cout << "get statement: " << number << std::endl;
			
			return sqlite3_statements[number].sql;
		}

		bool setup();

    plugin_info* plugin() { return m_plugin; }

    void vacuum();
    
	private:
		bool				connect(const CConnectionParams params);
		ISQLQuery*	query();

		sqlite3*			m_handle;
		plugin_info*	m_plugin;
};

class CSQLiteQuery: public ISQLQuery
{
	friend class CSQLiteConnection;
	
	public:		
		~CSQLiteQuery() {
			clear();
		}
		
		virtual bool select(const std::string sql = "");
		virtual bool exec(const std::string sql = "");
		virtual fuppes_off_t insert(const std::string sql = "");
		
		bool eof() { return (m_ResultListIterator == m_ResultList.end()); }
		void next() {
		  if(m_ResultListIterator != m_ResultList.end()) {
		    m_ResultListIterator++;
		  }
		}
		CSQLResult* result() { return *m_ResultListIterator; }	
		
		fuppes_off_t lastInsertId() { return m_lastInsertId; }
		
		void clear() {
			for(m_ResultListIterator = m_ResultList.begin(); m_ResultListIterator != m_ResultList.end();) {
				CSQLResult* pResult = *m_ResultListIterator;
				delete pResult;				
				m_ResultListIterator++;
			}		
			m_ResultList.clear();
			m_rowsReturned = 0;
			m_ResultListIterator = m_ResultList.end();
		}

		CDatabaseConnection* connection() { return m_connection; }


    unsigned int size() {
      return m_ResultList.size();
    }


    static std::string escape(std::string value);

    void  prepare(const std::string sql) {
      m_sqlPrepare = sql;
      m_sqlPrepared = "";
    }

    void bind(const std::string field, const std::string value) {
      std::string format = "'" + CSQLiteQuery::escape(value) + "'";
      setValue(field, format);
    }
    
	private:
		CSQLiteQuery(CDatabaseConnection* connection, sqlite3* handle);		
		sqlite3* m_handle;
		
		off_t		m_lastInsertId;
		
		std::list<CSQLResult*> m_ResultList;
    std::list<CSQLResult*>::iterator m_ResultListIterator;
		off_t m_rowsReturned;

    std::string   m_sqlPrepare;
    std::string   m_sqlPrepared;

    void setValue(std::string field, std::string value) {

      std::string tmp = m_sqlPrepare;
      field = ":" + field;
      
      while(tmp.find(field) != std::string::npos)	{
	      m_sqlPrepared += tmp.substr(0, tmp.find(field)) + value;
		    tmp   = tmp.substr(tmp.find(field) + value.length(), tmp.length());
	    }
      m_sqlPrepared += tmp;
    }
		
		CDatabaseConnection* m_connection;
};


class CSQLiteResult: public CSQLResult
{	
	friend class CSQLiteQuery;
	
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
			
			CSQLiteResult* result = new CSQLiteResult();			
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




CSQLiteConnection::CSQLiteConnection(plugin_info* plugin) //:CDatabaseConnection()
{
	m_handle = NULL;
	m_plugin = plugin;
}

CSQLiteConnection::~CSQLiteConnection()
{
  //std::cout << "~CSQLiteConnection" << std::endl;
	if(!m_handle)
		return;
	
	sqlite3_close(m_handle);  
}

bool CSQLiteConnection::connect(const CConnectionParams params)
{
  //std::cout << "open sqlite3 db file: " << params.filename << ":" << std::endl;
  
	if(sqlite3_open(params.filename.c_str(), &m_handle) != SQLITE_OK) {
    fprintf(stderr, "[sqlite] can't create/open database: %s :: %s\n", sqlite3_errmsg(m_handle), params.filename.c_str());
    sqlite3_close(m_handle);
    return false;
  }
  //JM: Tell sqlite3 to retry queries for up to 1 second if the database is locked.
  sqlite3_busy_timeout(m_handle, 1000);
	
	CSQLiteQuery qry(this, m_handle);
	qry.exec("pragma temp_store = MEMORY");
  qry.exec("pragma synchronous = OFF;");

	return true;
}

bool CSQLiteConnection::setup()
{
	return true;
}

ISQLQuery* CSQLiteConnection::query()
{	
	return new CSQLiteQuery(this, m_handle);
}

bool CSQLiteConnection::startTransaction()
{
	ISQLQuery* qry = query();
	bool result = qry->exec("begin transaction");
	delete qry;
	return result;
}

bool CSQLiteConnection::commit()
{
	ISQLQuery* qry = query();
	bool result = qry->exec("commit transaction");
	delete qry;
	return result;
}

void CSQLiteConnection::rollback()
{
	ISQLQuery* qry = query();
	qry->exec("rollback transaction");
	delete qry;
}

void CSQLiteConnection::vacuum()
{
  CSQLiteQuery qry(this, m_handle);
  qry.exec("vacuum");
}


CSQLiteQuery::CSQLiteQuery(CDatabaseConnection* connection, sqlite3* handle)
{
	m_handle = handle;
	m_connection = connection;
}

bool CSQLiteQuery::select(const std::string sql /* = "" */)
{
	clear();
  
  if(m_sqlPrepared.length() == 0 && sql.length() == 0)
    return false;

  if(m_sqlPrepared.length() > 0 && sql.length() > 0)
    return false;

  std::string statement = m_sqlPrepared;
  if(statement.length() == 0)
    statement = sql;
  

  
  char* szErr = NULL;
  char** szResult;
  int nRows = 0;
  int nCols = 0;
  
  int nResult = SQLITE_OK;  
  int nTry = 0;
  

  CSQLiteConnection* connection = (CSQLiteConnection*)m_connection;
  connection->plugin()->cb.log(connection->plugin(), 0, __FILE__, __LINE__, "query: %s", statement.c_str());
	
  do {
    nResult = sqlite3_get_table(m_handle, statement.c_str(), &szResult, &nRows, &nCols, &szErr);
    if(nTry > 0) {
#ifdef WIN32
      Sleep(1);
#else
      usleep(100);
#endif
    }
    nTry++;
  } while (nResult == SQLITE_BUSY);


  // select error
  if(nResult != SQLITE_OK) {
		std::cout << "SQL select error: " << szErr << " :: " << statement << std::endl;
    sqlite3_free(szErr);
    return false;
  }


  // get the result
  CSQLiteResult* pResult;     
  for(int i = 1; i < nRows + 1; i++) {
    pResult = new CSQLiteResult();
          
    for(int j = 0; j < nCols; j++) {        
      pResult->m_FieldValues[std::string(szResult[j])] =  std::string(szResult[(i * nCols) + j] ? szResult[(i * nCols) + j] : "");
    }
    
    m_ResultList.push_back(pResult);
		m_rowsReturned++;
  }
  m_ResultListIterator = m_ResultList.begin();     
  sqlite3_free_table(szResult);

	return true;
}

bool CSQLiteQuery::exec(const std::string sql /* = "" */)
{
  if(m_sqlPrepared.length() == 0 && sql.length() == 0)
    return false;

  if(m_sqlPrepared.length() > 0 && sql.length() > 0)
    return false;

  std::string statement = m_sqlPrepared;
  if(statement.length() == 0)
    statement = sql;

  
  char* szErr  = NULL;
  bool  bRetry = true;
  bool	result = false;
  int nResult;  

	//std::cout << "exec: " << sql << std::endl;
	
  while(bRetry) {  
    
    nResult = sqlite3_exec(m_handle, statement.c_str(), NULL, NULL, &szErr);
    switch(nResult) {
      case SQLITE_BUSY:
        bRetry = true;
#ifdef WIN32
        Sleep(1);
#else
				usleep(50);
#endif
        break;
      
      case SQLITE_OK:
        bRetry  = false;
				result = true;
        //nResult = sqlite3_last_insert_rowid(m_pDbHandle);
        break;
      
      default:
        bRetry = false;
				result = false;
				std::cout << "SQL exec error: " << szErr << " :: " << statement << std::endl;
        //CSharedLog::Log(L_NORM, __FILE__, __LINE__, "CContentDatabase::Insert - insert :: SQL error: %s\nStatement: %s", szErr, p_sStatement.c_str());
        sqlite3_free(szErr);
        //nResult = 0;
        break;
    }
    
  }
	
	return result;
}

fuppes_off_t CSQLiteQuery::insert(const std::string sql /* = "" */)
{
	if(!exec(sql)) {
		m_lastInsertId = 0;
		return 0;
	}
	
	m_lastInsertId = sqlite3_last_insert_rowid(m_handle);
	return m_lastInsertId;
}


std::string CSQLiteQuery::escape(std::string value) // static
{
  int nPos     = -2;
  int nLastPos = -1;
  do {
    
    if(nPos != -2)
      nLastPos = nPos;
    
    nPos = value.find('\'', nPos + 2);  
    if((nPos > -1) && (nPos != nLastPos))
      value = value.replace(nPos, 1, "\'\'");
    
  } while((nPos > -1) && (nPos != nLastPos));
  
  return value;
}



#ifdef __cplusplus
extern "C" {
#endif

void register_fuppes_plugin(plugin_info* plugin)
{
	plugin->plugin_type = PT_DATABASE_CONNECTION;
	
	strcpy(plugin->plugin_name, "sqlite3");
	strcpy(plugin->plugin_author, "Ulrich Voelkel");
	//strcpy(plugin->plugin_version, "");
	strcpy(plugin->library_version, sqlite3_libversion());
}

CDatabaseConnection* fuppes_plugin_create_db_connection(plugin_info* plugin ) // __attribute__((unused))
{
	return new CSQLiteConnection(plugin);
}


void unregister_fuppes_plugin(plugin_info* plugin ) // __attribute__((unused))
{
}
	
#ifdef __cplusplus
}
#endif
