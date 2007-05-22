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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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

void NotifyCallback(const char* sz_title, const char* sz_msg)
{
  pMainForm->Notify(sz_title, sz_msg);
}

void ErrorCallback(const char* sz_err)
{
  pMainForm->Error(sz_err);
}

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int nWindowStyle)
{
  MSG messages;
  pMainForm = new CMainForm(hInstance);
  pMainForm->ShowTrayIcon();  

  // fuppes_init()
  if(fuppes_init(0, NULL, LogCallback) == FUPPES_FALSE) {    
    pMainForm->HideTrayIcon();
    delete pMainForm;
    return 1;
  }

  fuppes_set_loglevel(0);

  
  fuppes_set_notify_callback(NotifyCallback);
  fuppes_set_error_callback(ErrorCallback);

  // fuppes_start()
  if(fuppes_start() == FUPPES_FALSE) {
    pMainForm->HideTrayIcon();
    delete pMainForm;
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
