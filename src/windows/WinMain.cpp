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
#include <iostream>

#include "../../include/fuppes.h"
#include "WinMainForm.h"

using namespace std;

void ShowTrayIcon(bool b_Visible, HICON pIcon, HWND pForm);

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nWindowStyle)
{  
  MSG messages;
  CMainForm* pForm = new CMainForm(hInstance);

  pForm->ShowTrayIcon();

  if(fuppes_init() == FUPPES_FALSE) {
    MessageBox(NULL, "Error 0", "FUPPES 0.7.2-dev", MB_OK);
    return 1;    
  }
    
  //fuppes_start();
  if(fuppes_start() == FUPPES_FALSE) {
    MessageBox(NULL, "Error 1", "FUPPES 0.7.2-dev", MB_OK);
    return 1;
  }
  
  // Run the message loop. It will run until GetMessage() returns 0
  while (GetMessage (&messages, NULL, 0, 0)) {
   // Translate virtual-key messages into character messages
   TranslateMessage(&messages);
   // Send message to WindowProcedure
   DispatchMessage(&messages);
  }

  // cleanup
  fuppes_stop();
  fuppes_cleanup();
      
  pForm->HideTrayIcon();
  delete pForm;

  return messages.wParam;
}
