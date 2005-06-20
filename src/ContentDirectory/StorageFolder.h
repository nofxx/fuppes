/***************************************************************************
 *            StorageFolder.h
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
 
#ifndef _STORAGEFOLDER_H
#define _STORAGEFOLDER_H

#include "UPnPObject.h"
#include "UPnPItem.h"
#include "../UPnPActions/UPnPBrowse.h"

#include <vector>
#include <string>
#include <libxml/xmlwriter.h>

class CStorageFolder: public CUPnPObject
{
  public:
    CStorageFolder();
    ~CStorageFolder();
  
    void AddUPnPObject(CUPnPObject*);
    std::string GetContentAsString(CUPnPBrowse* pBrowseAction, unsigned int* p_nNumberReturned, unsigned int* p_nTotalMatches);
    std::string GetChildCountAsString();
  
  private:
    std::vector<CUPnPObject*> m_vObjects;
    void BuildFolderDescription(CStorageFolder*, xmlTextWriterPtr);
    void BuildItemDescription(CUPnPItem*, xmlTextWriterPtr);
};

#endif /* _STORAGEFOLDER_H */
