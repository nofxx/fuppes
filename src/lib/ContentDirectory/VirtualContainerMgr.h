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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
  
    void CreateChildItems(CXMLNode* pParentNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string p_sFilter = "");
  
	private:
	  CVirtualContainerMgr();
    
	  static CVirtualContainerMgr* m_pInstance;  
    
    bool          m_bVFolderCfgValid;
  
    fuppesThread  m_RebuildThread;  
    
		#if SIZEOF_UNSIGNED_INT == 4
		unsigned int m_nIdCounter;
		#elif SIZEOF_UNSIGNED_SHORT == 4
		unsigned short m_nIdCounter;
		#endif
		
    void CreateSingleVFolder(CXMLNode* pFolderNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef);
    void CreateSingleVFolderFolder(CXMLNode* pNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nObjectId, unsigned int p_nParentId, bool p_bCreateRef);
    void CreateVFoldersFromProperty(CXMLNode* pFoldersNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string p_sFilter = "");
    void CreateVFoldersSplit(CXMLNode* pFoldersNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string p_sFilter = "");
    void CreateItemMappings(CXMLNode* pNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nParentId, bool p_bCreateRef, std::string p_sFilter = "");
    void CreateFolderMappings(CXMLNode* pNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nParentId, bool p_bCreateRef, std::string p_sFilter = "");
    void MapSharedDirsTo(CXMLNode* pNode, CContentDatabase* pIns, std::string p_sDevice, unsigned int p_nParentId, unsigned int p_nSharedParendId = 0);
  
		unsigned int GetId() { m_nIdCounter--; return m_nIdCounter; }		
};

#endif // _VIRTUALCONTAINERMGR_H
