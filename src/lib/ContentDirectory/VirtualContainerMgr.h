/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            VirtualContainerMgr.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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
#include "../Common/Thread.h"
#include "../Common/XMLParser.h"
#include "DatabaseConnection.h"

class CObjectDetails {
  public:
    bool Empty() { return sAlbum.length() == 0 && sArtist.length() == 0 && sGenre.length() == 0; }
    
    std::string sAlbum;
    std::string sArtist;
    std::string sGenre;
};




class CVirtualContainerMgr: private fuppes::Thread
{
  public:
	  static CVirtualContainerMgr* Shared();
    ~CVirtualContainerMgr();
				
    static bool isVirtualContainer(unsigned int p_nContainerId, std::string p_sDevice, CSQLQuery* qry = NULL);
    static bool hasVirtualChildren(unsigned int p_nParentId, std::string p_sDevice, CSQLQuery* qry = NULL);
	  int  GetChildCount(unsigned int p_nParentId, std::string p_sDevice);
  
    void RebuildContainerList();
    bool IsRebuilding();
  
    void CreateChildItems(CXMLNode* pParentNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string p_sFilter = "");
  
	private:
	  CVirtualContainerMgr();
    
	  static CVirtualContainerMgr* m_pInstance;  
    
    bool          m_bVFolderCfgValid;
  
    //fuppesThread  m_RebuildThread;  
		void run();
		
		#if SIZEOF_UNSIGNED_INT == 4
		unsigned int m_nIdCounter;
		#elif SIZEOF_UNSIGNED_SHORT == 4
		unsigned short m_nIdCounter;
		#endif
		
    void CreateSingleVFolder(CXMLNode* pFolderNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef);
    void CreateSingleVFolderFolder(CXMLNode* pNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nObjectId, unsigned int p_nParentId, bool p_bCreateRef);
    void CreateVFoldersFromProperty(CXMLNode* pFoldersNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string p_sFilter = "");
    void CreateVFoldersSplit(CXMLNode* pFoldersNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string p_sFilter = "");
    void CreateItemMappings(CXMLNode* pNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, bool p_bCreateRef, std::string p_sFilter = "");
    void CreateFolderMappings(CXMLNode* pNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, bool p_bCreateRef, std::string p_sFilter = "");
    void MapSharedDirsTo(CXMLNode* pNode, CSQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, unsigned int p_nSharedParendId = 0);
  
		unsigned int GetId() { m_nIdCounter--; return m_nIdCounter; }		
};

#endif // _VIRTUALCONTAINERMGR_H
