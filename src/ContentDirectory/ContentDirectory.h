/***************************************************************************
 *            ContentDirectory.h
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
 
#ifndef _CONTENTDIRECTORY_H
#define _CONTENTDIRECTORY_H

#include "../UPnPService.h"
#include "../HTTPMessage.h"
#include <map>
#include <string>

#include "UPnPObject.h"
#include "StorageFolder.h"
#include "../UPnPActions/UPnPBrowse.h"

class CContentDirectory: public CUPnPService
{
	public:
		CContentDirectory();
	  ~CContentDirectory();
    
    CHTTPMessage* HandleUPnPAction(CUPnPAction*);
    std::string GetFileNameFromObjectID(std::string);
  
  private:
    std::string HandleUPnPBrowse(CUPnPBrowse*);
    std::map<std::string, CUPnPObject*> m_ObjectList;
    std::map<std::string, CUPnPObject*>::iterator m_ListIterator;
    void BuildObjectList();
    void ScanDirectory(std::string, int*, CStorageFolder*);
};

#endif /* _CONTENTDIRECTORY_H */
