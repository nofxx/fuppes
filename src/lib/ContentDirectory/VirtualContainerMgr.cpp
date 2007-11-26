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

static std::string VFOLDER_CFG_VERSION = "0.2";
		
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
  m_bVFolderCfgValid = true;
  
	m_nIdCounter    = 0;
  m_RebuildThread = (fuppesThread)NULL;
  if(!FileExists(CSharedConfig::Shared()->GetVFolderConfigFileName())) {
    CSharedLog::Log(L_NORM, __FILE__, __LINE__, "no vfolder.cfg file available");
    m_bVFolderCfgValid = false;
  }
  else {
    CXMLDocument* pDoc = new CXMLDocument();
    if(pDoc->Load(CSharedConfig::Shared()->GetVFolderConfigFileName())) {
      if(pDoc->RootNode()->Attribute("version").compare(VFOLDER_CFG_VERSION) != 0) {
        CSharedLog::Log(L_NORM, __FILE__, __LINE__, "vfolder.cfg has wrong version");
        m_bVFolderCfgValid = false;
      }
    }
    else {
      m_bVFolderCfgValid = false;
    }
    delete pDoc;    
  }  
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
    
  time_t now;
  char nowtime[26];
  time(&now);
	#ifndef WIN32
  ctime_r(&now, nowtime);
	nowtime[24] = '\0';
	string sNowtime = nowtime;
	#else	
  char timeStr[9];  
  _strtime(timeStr);	
	string sNowtime = timeStr;	
	#endif
  CSharedLog::Print("[VirtualContainer] create virtual container layout started at %s", sNowtime.c_str());
  
  CXMLDocument* pDoc = new CXMLDocument();
  if(!pDoc->Load(CSharedConfig::Shared()->GetVFolderConfigFileName())) {    
    CSharedLog::Print("[VirtualContainer] error loading %s", CSharedConfig::Shared()->GetVFolderConfigFileName().c_str());
    delete pDoc;
    g_bIsRebuilding = false;
    fuppesThreadExit();
  }

  CContentDatabase* pIns = new CContentDatabase();  
  pIns->Execute("delete from OBJECTS where DEVICE is NOT NULL;");	
  pIns->Execute("delete from MAP_OBJECTS where DEVICE is NOT NULL;");	  
  pIns->Execute("vacuum");
 
  int i;
  CXMLNode* pChild;
  string    sDevice;
  bool      bContainerDetails = false;
	bool			bCreateRef = false;
  
  for(i = 0; i < pDoc->RootNode()->ChildCount(); i++) {
    pChild = pDoc->RootNode()->ChildNode(i);
 
    if((pChild->Name().compare("vfolder_layout") == 0) && 
       (pChild->Attribute("enabled").compare("true") == 0)) {
         
      sDevice = SQLEscape(pChild->Attribute("device"));
      if(pChild->Attribute("create_container_details").compare("true") == 0) {
        bContainerDetails = true;
      }
			if(pChild->Attribute("create_references").compare("true") == 0) {
        bCreateRef = true;
      }
					 
      pMgr->CreateChildItems(pChild, pIns, sDevice, 0, NULL, bContainerDetails, bCreateRef);
    }
    
  }
  
  delete pDoc;    
  
  delete pIns;

  time(&now); 
	#ifndef WIN32
  ctime_r(&now, nowtime);
	nowtime[24] = '\0';
	sNowtime = nowtime;
	#else	  
  _strtime(timeStr);	
	sNowtime = timeStr;	
	#endif
  CSharedLog::Print("[VirtualContainer] virtual container layout created at %s", sNowtime.c_str());

  g_bIsRebuilding = false;
  fuppesThreadExit();
}

void CVirtualContainerMgr::RebuildContainerList()
{
  if(CContentDatabase::Shared()->IsRebuilding()) {
    CSharedLog::Shared()->Log(L_NORM, "database rebuild in progress", __FILE__, __LINE__);
    return;
  }
 
  if(!m_bVFolderCfgValid) {
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
                                            CContentDatabase* pIns,
                                            std::string p_sDevice, 
                                            unsigned int p_nParentId,
                                            CObjectDetails* pDetails,
                                            bool p_bContainerDetails,
																						bool p_bCreateRef,
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
      CSharedLog::Shared()->Log(L_EXT, "create single vfolder: " + pNode->Attribute("name") + " :: " + p_sFilter, __FILE__, __LINE__);
      CreateSingleVFolder(pNode, pIns, p_sDevice, p_nParentId, pDetails, p_bContainerDetails, p_bCreateRef);
    }
    else if(pNode->Name().compare("vfolders") == 0) {      
      if(pNode->Attribute("property").length() > 0) {
        //cout << "create vfolders from property: " << pNode->Attribute("property") << " :: " << p_sFilter << endl;  fflush(stdout);      
        CSharedLog::Shared()->Log(L_EXT, "create vfolders from property: " + pNode->Attribute("property") + " :: " + p_sFilter, __FILE__, __LINE__);
        CreateVFoldersFromProperty(pNode, pIns, p_sDevice, p_nParentId, pDetails, p_bContainerDetails, p_bCreateRef, p_sFilter);
      }
      else if(pNode->Attribute("split").length() > 0) {
        CSharedLog::Shared()->Log(L_EXT, "create split vfolders :: " + p_sFilter, __FILE__, __LINE__);
        CreateVFoldersSplit(pNode, pIns, p_sDevice, p_nParentId, pDetails, p_bContainerDetails, p_bCreateRef, p_sFilter);
      }
    }
    else if(pNode->Name().compare("items") == 0) {
      //cout << "create item mappings for type: " << pNode->Attribute("type") << " :: " << p_sFilter << endl;
      CSharedLog::Shared()->Log(L_EXT, "create item mappings for type: " + pNode->Attribute("type") + " :: " + p_sFilter, __FILE__, __LINE__);
      CreateItemMappings(pNode, pIns, p_sDevice, p_nParentId, p_bCreateRef, p_sFilter);
    }
    else if(pNode->Name().compare("folders") == 0) {
      //cout << "create folder mappings for filter: " << pNode->Attribute("filter") << " :: " << p_sFilter << endl;
      CSharedLog::Shared()->Log(L_EXT, "create folder mappings - filter: " + pNode->Attribute("filter") + " :: " + p_sFilter, __FILE__, __LINE__);
      CreateFolderMappings(pNode, pIns, p_sDevice, p_nParentId, p_bCreateRef, p_sFilter);
    }
    else if(pNode->Name().compare("shared_dirs") == 0) {
      CSharedLog::Shared()->Log(L_EXT, "create shared dir mappings :: " + p_sFilter, __FILE__, __LINE__);
      MapSharedDirsTo(pNode, pIns, p_sDevice, p_nParentId);
    }
    
    if(bDetails) {
      delete pDetails;
      pDetails = NULL;
    }

  }
}

void CVirtualContainerMgr::CreateSingleVFolder(CXMLNode* pFolderNode,
                                               CContentDatabase* pIns,
                                               std::string p_sDevice,
                                               unsigned int p_nParentId,
                                               CObjectDetails* pDetails,
                                               bool p_bContainerDetails,
																							 bool p_bCreateRef)
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
  
  pIns->Execute(sSql.str());
  sSql.str("");
  
  sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
    "( "  << nId << 
    ", "  << p_nParentId << 
    ", '" << p_sDevice << "');";
  
  pIns->Insert(sSql.str());  
  //pDb->Commit();
  
  
  delete pDb;
  
  if(pFolderNode->ChildCount() > 0) {
    CreateChildItems(pFolderNode, pIns, p_sDevice, nId, pDetails, p_bContainerDetails, p_bCreateRef);
  }
}

void CVirtualContainerMgr::CreateVFoldersFromProperty(CXMLNode* pFoldersNode, 
                                                      CContentDatabase* pIns,
                                                      std::string p_sDevice, 
                                                      unsigned int p_nParentId, 
                                                      CObjectDetails* pDetails,
                                                      bool p_bContainerDetails,
																											bool p_bCreateRef,
                                                      std::string p_sFilter)
{
  string sProp = pFoldersNode->Attribute("property");
  string sField;
  string sFields;
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
    sField  = "d.A_ALBUM";
    if(p_bContainerDetails) {
      sFields = "d.A_ARTIST, d.A_GENRE";
    }
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
      sField << " as VALUE ";
    //"  d.A_ALBUM, d.A_ARTIST, d.A_GENRE " <<                           
         
  if(!sFields.empty()) {
    sSql << ", " << sFields << " ";
  }
                                                          
  sSql <<
    "from " <<
    "  OBJECT_DETAILS d, " <<
    "  OBJECTS o " <<
    "where " <<
    "  VALUE is not NULL and " <<
    "  o.DETAIL_ID = d.ID and " <<
		"  o.DEVICE is NULL and " <<
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
        // artist does not necessarily has exactly 1 genre
        //pDetails->sGenre  = SQLEscape(pDb->GetResult()->GetValue("d.A_GENRE"));
        break;
      case CONTAINER_ALBUM_MUSIC_ALBUM:
        pDetails->sAlbum  = SQLEscape(pDb->GetResult()->GetValue("VALUE"));
        pDetails->sArtist = SQLEscape(pDb->GetResult()->GetValue("d.A_ARTIST"));
        pDetails->sGenre  = SQLEscape(pDb->GetResult()->GetValue("d.A_GENRE"));
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
      sTitle = "Unknown";
    }    
    /*else if(nContainerType == CONTAINER_ALBUM_MUSIC_ALBUM) {
      sTitle = pDetails->sArtist + ": " + pDetails->sAlbum;
    }*/
    
    pIns->BeginTransaction();
    
    // insert container details
    sSql << "insert into OBJECT_DETAILS (A_ARTIST, A_ALBUM, A_GENRE) " <<
      "values " <<
      "( '" << pDetails->sArtist << "' " <<
      ", '" << pDetails->sAlbum << "' " <<
      ", '" << pDetails->sGenre << "')";
    
    nDetailId = pIns->Insert(sSql.str());
    sSql.str("");                            

		
		unsigned int nRefId = 0;

		// get ref id
		if(p_bCreateRef) {
			sSql << "select OBJECT_ID from OBJECTS where " <<
				"TYPE = " << nContainerType << " and " <<
				"PATH = 'virtual' and " <<
				"TITLE = " << sTitle << " and " <<
				"FILE_NAME = " << sTitle << " and " <<
				"DEVICE = " << p_sDevice << " and " <<
				"REF_ID is NULL;";
				
			pIns->Select(sSql.str());
			if(!pIns->Eof())
				nRefId = pIns->GetResult()->GetValueAsUInt("OBJECT_ID");
			sSql.str("");
		}
				
    // insert container
    sSql << "insert into OBJECTS " <<
			"(OBJECT_ID, DETAIL_ID, TYPE, PATH, TITLE, FILE_NAME, DEVICE, REF_ID) "
      "values " <<
      "( " << nId << 
      ", " << nDetailId << 
      ", " << nContainerType << 
      ", '" << "virtual" << "'" <<
      ", '" << sTitle << "'" <<
      ", '" << sTitle << "'" <<
      ", '" << p_sDevice << "' ";
			if(nRefId > 0) 
				sSql << ", " << nRefId << ");";
			else
				sSql << ", NULL );";
				
    
    pIns->Execute(sSql.str());    
    sSql.str("");
    
    // map container to parent
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) "
      "values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    
    pIns->Execute(sSql.str());    
    sSql.str("");  
       
    pIns->Commit();
    
    // build filter
    sSql << sField << " = '" << SQLEscape(pDb->GetResult()->GetValue("VALUE")) << "' ";    
    if(p_sFilter.length() > 0) {
      sSql << " and " << p_sFilter;
    }
    
    // create child items
    if(pFoldersNode->ChildCount() > 0) {
      CreateChildItems(pFoldersNode, pIns, p_sDevice, nId, pDetails, p_bContainerDetails, p_bCreateRef , sSql.str());
    }
    
    pDb->Next();
  }

  // cleanup
  delete pDb;
     
}

void CVirtualContainerMgr::CreateVFoldersSplit(CXMLNode* pFoldersNode, 
                                               CContentDatabase* pIns,
                                               std::string p_sDevice, 
                                               unsigned int p_nParentId,
                                               CObjectDetails* pDetails,
                                               bool p_bContainerDetails,
																							 bool p_bCreateRef,
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
                                                   
  while(sFolders[i].length() > 0) {
    
    nId = GetId();
      
    sSql.str("");
    sSql << "insert into OBJECTS (OBJECT_ID, TYPE, PATH, FILE_NAME, TITLE, DEVICE) values " <<
      "(" << nId << ", " << CONTAINER_STORAGE_FOLDER << ", " << "'virtual', '" << 
      sFolders[i] << "', '" << sFolders[i] << "', '" << p_sDevice << "')";
    pIns->Execute(sSql.str());
    
    sSql.str(""); 
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    
    pIns->Execute(sSql.str());    
    
    
    if(pFoldersNode->ChildCount() > 0) {
      CreateChildItems(pFoldersNode, pIns, p_sDevice, nId, pDetails, p_bContainerDetails, p_bCreateRef, sFilter[i]);
    }
      
    i++;
  }
                                                 
}

void CVirtualContainerMgr::CreateItemMappings(CXMLNode* pNode, 
                                              CContentDatabase* pIns,
                                              std::string p_sDevice, 
                                              unsigned int p_nParentId,
																							bool p_bCreateRef,
                                              std::string p_sFilter)
{
  CContentDatabase* pDb = new CContentDatabase;
  stringstream sSql;
  stringstream sObjType;
  bool bIncludeMap = false;  
                                                
  if(pNode->Attribute("type").compare("audioItem") == 0 ||
     pNode->Attribute("filter").compare("contains(audioItem)") == 0 ) {    
    sObjType << " in (" << ITEM_AUDIO_ITEM << ", " <<  ITEM_AUDIO_ITEM_MUSIC_TRACK << ")";
  }
  else if(pNode->Attribute("type").compare("imageItem") == 0 ||
          pNode->Attribute("filter").compare("contains(imageItem)") == 0 ) {    
    sObjType << " in (" << ITEM_IMAGE_ITEM << ", " <<  ITEM_IMAGE_ITEM_PHOTO << ")";
  }
  else if(pNode->Attribute("type").compare("videoItem") == 0 ||
          pNode->Attribute("filter").compare("contains(videoItem)") == 0 ) {    
    sObjType << " in (" << ITEM_VIDEO_ITEM << ", " <<  ITEM_VIDEO_ITEM_MOVIE << ")";
  }                     
    
  if(p_sFilter.length() > 10 && p_sFilter.compare(1, 10, "m.PARENT_ID", 1, 10) == 0) {
    bIncludeMap = true;
  }
                                                
  sSql.str("");
  sSql << 
    "select " << 
    "  o.DETAIL_ID, o.TITLE, o.TYPE, o.FILE_NAME, " <<
    "  o.PATH, d.A_ARTIST " <<
    "from " <<
    "  OBJECTS o ";

  if(bIncludeMap) {
    sSql << ", MAP_OBJECTS m ";
  }
  sSql <<                                                
    "left join " <<
    "  OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where ";
                                                
  if(bIncludeMap) {
    sSql << 
      "  m.OBJECT_ID = o.OBJECT_ID and " <<
      "  m.DEVICE is NULL and ";
  }

  sSql <<
    "  o.DEVICE is NULL and " <<
    "  o.TYPE " << sObjType.str();

  if(p_sFilter.length() > 0) {
    sSql << " and " << p_sFilter;
  }
    
    
  pDb->Select(sSql.str());  
  while(!pDb->Eof()) {
      
    sSql.str("");
    unsigned int nId = GetId();
		unsigned int nRefId = 0;
			
		if(p_bCreateRef) {
			sSql << "select OBJECT_ID from OBJECTS where " <<
				"DETAIL_ID = " << pDb->GetResult()->GetValue("DETAIL_ID") << " and " <<
				"DEVICE = '" << p_sDevice << "' and " <<
				"TYPE = " << pDb->GetResult()->GetValue("TYPE") << " and " <<
				"PATH = '" << SQLEscape(pDb->GetResult()->GetValue("PATH")) << "' and " <<
				"TITLE = '" << SQLEscape(pDb->GetResult()->GetValue("TITLE")) << "' and " <<
				"FILE_NAME = '" << SQLEscape(pDb->GetResult()->GetValue("FILE_NAME")) << "' and " <<
				"REF_ID is NULL";
			
			pIns->Select(sSql.str());
			if(!pIns->Eof())
				nRefId = pIns->GetResult()->GetValueAsUInt("OBJECT_ID");
			sSql.str("");
		}
			
    sSql << "insert into OBJECTS (OBJECT_ID, DETAIL_ID, DEVICE, TYPE, PATH, TITLE, FILE_NAME, REF_ID)" <<
      "values " <<
      "( " << nId << 
      ", " << pDb->GetResult()->GetValue("DETAIL_ID") << 
      ", '" << p_sDevice << "'" <<
      ", " << pDb->GetResult()->GetValue("TYPE") << 
      ", '" << SQLEscape(pDb->GetResult()->GetValue("PATH")) << "'" <<
      ", '" << SQLEscape(pDb->GetResult()->GetValue("TITLE")) << "'" <<
      ", '" << SQLEscape(pDb->GetResult()->GetValue("FILE_NAME")) << "'";
		if(nRefId > 0)
			sSql << ", " << nRefId << ");";
		else
			sSql << ", NULL);";
			
			
    pIns->Execute(sSql.str());
    
    sSql.str(""); 
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    
    pIns->Execute(sSql.str());   
    
    
    pDb->Next();
  }  
        
  delete pDb;
}

void CVirtualContainerMgr::CreateFolderMappings(CXMLNode* pNode, 
                                                CContentDatabase* pIns,
                                                std::string p_sDevice, 
                                                unsigned int p_nParentId, 
																								bool p_bCreateRef,
                                                std::string p_sFilter)
{
  CContentDatabase* pDb;
  stringstream sSql;
  string sFilter;  
  stringstream sObjType;
                                                  
  if(pNode->Attribute("filter").length() > 0) {
    sFilter = pNode->Attribute("filter");
    
    if(sFilter.compare("contains(audioItem)") == 0) {      
      sObjType << " in (" << ITEM_AUDIO_ITEM << ", " << ITEM_AUDIO_ITEM_MUSIC_TRACK << ")";
    }
    else if(sFilter.compare("contains(imageItem)") == 0) {      
      sObjType << " in (" << ITEM_IMAGE_ITEM << ", " << ITEM_IMAGE_ITEM_PHOTO << ")";
    }
    else if(sFilter.compare("contains(videoItem)") == 0) {      
      sObjType << " in (" << ITEM_VIDEO_ITEM << ", " << ITEM_VIDEO_ITEM_MOVIE << ")";
    }
  }
  else {
    cout << "unhandled folders attribute " << __FILE__ << " " << __LINE__ << endl;
    return;
  }

  pDb  = new CContentDatabase();
         
  sSql << 
    "select " <<
    "  distinct m.PARENT_ID " <<
    "from " <<
    "  OBJECTS o, " <<
    "  MAP_OBJECTS m " <<
    "where " <<
    "  o.TYPE " << sObjType.str() << " and " <<
    "  m.OBJECT_ID = o.OBJECT_ID and " <<
    "  o.DEVICE is NULL and " <<
    "  m.DEVICE is NULL";
                
  //cout << sSql.str() << endl;
                                                  
  pDb->Select(sSql.str());
  while(!pDb->Eof()) {
    CreateSingleVFolderFolder(pNode, pIns, p_sDevice, atoi(pDb->GetResult()->GetValue("PARENT_ID").c_str()), p_nParentId, p_bCreateRef);
    pDb->Next();
  }

  delete pDb;

}

void CVirtualContainerMgr::CreateSingleVFolderFolder(CXMLNode* pNode,
                                                     CContentDatabase* pIns,
                                                     std::string p_sDevice,
                                                     unsigned int p_nObjectId,
                                                     unsigned int p_nParentId,
																										 bool p_bCreateRef)
{
  CContentDatabase* pDb;
  stringstream sSql;
                                                  
  pDb  = new CContentDatabase();
         
  sSql << 
    "select " <<
    "  TITLE " <<
    "from " <<
    "  OBJECTS " <<
    "where " <<
		"  DEVICE is NULL and " <<
    "  OBJECT_ID = " << p_nObjectId;
                
  //cout << sSql.str() << endl; fflush(stdout);
                                                  
  pDb->Select(sSql.str());
  while(!pDb->Eof()) {
    sSql.str("");
    unsigned int nId = GetId();
    OBJECT_TYPE  nObjType = CONTAINER_STORAGE_FOLDER;
    
    sSql << "insert into OBJECTS (OBJECT_ID, TYPE, PATH, TITLE, FILE_NAME, DEVICE) " <<
            "values " <<
            "( " << nId << 
            ", " << nObjType << 
            ", 'virtual' " <<
            ", '" <<  SQLEscape(pDb->GetResult()->GetValue("TITLE")) << "'" <<
            ", '" <<  SQLEscape(pDb->GetResult()->GetValue("TITLE")) << "'" <<
            ", '" << p_sDevice << "')";
  
  //cout << sSql.str() << endl; fflush(stdout);
    pIns->Execute(sSql.str());
    sSql.str("");
  
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
  
  //cout << sSql.str() << endl; fflush(stdout);
    pIns->Insert(sSql.str());  
  
    stringstream sFilter;
    sFilter << "m.PARENT_ID = " << p_nObjectId;
    CreateItemMappings(pNode, pIns, p_sDevice, nId, p_bCreateRef, sFilter.str());

    pDb->Next();
  }

  delete pDb;
}

void CVirtualContainerMgr::MapSharedDirsTo(CXMLNode* pNode,
                                           CContentDatabase* pIns,
                                           std::string p_sDevice,
                                           unsigned int p_nParentId,
                                           unsigned int p_nSharedParendId)
{
  stringstream sSql;
  CContentDatabase* pSel = new CContentDatabase();
  
  sSql << "select OBJECT_ID from MAP_OBJECTS where PARENT_ID = " << p_nSharedParendId << " and DEVICE is NULL";
  pSel->Select(sSql.str());
  sSql.str("");
  
  bool bFullExtend = false;
  if((p_nSharedParendId == 0) && (pNode->Attribute("full_extend").compare("true") == 0))
    bFullExtend = true;
  else if(p_nSharedParendId > 0)
    bFullExtend = true;

  unsigned int nObjId;
  unsigned int nSharedObjId;
  
  //pIns->BeginTransaction();
  while(!pSel->Eof()) {
    
    nObjId = pSel->GetResult()->GetValueAsUInt("OBJECT_ID");
    
    // copy objects on full extend
    if(bFullExtend) {
      sSql << "select * from OBJECTS where OBJECT_ID = " << nObjId << " and DEVICE is NULL";     
      pIns->Select(sSql.str());      
      sSql.str("");
      
      if(!pIns->Eof()) {
        nObjId = GetId();
        nSharedObjId = pIns->GetResult()->GetValueAsUInt("OBJECT_ID");
        
        sSql <<
          "insert into OBJECTS " <<
          "  (OBJECT_ID, DETAIL_ID, TYPE, DEVICE, PATH, FILE_NAME, TITLE, MD5) " <<
          "values (" <<
          nObjId << ", " <<
          pIns->GetResult()->GetValue("DETAIL_ID") << ", " <<
          pIns->GetResult()->GetValue("TYPE") << ", " <<
          "'" << p_sDevice << "', " <<
          "'" << SQLEscape(pIns->GetResult()->GetValue("PATH")) << "', " <<
          "'" << SQLEscape(pIns->GetResult()->GetValue("FILE_NAME")) << "', " <<
          "'" << SQLEscape(pIns->GetResult()->GetValue("TITLE")) << "', " <<
          "'" << SQLEscape(pIns->GetResult()->GetValue("MD5")) << "' " <<          
          ");";
        
        pIns->Execute(sSql.str());
        sSql.str("");
      }
    }    
       
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nObjId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    pIns->Execute(sSql.str());
    
    // recursively add child objects on full extend
    if(bFullExtend) {
      MapSharedDirsTo(pNode, pIns, p_sDevice, nObjId, nSharedObjId);
    }
    
    pSel->Next();
    sSql.str(""); 
  }
  //pIns->Commit();  
  
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
