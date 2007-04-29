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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <windows.h>
#include <iostream>

#include "../../include/fuppes.h"
#include "WinMainForm.h"
#include "Common.h"

using namespace std;

CMainForm* pMainForm = NULL;

void LogCallback(const char* sz_log)
{
  pMainForm->Log(sz_log);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nWindowStyle)
{
  MSG messages;
  pMainForm = new CMainForm(hInstance);
  pMainForm->ShowTrayIcon();  

  // fuppes_init()
  if(fuppes_init(0, NULL, LogCallback) == FUPPES_FALSE) {
    MessageBox(NULL, "Error 0", GetAppShortName(), MB_OK);
    return 1;
  }

  // fuppes_start()
  if(fuppes_start() == FUPPES_FALSE) {
    MessageBox(NULL, "Error 1", GetAppShortName(), MB_OK);
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

  pMainForm->HideTrayIcon();
  delete pMainForm;

  return messages.wParam;
}
