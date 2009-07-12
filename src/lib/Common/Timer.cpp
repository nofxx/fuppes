/***************************************************************************
 *            Timer.cpp
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
 

#include "Timer.h"
#include "Common.h"

using namespace fuppes;

#include <string>
#include <iostream>
using namespace std;

Timer::Timer(ITimer* p_OnTimerHandler)
{
  m_pOnTimerHandler = p_OnTimerHandler;
  m_nTickCount      = 0;
}

Timer::~Timer()
{
  Stop();
}

void Timer::CallOnTimer()
{
  if(m_pOnTimerHandler != NULL)
    m_pOnTimerHandler->OnTimer(); 
}

void Timer::SetInterval(unsigned int p_nSeconds)
{
  m_nInterval = p_nSeconds;
}

void Timer::Start()
{
	start();
}

void Timer::Stop()
{
	stop();
}

void Timer::Reset()
{
	m_mutex.lock();
  m_nTickCount = 0;
	m_mutex.unlock();
}

unsigned int Timer::GetCount()
{
  return (m_nInterval - m_nTickCount);
}

void Timer::incTicCount() {
  m_mutex.lock();
	m_nTickCount++;
	m_mutex.unlock();
}

void Timer::run() {	

	while(!this->stopRequested()&& (this->m_nTickCount <= this->GetInterval())) {

		this->incTicCount();
    fuppesSleep(1000);    
    
    if(!this->stopRequested() && (this->m_nTickCount >= (this->GetInterval() - 1))) {
      this->CallOnTimer();
      this->Reset();
    }
  }

}
