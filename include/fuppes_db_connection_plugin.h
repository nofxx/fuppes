/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            fuppes_db_connection_plugin.h
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

#ifndef _FUPPES_DB_PLUGIN_H
#define _FUPPES_DB_PLUGIN_H

#include <map>
#include <list>
#include <string>
#include <sys/types.h>
#include <stdlib.h>

#include <iostream>

class CDatabaseConnection;

class CSQLResult
{
	public:
		virtual ~CSQLResult() {	}
		
    virtual bool isNull(std::string fieldName) = 0;
		virtual std::string	asString(std::string fieldName) = 0;		
		virtual unsigned int asUInt(std::string fieldName) = 0;
		virtual int asInt(std::string fieldName) = 0;
		virtual CSQLResult* clone() = 0;
};

class CSQLQuery
{
	public:
		virtual ~CSQLQuery() { }

		virtual bool select(const std::string sql) = 0;
		virtual bool exec(const std::string sql) = 0;
		virtual off_t insert(const std::string sql) = 0;
		
		virtual bool eof() = 0;
		virtual void next() = 0;
		virtual CSQLResult* result() = 0;
		virtual off_t lastInsertId() = 0;
		virtual void clear() = 0;
		
		virtual CDatabaseConnection* connection() = 0;
};

struct CConnectionParams
{
	std::string type;
	
	std::string filename;

	std::string hostname;
	std::string username;
	std::string password;
	std::string dbname;
	
	CConnectionParams& operator=(const CConnectionParams& params) {
		type = params.type;
		filename = params.filename;
		hostname = params.hostname;
		username = params.username;
		password = params.password;
		dbname = params.dbname;
		return *this;
	}
};

class CDatabaseConnection
{
	public:
		virtual ~CDatabaseConnection() { }

		virtual CSQLQuery*			query() = 0;
		virtual bool open(const CConnectionParams params) = 0;

		virtual bool startTransaction() = 0;
		virtual bool commit() = 0;
		virtual void rollback() = 0;
};

#endif // _FUPPES_DB_PLUGIN_H
