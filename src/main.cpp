/***************************************************************************
 *            main.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "Common.h"
#include "SharedConfig.h"
#include "SharedLog.h"
#include "Fuppes.h"
#include "Presentation/PresentationHandler.h"

#ifndef WIN32
#include <fcntl.h>
#include <signal.h>
#endif

using namespace std;

bool g_bExitApp;

void SignalHandler(int p_nSignal)
{
  cout << "SignalHandler: " << p_nSignal << endl;
  g_bExitApp = true;
  switch(p_nSignal)
  {
    case SIGINT:
      cout << "SIGINT" << endl;
      break;
    case SIGTERM:
      cout << "SIGTERM" << endl;
      break;    
  }
}

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
      cout << argv[i] << endl;      
      if(strcmp(argv[i], "-d") == 0)
        bDaemonMode = true;
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
    cout << "daemon mode" << endl;
    
    /* Our process ID and Session ID */
    pid_t pid, sid;
    
    /* Fork off the parent process */
    pid = fork();
    if (pid < 0) {
            exit(EXIT_FAILURE);
    }
    /* If we got a good PID, then
       we can exit the parent process. */
    if (pid > 0) {
            exit(EXIT_SUCCESS);
    }
  
    /* Change the file mode mask */
    umask(0);
            
    /* Open any logs here */        
            
    /* Create a new SID for the child process */
    sid = setsid();
    if (sid < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }
    
    /* Change the current working directory */
    if ((chdir("/")) < 0) {
            /* Log the failure */
            exit(EXIT_FAILURE);
    }
    
    /* Close out the standard file descriptors */
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
  }
  #endif
  
  #ifndef WIN32  
  signal(SIGINT, SignalHandler);  /* ctrl-c */  
  signal(SIGTERM, SignalHandler); /* start-stop-daemon -v --stop -nfuppes */ 
  #endif  
  
  cout << "FUPPES - Free UPnP(tm) Entertainment Service " << CSharedConfig::Shared()->GetAppVersion() << endl;
  cout << "http://fuppes.sourceforge.net" << endl;
  if(!CSharedConfig::Shared()->SetupConfig())
    return 0;  
	
  /* Create presentation handler */
  CPresentationHandler* pPresentationHandler = new CPresentationHandler();
  
  /* Create main server object (CFuppes) */
  
	CFuppes* pFuppes = new CFuppes(CSharedConfig::Shared()->GetIPv4Address(), CSharedConfig::Shared()->GetUUID(), pPresentationHandler);	
  pPresentationHandler->AddFuppesInstance(pFuppes);
  
  /* todo: create a fuppes instance for each network interface */
  //CFuppes* pFuppes2 = new CFuppes("127.0.0.1", pPresentationHandler);	
  //pPresentationHandler->AddFuppesInstance(pFuppes2);
  	
  cout << "Webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;
  //cout << "Webinterface: http://" << pFuppes2->GetHTTPServerURL() << "/index.html" << endl;
  cout << endl;
  cout << "m = send m-search" << endl;
  cout << "a = send notify-alive" << endl;
  cout << "b = send notify-byebye" << endl;
  cout << "l = toggle logging" << endl;
  cout << "i = info" << endl; 
  cout << endl;
  cout << "press \"q\" to  quit" << endl;
  cout << endl;
  
  /* Handle input */
  if(!bDaemonMode)
  {
    string input = "";
    while((input != "q") && !g_bExitApp)
    {		
      getline(cin, input);
      //fuppesSleep(2000);
      
      if (input == "m")
        pFuppes->GetSSDPCtrl()->send_msearch();
      else if (input == "a")
        pFuppes->GetSSDPCtrl()->send_alive();
      else if (input == "b")
        pFuppes->GetSSDPCtrl()->send_byebye();
      else if (input == "l")
        CSharedLog::Shared()->ToggleLog();    
      else if (input == "i")
      {
        cout << "version     : " << CSharedConfig::Shared()->GetAppVersion() << endl;
        cout << "hostname    : " << CSharedConfig::Shared()->GetHostname() << endl;
        cout << "address     : " << CSharedConfig::Shared()->GetIPv4Address() << endl;    
        cout << "webinterface: http://" << pFuppes->GetHTTPServerURL() << "/" << endl;          
      }
    }
  }
  else
  {
    while(!g_bExitApp)
      fuppesSleep(2000);
  }
  
  /* Destroy objects */
  SAFE_DELETE(pFuppes);
  SAFE_DELETE(pPresentationHandler);

  delete CSharedConfig::Shared();
  delete CSharedLog::Shared();
  
  /* Cleanup winsockets */
  #ifdef WIN32
  WSACleanup();
  #endif

  return 0;
}
