/***************************************************************************
 *            WinMainForm.cpp
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

#include "WinMainForm.h"

#include <sstream>

#define WM_TRAYICON WM_APP + 1

#define ID_EDITCHILD 12

using namespace std;

LRESULT CALLBACK WindowProcedure(HWND p_hWnd, UINT message, WPARAM wParam, LPARAM lParam);

CMainForm* pForm = NULL;

CMainForm::CMainForm(HINSTANCE hInstance)
{
  m_bVisible = false;
  hWnd = NULL;
                               
  // The Window structure 
  wincl.hInstance     = hInstance;
  wincl.lpszClassName = "CMainForm";
  wincl.lpfnWndProc   = WindowProcedure;      /* This function is called by windows */
  wincl.style         = CS_DBLCLKS;           /* Catch double-clicks */
  wincl.cbSize        = sizeof (WNDCLASSEX);

  // Use default icon and mouse-pointer 
  wincl.hIcon   = LoadIcon(hInstance, "A");  //LoadIcon (NULL, IDI_APPLICATION);
  wincl.hIconSm = LoadIcon(hInstance, "A");  //LoadIcon (NULL, IDI_APPLICATION);
  wincl.hCursor = LoadCursor (NULL, IDC_ARROW);
  wincl.lpszMenuName = NULL;                 /* No menu */
  wincl.cbClsExtra = 0;                      /* No extra bytes after the window class */
  wincl.cbWndExtra = 0;                      /* structure or the window instance */
  // Use Windows's default color as the background of the window 
  wincl.hbrBackground = (HBRUSH) COLOR_BACKGROUND;

  // Register the window class, and if it fails quit the program 
  RegisterClassEx(&wincl);
  /*if(!RegisterClassEx(&wincl))
    throw EException("RegisterClassEx()", __FILE__, __LINE__);                      */
        
  /* The class is registered, let's create the program*/
  hWnd = CreateWindowEx (
         0,                   /* Extended possibilites for variation */
         "CMainForm",         /* Classname */
         "FUPPES - the Free UPnP Entertainment Service 0.7.2-dev",       /* Title Text */
         WS_OVERLAPPEDWINDOW, /* default window */
         CW_USEDEFAULT,       /* Windows decides the position */
         CW_USEDEFAULT,       /* where the window ends up on the screen */
         544,                 /* The programs width */
         375,                 /* and height in pixels */
         HWND_DESKTOP,        /* The window is a child-window to desktop */
         NULL,                /* No menu */
         hInstance,           /* Program Instance handler */
         NULL                 /* No Window Creation data */
         );        
                      
  // create popup menu                
  hPopup = CreatePopupMenu();
  AppendMenu(hPopup, MF_STRING, 3, "Show webinterface");    
  AppendMenu(hPopup, MF_STRING, 2, "Rebuild database");  
  
  AppendMenu(hPopup, MF_SEPARATOR, 0, "" );    
  AppendMenu(hPopup, MF_STRING, 1, "Quit" );    
  pForm = this; 
  
  OnCreate(); 
}

CMainForm::~CMainForm()
{
  DestroyMenu(hPopup);
}

LRESULT CALLBACK WindowProcedure(HWND p_hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  return pForm->WindowProc(p_hWnd, message, wParam, lParam);
}

LRESULT CALLBACK CMainForm::WindowProc(HWND p_hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
  switch (message)                  /* handle the messages */
  {
    /*case WM_CREATE:  
      if(p_hWnd == hWnd)
        OnCreate(); 
      break;*/
      
    case WM_SIZE: 
      // Make the edit control the size of the window's client area. 
      MoveWindow(hWndMemo, 
                 8, 8,                  // starting x- and y-coordinates 
                 LOWORD(lParam) - 16,        // width of client area 
                 HIWORD(lParam) - 16,        // height of client area 
                 TRUE);                 // repaint window 
      return 0;
      break;
      
         
    case WM_TRAYICON:
      OnWmTrayicon(wParam, lParam);
      break;
         
    case WM_CLOSE:
      Hide();
      break;         
         
    case WM_DESTROY:
      PostQuitMessage (0);       /* send a WM_QUIT to the message queue */
      break;
      
    default:                      /* for messages that we don't deal with */
      return DefWindowProc(p_hWnd, message, wParam, lParam);
  }

  return 0;        
}


void CMainForm::OnCreate()
{
  hWndMemo = CreateWindow("EDIT",      // predefined class 
                          NULL,        // no window title 
                          WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
                          ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL,
//                           |  ES_READONLY ,
                          0, 0, 0, 0,  // set size in WM_SIZE message 
                          hWnd,        // parent window 
                          (HMENU)ID_EDITCHILD,   // edit control ID 
                          (HINSTANCE) GetWindowLong(hWnd, GWL_HINSTANCE), 
                          NULL);       // pointer not needed
                    
   //SendMessage(hWndMemo, WM_SETTEXT, 0, (LPARAM) "dahummm");              
}

void CMainForm::Show()
{
  ShowWindow(hWnd, SW_SHOW);
  m_bVisible = true;
}

void CMainForm::Hide()
{
  ShowWindow(hWnd, SW_HIDE);
  m_bVisible = false;    
}

void CMainForm::ShowTrayIcon()
{
  NOTIFYICONDATA tsym;
  ZeroMemory (&tsym, sizeof (NOTIFYICONDATA));

  tsym.cbSize = sizeof (NOTIFYICONDATA);
  tsym.hWnd   = hWnd;
  tsym.uID    = 1;
  tsym.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
  tsym.uCallbackMessage = WM_TRAYICON;
  tsym.hIcon  = wincl.hIcon;
  strcpy (tsym.szTip, "FUPPES 0.7.2");
  Shell_NotifyIcon (NIM_ADD, &tsym);     
}

void CMainForm::HideTrayIcon()
{
  NOTIFYICONDATA tsym;
  ZeroMemory (&tsym, sizeof (NOTIFYICONDATA));

  tsym.cbSize = sizeof (NOTIFYICONDATA);
  tsym.hWnd   = hWnd;
	tsym.uID    = 1;
	tsym.uFlags = 0;
	Shell_NotifyIcon (NIM_DELETE, &tsym);     
}


void CMainForm::Log(std::string p_sLog)
{
  //MessageBox(hWnd, p_sLog.c_str(), "FUPPES 0.7.2-dev", MB_OK);
  //SendMessage(hWndMemo, WM_SETSEL, (WPARAM), (LPARAM));
  p_sLog.append("\r\n");
  SendMessage(hWndMemo, EM_REPLACESEL, (WPARAM)FALSE, (LPARAM)p_sLog.c_str());
  

  //SendMessage(hWndMemo, WM_SETTEXT, 0, (LPARAM) p_sLog.c_str()); 
}

void CMainForm::OnWmTrayicon(WPARAM wParam, LPARAM lParam)
{
  switch (lParam)
  {
    case WM_LBUTTONDBLCLK:
      !m_bVisible ? Show() : Hide();
      break;
         
    //case WM_LBUTTONUP:
    case WM_RBUTTONUP:
      OnTrayIconLButtonUp();
      break;      
  }
}

void CMainForm::OnTrayIconLButtonUp()
{
  POINT pt;
  unsigned int ret;
	GetCursorPos(&pt);
	ret = TrackPopupMenu(hPopup, TPM_RETURNCMD|TPM_LEFTBUTTON, pt.x, pt.y, 0, hWnd, NULL);  
	
	stringstream sTmp;
	
	switch(ret)
	{
    case 1:
      if(MessageBox(hWnd, "Shutdown FUPPES?", "FUPPES 0.7.2-dev", MB_ICONQUESTION | MB_YESNO) == IDYES) {
         PostMessage(hWnd, WM_DESTROY, 0, 0);
      }
      break;
    case 2:
      if(MessageBox(hWnd, "Rebuild database?", "FUPPES 0.7.2-dev", MB_ICONQUESTION | MB_YESNO) == IDYES) {
        //RaiseEvent(EVT_REBUILD_DB);
      }               
      break;
    case 3:
      //sTmp << "http://" << CSharedConfig::Shared()->GetFuppesInstance(0)->GetHTTPServerURL();
      ShellExecute(hWnd, "open", sTmp.str().c_str(), NULL, NULL, SW_NORMAL);
      sTmp.str("");
      break;
  }
}

/*void CMainForm::AddLogMsg(std::string p_sMessage)
{
 // MessageBox(hWnd, p_sMessage.c_str(), "log", MB_ICONQUESTION | MB_OK);
}*/
