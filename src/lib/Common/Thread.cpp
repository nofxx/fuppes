/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Thread.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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


#include "Thread.h"
//#include "UUID.h"
#include "../SharedLog.h"

#include <errno.h>
#ifdef HAVE_CLOCK_GETTIME
#include <time.h>
#else
#include <sys/time.h>
#endif

using namespace fuppes;

Mutex::Mutex() 
{
	m_locked = false;
	
  #ifdef WIN32
  InitializeCriticalSection(&m_mutex);
  #else
  pthread_mutex_init(&m_mutex, NULL);
  #endif
}

Mutex::~Mutex() 
{
	if(m_locked)
		CSharedLog::Log(L_NORM, __FILE__, __LINE__, "WARNING: destroying locked mutex.");

	#ifdef WIN32
  DeleteCriticalSection(&m_mutex);
  #else
  pthread_mutex_destroy(&m_mutex);
  #endif
}

void Mutex::lock() 
{
	#ifdef WIN32
  EnterCriticalSection(&m_mutex);
  #else
  pthread_mutex_lock(&m_mutex);
  #endif
	m_locked = true;
}

void Mutex::unlock() 
{
  #ifdef WIN32
  LeaveCriticalSection(&m_mutex);
  #else
  pthread_mutex_unlock(&m_mutex);
  #endif
	m_locked = false;
}





Thread::Thread(std::string name /*= ""*/) 
{
	m_name = name; // + " " + GenerateUUID();
	m_handle = NULL;
	m_running = false;
	m_finished = false;
	m_stop = false;

#ifndef WIN32
	pthread_cond_init(&m_exitCondition, NULL);
  pthread_mutex_init(&m_mutex, NULL);
#endif
}

Thread::~Thread() 
{	

	if(!close()) {
		CSharedLog::Log(L_NORM, __FILE__, __LINE__, "error closing thread %s", m_name.c_str());
	}
		 
	if(m_handle) {
		CSharedLog::Log(L_NORM, __FILE__, __LINE__, "WARNING: destroying an unfinished thread %s", m_name.c_str());
	}

#ifndef WIN32
  pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_exitCondition);
#endif
}

	
bool Thread::start(void* arg /* = NULL*/)
{
	if(m_running)
		return false;
	
  m_arg = arg;
	m_stop = false;
	
#ifdef WIN32
	m_handle = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&Thread::threadFunc, this, 0, NULL);
	m_running = (m_handle != NULL);
#else 
	int ret = pthread_create(&m_handle, NULL, &Thread::threadFunc, this);
  m_running = (0 == ret);
#endif

	return m_running;	
}


bool Thread::close()
{
	//CSharedLog::Log(L_NORM, __FILE__, __LINE__, "close thread %s", m_name.c_str());
	
	if(!m_handle)
		return true;
	
	m_stop = true;
  bool result = false;
	
#ifdef WIN32
  DWORD nErrNo = WaitForSingleObject(m_handle, 5000);
  switch(nErrNo) {
    case WAIT_ABANDONED:
      /*cout << "WAIT_ABANDONED :: " << nErrNo << endl;*/
      break;
    case WAIT_OBJECT_0:
      //cout << "WAIT_OBJECT_0 :: " << nErrNo << endl;
      CloseHandle(m_handle);
      result = true;
      break;
    case WAIT_TIMEOUT:
      /*cout << "fuppesThreadClose() :: WAIT_TIMEOUT (" << nErrNo << ")" << endl; */
      break;
    case WAIT_FAILED:
      /*cout << "fuppesThreadClose() :: WAIT_FAILED (" << nErrNo << ")" << endl;*/
      break;
    default:
      /*cout << "fuppesThreadClose - DEFAULT :: " << nErrNo << endl; */
      break;            
  }
#else    
  result = true;
  int err = 0;

	pthread_mutex_lock(&m_mutex);
	if(!m_finished) {
	

		timespec timeout;
#ifdef HAVE_CLOCK_GETTIME
		clock_gettime(CLOCK_REALTIME, &timeout);		
		timeout.tv_sec += 5;
#else
		timeval time;
		gettimeofday(&time, NULL);
		timeout.tv_sec = time.tv_sec + 5;
#endif
		
		err = pthread_cond_timedwait(&m_exitCondition, &m_mutex, &timeout);
		if(err == ETIMEDOUT && !m_finished) {
			CSharedLog::Log(L_NORM, __FILE__, __LINE__, "FATAL ERROR: pthread_cond_timedwait failed on thread %s", m_name.c_str());
			result = false;
		}
		
	}
	pthread_mutex_unlock(&m_mutex);
	
	err = pthread_join(m_handle, NULL);  
  if (err != 0) {
    result = false;
    /*switch(nErrNo) {
      case EINVAL:
        cout << "pthread_join() :: " << nErrNo << " EINVAL = handle does not refer to a joinable thread" << endl;      
        break;
      case ESRCH:
        cout << "pthread_join() :: " << nErrNo << " ESRCH = No thread found with the given thread handle" << endl;
        break;
      case EDEADLK:
        cout << "pthread_join() :: " << nErrNo << " EDEADLK = deadlock detected" << endl;      
        break;
    }*/
  }
#endif

	m_handle = NULL;
  return result;
}

#ifdef WIN32
DWORD Thread::threadFunc(void* thread)
#else
void* Thread::threadFunc(void* thread)
#endif
{
  Thread* pt = (Thread*)thread;
	pt->run();
	
#ifdef WIN32
	pt->m_running = false;
	pt->m_finished = true;
	ExitThread(0);
	return 0;
#else
	pthread_mutex_lock(&pt->m_mutex);
	pt->m_running = false;
	pt->m_finished = true;
	
	pthread_cond_signal(&pt->m_exitCondition);
	pthread_mutex_unlock(&pt->m_mutex);
	pthread_exit(NULL);
#endif
}
