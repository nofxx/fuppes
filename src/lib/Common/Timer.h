/***************************************************************************
 *            Timer.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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
 
#ifndef _TIMER_H
#define _TIMER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "Common.h"

class ITimer
{
  public:
    virtual void OnTimer() = 0;
    virtual ~ITimer() {}; 
};

class CTimer
{
  public:
    CTimer(ITimer* p_OnTimerHandler);
    ~CTimer();
  
    void CallOnTimer();
    void SetInterval(unsigned int p_nSeconds);
    unsigned int GetInterval() { return m_nInterval; }
    void Start();
    void Stop();
    void Reset();
  
    unsigned int GetCount();
  
    unsigned int m_nTickCount;    
    bool         m_bDoBreak;    
    
    void Lock();
    void Unlock();
    
  private:
    fuppesThread      m_TimerThread;
    fuppesThreadMutex m_TimerMutex;
    bool              bMutexInitialized;    
        
    ITimer*       m_pOnTimerHandler;
    unsigned int  m_nInterval;        
    void          Cleanup();
};

#endif // _TIMER_H
