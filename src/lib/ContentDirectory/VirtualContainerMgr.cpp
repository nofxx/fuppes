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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "VirtualContainerMgr.h"

#include "../Common/Common.h"
#include "ContentDatabase.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include <iostream>
using namespace std;
		
CVirtualContainerMgr* CVirtualContainerMgr::m_pInstance = 0;

static bool g_bIsRebuilding;

CVirtualContainerMgr* CVirtualContainerMgr::Shared()
{
  if(m_pInstance == 0)
	  m_pInstance = new CVirtualContainerMgr();
	return m_pInstance;
}

CVirtualContainerMgr::CVirtualContainerMgr()
{
	m_nIdCounter    = 0;
  m_RebuildThread = (fuppesThread)NULL;
}


CVirtualContainerMgr::~CVirtualContainerMgr()
{
	if(m_RebuildThread != (fuppesThread)NULL) {
    fuppesThreadClose(m_RebuildThread);  
  }
}


bool CVirtualContainerMgr::IsRebuilding()
{
  return g_bIsRebuilding;
}    

fuppesThreadCallback VirtualContainerBuildLoop(void *arg)
{
  CVirtualContainerMgr* pMgr = (CVirtualContainerMgr*)arg;
    
  CSharedLog::Shared()->Log(L_NORMAL, "[VirtualContainer] create virtual container layout", __FILE__, __LINE__);
  
  CXMLDocument* pDoc = new CXMLDocument();
  if(!pDoc->Load(CSharedConfig::Shared()->GetConfigDir() + "vfolder.cfg")) {    
    CSharedLog::Shared()->Log(L_ERROR, "[VirtualContainer] error loading vfolder.cfg", __FILE__, __LINE__);
    delete pDoc;
    g_bIsRebuilding = false;
    fuppesThreadExit();
  }

  CContentDatabase* pDb = new CContentDatabase();  
  pDb->Execute("delete from OBJECTS where DEVICE is NOT NULL;");	
  pDb->Execute("delete from MAP_OBJECTS where DEVICE is NOT NULL;");	
  delete pDb;
 
  int i;
  CXMLNode* pChild;
  string sDevice;
  
  for(i = 0; i < pDoc->RootNode()->ChildCount(); i++) {
    pChild = pDoc->RootNode()->ChildNode(i);
 
    if((pChild->Name().compare("vfolder_layout") == 0) && 
       (pChild->Attribute("enabled").compare("true") == 0)) {
         
      sDevice = pChild->Attribute("device");
      pMgr->CreateChildItems(pChild, sDevice, 0, NULL);      
    }
    
  }
  
  delete pDoc;    
  
  CSharedLog::Shared()->Log(L_NORMAL, "[VirtualContainer] virtual container layout created", __FILE__, __LINE__);
  
  g_bIsRebuilding = false;
  fuppesThreadExit();
}

void CVirtualContainerMgr::RebuildContainerList()
{
  if(CContentDatabase::Shared()->IsRebuilding()) {
    CSharedLog::Shared()->Log(L_NORMAL, "database rebuild in progress", __FILE__, __LINE__);
    return;
  }
  
  if(!g_bIsRebuilding) {
    g_bIsRebuilding = true;
    
    if(m_RebuildThread != (fuppesThread)NULL) {
      fuppesThreadClose(m_RebuildThread);
      m_RebuildThread = (fuppesThread)NULL;
    }
		
    fuppesThreadStart(m_RebuildThread, VirtualContainerBuildLoop);    
  }  
}

void CVirtualContainerMgr::CreateChildItems(CXMLNode* pParentNode, 
                                            std::string p_sDevice, 
                                            unsigned int p_nParentId,
                                            CObjectDetails* pDetails,
                                            std::string p_sFilter)
{
  CXMLNode* pNode;
  int i;
  bool bDetails = false;                                              
                                              
  for(i = 0; i < pParentNode->ChildCount(); i++) {
    pNode = pParentNode->ChildNode(i);
        
    if(pDetails == NULL) {
      pDetails = new CObjectDetails();
      bDetails = true;
    }      
      
    if(pNode->Name().compare("vfolder") == 0) {
      //cout << "create single vfolder: " << pNode->Attribute("name") << " :: " << p_sFilter << endl; fflush(stdout);
      CreateSingleVFolder(pNode, p_sDevice, p_nParentId, pDetails);
    }
    else if(pNode->Name().compare("vfolders") == 0) {
      //cout << "create vfolders from property: " << pNode->Attribute("property") << " :: " << p_sFilter << endl;  fflush(stdout);
      if(pNode->Attribute("property").length() > 0) {
        CreateVFoldersFromProperty(pNode, p_sDevice, p_nParentId, pDetails, p_sFilter);
      }
      else if(pNode->Attribute("split").length() > 0) {
        CreateVFoldersSplit(pNode, p_sDevice, p_nParentId, pDetails, p_sFilter);
      }
    }
    else if(pNode->Name().compare("items") == 0) {
      //cout << "create item mappings for type: " << pNode->Attribute("type") << " :: " << p_sFilter << endl;
      CreateItemMappings(pNode, p_sDevice, p_nParentId, p_sFilter);
    }
    else if(pNode->Name().compare("folders") == 0) {
      //cout << "create folder mappings for filter: " << pNode->Attribute("filter") << " :: " << p_sFilter << endl;
      CreateFolderMappings(pNode, p_sDevice, p_nParentId, p_sFilter);
    }
    else if(pNode->Name().compare("shared_dirs") == 0) {
      MapSharedDirsTo(pNode, p_sDevice, p_nParentId);
    }
    
    if(bDetails) {
      delete pDetails;
      pDetails = NULL;
    }

  }
}

void CVirtualContainerMgr::CreateSingleVFolder(CXMLNode* pFolderNode, std::string p_sDevice, unsigned int p_nParentId, CObjectDetails* pDetails)
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
  
  //pDb->BeginTransaction();
  
  sSql << "insert into OBJECTS (OBJECT_ID, TYPE, PATH, TITLE, FILE_NAME, DEVICE) " <<
          "values " <<
          "( " << nId << 
          ", " << nObjType << 
          ", 'virtual' " <<
          ", '" <<  SQLEscape(pFolderNode->Attribute("name")) << "'" <<
          ", '" <<  SQLEscape(pFolderNode->Attribute("name")) << "'" <<
          ", '" << p_sDevice << "')";
  
  pDb->Execute(sSql.str());
  sSql.str("");
  
  sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
    "( "  << nId << 
    ", "  << p_nParentId << 
    ", '" << p_sDevice << "');";
  
  pDb->Insert(sSql.str());  
  //pDb->Commit();
  
  
  delete pDb;
  
  if(pFolderNode->ChildCount() > 0) {
    CreateChildItems(pFolderNode, p_sDevice, nId, pDetails);
  }
}

void CVirtualContainerMgr::CreateVFoldersFromProperty(CXMLNode* pFoldersNode, 
                                                      std::string p_sDevice, 
                                                      unsigned int p_nParentId, 
                                                      CObjectDetails* pDetails,
                                                      std::string p_sFilter)
{
  string sProp = pFoldersNode->Attribute("property");
  string sField;
  OBJECT_TYPE nContainerType;
  
  if(sProp.compare("genre") == 0) {
    sField = "d.A_GENRE"; 
    nContainerType = CONTAINER_GENRE_MUSIC_GENRE;
  }
  else if(sProp.compare("artist") == 0) {
    sField = "d.A_ARTIST";
    nContainerType = CONTAINER_PERSON_MUSIC_ARTIST;
  }
  else if(sProp.compare("album") == 0) {
    sField = "d.A_ALBUM"; 
    nContainerType = CONTAINER_ALBUM_MUSIC_ALBUM;
  } 
  else {
    #warning: todo properties
    cout << "unhandled property '" << sProp << "'" << endl;
    return;
  }
  
  stringstream sSql;
  sSql << 
    "select distinct " <<
      sField << " as VALUE " <<
    "from " <<
    "  OBJECT_DETAILS d, " <<
    "  OBJECTS o " <<
    "where " <<
    "  o.DETAIL_ID = d.ID and " <<
    "  o.TYPE > " << ITEM;

  string sTmp;
  if(p_sFilter.length() > 0) {
    sTmp.resize(p_sFilter.length() + sField.length());    
    sprintf(&sTmp[0], p_sFilter.c_str(), sField.c_str());
    p_sFilter = sTmp;
    sSql << " and " << p_sFilter;
  }
  //cout << "CreateVFoldersFromProperty: " << sSql.str() << endl;
                                                          
  
  CContentDatabase* pDb    = new CContentDatabase();
  CContentDatabase* pTmpDb = new CContentDatabase();
  unsigned int nId;
  unsigned int nDetailId;
  string sTitle;

  pDb->Select(sSql.str());
  while(!pDb->Eof()) {
    
    sSql.str("");
    nId = GetId();
    
    // build details
    switch(nContainerType)
    {
      case CONTAINER_GENRE_MUSIC_GENRE:
        pDetails->sGenre = SQLEscape(pDb->GetResult()->GetValue("VALUE"));
        break;
      case CONTAINER_PERSON_MUSIC_ARTIST:
        pDetails->sArtist = SQLEscape(pDb->GetResult()->GetValue("VALUE"));
        break;
      case CONTAINER_ALBUM_MUSIC_ALBUM:
        pDetails->sAlbum = SQLEscape(pDb->GetResult()->GetValue("VALUE"));
        break;      
      default:
        cout << "unhandled property '" << sProp << "'" << endl;
        pDb->Next();
        continue;
        break;
    }    
    
    // escape title
    sTitle = SQLEscape(pDb->GetResult()->GetValue("VALUE"));
    if(sTitle.length() == 0) {
      sTitle = "unknown";
    }    
    
    // insert container details
    sSql << "insert into OBJECT_DETAILS (A_ARTIST, A_ALBUM, A_GENRE) " <<
      "values " <<
      "( '" << pDetails->sArtist << "' " <<
      ", '" << pDetails->sAlbum << "' " <<
      ", '" << pDetails->sGenre << "')";
    
    nDetailId = pTmpDb->Insert(sSql.str());
    sSql.str("");                            
           
    // insert container    
    sSql << "insert into OBJECTS (OBJECT_ID, DETAIL_ID, TYPE, PATH, TITLE, FILE_NAME, DEVICE) "
      "values " <<
      "( " << nId << 
      ", " << nDetailId << 
      ", " << nContainerType << 
      ", '" << "virtual" << "'" <<
      ", '" << sTitle << "'" <<
      ", '" << sTitle << "'" <<
      ", '" << p_sDevice << "')";    
    
    pTmpDb->Execute(sSql.str());    
    sSql.str("");
    
    // map container to parent
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) "
      "values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    
    pTmpDb->Execute(sSql.str());    
    sSql.str("");  
        
    // build filter
    sSql << sField << " = '" << SQLEscape(pDb->GetResult()->GetValue("VALUE")) << "' ";    
    if(p_sFilter.length() > 0) {
      sSql << " and " << p_sFilter;
    }
    
    // create child items
    if(pFoldersNode->ChildCount() > 0) {
      CreateChildItems(pFoldersNode, p_sDevice, nId, pDetails, sSql.str());
    }
    
    pDb->Next();
  }

  // cleanup
  delete pDb;
  delete pTmpDb;
     
}

void CVirtualContainerMgr::CreateVFoldersSplit(CXMLNode* pFoldersNode, 
                                               std::string p_sDevice, 
                                               unsigned int p_nParentId,
                                               CObjectDetails* pDetails,
                                               std::string p_sFilter)
{
  string sFolders[] = {
    "0-9", "ABC", "DEF", "GHI", "JKL",
    "MNO" , "PQR", "STU", "VWX", "YZ",
    ""
  };
                                                   
  string sFilter[] = {
    " substr(%s, 0, 1) in (0, 1, 2, 3, 4, 5, 6, 7, 8, 9) ", 
    " upper(substr(%s, 0, 1)) in ('A', 'B', 'C') ",
    " upper(substr(%s, 0, 1)) in ('D', 'E', 'F') ",
    " upper(substr(%s, 0, 1)) in ('G', 'H', 'I') ",
    " upper(substr(%s, 0, 1)) in ('J', 'K', 'L') ",
    " upper(substr(%s, 0, 1)) in ('M', 'N', 'O') ",
    " upper(substr(%s, 0, 1)) in ('P', 'Q', 'R') ",
    " upper(substr(%s, 0, 1)) in ('S', 'T', 'U') ",
    " upper(substr(%s, 0, 1)) in ('V', 'W', 'X') ",
    " upper(substr(%s, 0, 1)) in ('Y', 'Z') ",
    ""
  };
                                                   
  int i = 0;
  unsigned int nId;
  stringstream sSql;
  CContentDatabase* pDb = new CContentDatabase();
                                                   
  while(sFolders[i].length() > 0) {
    
    nId = GetId();
      
    sSql.str("");
    sSql << "insert into OBJECTS (OBJECT_ID, TYPE, PATH, FILE_NAME, TITLE, DEVICE) values " <<
      "(" << nId << ", " << CONTAINER_STORAGE_FOLDER << ", " << "'virtual', '" << 
      sFolders[i] << "', '" << sFolders[i] << "', '" << p_sDevice << "')";
    pDb->Execute(sSql.str());
    
    sSql.str(""); 
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    
    pDb->Execute(sSql.str());    
    
    
    if(pFoldersNode->ChildCount() > 0) {
      CreateChildItems(pFoldersNode, p_sDevice, nId, pDetails, sFilter[i]);
    }
      
    i++;
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
  else if(pNode->Attribute("type").compare("videoItem") == 0) {
    nObjectType = ITEM_VIDEO_ITEM_MOVIE;
  }                                             
    
  sSql.str("");
  sSql << 
    "select " << 
    "  o.DETAIL_ID, o.TITLE, o.TYPE, o.FILE_NAME, o.PATH, o.MIME_TYPE " <<
    "from " <<
    "  OBJECTS o " <<
    "left join " <<
    "  OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  o.DEVICE is NULL and " <<
    "  o.TYPE = " << nObjectType;                                                 


  if(p_sFilter.length() > 0) {
    sSql << " and " << p_sFilter;
  }
        
  //cout << "CreateItemMappings: " << sSql.str() << endl;
    
  pDb->Select(sSql.str());
  //pIns->BeginTransaction();
  while(!pDb->Eof()) {
      
    sSql.str("");
    unsigned int nId = GetId();
    
    sSql << "insert into OBJECTS (OBJECT_ID, DETAIL_ID, DEVICE, TYPE, PATH, TITLE, FILE_NAME, MIME_TYPE)" <<
      "values " <<
      "( " << nId << 
      ", " << pDb->GetResult()->GetValue("DETAIL_ID") << 
      ", '" << p_sDevice << "'" <<
      ", " << pDb->GetResult()->GetValue("TYPE") << 
      ", '" << SQLEscape(pDb->GetResult()->GetValue("PATH")) << "'" <<
      ", '" << SQLEscape(pDb->GetResult()->GetValue("TITLE")) << "'" <<
      ", '" << SQLEscape(pDb->GetResult()->GetValue("FILE_NAME")) << "'" <<
      ", '" << pDb->GetResult()->GetValue("MIME_TYPE") << "')";
           
    pIns->Execute(sSql.str());
    
    sSql.str(""); 
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    
    pIns->Execute(sSql.str());   
    
    
    pDb->Next();
  }
  //pIns->Commit();
        
  delete pIns;
  delete pDb;
}

void CVirtualContainerMgr::CreateFolderMappings(CXMLNode* pNode, 
                                                std::string p_sDevice, 
                                                unsigned int p_nParentId, 
                                                std::string p_sFilter)
{

  CContentDatabase* pDb;
  CContentDatabase* pIns = NULL;
  stringstream sSql;
  string sFilter;
  OBJECT_TYPE nObjectType; 
                                                  
  if(pNode->Attribute("filter").length() > 0) {
    sFilter = pNode->Attribute("filter");
    
    if(sFilter.compare("contains(audioItem)") == 0) {
      nObjectType = ITEM_AUDIO_ITEM_MUSIC_TRACK;
    }
    else if(sFilter.compare("contains(imageItem)") == 0) {
      nObjectType = ITEM_IMAGE_ITEM_PHOTO;
    }
    else if(sFilter.compare("contains(videoItem)") == 0) {
      nObjectType = ITEM_VIDEO_ITEM_MOVIE;
    }
  }
  else {
    cout << "unhandled folders attribute " << __FILE__ << " " << __LINE__ << endl;
    return;
  }
  
  pDb  = new CContentDatabase();
  pIns = new CContentDatabase();
         
  sSql << 
    "select " <<
    "  distinct m.PARENT_ID " <<
    "from " <<
    "  OBJECTS o, " <<
    "  MAP_OBJECTS m " <<
    "where " <<
    "  o.TYPE = " << nObjectType << " and " <<
    "  m.OBJECT_ID = o.OBJECT_ID and " <<
    "  o.DEVICE is NULL and " <<
    "  m.DEVICE is NULL";
                
  //cout << sSql.str() << endl; fflush(stdout);
                                                  
  pDb->Select(sSql.str());
  while(!pDb->Eof()) {
    
    sSql.str("");    
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << pDb->GetResult()->GetValue("PARENT_ID") << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    pIns->Execute(sSql.str());
    
    pDb->Next();
  }

  delete pIns;
  delete pDb;
}

void CVirtualContainerMgr::MapSharedDirsTo(CXMLNode* pNode, std::string p_sDevice, unsigned int p_nParentId)
{
  stringstream sSql;
  CContentDatabase* pSel = new CContentDatabase();
  CContentDatabase* pIns = new CContentDatabase();
  
  sSql << "select OBJECT_ID from MAP_OBJECTS where PARENT_ID = 0 and DEVICE is NULL";
  pSel->Select(sSql.str());
  
  //pIns->BeginTransaction();
  while(!pSel->Eof()) {
    
    sSql.str("");    
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << pSel->GetResult()->GetValue("OBJECT_ID") << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    pIns->Execute(sSql.str());
    
    pSel->Next();
  }
  //pIns->Commit();
  
  delete pIns;
  delete pSel;
}

bool CVirtualContainerMgr::IsVirtualContainer(unsigned int p_nContainerId, std::string p_sDevice)
{
  bool bResult = false;
  
  if(p_nContainerId == 0)
    return bResult;
  
	CContentDatabase* pDb = new CContentDatabase();
	stringstream sSql;
	sSql << "select count(*) as VALUE from OBJECTS where OBJECT_ID = " << p_nContainerId << " and DEVICE = '" << p_sDevice << "';";
		
	pDb->Select(sSql.str());
	bResult = (pDb->GetResult()->GetValue("VALUE").compare("0") != 0);
    
	delete pDb;
	
	return bResult;
}


bool CVirtualContainerMgr::HasVirtualChildren(unsigned int p_nParentId, std::string p_sDevice)
{
  bool bResult = false;
	CContentDatabase* pDb = new CContentDatabase();
	stringstream sSql;
	sSql << "select count(*) as VALUE from MAP_OBJECTS where PARENT_ID = " << p_nParentId << " and DEVICE = '" << p_sDevice << "';";
	
	pDb->Select(sSql.str());
	bResult = (pDb->GetResult()->GetValue("VALUE").compare("0") != 0);
	
	delete pDb;
	
	return bResult;  
}

int CVirtualContainerMgr::GetChildCount(unsigned int p_nParentId, std::string p_sDevice)
{
  int nResult = 0;
  
  CContentDatabase* pDb = new CContentDatabase();
  stringstream sSql;
  sSql << "select count(*) as COUNT from OBJECTS where " << 
          "  PARENT_ID = " << p_nParentId << " and device = '" << p_sDevice << "' ";
  
  pDb->Select(sSql.str());
  nResult = atoi(pDb->GetResult()->GetValue("COUNT").c_str());
  
  delete pDb;
  return nResult;
}
