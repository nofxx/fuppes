/***************************************************************************
 *            SharedLog.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "SharedLog.h"
#include <iostream>

/*===============================================================================
 DEFINITIONS
===============================================================================*/

/* Define this to disable logging */
//#define DISABLELOG

/*===============================================================================
 CLASS CMessageBase
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 INSTANCE
===============================================================================*/

CSharedLog* CSharedLog::m_Instance = 0;

CSharedLog* CSharedLog::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedLog();
	return m_Instance;
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CSharedLog::CSharedLog()
{
  #ifndef DISABLELOG
  fuppesThreadInitMutex(&m_Mutex);
  #endif
}

/* <\PRIVATE> */

/* <PUBLIC> */

/*===============================================================================
 LOGGING
===============================================================================*/

void CSharedLog::Log(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG
  
  fuppesThreadLockMutex(&m_Mutex);
  
  std::cout << "[" << p_sSender << "] " << p_sMessage << std::endl;  
  fflush(stdout);
  
  fuppesThreadUnlockMutex(&m_Mutex);
  
  #endif
}

void CSharedLog::Log(std::string p_sSender, std::string p_asMessages[], unsigned int p_nCount, std::string p_sSeparator)
{
  #ifndef DISABLELOG
  
  fuppesThreadLockMutex(&m_Mutex);
  
  std::cout << "[" << p_sSender << "] ";
  for(unsigned int i = 0; i < p_nCount; i++)
  {
    std::cout << p_asMessages[i] << p_sSeparator;
  }
  std::cout  << std::endl;
  fflush(stdout);
  
  fuppesThreadUnlockMutex(&m_Mutex);
  
  #endif
}

void CSharedLog::Error(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG
  
  fuppesThreadLockMutex(&m_Mutex);
  
  std::cout << "[ERROR :: " << p_sSender << "] " << p_sMessage << std::endl;  
  fflush(stdout);

  fuppesThreadUnlockMutex(&m_Mutex);
  
  #endif
}

/* <\PUBLIC> */
