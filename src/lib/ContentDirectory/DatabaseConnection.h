/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            DatabaseConnection.h
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

#ifndef _DATABASECONNECTION_H
#define _DATABASECONNECTION_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../../../include/fuppes_db_connection_plugin.h"


typedef ISQLQuery CSQLQuery;

class SQLQuery
{
	public:
		SQLQuery(CDatabaseConnection* connection = NULL);
		~SQLQuery();

    std::string build(fuppes_sql_no queryNo, std::string objectId, std::string device = "");
    std::string build(fuppes_sql_no queryNo, unsigned int objectId, std::string device = "");
    
		bool select(const std::string sql = "");
    bool exec(const std::string sql = "");
		fuppes_off_t insert(const std::string sql = "");

    void prepare(const std::string sql);
    void bind(const std::string field, const std::string value);

    static std::string escape(std::string value);
    
    bool eof();
		void next();
		CSQLResult* result();
		fuppes_off_t lastInsertId();
		void clear();
		
    CDatabaseConnection* connection();

   unsigned int size();
    
	private:
		ISQLQuery*	m_query;
};

class CDatabase
{
	public:
    /**
     * establish database connection
     */
		static bool connect(const CConnectionParams params);
    /**
     * 
     */
 		static bool setup();
    /**
     * close database connection
     */
		static void close();

		static ISQLQuery* query();
		static CDatabaseConnection* connection(bool create = false);

    static CConnectionParams connectionParams();
    
	private:
		static CDatabaseConnection* m_connection;
    //static CConnectionParams    m_connectionParams;    
};

#endif // _DATABASECONNECTION_H
