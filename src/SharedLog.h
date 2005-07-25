/***************************************************************************
 *            SharedLog.h
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
 
#ifndef _SHAREDLOG_H
#define _SHAREDLOG_H

#include "Common.h"
#include <string>

class CSharedLog
{
  public:
    static CSharedLog* Shared();
    void   Log(std::string p_sSender, std::string p_sMessage);
    void   Log(std::string p_sSender, std::string p_asMessages[], unsigned int p_nCount, std::string p_sSeparator = "");    
    void   Error(std::string p_sSender, std::string p_sMessage);
  
  private:
    CSharedLog();
		static CSharedLog* m_Instance;

    fuppesThreadMutex m_Mutex;
};

#endif /* _SHAREDLOG_H */
