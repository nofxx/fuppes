/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            DatabaseConnection.cpp
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

#include "DatabaseConnection.h"
#include "../Plugins/Plugin.h"
#include "../Common/Common.h"
#include "../Common/Thread.h"
#include "../SharedLog.h"

#include <iostream>
using namespace std;

using namespace fuppes;

SQLQuery::SQLQuery()
{
	m_query = CDatabase::query();
}

SQLQuery::~SQLQuery()
{
	if(m_query)
		delete m_query;
}

bool SQLQuery::select(const std::string sql)
{
  if(!m_query)
    return false;
  return m_query->select(sql);
}

bool SQLQuery::exec(const std::string sql)
{
  if(!m_query)
    return false;
  return m_query->exec(sql);
}

fuppes_off_t SQLQuery::insert(const std::string sql)
{
  if(!m_query)
    return 0;
  return m_query->insert(sql);
}

bool SQLQuery::eof()
{
  if(!m_query)
    return false;
  return m_query->eof();
}

void SQLQuery::next()
{
  if(m_query)
    m_query->next();
}

CSQLResult* SQLQuery::result()
{
  if(!m_query)
    return NULL;
  return m_query->result();
}

fuppes_off_t SQLQuery::lastInsertId()
{
  if(!m_query)
    return 0;
  return m_query->lastInsertId();
}

void SQLQuery::clear()
{
  if(m_query)
    m_query->clear();
}
		
CDatabaseConnection* SQLQuery::connection()
{
  if(!m_query)
    return NULL;
  return m_query->connection();
}

unsigned int SQLQuery::size()
{
  if(m_query)
    return m_query->size();
  else
    return 0;
}



std::string SQLQuery::build(fuppes_sql_no queryNo, std::string objectId, std::string device /* = ""*/)
{
  if(!m_query)
    return "";

  string sql = connection()->getStatement(queryNo);

  //cout << "SQL: " << sql << endl;

  if(device.empty())
    device = "DEVICE is NULL";
  else
    device = "DEVICE = '" + device + "'";

  sql = StringReplace (sql, "%OBJECT_ID%", objectId);
  sql = StringReplace (sql, "%DEVICE%", device);
  
  //cout << "SQL: " << sql << endl;
  
  return sql;
}


std::string SQLQuery::build(fuppes_sql_no queryNo, unsigned int objectId, std::string device /*= ""*/)
{
  std::stringstream stream;
  stream << objectId;
  return build(queryNo, stream.str(), device);
}








CDatabaseConnection* CDatabase::m_connection = NULL;

static CConnectionParams m_connectionParams;
//static fuppesThreadMutex mutex;
static fuppes::Mutex mutex;

bool CDatabase::connect(const CConnectionParams params) // static
{
	if(!m_connection) {
	  CDatabasePlugin* plugin = CPluginMgr::databasePlugin(params.type);
	  if(!plugin) {
      Log::error(Log::plugin, Log::normal, __FILE__, __LINE__, "failed to initialize %s database plugin.", params.type.c_str());
		  return false;
    }
	
    m_connection = plugin->createConnection();
  }
  
	if(m_connection->connect(params)) {
		m_connectionParams = params;
		return true;
	}

	return false;
}

CConnectionParams CDatabase::connectionParams() // static
{
  return m_connectionParams;
}

bool CDatabase::setup() // static
{
  if(!m_connection) {
		return false;
	}

  return m_connection->setup();
}

void CDatabase::close() // static
{
	if(!m_connection) {
		return;
	}
	
	delete m_connection;
	m_connection = NULL;
}

ISQLQuery* CDatabase::query() // static
{	
	fuppes::MutexLocker locker(&mutex);

  if(!m_connection)
		return NULL;
	
	return m_connection->query();
}


CDatabaseConnection* CDatabase::connection(bool create /*= false*/) // static
{
	fuppes::MutexLocker locker(&mutex);
	
	if(!m_connection) {
		return NULL;
	}

	if(!create) {
		return m_connection;
	}
	
	CDatabasePlugin* plugin = CPluginMgr::databasePlugin(m_connectionParams.type);
	if(!plugin)
		return NULL;

	CDatabaseConnection* result = plugin->createConnection();
	if(!result->connect(m_connectionParams)) {
		delete result;
		return NULL;
	}
	
	return result;
}
