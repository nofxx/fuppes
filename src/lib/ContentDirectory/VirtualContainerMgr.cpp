/***************************************************************************
 *            VirtualContainerMgr.cpp
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

#include "VirtualContainerMgr.h"

#include "../Common/Common.h"
#include "ContentDatabase.h"
#include "../SharedConfig.h"
#include <iostream>
using namespace std;
		
CVirtualContainerMgr* CVirtualContainerMgr::m_pInstance = 0;
		
CVirtualContainerMgr* CVirtualContainerMgr::Shared()
{
  if(m_pInstance == 0)
	  m_pInstance = new CVirtualContainerMgr();
	return m_pInstance;
}

CVirtualContainerMgr::CVirtualContainerMgr()
{
	m_nIdCounter = 0; 
  //RebuildContainerList();
}


void CVirtualContainerMgr::RebuildContainerList()
{
  #warning todo: threaded
  CXMLDocument* pDoc = new CXMLDocument();
  if(!pDoc->Load(CSharedConfig::Shared()->GetConfigDir() + "vfolder.cfg")) {
    return;
  }
  
  CContentDatabase* pDb = new CContentDatabase();
  pDb->Execute("delete from  VIRTUAL_CONTAINERS;");
  pDb->Execute("delete from  MAP_ITEMS2VC;");	
  delete pDb;
  
  
  int i;
  CXMLNode* pChild;
  string sDevice;
  
  for(i = 0; i < pDoc->RootNode()->ChildCount(); i++) {
    
    pChild = pDoc->RootNode()->ChildNode(i);
    
    if(pChild->Name().compare("vfolder_layout") == 0) {
      sDevice = pChild->Attribute("device");
      CreateChildItems(pChild, sDevice, 0);
    }    
    delete pChild;
  }
  
  cout << "done" << endl;
}

void CVirtualContainerMgr::CreateChildItems(CXMLNode* pParentNode, 
                                            std::string p_sDevice, 
                                            unsigned int p_nParentId, 
                                            std::string p_sFilter)
{
  CXMLNode* pNode;
  int i;
  
  for(i = 0; i < pParentNode->ChildCount(); i++) {
    pNode = pParentNode->ChildNode(i);
        
    if(pNode->Name().compare("vfolder") == 0) {
      cout << "create single vfolder: " << pNode->Attribute("name") << " :: " << p_sFilter << endl;
      CreateSingleVFolder(pNode, p_sDevice, p_nParentId);
    }
    else if(pNode->Name().compare("vfolders") == 0) {
      cout << "create vfolders from property: " << pNode->Attribute("property") << " :: " << p_sFilter << endl;
      if(pNode->Attribute("property").length() > 0) {
        CreateVFoldersFromProperty(pNode, p_sDevice, p_nParentId, p_sFilter);
      }
    }
    else if(pNode->Name().compare("items") == 0) {
      cout << "create item mappings for type: " << pNode->Attribute("type") << " :: " << p_sFilter << endl;
      CreateItemMappings(pNode, p_sDevice, p_nParentId, p_sFilter);
    }
    else if(pNode->Name().compare("folders") == 0) {
      cout << "create folder mappings for filter: " << pNode->Attribute("filter") << " :: " << p_sFilter << endl;
      CreateFolderMappings(pNode, p_sDevice, p_nParentId, p_sFilter);
    }
    
    delete pNode;
  }
}

void CVirtualContainerMgr::CreateSingleVFolder(CXMLNode* pFolderNode, std::string p_sDevice, unsigned int p_nParentId)
{
  CContentDatabase* pDb = new CContentDatabase();
  stringstream sSql;
  unsigned int nId;
  OBJECT_TYPE  nObjType = CONTAINER_STORAGE_FOLDER;
    
  if(pFolderNode->AttributeAsUInt("id") > 0) {
    nId = pFolderNode->AttributeAsUInt("id");
  }
  else {
    nId = GetId();
  }
  
  sSql << "insert into VIRTUAL_CONTAINERS (ID, PARENT_ID, TYPE, TITLE, DEVICE) " <<
          "values(" << nId << ", " << p_nParentId << ", " << nObjType << ", '" <<
          pFolderNode->Attribute("name") << "', '" << p_sDevice << "')";
    
  pDb->Execute(sSql.str());    
  delete pDb;
  
  if(pFolderNode->ChildCount() > 0) {
    CreateChildItems(pFolderNode, p_sDevice, nId);
  }
}

void CVirtualContainerMgr::CreateVFoldersFromProperty(CXMLNode* pFoldersNode, 
                                                      std::string p_sDevice, 
                                                      unsigned int p_nParentId, 
                                                      std::string p_sFilter)
{
  string sProp = pFoldersNode->Attribute("property");
  string sField;
  OBJECT_TYPE nContainerType;
  
  if(sProp.compare("genre") == 0) {
    sField = "A_GENRE"; 
    nContainerType = CONTAINER_GENRE_MUSIC_GENRE;
  }
  else if(sProp.compare("artist") == 0) {
    sField = "A_ARTIST";
    nContainerType = CONTAINER_PERSON_MUSIC_ARTIST;
  }
  else if(sProp.compare("album") == 0) {
    sField = "A_ALBUM"; 
    nContainerType = CONTAINER_ALBUM_MUSIC_ALBUM;
  } 
  else {
    cout << "unhandled property '" << sProp << "'" << endl;
    return;
  }
 

  
  stringstream sSql;
  sSql << "select distinct d." << sField << " from OBJECT_DETAILS d ";
  if(p_sFilter.length() > 0) {
    sSql << " where " << p_sFilter;
  }
  
  CContentDatabase* pDb = new CContentDatabase();
  CContentDatabase* pTmpDb = new CContentDatabase();
  unsigned int nId;
  pDb->Select(sSql.str());
  while(!pDb->Eof()) {
    
    nId = GetId();
    //cout << nId << ": " << pDb->GetResult()->GetValue(sField) << endl;
    
    sSql.str("");
    sSql << "insert into VIRTUAL_CONTAINERS (ID, PARENT_ID, TYPE, TITLE, DEVICE) values " <<
      "(" << nId << ", " << p_nParentId << ", " << nContainerType << ", '" <<
      pDb->GetResult()->GetValue(sField) << "', '" << p_sDevice << "')";
    
    pTmpDb->Execute(sSql.str());
    
    sSql.str("");
    sSql << " d." << sField << " = '" << pDb->GetResult()->GetValue(sField) << "' ";
    
    if(p_sFilter.length() > 0) {
      sSql << " and " << p_sFilter;
    }
    
    if(pFoldersNode->ChildCount() > 0) {
      CreateChildItems(pFoldersNode, p_sDevice, nId, sSql.str());
    }
    
    pDb->Next();
  }
  delete pDb;
     
}

void CVirtualContainerMgr::CreateItemMappings(CXMLNode* pNode, 
                                              std::string p_sDevice, 
                                              unsigned int p_nParentId, 
                                              std::string p_sFilter)
{
  CContentDatabase* pDb = new CContentDatabase;
  CContentDatabase* pIns = new CContentDatabase;
  stringstream sSql;
  OBJECT_TYPE nObjectType;                                                
                                                
  if(pNode->Attribute("type").compare("audioItem") == 0) {
    nObjectType = ITEM_AUDIO_ITEM_MUSIC_TRACK;
  }
  else if(pNode->Attribute("type").compare("imageItem") == 0) {
    nObjectType = ITEM_IMAGE_ITEM_PHOTO;
  }
    
  sSql.str("");
  sSql <<  "select * from OBJECTS o " <<
           "left join OBJECT_DETAILS d on (d.OBJECT_ID = o.ID) " <<
           "where TYPE = " << nObjectType;    
  
  if(p_sFilter.length() > 0) {
    sSql << " and " << p_sFilter;
  }
        
  //cout << sSql.str() << endl;
    
  pDb->Select(sSql.str());
  pIns->BeginTransaction();
  while(!pDb->Eof()) {
      
    sSql.str("");
    sSql << "insert into MAP_ITEMS2VC " <<
            "(OBJECT_ID, VCONTAINER_ID, DEVICE) values " <<
            "(" << pDb->GetResult()->GetValue("ID") << ", " << p_nParentId << ", '" << p_sDevice << "')";      
    pIns->Execute(sSql.str());
    
    pDb->Next();
  }
  pIns->Commit();
        
  delete pIns;
  delete pDb;
}

void CVirtualContainerMgr::CreateFolderMappings(CXMLNode* pNode, 
                                                std::string p_sDevice, 
                                                unsigned int p_nParentId, 
                                                std::string p_sFilter)
{
  #warning TODO: doubles

  CContentDatabase* pDb;
  CContentDatabase* pIns = NULL;
  stringstream sSql;
  string sFilter;
  OBJECT_TYPE nObjectType; 
                                                  
  if(pNode->Attribute("filter").length() > 0) {
    sFilter = pNode->Attribute("filter");
    
    if(sFilter.compare("count(audioItem) > 0") == 0) {
      nObjectType = ITEM_AUDIO_ITEM_MUSIC_TRACK;
    }
    else if(sFilter.compare("count(imageItem) > 0") == 0) {
      nObjectType = ITEM_IMAGE_ITEM_PHOTO;
    }    
  }
  else {
    cout << "unhandled folders attribute " << __FILE__ << " " << __LINE__ << endl;
    return;
  }
  
  pDb  = new CContentDatabase();
  pIns = new CContentDatabase();
                                                  
  sSql << "select * from OBJECTS where ID in " <<
          "(select distinct PARENT_ID from OBJECTS where TYPE = " << nObjectType << ")";
                
  //cout << sSql.str() << endl; fflush(stdout);
                                                  
  pDb->Select(sSql.str());
  while(!pDb->Eof()) {
    sSql.str("");
    sSql << "insert into MAP_ITEMS2VC " <<
            "(OBJECT_ID, VCONTAINER_ID, DEVICE) values " <<
            "(" << pDb->GetResult()->GetValue("ID") << ", " << p_nParentId << ", '" << p_sDevice << "')";      
    pIns->Execute(sSql.str());
    
    pDb->Next();
  }

  delete pIns;
  delete pDb;
}


bool CVirtualContainerMgr::IsVirtualContainer(unsigned int p_nContainerId, std::string p_sDevice)
{
  bool bResult = false;
  
  if(p_nContainerId == 0)
    return bResult;
  
	CContentDatabase* pDb = new CContentDatabase();
	stringstream sSql;
	sSql << "select count(*) as VALUE from VIRTUAL_CONTAINERS where ID = " << p_nContainerId << " and DEVICE = '" << p_sDevice << "';";
	
	cout << __FILE__ << " is virtual cont: " << p_nContainerId;
	
	pDb->Select(sSql.str());
	bResult = (pDb->GetResult()->GetValue("VALUE").compare("0") != 0);
	
	cout << bResult << endl;
	
	delete pDb;
	
	return bResult;
}


bool CVirtualContainerMgr::HasVirtualChildren(unsigned int p_nParentId, std::string p_sDevice, bool* p_bContainerChildren)
{
  bool bResult = false;
	CContentDatabase* pDb = new CContentDatabase();
	stringstream sSql;
	sSql << "select count(*) as VALUE from VIRTUAL_CONTAINERS where PARENT_ID = " << p_nParentId << " and DEVICE = '" << p_sDevice << "';";
	
	cout << __FILE__ << " has virtual children: " << p_nParentId << endl;
	
	pDb->Select(sSql.str());
	bResult = (pDb->GetResult()->GetValue("VALUE").compare("0") != 0);
	
  if(bResult) {
    *p_bContainerChildren = true;
    delete pDb;
    return true;
  }
  else {
    *p_bContainerChildren = false;
    
    sSql.str("");
    sSql << "select count(*) as VALUE from MAP_ITEMS2VC where VCONTAINER_ID = " << p_nParentId << " and DEVICE = '" << p_sDevice << "';";
    pDb->Select(sSql.str());
	  bResult = (pDb->GetResult()->GetValue("VALUE").compare("0") != 0);
  }
  
  
  if(bResult)
	  cout << "true" << endl;
  else
    	cout << "false" << endl;
	
	delete pDb;
	
	return bResult;  
}

int CVirtualContainerMgr::GetChildCount(unsigned int p_nParentId, std::string p_sDevice)
{
  int nResult = 0;
  
  CContentDatabase* pDb = new CContentDatabase();
  stringstream sSql;
  sSql << "select count(*) as COUNT from VIRTUAL_CONTAINERS where " << 
          "  PARENT_ID = " << p_nParentId << " and device = '" << p_sDevice << "' " <<
          "union all " <<
          "select count(*) as COUNT from MAP_ITEMS2VC where " <<
          "  VCONTAINER_ID = " << p_nParentId << " and device = '" << p_sDevice << "' ";
  
  pDb->Select(sSql.str());
  nResult = atoi(pDb->GetResult()->GetValue("COUNT").c_str());
  // if we have no child containers
  // check if we have child item mappings
  if(nResult == 0) {
    pDb->Next();
    nResult = atoi(pDb->GetResult()->GetValue("COUNT").c_str());  
  }
  
  delete pDb;
  return nResult;
}
