/***************************************************************************
 *            main.cpp
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
#include <string>

#ifndef WIN32
#include <signal.h>
#include <termios.h>
#endif

using namespace std;

#ifndef WIN32
// non-blocking getchar()
struct termios savetty;

void unsetcbreak (void)
{
	tcsetattr(0, TCSADRAIN, &savetty);
}

void setcbreak (void)
{
  // set console to raw mode
  struct termios tty;
	tcgetattr(0, &savetty);
	tcgetattr(0, &tty);
	tty.c_lflag &= ~(ECHO|ECHONL|ICANON|IEXTEN);
	tty.c_cc[VTIME] = 0;
	tty.c_cc[VMIN] = 0;
	tcsetattr(0, TCSADRAIN, &tty);
}

int fuppes_getch (void)
{
  setcbreak();  
  static char line [2];
	if (read (0, line, 1)) {    
    unsetcbreak();    
		return line[0];
	}  
  unsetcbreak();  
	return -1;
}
#endif

void PrintParams()
{
  cout << endl;
  cout << "  --help" << endl;
  cout << "  --loglevel [none|normal|extended|debug]" << endl;
  cout << "  --config-dir /home/user/.fuppes/" << endl;  
  //cout << "  --daemon" << endl;
  cout << endl;
}

void PrintHelp()
{
  cout << endl;
  cout << "l = change log-level" << endl;
  cout << "    (disabled, normal, extended, debug) default is \"normal\"" << endl;
  cout << "i = print system info" << endl;
  cout << "s = print device settings" << endl;
  cout << "r = rebuild database" << endl;
  cout << "v = rebuild virtual container layout" << endl;
  //cout << "c = refresh configuration" << endl;
  cout << "h = print this help" << endl;
  cout << endl;
  cout << "m = send m-search" << endl;
  cout << "a = send notify-alive" << endl;
  cout << "b = send notify-byebye" << endl;
  cout << endl;

  #ifdef WIN32
  cout << "q = quit" << endl;
  #else
  cout << "ctrl-c or q = quit" << endl;
  #endif

  cout << endl;
}


bool g_bExitApp;

void SignalHandler(int p_nSignal)
{  
  g_bExitApp = true;
}

int main(int argc, char* argv[])
{
  g_bExitApp = false;

  // install signal handler
  #ifndef WIN32
  signal(SIGINT, SignalHandler);  // ctrl-c
  signal(SIGTERM, SignalHandler); // start-stop-daemon -v --stop -nfuppes
  #endif

  // initialize libfuppes
	if(fuppes_init(argc, argv, NULL) != FUPPES_TRUE) {
	  return -1;
	}
  if(fuppes_start() != FUPPES_TRUE) {
    return -1;
  }
    
  char szHTTP[100];
  fuppes_get_http_server_address(szHTTP, 100);
  cout << "webinterface: " << szHTTP << endl;
  cout << endl;
  cout << "r = rebuild database" << endl;
  cout << "i = print system info" << endl;
  cout << "h = print help" << endl;
  cout << endl;
  #ifdef WIN32
  cout << "press \"q\" to quit" << endl;
  #else
  cout << "press \"ctrl-c\" or \"q\" to quit" << endl;
  #endif
  cout << endl;

  string input = "";
  #ifdef WIN32
  while(input != "q")
  #else
  while(!g_bExitApp && (input != "q"))
  #endif
  {
    input = "";
    #ifdef WIN32
    getline(cin, input);
    #else
    int nRes = -1;
    do {
      usleep(100 * 1000);
      
      nRes = fuppes_getch();
      if ((nRes > -1) && (nRes != 10) && (nRes != 13))
        input += nRes;        
    }
    while ((nRes != 10) && (nRes != 13) && !g_bExitApp);
    #endif
    
    if (input == "m") {
      fuppes_send_msearch();
    }
    else if (input == "a") {
      fuppes_send_alive();
    }
    else if (input == "b") {
      fuppes_send_byebye();
    }
    else if (input == "l") {
      fuppes_inc_loglevel();
    }
    else if(input == "r") {
      fuppes_rebuild_db();
    }
    else if(input == "u") {
      fuppes_update_db();
    }
    else if(input == "v") {
      fuppes_rebuild_vcontainers();
    }
    else if (input == "h") {
      PrintHelp();
    }
    else if (input == "i") {
      fuppes_print_info();
    }
    else if (input == "s") {
      fuppes_print_device_settings();
    }
  }  // while (!g_bExitApp)
  
  cout << "[FUPPES] shutting down" << endl;
  fuppes_stop();
  fuppes_cleanup();
  cout << "[FUPPES] exit" << endl;
}
