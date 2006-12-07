/***************************************************************************
 *            SharedLog.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _SHAREDLOG_H
#define _SHAREDLOG_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "Common/Common.h"
#include <string>

/*===============================================================================
 CLASS CMessageBase
===============================================================================*/

class CSharedLog
{

/* <PRIVATE> */

private:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  CSharedLog();  

/* <\PRIVATE> */

/* <PUBLIC> */

public:
  ~CSharedLog();
/*===============================================================================
 INSTANCE
===============================================================================*/

  static CSharedLog* Shared();

/*===============================================================================
 LOGGING
===============================================================================*/
  
  void  Log(std::string p_sSender, std::string p_sMessage);
  void  ExtendedLog(std::string p_sSender, std::string p_sMessage);
  void  DebugLog(std::string p_sSender, std::string p_sMessage);
  void  Log(std::string p_sSender, std::string p_asMessages[], unsigned int p_nCount, std::string p_sSeparator = "");    
  void  Warning(std::string p_sSender, std::string p_sMessage);
  void  Critical(std::string p_sSender, std::string p_sMessage);
  void  Error(std::string p_sSender, std::string p_sMessage);

/*===============================================================================
 SET
===============================================================================*/  
  /** set the level of log verbosity
   *  @param  p_nLogLevel  0 = no log, 1 = std log, 2 = extended log, 3 = debug log
   */
  void  SetLogLevel(int p_nLogLevel, bool p_bPrintLogLevel = true);  
  std::string GetLogLevel();
  
  void  ToggleLog();
  
/* <\PUBLIC> */
    
/* <PRIVATE> */

private:
/*===============================================================================
 MEMBERS
===============================================================================*/
		
  static CSharedLog* m_Instance;
  fuppesThreadMutex  m_Mutex;
  bool               m_bShowLog;
  bool               m_bShowExtendedLog;
  bool               m_bShowDebugLog;
  int                m_nLogLevel;

/* <\PRIVATE> */

};

#endif /* _SHAREDLOG_H */
