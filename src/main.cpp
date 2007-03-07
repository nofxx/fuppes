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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

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
	fuppes_init();
  fuppes_start();

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
        input = nRes;        
    }
    while ((nRes != 10) && (nRes != 13) && !g_bExitApp);
    #endif

    /*if (input == "m")
      pFuppes->GetSSDPCtrl()->send_msearch();
    else if (input == "a")
        pFuppes->GetSSDPCtrl()->send_alive();
      else if (input == "b")
        pFuppes->GetSSDPCtrl()->send_byebye();
      else if (input == "l")
        CSharedLog::Shared()->ToggleLog();
      else if (input == "h")
        PrintHelp();
      else if (input == "i")
      {
        cout << "general information:" << endl;
        cout << "  version     : " << CSharedConfig::Shared()->GetAppVersion() << endl;
        cout << "  hostname    : " << CSharedConfig::Shared()->GetHostname() << endl;
        cout << "  OS          : " << CSharedConfig::Shared()->GetOSName() << " " << CSharedConfig::Shared()->GetOSVersion() << endl;
        cout << "  build at    : " << __DATE__ << " " << __TIME__ << endl;
        cout << "  build with  : " << __VERSION__ << endl;
        cout << "  address     : " << CSharedConfig::Shared()->GetIPv4Address() << endl;
        cout << "  sqlite      : " << CContentDatabase::Shared()->GetLibVersion() << endl;
        cout << "  log-level   : " << CSharedLog::Shared()->GetLogLevel() << endl;
        cout << "  webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;
        cout << endl;
        CSharedConfig::Shared()->PrintTranscodingSettings();
				cout << endl;
				cout << "configuration file:" << endl;
				cout << "  " << CSharedConfig::Shared()->GetConfigFileName() << endl;
				cout << endl;
      }
      else if (input == "r")
      {
        CSharedConfig::Shared()->Refresh();
        CContentDatabase::Shared()->BuildDB();
      }
      else if (input == "c")
        CSharedConfig::Shared()->Refresh();*/

  }  // while (!g_bExitApp)
  
  fuppes_stop();
  fuppes_cleanup();
}
