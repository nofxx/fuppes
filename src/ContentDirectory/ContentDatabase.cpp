/***************************************************************************
 *            ContentDatabase.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "../SharedLog.h"
 
#include <sstream>
#include <string>
#include <stdio.h>
#include <iostream>
 
using namespace std;

const string LOGNAME = "ContentDatabase";
 
static int SelectCallback(void *pDatabase, int argc, char **argv, char **azColName)
{
  /*for(int i = 0; i<argc; i++){
    printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
  }*/
    
  /* build new result set */  
  ((CContentDatabase*)pDatabase)->m_nRowsReturned++;
  
  CSelectResult* pResult = new CSelectResult();
  for(int i = 0; i < argc; i++)
  {
    string sFieldName = azColName[i];
    pResult->m_FieldValues[sFieldName] = argv[i] ? argv[i] : "NULL";        
  }  
  ((CContentDatabase*)pDatabase)->m_ResultList.push_back(pResult);
    
  /* select first entry */
  ((CContentDatabase*)pDatabase)->m_ResultListIterator = ((CContentDatabase*)pDatabase)->m_ResultList.begin();

  return 0;
}


CContentDatabase* CContentDatabase::m_Instance = 0;

CContentDatabase* CContentDatabase::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CContentDatabase();
	return m_Instance;
}

CContentDatabase::CContentDatabase()
{ 
  stringstream sDbFile;
  sDbFile << CSharedConfig::Shared()->GetConfigDir() << "fuppes.db";  
  m_sDbFileName = sDbFile.str();
  
  m_nRowsReturned = 0;
  
  fuppesThreadInitMutex(&m_Mutex);
}
 
CContentDatabase::~CContentDatabase()
{
  fuppesThreadDestroyMutex(&m_Mutex);
  ClearResult();
  sqlite3_close(m_pDbHandle);
}

std::string CContentDatabase::GetLibVersion()
{
  return sqlite3_libversion();
}

std::string CSelectResult::GetValue(std::string p_sFieldName)
{
  return m_FieldValues[p_sFieldName];
}

bool CContentDatabase::Init(bool* p_bIsNewDB)
{
  bool bIsNewDb = !FileExists(m_sDbFileName);
  *p_bIsNewDB = bIsNewDb;
  int nRes = sqlite3_open(m_sDbFileName.c_str(), &m_pDbHandle);   
  if(nRes)
  {
    fprintf(stderr, "Can't create/open database: %s\n", sqlite3_errmsg(m_pDbHandle));
    sqlite3_close(m_pDbHandle);
    return false;
  }
  
  if(bIsNewDb)
  {
    string sTableObjects = 
      "create table OBJECTS ("
      "  ID INTEGER PRIMARY KEY AUTOINCREMENT,"
      "  PARENT_ID INTEGER NOT NULL DEFAULT 0,"
      "  TYPE INTEGER NOT NULL,"
      "  PATH TEXT NOT NULL,"
      "  FILE_NAME TEXT DEFAULT NULL,"
      "  MD5 TEXT DEFAULT NULL,"
      "  MIME_TYPE TEXT DEFAULT NULL,"
      "  DETAILS TEXT DEFAULT NULL"
      ");";        
    
    if(Insert(sTableObjects) < 0)
      return false;
    if(Insert("CREATE INDEX IDX_FILE_NAME ON OBJECTS (FILE_NAME);") < 0)
      return false;
    if(Insert("CREATE INDEX IDX_PARENT_ID ON OBJECTS (PARENT_ID);") < 0)
      return false;
  }
  else
  {
    //Insert("delete from OBJECTS;");
  }  
  
  sqlite3_close(m_pDbHandle);
  return true;
}

void CContentDatabase::Lock()
{
  fuppesThreadLockMutex(&m_Mutex);
}

void CContentDatabase::Unlock()
{
  fuppesThreadUnlockMutex(&m_Mutex);
}

void CContentDatabase::ClearResult()
{
  /* clear old results */ 
  for(m_ResultListIterator = m_ResultList.begin(); m_ResultListIterator != m_ResultList.end();)
  {
    if(m_ResultList.empty())
      break;
    
    CSelectResult* pResult = *m_ResultListIterator;
    std::list<CSelectResult*>::iterator tmpIt = m_ResultListIterator;          
    ++tmpIt;
    m_ResultList.erase(m_ResultListIterator);
    m_ResultListIterator = tmpIt;
    delete pResult;
  } 
  
  m_ResultList.clear();
  m_nRowsReturned = 0;
}

bool CContentDatabase::Open()
{  
  if(sqlite3_open(m_sDbFileName.c_str(), &m_pDbHandle))
  {
    fprintf(stderr, "Can't create/open database: %s\n", sqlite3_errmsg(m_pDbHandle));
    sqlite3_close(m_pDbHandle);
    return false;
  }
  return true;
}

void CContentDatabase::Close()
{
  sqlite3_close(m_pDbHandle);
}

long long int CContentDatabase::Insert(std::string p_sStatement)
{
  Open();
  
  int nTrans = sqlite3_exec(m_pDbHandle, "BEGIN TRANSACTION;", NULL, NULL, NULL);
  if(nTrans != SQLITE_OK)
    cout << "error start transaction" << endl;
  
  char* szErr = 0;
  int nResult = sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), NULL, NULL, &szErr);  
  if(nResult != SQLITE_OK)
  {
    cout << szErr << endl;
    nResult = -1;
  }
  else  
  {
    nResult = sqlite3_last_insert_rowid(m_pDbHandle);
  }
  nTrans = sqlite3_exec(m_pDbHandle, "COMMIT;", NULL, NULL, NULL);
  if(nTrans != SQLITE_OK)
    cout << "error commit transaction" << endl;
  
  Close();
  return nResult;  
}

bool CContentDatabase::Select(std::string p_sStatement)
{  
  Open();  
  ClearResult();    
  bool bResult = true;
  
  char* szErr = 0;
  int nResult =  sqlite3_exec(m_pDbHandle, p_sStatement.c_str(), SelectCallback, this, &szErr);
  if(nResult != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", szErr);
    sqlite3_close(m_pDbHandle);    
    bResult = false;
  }
  
  Close();
  return bResult;
}

bool CContentDatabase::Eof()
{
  return (m_ResultListIterator == m_ResultList.end());
}
    
CSelectResult* CContentDatabase::GetResult()
{
  return *m_ResultListIterator;  
}

void CContentDatabase::Next()
{
  m_ResultListIterator++;
}
