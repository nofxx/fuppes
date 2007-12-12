/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            daemon_main.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "../include/fuppes.h"

#include <iostream>
#include <signal.h>
#include <termios.h>
//#include <fcntl.h>

using namespace std;

bool g_bExitApp;

void SignalHandler(int p_nSignal)
{  
  g_bExitApp = true;
}

int main(int argc, char* argv[])
{
  if(fuppes_init(argc, argv, NULL) != FUPPES_TRUE)
    return 1;

  pid_t pid;
  pid = fork();

  // error
  if (pid < 0) {
    cout << "could not create child process" << endl;
    return 1;
  }
  
  // parent process
  else if (pid > 0) {
    cout << "[started]" << endl;
    return 0;
  }
  
  // child process
  else if(pid == 0) {
    //cout << "child process" << endl;
    //close(STDIN_FILENO);
    //close(STDOUT_FILENO);
    //close(STDERR_FILENO);
  }
    
  // install signal handler
  signal(SIGINT, SignalHandler);  // ctrl-c
  signal(SIGTERM, SignalHandler); // start-stop-daemon -v --stop -nfuppes
  
  
  // start fuppes
  fuppes_start();
  
  while(!g_bExitApp) {
    usleep(100 * 1000);
  }
  
  fuppes_stop();
  fuppes_cleanup();  
  
  return 0;
}
