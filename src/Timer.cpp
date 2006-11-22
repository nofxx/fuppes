/***************************************************************************
 *            Timer.cpp
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
 

#include "Timer.h"

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

fuppesThreadCallback TimerLoop(void *arg);
fuppesThreadMutex TimerMutex;

CTimer::CTimer(ITimer* p_OnTimerHandler)
{
  m_pOnTimerHandler = p_OnTimerHandler;
  m_TimerThread     = (fuppesThread)NULL;
  m_nTickCount      = 0;
  m_bDoBreak        = false;
  
  fuppesThreadInitMutex(&TimerMutex);
}

CTimer::~CTimer()
{
  Stop();
  Cleanup();  
  
  fuppesThreadDestroyMutex(&TimerMutex);
}
  
void CTimer::Cleanup()
{
  if(m_TimerThread)
  {
    fuppesThreadClose(m_TimerThread);   
    m_TimerThread = (fuppesThread)NULL;
  }
}

void CTimer::CallOnTimer()
{
  if(m_pOnTimerHandler != NULL)
    m_pOnTimerHandler->OnTimer(); 
}

void CTimer::SetInterval(unsigned int p_nSeconds)
{
  m_nInterval = p_nSeconds;
}

void CTimer::Start()
{
  void Cleanup(); 
  m_bDoBreak = false;  
  fuppesThreadStart(m_TimerThread, TimerLoop);  
}

void CTimer::Stop()
{
  m_bDoBreak = true; 
}

void CTimer::Reset()
{
  fuppesThreadLockMutex(&TimerMutex);
  m_nTickCount = 0;
  fuppesThreadUnlockMutex(&TimerMutex);
}

unsigned int CTimer::GetCount()
{
  unsigned int nResult = 0;  
  fuppesThreadLockMutex(&TimerMutex);
  nResult = m_nInterval - m_nTickCount;
  fuppesThreadUnlockMutex(&TimerMutex);
  return nResult;
}

fuppesThreadCallback TimerLoop(void *arg)
{
  CTimer* pTimer = (CTimer*)arg;   
  
  while(!pTimer->m_bDoBreak && (pTimer->m_nTickCount <= pTimer->GetInterval()))
  {
    if (pTimer->m_bDoBreak)
      break;
                          
    fuppesThreadLockMutex(&TimerMutex);
    pTimer->m_nTickCount++;
    fuppesThreadUnlockMutex(&TimerMutex);
    fuppesSleep(1000);    
    
    if(!pTimer->m_bDoBreak && (pTimer->m_nTickCount >= (pTimer->GetInterval() - 1)))
    {
      pTimer->CallOnTimer();
      fuppesThreadLockMutex(&TimerMutex);
      pTimer->m_nTickCount = 0;
      fuppesThreadUnlockMutex(&TimerMutex);
    }
  }  
  
  /* exit thread */
  fuppesThreadExit();
}
