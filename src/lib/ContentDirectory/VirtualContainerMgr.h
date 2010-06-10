/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            VirtualContainerMgr.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "DatabaseObject.h"


class CObjectDetails {
  public:
    bool Empty() { return sAlbum.length() == 0 && sArtist.length() == 0 && sGenre.length() == 0; }
    
    std::string sAlbum;
    std::string sArtist;
    std::string sGenre;
};


class VirtualContainerMgr;


class CVirtualContainerMgr
{
  friend class VirtualContainerMgr;
  
  public:
	  static CVirtualContainerMgr* Shared();
    ~CVirtualContainerMgr();
				
    //static bool isVirtualContainer(unsigned int p_nContainerId, std::string p_sDevice, SQLQuery* qry = NULL);
    //static bool hasVirtualChildren(unsigned int p_nParentId, std::string p_sDevice, SQLQuery* qry = NULL);
	  //int  GetChildCount(unsigned int p_nParentId, std::string p_sDevice);
  
    void RebuildContainerList(bool force = false, bool insertFiles = true);
    bool IsRebuilding();
  
    //void CreateChildItems(CXMLNode* pParentNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, std::string p_sFilter = "");
  
	private:
	  CVirtualContainerMgr();
    
    bool HandleFile(std::string device, std::string file, SQLQuery* qry);

	  static CVirtualContainerMgr* m_pInstance;  

		
		#if SIZEOF_UNSIGNED_INT == 4
		unsigned int m_nIdCounter;
		#elif SIZEOF_UNSIGNED_SHORT == 4
		unsigned short m_nIdCounter;
		#endif

    void createLayout(CXMLNode* node, object_id_t pid, SQLQuery* qry, std::string layout);

    
    //void CreateSingleVFolder(CXMLNode* pFolderNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, std::string vcontainerPath);
    //void CreateSingleVFolderFolder(CXMLNode* pNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nObjectId, unsigned int p_nParentId, bool p_bCreateRef);
    //void CreateVFoldersFromProperty(CXMLNode* pFoldersNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string vcontainerPath, std::string p_sFilter = "");
    //void CreateVFoldersSplit(CXMLNode* pFoldersNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails, bool p_bContainerDetails, bool p_bCreateRef, std::string p_sFilter = "");
    //void CreateItemMappings(CXMLNode* pNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, bool p_bCreateRef, std::string vcontainerPath, std::string p_sFilter = "");
    //void CreateFolderMappings(CXMLNode* pNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, bool p_bCreateRef, std::string p_sFilter = "");
    //void MapSharedDirsTo(CXMLNode* pNode, SQLQuery* pIns, std::string p_sDevice, unsigned int p_nParentId, unsigned int p_nSharedParendId = 0);
  
		unsigned int GetId() { m_nIdCounter--; return m_nIdCounter; }		
};


class VirtualContainerMgr
{
  public:
    /**
     * object the original file object (REF_ID NULL and DEVICE = NULL)
     */
    static void insertFile(fuppes::DbObject* object);

    /**
     * object the original file object with the updated details (REF_ID NULL and DEVICE = NULL)
     * details the old file details
     */
    static void updateFile(fuppes::DbObject* object, fuppes::ObjectDetails* oldDetails);     


    /**
     * object the file object to delete (REF_ID NULL and DEVICE = NULL)
     */
    static void deleteFile(fuppes::DbObject* object);

    /**
     * directory the directory object to delete including containing files (REF_ID NULL and DEVICE = NULL)
     */
    static void deleteDirectory(fuppes::DbObject* directory);
    
  private:
    static void insertFileForLayout(fuppes::DbObject* object, std::string layout);
    static object_id_t createFolderIfNotExists(fuppes::DbObject* object, object_id_t pid, fuppes::DbObject::VirtualContainerType type, std::string path, std::string layout);

    static void updateFileForLayout(fuppes::DbObject* object, fuppes::ObjectDetails* oldDetails, std::string layout);

    static void deleteFileForLayout(fuppes::DbObject* object, std::string layout);
    static void deleteFolderIfEmpty(fuppes::DbObject* vfolder);
};

#endif // _VIRTUALCONTAINERMGR_H
