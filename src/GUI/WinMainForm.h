/***************************************************************************
 *            WinMainForm.h
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
#include "GUIWrapper.h"

class CMainForm: public IGUI
{
  public:
    CMainForm(HINSTANCE hInstance);
    ~CMainForm();    
      
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
      
    void Show();
    void Hide();
      
    void ShowTrayIcon();
    void HideTrayIcon();
      
    void OnWmTrayicon(WPARAM wParam, LPARAM lParam);
      
    void OnTrayIconLButtonUp();
      
    // virtual interface functions
    
    void AddLogMsg(std::string p_sMessage);
    
      
  private:    
    HWND        hWnd;     // This is the handle for our window 
    WNDCLASSEX  wincl;    // Data structure for the windowclass 
    HMENU       hPopup;   // popup menu
    bool        m_bVisible;
};
