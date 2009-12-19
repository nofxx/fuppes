/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Thread.h
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

#ifndef _THREAD_H
#define _THREAD_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#ifdef WIN32
//#pragma comment(lib,"shlwapi.lib")
#include <shlwapi.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#include <string>

namespace fuppes {

class Mutex
{
	public:
		Mutex();
		~Mutex();

		void lock();
		void unlock();

		bool locked() { return m_locked; }
		
	private:
		#ifdef WIN32
		CRITICAL_SECTION  m_mutex;
		#else
		pthread_mutex_t   m_mutex;
		#endif
		bool m_locked;
};


class MutexLocker
{
	public:
		MutexLocker(Mutex* mutex) {
			m_mutex = mutex;
			m_mutex->lock();
		}
		~MutexLocker() {
			m_mutex->unlock();
		}

	private:
		Mutex* m_mutex;
};


class Thread
{
	public:
		Thread(std::string name = "");
		virtual ~Thread();

    bool start(void* arg = NULL);
		void stop() { m_stop = true; }
		bool close();

		bool running() { return m_running; }

		void name(std::string name) { m_name = name; }
		std::string name() { return m_name; }
		
  protected:
		virtual void run() { }
		bool stopRequested() { return m_stop; }
		
		#ifdef WIN32
    static DWORD threadFunc(void*);
    #else
    static void* threadFunc(void*);
    #endif

    void msleep(unsigned int milliseconds);
		
  private:

		std::string m_name;
    void* m_arg;
		bool	m_running;
		bool  m_finished;
		bool  m_stop;
    
	#ifdef WIN32
		HANDLE		m_handle;
	#else
		pthread_t				m_handle;
		pthread_cond_t	m_exitCondition;
		pthread_mutex_t m_mutex;
	#endif
    
};

}

#endif // _THREAD_H
