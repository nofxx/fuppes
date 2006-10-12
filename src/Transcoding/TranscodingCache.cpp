/***************************************************************************
 *            TranscodingCache.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net> 
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

#include "TranscodingCache.h"

CTranscodingCacheObject::CTranscodingCacheObject()
{
  m_nRefCount = 0;
    
  m_szBuffer    = NULL;
  m_nBufferSize = 0;  
  
  m_bIsTranscoding  = false;
  m_pTranscodingMgr = NULL;
  
  fuppesThreadInitMutex(&m_Mutex); 
}

CTranscodingCacheObject::~CTranscodingCacheObject()
{
  fuppesThreadDestroyMutex(&m_Mutex); 
  
  delete[] m_szBuffer;
}

void CTranscodingCacheObject::Retain()
{
  m_nRefCount++;
}

void CTranscodingCacheObject::Release()
{
  if(m_nRefCount > 0)
    m_nRefCount--;
}

bool CTranscodingCacheObject::IsReleased()
{
  return (m_nRefCount == 0);
}

void CTranscodingCacheObject::Lock()
{
  fuppesThreadLockMutex(&m_Mutex);
}

void CTranscodingCacheObject::Unlock()
{
  fuppesThreadUnlockMutex(&m_Mutex);
}



CTranscodingCache* CTranscodingCache::m_pInstance = 0;

CTranscodingCache* CTranscodingCache::Shared()
{
	if (m_pInstance == 0)
		m_pInstance = new CTranscodingCache();
	return m_pInstance;
}

CTranscodingCache::CTranscodingCache()
{
  fuppesThreadInitMutex(&m_Mutex); 
}

CTranscodingCache::~CTranscodingCache()
{
  fuppesThreadDestroyMutex(&m_Mutex);
}


CTranscodingCacheObject* CTranscodingCache::GetCacheObject(std::string p_sFileName)
{
  fuppesThreadLockMutex(&m_Mutex);  
  CTranscodingCacheObject* pResult = new CTranscodingCacheObject();
  pResult->Retain();
  
  
  fuppesThreadUnlockMutex(&m_Mutex);  
  return pResult;
}

void CTranscodingCache::ReleaseCacheObject(CTranscodingCacheObject* pCacheObj)
{
  fuppesThreadLockMutex(&m_Mutex);  
  
  delete pCacheObj;
  
  // clear cache
  
  fuppesThreadUnlockMutex(&m_Mutex);  
}
