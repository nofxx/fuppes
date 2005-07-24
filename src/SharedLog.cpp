/***************************************************************************
 *            SharedLog.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
 
#include "SharedLog.h"
#include "Common.h"
#include <iostream>

//#define DISABLELOG

CSharedLog* CSharedLog::m_Instance = 0;

CSharedLog* CSharedLog::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedLog();
	return m_Instance;
}

CSharedLog::CSharedLog()
{
  #ifndef DISABLELOG
  
  #ifdef WIN32
  InitializeCriticalSectionAndSpinCount(&m_Mutex, 0x80000400);
  #else
  pthread_mutex_init(&m_Mutex, NULL);
  #endif
  
  #endif
}

void CSharedLog::Log(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG
  
  #ifdef WIN32
  EnterCriticalSection(&m_Mutex);
  #else
  pthread_mutex_lock(&m_Mutex);
  #endif
  
  std::cout << "[" << p_sSender << "] " << p_sMessage << std::endl;  
  fflush(stdout);
  
  #ifdef WIN32
  LeaveCriticalSection(&m_Mutex);
  #else
  pthread_mutex_unlock(&m_Mutex);
  #endif
  
  #endif
}

void CSharedLog::Log(std::string p_sSender, std::string p_asMessages[], unsigned int p_nCount, std::string p_sSeparator)
{
  #ifndef DISABLELOG
  
  #ifdef WIN32
  EnterCriticalSection(&m_Mutex);
  #else
  pthread_mutex_lock(&m_Mutex);
  #endif
  
  std::cout << "[" << p_sSender << "] ";
  for(unsigned int i = 0; i < p_nCount; i++)
  {
    std::cout << p_asMessages[i] << p_sSeparator;
  }
  std::cout  << std::endl;
  fflush(stdout);
  
  #ifdef WIN32
  LeaveCriticalSection(&m_Mutex);
  #else
  pthread_mutex_unlock(&m_Mutex);
  #endif  
  
  #endif
}

void CSharedLog::Error(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG
  
  #ifdef WIN32
  /* TODO: TS - win aequivalent einbauen */
  #else
  pthread_mutex_lock(&m_Mutex);
  #endif
  
  std::cout << "[ERROR :: " << p_sSender << "] " << p_sMessage << std::endl;  
  fflush(stdout);

  #ifdef WIN32
  /* TODO: TS - win aequivalent einbauen */
  #else
  pthread_mutex_unlock(&m_Mutex);
  #endif
  
  #endif
}
