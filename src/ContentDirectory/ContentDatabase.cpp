/***************************************************************************
 *            ContentDatabase.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#include "ContentDatabase.h"
#include "../SharedConfig.h"
 
#include <sstream>
#include <string>
#include <stdio.h>
 
using namespace std;
 
const string LOGNAME = "ContentDatabase";
 
static int callback(void *NotUsed, int argc, char **argv, char **azColName)
{
  int i;
  for(i=0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}
 
CContentDatabase::CContentDatabase()
{
  stringstream sDbFile;
  sDbFile << CSharedConfig::Shared()->GetConfigDir() << "fuppes.db";
  int rc = sqlite3_open(sDbFile.str().c_str(), &m_pDbHandle);   
   
  if(rc)
  {
   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(m_pDbHandle));
   sqlite3_close(m_pDbHandle);
  }
   
  /*rc = sqlite3_exec(db, argv[2], callback, 0, &zErrMsg);
  if( rc!=SQLITE_OK )
  {
    fprintf(stderr, "SQL error: %s\n", zErrMsg);
  }
  sqlite3_close(db);*/
}
 
CContentDatabase::~CContentDatabase()
{
  sqlite3_close(m_pDbHandle);
}
