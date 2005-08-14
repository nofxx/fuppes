/***************************************************************************
 *            Timer.cpp
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 

#include "Timer.h"

fuppesThreadCallback TimerLoop(void *arg);

unsigned int g_nTickCount;

CTimer::CTimer(ITimer* p_OnTimerHandler)
{
  m_pOnTimerHandler = p_OnTimerHandler;
  m_TimerThread     = (fuppesThread)NULL;
  g_nTickCount      = 0;
}

CTimer::~CTimer()
{
  if(m_TimerThread != (fuppesThread)NULL)
    fuppesThreadClose(m_TimerThread, 500);
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
  fuppesThreadStart(m_TimerThread, TimerLoop);
}

void CTimer::Stop()
{
  if(m_TimerThread != (fuppesThread)NULL)
    fuppesThreadClose(m_TimerThread, 500);
}

void CTimer::Reset()
{
  Stop();
  g_nTickCount = 0;
  Start();  
}

fuppesThreadCallback TimerLoop(void *arg)
{
  CTimer* pTimer = (CTimer*)arg;  
 
  while(g_nTickCount <= pTimer->GetInterval())
  {
    g_nTickCount++;
    upnpSleep(1000000);    
    
    if(g_nTickCount == pTimer->GetInterval() - 1)
    {
      pTimer->CallOnTimer();
      g_nTickCount = 0;
    }    
  }
  
  fuppesThreadExit(NULL);
}
