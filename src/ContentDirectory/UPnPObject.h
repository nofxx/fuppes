/***************************************************************************
 *            UPnPObject.h
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
 
#ifndef _UPNPOBJECT_H
#define _UPNPOBJECT_H

#include <string>

enum eUPnPObjectType
{
  uotStorageFolder,
  uotItem,
  uotAudioItem
};

class CUPnPObject
{
  protected:
    CUPnPObject(eUPnPObjectType p_UPnPObjectType) { m_UPnPObjectType = p_UPnPObjectType; };
  
  public:
    eUPnPObjectType GetObjectType() { return m_UPnPObjectType; }      
  
    std::string GetObjectID() { return m_sObjectID; }
    void SetObjectID(std::string p_sObjectID) { m_sObjectID = p_sObjectID; }
    CUPnPObject* GetParent() { return m_ParentObject; }
    void SetParent(CUPnPObject* p_ParentObject) { m_ParentObject = p_ParentObject; }
    void SetName(std::string p_sName) { m_sName = p_sName; }
    std::string GetName() { return m_sName; }
    void SetFileName(std::string p_sFileName) { m_sFileName = p_sFileName; }
    std::string GetFileName() { return m_sFileName; }
    
  private:
    std::string m_sObjectID;
    eUPnPObjectType m_UPnPObjectType;
    CUPnPObject* m_ParentObject;
    std::string m_sName;
    std::string m_sFileName;
};

#endif /* _UPNPOBJECT_H */
