/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            DatabaseCommection.cpp
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

#include "DatabaseConnection.h"
#include "../Plugins/Plugin.h"
#include "../Common/Common.h"

CDatabaseConnection* CDatabase::m_connection = NULL;

static CConnectionParams connectionParams;
static fuppesThreadMutex mutex;

bool CDatabase::init(const CConnectionParams params)
{
	if(m_connection)
		return true;
	
	CDatabasePlugin* plugin = CPluginMgr::databasePlugin(params.type);
	if(!plugin)
		return false;
	
	CDatabaseConnection* db = plugin->createConnection();
	if(db->open(params)) {
		m_connection = db;
		connectionParams = params;
		fuppesThreadInitMutex(&mutex);
		return true;
	}
	
	return false;
}

void CDatabase::close()
{
	if(!m_connection) {
		return;
	}
	
	delete m_connection;
	m_connection = NULL;
}

CSQLQuery* CDatabase::query()
{	
	MutexLocker locker(&mutex);
	
	if(!m_connection)
		return NULL;
	
	return m_connection->query();
}


CDatabaseConnection* CDatabase::connection(bool create /*= false*/)
{
	MutexLocker locker(&mutex);
	
	if(!m_connection) {
		return NULL;
	}

	if(!create) {
		return m_connection;
	}
	
	CDatabasePlugin* plugin = CPluginMgr::databasePlugin(connectionParams.type);
	if(!plugin)
		return NULL;
	
	CDatabaseConnection* result = plugin->createConnection();
	if(!result->open(connectionParams)) {
		delete result;
		return NULL;
	}
	
	return result;
}
