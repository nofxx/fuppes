/***************************************************************************
 *            WinMain.cpp
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
 
#include <windows.h>

#include "Common/Common.h"
#include "SharedConfig.h"
#include "Fuppes.h"

#include "GUI/GUIWrapper.h"
#include "GUI/WinMainForm.h"

void ShowTrayIcon(bool b_Visible, HICON pIcon, HWND pForm);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nWindowStyle)
{
  MSG messages;

  CMainForm* pForm = new CMainForm(hInstance);
  CGUIWrapper::Shared()->SetGUI(pForm);

  // init wsock
  WSADATA wsa;
  WSAStartup(MAKEWORD(2,2), &wsa);

  // init config
  CSharedConfig::Shared()->SetupConfig();
    
  // create main fuppes object
  CFuppes* pFuppes = NULL;
  try {
    pFuppes = new CFuppes(CSharedConfig::Shared()->GetIPv4Address(), CSharedConfig::Shared()->GetUUID());
    CSharedConfig::Shared()->AddFuppesInstance(pFuppes);
  }
  catch(EException ex) {
    MessageBox(NULL, "error", ex.What().c_str(), MB_OK);
    cout << ex.What() << endl;
    cout << "[exiting]" << endl;
    return 1;
  }

  // everything up and running
  // let's display the tray icon
  pForm->ShowTrayIcon();

  // Run the message loop. It will run until GetMessage() returns 0
  while (GetMessage (&messages, NULL, 0, 0)) {
   // Translate virtual-key messages into character messages
   TranslateMessage(&messages);
   // Send message to WindowProcedure
   DispatchMessage(&messages);
  }

  // cleanup
  delete pFuppes;
  delete CSharedConfig::Shared();
      
  pForm->HideTrayIcon();
  delete pForm;

  // uninit wsock
  WSACleanup();

  return messages.wParam;
}
