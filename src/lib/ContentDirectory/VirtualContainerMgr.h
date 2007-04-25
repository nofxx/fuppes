/***************************************************************************
 *            VirtualContainerMgr.h
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

#ifndef _VIRTUALCONTAINERMGR_H
#define _VIRTUALCONTAINERMGR_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../Common/Common.h"
#include "../Common/XMLParser.h"
#include "../UPnPActions/UPnPBrowse.h"

class CObjectDetails {
  public:
    bool Empty() { return sAlbum.length() == 0 && sArtist.length() == 0 && sGenre.length() == 0; }
    
    std::string sAlbum;
    std::string sArtist;
    std::string sGenre;
};

class CVirtualContainerMgr
{
  public:
	  static CVirtualContainerMgr* Shared();
    ~CVirtualContainerMgr();
				
    bool IsVirtualContainer(unsigned int p_nContainerId, std::string p_sDevice);    
    bool HasVirtualChildren(unsigned int p_nParentId, std::string p_sDevice);
	  int  GetChildCount(unsigned int p_nParentId, std::string p_sDevice);
  
    void RebuildContainerList();
    bool IsRebuilding();
  
    void CreateChildItems(CXMLNode* pParentNode, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, std::string p_sFilter = "");
  
	private:
	  CVirtualContainerMgr();
    
	  static CVirtualContainerMgr* m_pInstance;  
    
    fuppesThread m_RebuildThread;
  
    unsigned int m_nIdCounter;   
  
    void CreateSingleVFolder(CXMLNode* pFolderNode, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails);
    void CreateVFoldersFromProperty(CXMLNode* pFoldersNode, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, std::string p_sFilter = "");
    void CreateVFoldersSplit(CXMLNode* pFoldersNode, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, std::string p_sFilter = "");
    void CreateItemMappings(CXMLNode* pNode, std::string p_sDevice, unsigned int p_nParentId, std::string p_sFilter = "");
    void CreateFolderMappings(CXMLNode* pNode, std::string p_sDevice, unsigned int p_nParentId, std::string p_sFilter = "");
    void MapSharedDirsTo(CXMLNode* pNode, std::string p_sDevice, unsigned int p_nParentId);
  
		unsigned int GetId() { m_nIdCounter--; return m_nIdCounter; }		
};

#endif // _VIRTUALCONTAINERMGR_H
