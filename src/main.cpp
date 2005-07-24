/***************************************************************************
 *            main.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
#include "Fuppes.h"
#include "Presentation/PresentationHandler.h"

using namespace std;

/*===============================================================================
 MAIN
===============================================================================*/

int main()
{
  // Setup winsockets	
#ifdef WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,0), &wsa);
#endif
  
  cout << "FUPPES - Free UPnP(tm) Entertainment Service " << CSharedConfig::Shared()->GetAppVersion() << endl;
  if(!CSharedConfig::Shared()->SetupConfig())
    return 0;
  cout << "hostname: " << CSharedConfig::Shared()->GetHostname() << endl;
  cout << "address : " << CSharedConfig::Shared()->GetIPv4Address() << endl;
  cout << endl;
	
  // Create presentation handler
  CPresentationHandler* pPresentationHandler = new CPresentationHandler();
  
  // Create main server object (CFuppes)
  
	CFuppes* pFuppes = new CFuppes(CSharedConfig::Shared()->GetIPv4Address(), CSharedConfig::Shared()->GetUDN(), pPresentationHandler);	
  pPresentationHandler->AddFuppesInstance(pFuppes);
  
  // todo: create a fuppes instance for each network interface
  //CFuppes* pFuppes2 = new CFuppes("127.0.0.1", pPresentationHandler);	
  //pPresentationHandler->AddFuppesInstance(pFuppes2);
  	
  cout << "Webinterface: http://" << pFuppes->GetHTTPServerURL() << "/index.html" << endl;
  //cout << "Webinterface: http://" << pFuppes2->GetHTTPServerURL() << "/index.html" << endl;
  cout << endl;
  cout << "press \"q\" to  quit" << endl;
  cout << endl;
  
  // Handle input
  string input = "";
  while(input != "q")
  {		
    getline(cin, input);

    if (input == "m")
      pFuppes->GetSSDPCtrl()->send_msearch();
    else if (input == "a")
      pFuppes->GetSSDPCtrl()->send_alive();
    else if (input == "b")
      pFuppes->GetSSDPCtrl()->send_byebye();
  }
  
  // Destroy objects
  SAFE_DELETE(pFuppes);
  SAFE_DELETE(pPresentationHandler);

  // Cleanup winsockets
#ifdef WIN32  
  WSACleanup();
#endif

  return 0;
}