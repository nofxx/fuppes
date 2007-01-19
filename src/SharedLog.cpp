/***************************************************************************
 *            SharedLog.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "SharedLog.h"
#include <iostream>
#include <sstream>

#ifdef WIN32
  #undef HAVE_SYSLOG_H
#else
  #include "config.h"
  #ifdef HAVE_SYSLOG_H
  #include <syslog.h>
  #endif
#endif

using namespace std;

CSharedLog* CSharedLog::m_Instance = 0;

CSharedLog* CSharedLog::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedLog();
	return m_Instance;
}


CSharedLog::CSharedLog()
{
  m_bUseSyslog = false;

  SetLogLevel(1, false);
  #ifndef DISABLELOG
  fuppesThreadInitMutex(&m_Mutex);  
  #endif
}

CSharedLog::~CSharedLog()
{
  #ifdef HAVE_SYSLOG_H
  if(m_bUseSyslog)
    closelog();
  #endif
  
  #ifndef DISABLELOG
  fuppesThreadDestroyMutex(&m_Mutex);
  #endif
}

void CSharedLog::SetUseSyslog(bool p_bUseSyslog)
{
  #ifdef HAVE_SYSLOG_H
  m_bUseSyslog = p_bUseSyslog;
    
  if(m_bUseSyslog)
    openlog("fuppes", 0, LOG_USER);
  else
    closelog();
  #else
  m_bUseSyslog = false;
  #endif
}


void CSharedLog::SetLogLevel(int p_nLogLevel, bool p_bPrintLogLevel)
{
  m_bShowLog         = false;
  m_bShowExtendedLog = false;
  m_bShowDebugLog    = false;

  m_nLogLevel = p_nLogLevel;  
  switch(m_nLogLevel)
  {
    case 0:
      if(p_bPrintLogLevel)
        std::cout << "log-level: 0 (disabled)" << std::endl;
      break;    
    case 1:
      m_bShowLog = true;
      if(p_bPrintLogLevel)
        std::cout << "log-level: 1 (normal)" << std::endl;
      break;
    case 2:
      m_bShowLog         = true;
      m_bShowExtendedLog = true;
      if(p_bPrintLogLevel)
        std::cout << "log-level: 2 (extended)" << std::endl;
      break;
    case 3:
      m_bShowLog         = true;
      m_bShowExtendedLog = true;
      m_bShowDebugLog    = true;
      if(p_bPrintLogLevel)
        std::cout << "log-level: 3 (debug)" << std::endl;
      break;
    default:
      break;
  }

}

std::string CSharedLog::GetLogLevel()
{
  std::string sResult;
  
  switch(m_nLogLevel)
  {
    case 0:      
      sResult = "0 (disabled)";
      break;    
    case 1:
      sResult = "1 (normal)";
      break;
    case 2:
      sResult = "2 (extended)";
      break;
    case 3:
      sResult = "3 (debug)";
      break;
    default:
      break;
  }  
  
  return sResult;
}

void CSharedLog::ToggleLog()
{
  if(m_nLogLevel < 3)
    m_nLogLevel++;
  else
    m_nLogLevel = 0;
  
  SetLogLevel(m_nLogLevel);
}

/*===============================================================================
 LOGGING
===============================================================================*/

void CSharedLog::Log(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG  
  if(m_bShowLog)
  {
    //fuppesThreadLockMutex(&m_Mutex);
    stringstream sLog;
    sLog << "[" << p_sSender << "] " << p_sMessage << std::endl;
    #ifdef USE_SYSLOG
    syslog(LOG_INFO, sLog.str().c_str());
    #else
    cout << sLog.str() << endl;
    fflush(stdout);
    #endif
    //fuppesThreadUnlockMutex(&m_Mutex);    
  }  
  #endif
}

void CSharedLog::ExtendedLog(std::string p_sSender, std::string p_sMessage)
{
  if(m_bShowExtendedLog)
    this->Log(p_sSender, p_sMessage);
}

void CSharedLog::DebugLog(std::string p_sSender, std::string p_sMessage)
{
  if(m_bShowDebugLog)
    this->Log(p_sSender, p_sMessage);
}

void CSharedLog::Log(std::string p_sSender, std::string p_asMessages[], unsigned int p_nCount, std::string p_sSeparator)
{
  #ifndef DISABLELOG  
  if(m_bShowLog)
  {
    fuppesThreadLockMutex(&m_Mutex);
    
    std::cout << "[" << p_sSender << "] ";
    for(unsigned int i = 0; i < p_nCount; i++)
    {
      std::cout << p_asMessages[i] << p_sSeparator;
    }
    std::cout  << std::endl;
    fflush(stdout);
    
    fuppesThreadUnlockMutex(&m_Mutex);
  }
  #endif
}

void CSharedLog::Warning(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG  
  if(m_bShowLog)
  {
    /*fuppesThreadLockMutex(&m_Mutex);    
    std::cout << "[WARNING :: " << p_sSender << "] " << p_sMessage << std::endl;  
    fflush(stdout);  
    fuppesThreadUnlockMutex(&m_Mutex);*/
    
    stringstream sLog;
    sLog << "[WARNING :: " << p_sSender << "] " << p_sMessage << std::endl;
    #ifdef USE_SYSLOG
    syslog(LOG_WARNING, sLog.str().c_str());
    #else
    cout << sLog.str() << endl;
    fflush(stdout);  
    #endif    
    
  }
  #endif
}

void CSharedLog::Critical(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG  
  if(m_bShowExtendedLog)
  {
    fuppesThreadLockMutex(&m_Mutex);    
    std::cout << "[CRITICAL :: " << p_sSender << "] " << p_sMessage << std::endl;  
    fflush(stdout);  
    fuppesThreadUnlockMutex(&m_Mutex);
  }
  #endif
}

void CSharedLog::Error(std::string p_sSender, std::string p_sMessage)
{
  #ifndef DISABLELOG  
  if (m_bShowExtendedLog)
  {
    fuppesThreadLockMutex(&m_Mutex);    
    std::cout << "[ERROR :: " << p_sSender << "] " << p_sMessage << std::endl;  
    fflush(stdout);  
    fuppesThreadUnlockMutex(&m_Mutex);
  }
  #endif
}

void CSharedLog::Log(int nLogLevel, std::string p_sMessage, char* p_szFileName, int p_nLineNumber, bool p_bPrintLine)
{
  #ifdef DISABLELOG
  return;
  #endif
  
  // printLine is always true on debug
  if(m_nLogLevel == 3)
    p_bPrintLine = true;
  
  switch(nLogLevel)
  {
    case L_NORMAL:    
      cout << p_sMessage << endl;
      break;
    
    // ERROR
    case L_ERROR:
      if(p_bPrintLine) {
        cout << "==== " << p_szFileName << " " << p_nLineNumber << " ====" << endl;
      }
      
      cout << p_sMessage << endl;
      
      if(p_bPrintLine) {
        cout << endl;
      }
      break;
    
      // WARNING
    case L_WARNING:
      cout << p_sMessage << endl;
      break;
    case L_CRITICAL:
      cout << p_sMessage << endl;
      break;
    
    // EXTENDED
    case L_EXTENDED:      
    case L_EXTENDED_ERR:      
    case L_EXTENDED_WARN:      
    
      if(m_nLogLevel < 2)
        break;
      
      if(p_bPrintLine) {
        cout << "==== ";
        if(nLogLevel == L_EXTENDED_ERR)
          cout << "ERROR ===";
        else if(nLogLevel == L_EXTENDED_WARN)
          cout << "WARNING ===";      
        cout << p_szFileName << " " << p_nLineNumber << " ====" << endl;
      }
      
      cout << p_sMessage << endl;
      
      if(p_bPrintLine) {
        cout << endl;
      }      
      break;
      
    // DEBUG
    case L_DEBUG:
      if(m_nLogLevel < 3)
        break;     
      
      if(p_bPrintLine) {
        cout << "==== " << p_szFileName << " " << p_nLineNumber << " ====" << endl;
      }
      
      cout << p_sMessage << endl;
      
      if(p_bPrintLine) {
        cout << endl;
      }
      break;
  }
  
  #ifdef HAVE_SYSLOG_H
  std::stringstream sLog;
  
  if(m_bUseSyslog)
  {  
    switch(nLogLevel)
    {
      case L_NORMAL:    
        syslog(LOG_INFO, p_sMessage.c_str());
        break;    
      case L_ERROR:      
        syslog(LOG_ERR, p_sMessage.c_str());
        break;
      case L_WARNING:
        syslog(LOG_WARNING, p_sMessage.c_str());
        break;
      case L_CRITICAL:
        syslog(LOG_WARNING, p_sMessage.c_str());
        break;
      case L_EXTENDED:
      case L_EXTENDED_ERR:
      case L_EXTENDED_WARN:
        syslog(LOG_DEBUG, p_sMessage.c_str());
        break;
      case L_DEBUG:
        syslog(LOG_DEBUG, p_sMessage.c_str());
        break;
    }
  }  
  #endif 
  
}

void CSharedLog::Syslog(int nLogLevel, std::string p_sMessage, char* p_szFileName, int p_nLineNumber)
{
  #ifdef HAVE_SYSLOG_H 
  if(m_bUseSyslog)
  { 
    Log(nLogLevel, p_sMessage, p_szFileName, p_nLineNumber);
  }
  #endif
}

/* <\PUBLIC> */
