/***************************************************************************
 *            GUIWrapper.h
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
 
#ifndef _GUI_WRAPPER_H
#define _GUI_WRAPPER_H

#include <string>

// events
typedef enum GUI_EVENT {
  EVT_REBUILD_DB
} GUI_EVENT;

class IGUIWrapper
{
  public:
    virtual void GUIEvent(GUI_EVENT pEvent);
};

class IGUI
{
  public:
    void SetEventHandler(IGUIWrapper* pEventHandler) {
      m_pEventHandler = pEventHandler;
    }
    
    void RaiseEvent(GUI_EVENT pEvent) {
      if(m_pEventHandler)
        m_pEventHandler->GUIEvent(pEvent);
    }
    
    virtual void AddLogMsg(std::string p_sMessage);
    
  private:
    IGUIWrapper* m_pEventHandler;    
};

class CGUIWrapper: public IGUIWrapper
{
  public:
    CGUIWrapper();
    virtual ~CGUIWrapper();
    
    static CGUIWrapper* Shared();

    void SetGUI(IGUI* pGUI);
    virtual void GUIEvent(GUI_EVENT pEvent);
    
    bool HasGUI() { return m_pGUI; }
    
    void AddLogMsg(std::string p_sMessage);
    
  private:                    
    static CGUIWrapper* m_pInstance;

    IGUI*  m_pGUI;
};

#endif // _GUI_WRAPPER_H
