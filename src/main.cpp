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


#include <iostream>

#include "SharedConfig.h"
#include "Fuppes.h"

#include "win32.h"

using namespace std;

int main()
{
	/*
	 * setup winsockets
	 */
  #ifdef WIN32
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,0),&wsa);
  #endif
  
	cout << "FUPPES - Free UPnP(tm) Entertainment Service " << CSharedConfig::Shared()->GetAppVersion() << endl;	
	cout << "hostname: " << CSharedConfig::Shared()->GetHostname() << endl;
	cout << "address : " << CSharedConfig::Shared()->GetIP() << endl;
	cout << endl;
	
	CFuppes* pFuppes = new CFuppes();	
	
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
		
		upnpSleep(300);
	}
  
	delete pFuppes;
		
	return 0;
}
