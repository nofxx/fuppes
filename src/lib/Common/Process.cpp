/***************************************************************************
 *            Process.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "Process.h"

#include <iostream>
#include <string.h>
using namespace std;

void on_signal(int sig)
{
	pid_t pid;
	int status;
  
  while (1) {
    pid = waitpid (WAIT_ANY, &status, WNOHANG);
    
		if(pid < 0)
      break;
    if (pid == 0)
      break;
		
		CProcessMgr::signal(pid, status);
  }
}

CProcessMgr* CProcessMgr::m_instance = NULL;

CProcessMgr::CProcessMgr()
{
	fuppesThreadInitMutex(&m_mutex);
}

CProcessMgr::~CProcessMgr()
{
	fuppesThreadDestroyMutex(&m_mutex);
}

void CProcessMgr::init()
{
	if(m_instance == 0) {
		m_instance = new CProcessMgr();
	}
	
	::signal(SIGCHLD, &on_signal);
}

void CProcessMgr::signal(pid_t pid, int sig)
{
	CProcess* proc = m_instance->m_processes[pid];	
	proc->m_isRunning = false;
}
	
void CProcessMgr::register_proc(CProcess* proc)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	
	m_instance->m_processes[proc->pid()] = proc;
	
	fuppesThreadUnlockMutex(&m_instance->m_mutex);
}

void CProcessMgr::unregister_proc(CProcess* proc)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	
	m_instance->m_processesIter = m_instance->m_processes.find(proc->pid());      
  if(m_instance->m_processesIter != m_instance->m_processes.end()) { 
		m_instance->m_processes.erase(proc->pid());
	}		
	
	fuppesThreadUnlockMutex(&m_instance->m_mutex);
}



CProcess::CProcess()
{
	m_isRunning = false;
}

CProcess::~CProcess()
{
	CProcessMgr::unregister_proc(this);

	if(m_isRunning) {
		stop();
	}
	
	for(int i = 0; i < m_numArgs; i++) {
		free((void*)m_args[i]);
	}
	free(m_args);
}

bool CProcess::start(std::string cmd)
{
#ifndef WIN32
	m_pid =	fork();
	
	// child process
	if(m_pid == 0) {
		parseArgs(cmd);
		execv(m_args[0], (char**)m_args);
		cout << "error execv" << endl;
    _exit(-1);
  }
	// parent process
	else if(m_pid > 0) {
		
		CProcessMgr::register_proc(this);
    return true;
  }
	// fork() error
	else {
		cout << "fork() failed" << endl;
		return false;
  }
#else
	return false;
#endif
}

void CProcess::stop()
{
	m_isRunning = false;
}

#define MAX_ARGS 31

void CProcess::parseArgs(std::string cmd)
{		
	string arg;
	string tmp;
	string::size_type pos;
	
	bool escape = false;
	bool quote = false;
	
	m_numArgs = 0;
	m_args = (char const**)malloc(MAX_ARGS + 1 * sizeof(*m_args));
	
	cmd += " ";
	
	while(!cmd.empty()) {
		
		if((pos = cmd.find(" ", 0)) == string::npos)
			break;
		
		tmp = cmd.substr(0, pos);
		if(tmp.compare("%in%") == 0) {
			tmp = m_inFile;
		}
		else if(tmp.compare("%out%") == 0) {
			tmp = m_outFile;
		}
			 
		if(!escape && !quote) {			
			arg = tmp;
		}

		cmd = cmd.substr(pos + 1);
		
		if(escape || quote) {
			arg = arg + " " + tmp;
			escape = false;
		}
		
		// quote start
		if(tmp.substr(0, 1).compare("\"") == 0) {
			quote = true;
			arg = arg.substr(1);
		}
		// quote end
		else if(tmp.substr(tmp.length()-1, 1).compare("\"") == 0) {
			quote = false;
			arg = arg.substr(0, arg.length() - 1);
			
			m_args[m_numArgs] = (char*)malloc(arg.length() + 1 * sizeof(char*));
			strcpy((char*)m_args[m_numArgs++], arg.c_str());
		}
		// escape
		else if(tmp.substr(tmp.length()-1, 1).compare("\\") == 0) {
			escape = true;
			arg = arg.substr(0, arg.length() - 1);
		}
		else {
			m_args[m_numArgs] = (char*)malloc(arg.length() + 1 * sizeof(char*));
			strcpy((char*)m_args[m_numArgs++], arg.c_str());
		}
	}
	
	m_args[m_numArgs] = (char*)0;
}
