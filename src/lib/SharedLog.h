/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            SharedLog.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _SHAREDLOG_H
#define _SHAREDLOG_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "Common/Thread.h"
#include <string>
#include <stdarg.h>
#include <iostream>
#include <fstream>
#include <list>

#define L_NORM 1
#define L_EXT  2
#define L_DBG  3

class CSharedLog;

namespace fuppes {

	class Exception;
	
  class Log {

		public:

			enum Sender {
				unknown,
				http,
        soap,
        gena,
				ssdp,
				fam,
				plugin
			};
		
		  enum Level {
		    none			= 0,
		    normal		= 1,
		    extended	= 2,
		    debug			= 3
		  };

			static std::string senderToString(Log::Sender sender);
			static Log::Sender stringToSender(std::string sender);
      
      static void init();
      static void uninit();
      
		  static void log(Log::Sender sender, 
		    Log::Level level, 
		    const std::string fileName, 
		    int lineNo,
		    const char* format,
		    ...);
			
			static void log(Log::Sender sender, 
		    Log::Level level, 
		    const std::string fileName, 
		    int lineNo,
		    const std::string msg);

      static bool isActiveSender(Log::Sender sender) {
        std::list<Log::Sender>::iterator iter;
        for(iter = m_instance->m_logSenders.begin(); 
            iter != m_instance->m_logSenders.end(); 
            iter++) {
          if(*iter == sender)
            return true;
        }
        return false;
      }

      static void addActiveSender(Log::Sender sender) {
        if(sender == Log::unknown || isActiveSender(sender))
          return;
        m_instance->m_logSenders.push_back(sender);
      }

      static void removeActiveSender(Log::Sender sender) {
        if(!isActiveSender(sender))
          return;
        
        m_instance->m_logSenders.remove(sender);
      }

    private:
      static Log* m_instance;
      std::list<Log::Sender> m_logSenders;
  };

}


class CSharedLog
{

private:
  CSharedLog();  


public:
  ~CSharedLog();

  static CSharedLog* Shared();
  //void SetUseSyslog(bool p_bUseSyslog);
  static bool SetLogFileName(std::string p_sLogFileName);
  
  // set callback functions
  void SetCallback(void(*p_log_cb)(const char* sz_log)) { m_log_cb = p_log_cb; }
  void SetErrorCallback(void(*p_err_cb)(const char* sz_err)) { m_err_cb = p_err_cb; }
  void SetNotifyCallback(void(*p_notify_cb)(const char* sz_title, const char* sz_msg)) { m_notify_cb = p_notify_cb; }
  void SetUserInputCallback(void(p_user_input_cb)(const char* sz_msg, char* sz_result, unsigned int n_buffer_size)) { m_user_input_cb = p_user_input_cb; }
  

  /**
   *  use this for error messages that MUST be shown to the user
   */
  void UserError(std::string p_sErrMsg);
  void UserNotify(std::string p_sTitle, std::string p_sNotifyMsg);
  
  std::string UserInput(std::string p_sMessage);
  
  // deprecated
  //void  Log(std::string p_sSender, std::string p_sMessage);
  //void  ExtendedLog(std::string p_sSender, std::string p_sMessage);
  //void  DebugLog(std::string p_sSender, std::string p_sMessage);
  //void  Log(std::string p_sSender, std::string p_asMessages[], unsigned int p_nCount, std::string p_sSeparator = "");    
  //void  Warning(std::string p_sSender, std::string p_sMessage);
  //void  Critical(std::string p_sSender, std::string p_sMessage);
  //void  Error(std::string p_sSender, std::string p_sMessage);

  
  void  Log(int nLogLevel, std::string p_sMessage, char* p_szFileName, int p_nLineNumber);
  //void  Syslog(int nLogLevel, std::string p_sMessage, char* p_szFileName, int p_nLineNumber);

		
	static void Log(int p_nLogLevel, const std::string p_sFileName, int p_nLineNumber, const std::string msg);
	static void Log(int p_nLogLevel, const std::string p_sFileName, int p_nLineNumber, const char* p_szFormat, ...);
	static void LogArgs(int p_nLogLevel, const std::string p_sFileName, int p_nLineNumber, const char* p_szFormat, va_list args);
	static void Log(int p_nLogLevel, fuppes::Exception exception);
	static void Print(const char* p_szFormat, ...);
		
  /** set the level of log verbosity
   *  @param  p_nLogLevel  0 = no log, 1 = std log, 2 = extended log, 3 = debug log
   */
  void  SetLogLevel(int p_nLogLevel, bool p_bPrintLogLevel = true);  
  std::string GetLogLevel();
  
  void  ToggleLog();
  


private:
	
  static CSharedLog*  m_Instance;
  //fuppesThreadMutex   m_Mutex;
	fuppes::Mutex				m_mutex;
  bool                m_bShowLog;
  bool                m_bShowExtendedLog;
  bool                m_bShowDebugLog;
  int                 m_nLogLevel;
  static std::string  m_sLogFileName;
  static std::ofstream*  m_fsLogFile;
  
  void(*m_log_cb)(const char* sz_log);
  void(*m_err_cb)(const char* sz_err);
  void(*m_notify_cb)(const char* sz_title, const char* sz_msg);
  void(*m_user_input_cb)(const char* sz_msg, char* sz_result, unsigned int n_buffer_size);
};

#endif // _SHAREDLOG_H 
