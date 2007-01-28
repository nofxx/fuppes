/***************************************************************************
 *            main.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <iostream>
#include "Common/Common.h"
#include "SharedConfig.h"
#include "SharedLog.h"
#include "Fuppes.h"
#include "Presentation/PresentationHandler.h"
#include "ContentDirectory/ContentDatabase.h"

#ifndef WIN32
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#endif

#ifdef WIN32
#include <shellapi.h>
#include <windows.h>
#endif

using namespace std;

bool g_bExitApp;

#ifndef WIN32
/* non-blocking getchar() */
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

int fuppesGetch (void)
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


void SignalHandler(int p_nSignal)
{  
  g_bExitApp = true;
}

void PrintHelp()
{
  cout << endl;
  cout << "l = change log-level" << endl;
  cout << "    (disabled, normal, extended, debug) default is \"normal\"" << endl;
  cout << "i = print system info" << endl;
  cout << "r = rebuild database" << endl;
  cout << "c = refresh configuration" << endl;
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

#ifdef WIN32
bool CreateTrayIcon()
{
  NOTIFYICONDATA  m_tnd;
  m_tnd.cbSize = sizeof(NOTIFYICONDATA);
  /*m_tnd.hWnd   = void; //pParent->GetSafeHwnd()? pParent->GetSafeHwnd() : m_hWnd;
  m_tnd.uID    = uID;
  m_tnd.hIcon  = void; //icon;
  m_tnd.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
  m_tnd.uCallbackMessage = WM_ICON_NOTIFY; //uCallbackMessage;
  _tcscpy(m_tnd.szTip, szToolTip); */
  m_tnd.uFlags = NIF_MESSAGE; // | NIF_ICON | NIF_TIP;
  m_tnd.uCallbackMessage = WM_USER + 10;

  return Shell_NotifyIcon(NIM_ADD, &m_tnd);
}
#endif

/*===============================================================================
 MAIN
===============================================================================*/

/** main function
 *  @return int
 *  @todo   create a CFuppes instance for each network interface
 */
int main(int argc, char* argv[])
{
  bool bDaemonMode = false;
  g_bExitApp = false;

  if(argc > 1)
  {
    for(int i = 1; i < argc; i++)
    {
      // --daemon
			if((strcmp(argv[i], "-d") == 0) || (strcmp(argv[i], "--daemon") == 0))
        bDaemonMode = true;
			// -- syslog 
      else if(strcmp(argv[i], "--syslog") == 0)
        CSharedLog::Shared()->SetUseSyslog(true);
			// --loglevel
			else if((strcmp(argv[i], "--loglevel") == 0) && (argc > i + 1)) {
			
			  if((strcmp(argv[i + 1], "none") == 0) || (strcmp(argv[i + 1], "0") == 0))
				  CSharedLog::Shared()->SetLogLevel(0, false);
				else if((strcmp(argv[i + 1], "normal") == 0) || (strcmp(argv[i + 1], "1") == 0))
				  CSharedLog::Shared()->SetLogLevel(1, false);
				else if((strcmp(argv[i + 1], "extended") == 0) || (strcmp(argv[i + 1], "2") == 0))
				  CSharedLog::Shared()->SetLogLevel(2, false);
				else if((strcmp(argv[i + 1], "debug") == 0) || (strcmp(argv[i + 1], "3") == 0))
				  CSharedLog::Shared()->SetLogLevel(3, false);
			} // end --loglevel
			
    }
  }

  /* Setup winsockets	*/
  #ifdef WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);
  #endif


  /* daemon process */
  #ifndef WIN32
  if(bDaemonMode)
  {
    CSharedLog::Shared()->SetUseSyslog(true);

    if(!CSharedConfig::Shared()->SetupConfig())
    return 1;

    cout << "daemon mode" << endl;

    pid_t pid;
    pid = fork();

    // error
    if (pid < 0)
    {
      CSharedLog::Shared()->Log(L_ERROR, "could not create child process", __FILE__, __LINE__);
      return 1;
    }
    // parent process
    else if (pid > 0)
    {
      cout << "[started]" << endl;
      return 0;
    }
    // child process
    else if(pid == 0)
    {
      //cout << "child process" << endl;
      close(STDIN_FILENO);
      close(STDOUT_FILENO);
      close(STDERR_FILENO);
    }
  }
  #endif

  #ifndef WIN32
  signal(SIGINT, SignalHandler);  /* ctrl-c */
  signal(SIGTERM, SignalHandler); /* start-stop-daemon -v --stop -nfuppes */
  #endif

  /*#ifdef WIN32
  CreateTrayIcon();
  #endif*/

  cout << "FUPPES - Free UPnP Entertainment Service " << CSharedConfig::Shared()->GetAppVersion() << endl;
  cout << "http://fuppes.sourceforge.net" << endl << endl;

  if(!CSharedConfig::Shared()->SetupConfig())
    return 1;

  // create presentation handler
  CPresentationHandler* pPresentationHandler = new CPresentationHandler();

  // create main fuppes object
  CFuppes* pFuppes = NULL;
	try {
    pFuppes = new CFuppes(CSharedConfig::Shared()->GetIPv4Address(), CSharedConfig::Shared()->GetUUID(), pPresentationHandler);
    CSharedConfig::Shared()->AddFuppesInstance(pFuppes);
  }
  catch(EException ex) {
    cout << ex.What() << endl;
    cout << "[exiting]" << endl;
    return 1;
  }

  cout << "Webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;
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

  // handle input
  if(!bDaemonMode)
  {
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
        fuppesSleep(100);
        
        nRes = fuppesGetch();
        if ((nRes > -1) && (nRes != 10) && (nRes != 13))
          input = nRes;        
      }
      while ((nRes != 10) && (nRes != 13) && !g_bExitApp);
      #endif

      if (input == "m")
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
        cout << "  build at    : " << __DATE__ << "" << __TIME__ << endl;
        cout << "  build with  : " << __VERSION__ << endl;
        cout << "  address     : " << CSharedConfig::Shared()->GetIPv4Address() << endl;
        cout << "  sqlite      : " << CContentDatabase::Shared()->GetLibVersion() << endl;
        cout << "  log-level   : " << CSharedLog::Shared()->GetLogLevel() << endl;
        cout << "  webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;
        cout << endl;
        CSharedConfig::Shared()->PrintTranscodingSettings();
      }
      else if (input == "r")
      {
        CSharedConfig::Shared()->Refresh();
        CContentDatabase::Shared()->BuildDB();
      }
      else if (input == "c")
        CSharedConfig::Shared()->Refresh();

    }
  }
  else
  {
    while(!g_bExitApp)
      fuppesSleep(1000);
  }

  // destroy objects
  delete pFuppes;
  delete pPresentationHandler;

  delete CSharedConfig::Shared();
  delete CSharedLog::Shared();

  // cleanup winsockets
  #ifdef WIN32
  WSACleanup();
  #endif

  cout << "[FUPPES] exit" << endl;
  return 0;
}
