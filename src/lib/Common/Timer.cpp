/***************************************************************************
 *            Timer.cpp
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
 

#include "Timer.h"

#include <string>
#include <sstream>
#include <iostream>

using namespace std;

fuppesThreadCallback TimerLoop(void *arg);

CTimer::CTimer(ITimer* p_OnTimerHandler)
{
  m_pOnTimerHandler = p_OnTimerHandler;
  m_TimerThread     = (fuppesThread)NULL;
  m_nTickCount      = 0;
  m_bDoBreak        = false;
  
  fuppesThreadInitMutex(&m_TimerMutex);
}

CTimer::~CTimer()
{                
  Stop();
  Cleanup();  
  fuppesThreadDestroyMutex(&m_TimerMutex);
}
  
void CTimer::Cleanup()
{
  if(m_TimerThread) {
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
  fuppesThreadLockMutex(&m_TimerMutex);
  m_nTickCount = 0;
  fuppesThreadUnlockMutex(&m_TimerMutex);
}

unsigned int CTimer::GetCount()
{
  unsigned int nResult = 0;  
  nResult = m_nInterval - m_nTickCount;
  return nResult;
}

void CTimer::Lock()
{
  fuppesThreadLockMutex(&m_TimerMutex);     
}

void CTimer::Unlock()
{
  fuppesThreadUnlockMutex(&m_TimerMutex);
}

fuppesThreadCallback TimerLoop(void *arg)
{
  CTimer* pTimer = (CTimer*)arg;   
  
  while(!pTimer->m_bDoBreak && (pTimer->m_nTickCount <= pTimer->GetInterval()))
  {
    if (pTimer->m_bDoBreak)
      break;
                          
    pTimer->Lock();
    pTimer->m_nTickCount++;
    pTimer->Unlock();
    fuppesSleep(1000);    
    
    if(!pTimer->m_bDoBreak && (pTimer->m_nTickCount >= (pTimer->GetInterval() - 1)))
    {
      pTimer->CallOnTimer();
      pTimer->Reset();
    }
  }  

  fuppesThreadExit();
}
