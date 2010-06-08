/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
// vim: set sw=2 et :
/***************************************************************************
 *            VirtualContainerMgr.cpp
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

#include "VirtualContainerMgr.h"

#include "../Common/Common.h"
#include "../Common/File.h"
#include "ContentDatabase.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include <iostream>
#include <time.h>


#include "DatabaseObject.h"

using namespace std;
using namespace fuppes;

static std::string VFOLDER_CFG_VERSION = "0.2";
		
CVirtualContainerMgr* CVirtualContainerMgr::m_pInstance = 0;

CVirtualContainerMgr* CVirtualContainerMgr::Shared()
{
  if(m_pInstance == 0)
	  m_pInstance = new CVirtualContainerMgr();
	return m_pInstance;
}

CVirtualContainerMgr::CVirtualContainerMgr()
{
	m_nIdCounter    = 0;
}

CVirtualContainerMgr::~CVirtualContainerMgr()
{
}

bool CVirtualContainerMgr::IsRebuilding()
{
  return false;
}    


bool CVirtualContainerMgr::HandleFile(std::string device, std::string file, SQLQuery* qry)
{
  assert(!file.empty());


  CSharedLog::Print("[VirtualContainer] load '%s'", file.c_str());
  
  CXMLDocument doc;
  if(doc.LoadFromFile(file)) {
    CXMLNode* root = doc.RootNode();
    if(root->Attribute("version").compare(VFOLDER_CFG_VERSION) == 0 &&
       root->Name().compare("vfolder_layout") == 0) {
      //CreateChildItems(root, qry, device, 0, NULL);
      createLayout(root, 0, qry, device);
    } else {
      CSharedLog::Print("[VirtualContainer] '%s' has an invalid version number %s when it should be %s. Please get a more recent config file, or (if you know what you are doing) you can update it yourself.", file.c_str(), root->Attribute("version").c_str(), VFOLDER_CFG_VERSION.c_str());
      return false;
    }
  } else {
    CSharedLog::Print("[VirtualContainer] failed to load '%s' virtual configuration file: Invalid XML.", file.c_str());
    return false;
  }


  return true;
}




void CVirtualContainerMgr::RebuildContainerList(bool force /*= false*/, bool insertFiles /*= true*/)
{
  if(!force && CContentDatabase::Shared()->IsRebuilding()) {
    CSharedLog::Log(L_NORM, __FILE__, __LINE__, "database rebuild in progress");
    return;
  }

  fuppes::DateTime start = DateTime::now();
  CSharedLog::Print("[VirtualContainer] create virtual container layout started at %s", start.toString().c_str());

  // drop all virtual folders and files
	SQLQuery qry;
  qry.exec("delete from OBJECTS where DEVICE is NOT NULL;");
  qry.connection()->vacuum();


  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {
    
    string file = CSharedConfig::Shared()->pathFinder->findVFolderInPath(vfolders.at(i), File::readable); // only need read access
    if(!file.empty()) {
      CSharedLog::Print("[VirtualContainer] read vfolder layout from '%s'.", (vfolders.at(i) + VFOLDER_EXT).c_str());
      HandleFile(vfolders.at(i), file, &qry);
    } else {
      CSharedLog::Print("[VirtualContainer] '%s' could not be found in the path.", (vfolders.at(i) + VFOLDER_EXT).c_str());
    }
  }


  if(!insertFiles) {
    fuppes::DateTime end = DateTime::now();
    CSharedLog::Print("[VirtualContainer] virtual container layout created at %s", end.toString().c_str());
    return;
  }
  

  // insert files
  stringstream sql;
  sql << "select * from OBJECTS where DEVICE is NULL and REF_ID = 0 and TYPE > " << ITEM;
  DbObject* obj;
  qry.select(sql.str());
  while(!qry.eof()) {

    obj = new DbObject(qry.result());

    switch(obj->type()) {

      case ITEM_IMAGE_ITEM:
      case ITEM_IMAGE_ITEM_PHOTO:
        VirtualContainerMgr::insertGenericFile(obj);
        break;

      case ITEM_AUDIO_ITEM:
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:
        VirtualContainerMgr::insertAudioFile(obj);
        break;
      case ITEM_AUDIO_ITEM_AUDIO_BROADCAST:
        break;

      case ITEM_VIDEO_ITEM:
      case ITEM_VIDEO_ITEM_MOVIE:
        VirtualContainerMgr::insertGenericFile(obj);
        break;
      case ITEM_VIDEO_ITEM_VIDEO_BROADCAST:
        break;

      default:
        break;          
    }
    

    delete obj;        
    qry.next();
  }


  fuppes::DateTime end = DateTime::now();
  CSharedLog::Print("[VirtualContainer] virtual container layout created at %s", end.toString().c_str());

}


std::string vcontainerNodeType(CXMLNode* node, bool &final)
{
  final = false;
  if(node->Name().compare("vfolder") == 0) {
    return "folder";
  }
  else if(node->Name().compare("vfolders") == 0) {

    if(node->Attribute("property").length() > 0) {
      string prop = node->Attribute("property");
      
      if(prop.compare("genre") == 0)
        return "genre";
      else if(prop.compare("artist") == 0)
        return "artist";
      else if(prop.compare("album") == 0)
        return "album";
      else if(prop.compare("composer") == 0)
        return "composer";

    }
    else if(node->Attribute("split").length() > 0) {
      return "split";
    }
  }
  else if(node->Name().compare("items") == 0) {

    if(node->Attribute("type").length() > 0) {
      string type = node->Attribute("type");

       final = true;
      
      if(type.compare("audioItem") == 0)
        return "audioItem";
      else if(type.compare("imageItem") == 0)
        return "imageItem";
      else if(type.compare("videoItem") == 0)
        return "videoItem";
    }
    
  }

  else if(node->Name().compare("folders") == 0) {
    final = true;
    return node->Attribute("filter");
  }  

  return "";
}

void vcontainerPathParents(std::string& vcontainerPath, CXMLNode* node)
{
  if(node == NULL)
    return;

  bool final;
  string type = vcontainerNodeType(node, final);
  if(type.length() == 0) // || type == "folder")
    return;
  
  vcontainerPath = type + "|" + vcontainerPath;
  
  vcontainerPathParents(vcontainerPath, node->parent());
}

void vcontainerPathChildren(std::string& vcontainerPath, CXMLNode* node, bool sibling)
{
  /*
  DbObject::VirtualContainerType type = vcontainerNodeType(node);
  if(type != DbObject::None)
    *vcontainerPath |= type;
  */

  bool final;
  string type = vcontainerNodeType(node, final);
  if(type.length() == 0)
    return;

  if(!sibling)
    vcontainerPath = vcontainerPath + "|" + type;
  /*else
    vcontainerPath = vcontainerPath + "&" + type;*/

  bool first = true;
  for(int i = 0; i < node->ChildCount(); i++) {

    if(node->ChildNode(i)->type() != CXMLNode::ElementNode)
      continue;
    
    vcontainerPathChildren(vcontainerPath, node->ChildNode(i), !first);
    first = false;
  }
}


void createVcontainerPath(std::string& vcontainerPath, CXMLNode* node)
{  
  bool final;
  string type = vcontainerNodeType(node, final);
  vcontainerPath = type;
  
  cout << "createVcontainerPath for: " << node->Name() << "* type: " <<  type << "*" << endl;
  
  vcontainerPathParents(vcontainerPath, node->parent()); 


  // get the child items
  if(!type.empty() && type != "folder" && !final) {  
  
    for(int i = 0; i < node->ChildCount(); i++) {

      if(node->ChildNode(i)->type() != CXMLNode::ElementNode)
        continue;


      type = vcontainerNodeType(node->ChildNode(i), final);
      if(type.length() > 0)    
        vcontainerPathChildren(vcontainerPath, node->ChildNode(i), false);
    }
    
  }
  // get the siblings for all final elements
  else {

    for(int i = 0; i < node->parent()->ChildCount(); i++) {

      if(node->parent()->ChildNode(i)->type() != CXMLNode::ElementNode || 
         node->parent()->ChildNode(i) == node)
        continue;


      type = vcontainerNodeType(node->ChildNode(i), final);
      if(!final || type.length() == 0) {
        continue;
      }

      vcontainerPath += (" & " + type);
    }   
    
  }
  
}



std::string createVFolderPath(CXMLNode* node)
{
  string result;
  string type;
  bool final;
  
  if(node->name().compare("vfolder") == 0) {
    result = "folder | ";
  }
    
  for(int i = 0; i < node->ChildCount(); i++) {

    if(node->ChildNode(i)->type() != CXMLNode::ElementNode)
      continue;

    if(node->ChildNode(i)->name().compare("vfolder") == 0)
      continue;

    type = vcontainerNodeType(node->ChildNode(i), final);
    if(final) {
      result += (type + " & ");
    }
    else {
      result += type + " | ";
      result += createVFolderPath(node->ChildNode(i));
    }

  }


  if(result.length() > 2 && 
     (result[result.length() - 2] == '|' ||
      result[result.length() - 2] == '&')) {
    result = result.substr(0, result.length() - 3);
  }

  return result;
  
}


void CVirtualContainerMgr::createLayout(CXMLNode* node, object_id_t pid, SQLQuery* qry, std::string layout)
{
  if(node->type() != CXMLNode::ElementNode)
    return;


  if(node->name().compare("vfolder_layout") == 0) { // root node
    for(int i = 0; i < node->ChildCount(); i++) {
      if(node->ChildNode(i)->type() != CXMLNode::ElementNode)
        continue;
      createLayout(node->ChildNode(i), pid ,qry, layout);
    }
    return;
  }


    
  DbObject folder;
  if(node->name().compare("vfolder") == 0) {

    std::string path = createVFolderPath(node);

    folder.setObjectId(GetId());
    folder.setParentId(pid);
    folder.setType(CONTAINER_STORAGE_FOLDER);
    folder.setTitle(node->attribute("name"));
    folder.setDevice(layout);
    folder.setVirtualContainerType(DbObject::Folder);
    folder.setVirtualContainerPath(path);
    folder.save();
    
    for(int i = 0; i < node->ChildCount(); i++) {
      if(node->ChildNode(i)->type() != CXMLNode::ElementNode)
        continue;
      createLayout(node->ChildNode(i), folder.objectId() ,qry, layout);
    }
    
  }

}

void CVirtualContainerMgr::CreateChildItems(CXMLNode* pParentNode, 
                                            SQLQuery* pIns,
                                            std::string p_sDevice, 
                                            unsigned int p_nParentId,
                                            CObjectDetails* pDetails,
                                            std::string p_sFilter)
{
  CXMLNode* pNode;
  int i;
  bool bDetails = false;  // this variable is not required                                              


  string vcontainerPath;
  
cout << "CreateChildItems() " << pParentNode->Name() << " pid: " << p_nParentId << endl;
  
  for(i = 0; i < pParentNode->ChildCount(); i++) {
    pNode = pParentNode->ChildNode(i);
    if(pNode->type() != CXMLNode::ElementNode)
      continue;
    

    vcontainerPath = "";
    createVcontainerPath(vcontainerPath, pNode);

    cout << "VCONTPATH: " << vcontainerPath << endl;
    
    if(pDetails == NULL) {
      pDetails = new CObjectDetails();
      bDetails = true;
    }      
      
    if(pNode->Name().compare("vfolder") == 0) {
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, "create single vfolder: %s :: %s", pNode->Attribute("name").c_str(), p_sFilter.c_str());
      CreateSingleVFolder(pNode, pIns, p_sDevice, p_nParentId, pDetails, vcontainerPath);
    }
    else if(pNode->Name().compare("vfolders") == 0) {      
      if(pNode->Attribute("property").length() > 0) {
        CSharedLog::Log(L_EXT, __FILE__, __LINE__, "create vfolders from property: %s :: %s", pNode->Attribute("property").c_str(), p_sFilter.c_str());
        //CreateVFoldersFromProperty(pNode, pIns, p_sDevice, p_nParentId, pDetails, p_bContainerDetails, p_bCreateRef, vcontainerPath, p_sFilter);
      }
      else if(pNode->Attribute("split").length() > 0) {
        CSharedLog::Log(L_EXT, __FILE__, __LINE__, "create split vfolders :: %s", p_sFilter.c_str());
        CreateVFoldersSplit(pNode, pIns, p_sDevice, p_nParentId, pDetails, true, true, p_sFilter);
      }
    }
    /*else if(pNode->Name().compare("items") == 0) {
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, "create item mappings for type: %s :: %s", pNode->Attribute("type").c_str(), p_sFilter.c_str());
      CreateItemMappings(pNode, pIns, p_sDevice, p_nParentId, p_bCreateRef, vcontainerPath, p_sFilter);
    }*/
    /*else if(pNode->Name().compare("folders") == 0) {
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, "create folder mappings - filter: %s :: %s", pNode->Attribute("filter").c_str(), p_sFilter.c_str());
      CreateFolderMappings(pNode, pIns, p_sDevice, p_nParentId, p_bCreateRef, p_sFilter);
    }*/
    else if(pNode->Name().compare("shared_dirs") == 0) {
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, "create shared dir mappings :: %s", p_sFilter.c_str());
      MapSharedDirsTo(pNode, pIns, p_sDevice, p_nParentId);
    }
    
    if(bDetails) {
      delete pDetails;
      pDetails = NULL;
    }

  }
}

void CVirtualContainerMgr::CreateSingleVFolder(CXMLNode* pFolderNode,
                                               SQLQuery* pIns,
                                               std::string p_sDevice,
                                               unsigned int p_nParentId,
                                               CObjectDetails* pDetails,
                                               std::string vcontainerPath)
{


cout << "CreateSingleVFolder() " << pFolderNode->Name() << endl;
  
  //CContentDatabase* pDb = new CContentDatabase();
  stringstream sSql;
  unsigned int nId;
  OBJECT_TYPE  nObjType = CONTAINER_STORAGE_FOLDER;
    
  if(pFolderNode->AttributeAsUInt("id") > 0) {
    nId = pFolderNode->AttributeAsUInt("id");
  }
  else {
    nId = GetId();
  }
  

  // create folder
  DbObject obj;
  obj.setObjectId(nId);
  obj.setParentId(p_nParentId);
  obj.setType(nObjType);
  obj.setPath("virtual");
  //obj.setFileName(pFolderNode->Attribute("name"));
  obj.setTitle(pFolderNode->Attribute("name"));
  obj.setDevice(p_sDevice);
  obj.setVirtualContainerType(DbObject::Folder);
  obj.setVirtualContainerPath(vcontainerPath);
  obj.save(pIns);
  
  
  if(pFolderNode->ChildCount() > 0) {
    CreateChildItems(pFolderNode, pIns, p_sDevice, nId, pDetails);
  }
}


void CVirtualContainerMgr::CreateVFoldersSplit(CXMLNode* pFoldersNode, 
                                               SQLQuery* pIns,
                                               std::string p_sDevice, 
                                               unsigned int p_nParentId,
                                               CObjectDetails* pDetails,
                                               bool p_bContainerDetails,
																							 bool p_bCreateRef,
                                               std::string /*p_sFilter*/)
{
  string sFolders[] = {
    "0-9", "ABC", "DEF", "GHI", "JKL",
    "MNO" , "PQR", "STU", "VWX", "YZ",
    ""
  };
                                                   
  string sFilter[] = {
    " substr(%s, 1, 1) in (0, 1, 2, 3, 4, 5, 6, 7, 8, 9) ", 
    " upper(substr(%s, 1, 1)) in ('A', 'B', 'C') ",
    " upper(substr(%s, 1, 1)) in ('D', 'E', 'F') ",
    " upper(substr(%s, 1, 1)) in ('G', 'H', 'I') ",
    " upper(substr(%s, 1, 1)) in ('J', 'K', 'L') ",
    " upper(substr(%s, 1, 1)) in ('M', 'N', 'O') ",
    " upper(substr(%s, 1, 1)) in ('P', 'Q', 'R') ",
    " upper(substr(%s, 1, 1)) in ('S', 'T', 'U') ",
    " upper(substr(%s, 1, 1)) in ('V', 'W', 'X') ",
    " upper(substr(%s, 1, 1)) in ('Y', 'Z') ",
    ""
  };
                                                   
  int i = 0;
  unsigned int nId;
  stringstream sSql;
                                                   
  while(sFolders[i].length() > 0) {
    
    nId = GetId();
      
    sSql.str("");
    sSql << "insert into OBJECTS (OBJECT_ID, PARENT_ID, TYPE, PATH, FILE_NAME, TITLE, DEVICE) values " <<
      "(" << nId << ", " << p_nParentId << ", " << CONTAINER_STORAGE_FOLDER << ", " << "'virtual', '" << 
      sFolders[i] << "', '" << sFolders[i] << "', '" << p_sDevice << "')";
    pIns->exec(sSql.str());
    
    /*sSql.str(""); 
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    
    pIns->exec(sSql.str());    */
    
    
    if(pFoldersNode->ChildCount() > 0) {
      CreateChildItems(pFoldersNode, pIns, p_sDevice, nId, pDetails, sFilter[i]);
    }
      
    i++;
  }
                                                 
}



/*
void CVirtualContainerMgr::CreateFolderMappings(CXMLNode* pNode, 
                                                SQLQuery* pIns,
                                                std::string p_sDevice, 
                                                unsigned int p_nParentId, 
																								bool p_bCreateRef,
                                                std::string filter)
{

  DbObject folder;
  folder->setObjectId(GetId());
  folder->setTitle();

  
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

  SQLQuery qry;
         
  sSql << 
    "select distinct PARENT_ID from OBJECTS o where " <<
    "TYPE " << sObjType.str() << " and " <<
    "REF_ID = 0 and " <<
    "DEVICE is NULL";
                
  //cout << sSql.str() << endl;
                                                  
  qry.select(sSql.str());
  while(!qry.eof()) {
    CreateSingleVFolderFolder(pNode, pIns, p_sDevice, atoi(qry.result()->asString("PARENT_ID").c_str()), p_nParentId, p_bCreateRef);
    qry.next();
  }

}

void CVirtualContainerMgr::CreateSingleVFolderFolder(CXMLNode* pNode,
                                                     SQLQuery* pIns,
                                                     std::string p_sDevice,
                                                     unsigned int p_nObjectId,
                                                     unsigned int p_nParentId,
																										 bool p_bCreateRef)
{
  stringstream sSql;
                                                  
  SQLQuery qry;
         
  sSql << 
    "select " <<
    "  TITLE " <<
    "from " <<
    "  OBJECTS " <<
    "where " <<
		"  DEVICE is NULL and " <<
    "  OBJECT_ID = " << p_nObjectId;
                
  //cout << sSql.str() << endl; fflush(stdout);
                                                  
  qry.select(sSql.str());
  while(!qry.eof()) {
    sSql.str("");
    unsigned int nId = GetId();
    OBJECT_TYPE  nObjType = CONTAINER_STORAGE_FOLDER;
    
    sSql << "insert into OBJECTS (OBJECT_ID, TYPE, PATH, TITLE, FILE_NAME, DEVICE) " <<
            "values " <<
            "( " << nId << 
            ", " << nObjType << 
            ", 'virtual' " <<
            ", '" <<  SQLEscape(qry.result()->asString("TITLE")) << "'" <<
            ", '" <<  SQLEscape(qry.result()->asString("TITLE")) << "'" <<
            ", '" << p_sDevice << "')";
  
  //cout << sSql.str() << endl; fflush(stdout);
    pIns->exec(sSql.str());
    sSql.str("");
  
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
  
  //cout << sSql.str() << endl; fflush(stdout);
    pIns->insert(sSql.str());  
  
    stringstream sFilter;
    sFilter << "m.PARENT_ID = " << p_nObjectId;
    //CreateItemMappings(pNode, pIns, p_sDevice, nId, p_bCreateRef, 0, sFilter.str());

    qry.next();
  }

}
*/

void CVirtualContainerMgr::MapSharedDirsTo(CXMLNode* pNode,
                                           SQLQuery* pIns,
                                           std::string p_sDevice,
                                           unsigned int p_nParentId,
                                           unsigned int p_nSharedParendId)
{
  stringstream sSql;  
  SQLQuery pSel;
  
  sSql << "select OBJECT_ID from MAP_OBJECTS where PARENT_ID = " << p_nSharedParendId << " and DEVICE is NULL";
  pSel.select(sSql.str());
  sSql.str("");
  
  bool bFullExtend = false;
  if((p_nSharedParendId == 0) && (pNode->Attribute("full_extend").compare("true") == 0))
    bFullExtend = true;
  else if(p_nSharedParendId > 0)
    bFullExtend = true;

  unsigned int nObjId = 0;
  unsigned int nSharedObjId = 0;
  
  //pIns->BeginTransaction();
  while(!pSel.eof()) {
    
    nObjId = pSel.result()->asUInt("OBJECT_ID");
    
    // copy objects on full extend
    if(bFullExtend) {
      sSql << "select * from OBJECTS where OBJECT_ID = " << nObjId << " and DEVICE is NULL";     
      pIns->select(sSql.str());      
      sSql.str("");
      
      if(!pIns->eof()) {
        nObjId = GetId();
        nSharedObjId = pIns->result()->asUInt("OBJECT_ID");
        
        sSql <<
          "insert into OBJECTS " <<
          "  (OBJECT_ID, DETAIL_ID, TYPE, DEVICE, PATH, FILE_NAME, TITLE, MD5) " <<
          "values (" <<
          nObjId << ", " <<
          pIns->result()->asString("DETAIL_ID") << ", " <<
          pIns->result()->asString("TYPE") << ", " <<
          "'" << p_sDevice << "', " <<
          "'" << SQLEscape(pIns->result()->asString("PATH")) << "', " <<
          "'" << SQLEscape(pIns->result()->asString("FILE_NAME")) << "', " <<
          "'" << SQLEscape(pIns->result()->asString("TITLE")) << "', " <<
          "'" << SQLEscape(pIns->result()->asString("MD5")) << "' " <<          
          ");";
        
        pIns->exec(sSql.str());
        sSql.str("");
      }
    }    
       
    sSql << "insert into MAP_OBJECTS (OBJECT_ID, PARENT_ID, DEVICE) values " <<
      "( "  << nObjId << 
      ", "  << p_nParentId << 
      ", '" << p_sDevice << "');";
    pIns->exec(sSql.str());
    
    // recursively add child objects on full extend
    if(bFullExtend) {
      MapSharedDirsTo(pNode, pIns, p_sDevice, nObjId, nSharedObjId);
    }
    
    pSel.next();
    sSql.str(""); 
  }
  //pIns->Commit();  

}





void VirtualContainerMgr::insertAudioFile(DbObject* object) // static
{
  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {  
    insertAudioFileForLayout(object, vfolders.at(i));
  }
}

void VirtualContainerMgr::insertAudioFileForLayout(DbObject* object, std::string layout) // static
{
  SQLQuery qry;
  stringstream sql;
  
  // get all paths containing audio items
  StringList paths;
  qry.select("select distinct(VCONTAINER_PATH) from OBJECTS where VCONTAINER_PATH like '%audioItem%' and DEVICE = '" + layout + "'");
  while(!qry.eof()) {
    paths.push_back(qry.result()->asString("VCONTAINER_PATH"));
    qry.next();
  }

  
  StringList parts;
  object_id_t pid;
  for(unsigned int i = 0; i < paths.size(); i++) {
    parts = String::split(paths.at(i), "|");

    ASSERT(parts.at(0) == "folder");

    // get folder id
    sql << "select OBJECT_ID from OBJECTS where " <<
      "VCONTAINER_TYPE = " << DbObject::Folder << " and " <<
      "VCONTAINER_PATH = '" << paths.at(i) << "' and " <<
      "DEVICE = '" + layout + "'";
    qry.select(sql.str());
    sql.str("");
    ASSERT(qry.eof() == false);
    
    pid = qry.result()->asUInt("OBJECT_ID");
    
    // loop through the parts of the path
    // except for the first item (folder) and the last (audioItem)
    for(unsigned int j = 1; j < parts.size() - 1; j++) {

      DbObject::VirtualContainerType type = DbObject::None;
      if(parts.at(j) == "genre")
        type = DbObject::Genre;
      else if(parts.at(j) == "artist")
        type = DbObject::Artist;
      else if(parts.at(j) == "composer")
        type = DbObject::Composer;
      else if(parts.at(j) == "album")
        type = DbObject::Album;
      
      pid = createFolderIfNotExists(object, pid, type, paths.at(i), layout);
    }

    // insert the file
    DbObject file(object);
    file.setObjectId(CVirtualContainerMgr::Shared()->GetId());
    file.setParentId(pid);
    file.setDevice(layout);
    file.setVirtualContainerPath(paths.at(i));
    file.setVirtualRefId(object->objectId());
    file.save();

    cout << "insertAudioFileForDevice" << endl << DbObject::toString(&file) << endl;
  }

}

void VirtualContainerMgr::insertGenericFile(fuppes::DbObject* object) // static
{
  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {  
    insertGenericFileForLayout(object, vfolders.at(i));
  }
}

void VirtualContainerMgr::insertGenericFileForLayout(fuppes::DbObject* object, std::string layout) // static
{
  string path;
  switch(object->type()) {
    case ITEM_IMAGE_ITEM:
    case ITEM_IMAGE_ITEM_PHOTO:
      path = "imageItem";
      break;
    case ITEM_VIDEO_ITEM:
    case ITEM_VIDEO_ITEM_MOVIE:
      path = "videoItem";
      break;
    default:
      return;
      break;
  }
  

  SQLQuery qry;
  stringstream sql;
  
  // get all paths containing the object type
  StringList paths;
  qry.select("select distinct(VCONTAINER_PATH) from OBJECTS where VCONTAINER_PATH like '%" + path + "%' and DEVICE = '" + layout + "'");
  while(!qry.eof()) {
    paths.push_back(qry.result()->asString("VCONTAINER_PATH"));
    qry.next();
  }

  
  
}


object_id_t VirtualContainerMgr::createFolderIfNotExists(DbObject* object, 
                                                         object_id_t pid, 
                                                         DbObject::VirtualContainerType type, 
                                                         std::string path, 
                                                         std::string layout) // static
{
  string title;
  OBJECT_TYPE objType = CONTAINER_STORAGE_FOLDER;
  switch(type) {
    //case Split:
    //  break;
    case DbObject::Genre:
      title = object->details()->genre();
      objType = CONTAINER_GENRE_MUSIC_GENRE;
      break;
    case DbObject::Artist:
      title = object->details()->artist();
      objType = CONTAINER_PERSON_MUSIC_ARTIST;
      break;
    case DbObject::Composer:
      title = object->details()->composer();
      break;
    case DbObject::Album:
      title = object->details()->album();
      objType = CONTAINER_ALBUM_MUSIC_ALBUM;
      break;
    default:
      ASSERT(true == false);
      break;
  }
  
  SQLQuery qry;
  stringstream sql;
  sql << "select OBJECT_ID from OBJECTS where "
    "PARENT_ID = " << pid << " and " <<
    "VCONTAINER_TYPE = " << type << " and " <<
    "VCONTAINER_PATH = '" << path << "' and " <<
    "TITLE = '" << title << "' and " <<
    "DEVICE = '" << layout << "'";
  qry.select(sql.str());

  ASSERT(qry.size() == 0 || qry.size() == 1);

  if(qry.size() == 1) {
    cout << "VFOLDER EXISTS: " << sql.str() << endl;
    return qry.result()->asUInt("OBJECT_ID");
  }
  

  DbObject folder;
  folder.setObjectId(CVirtualContainerMgr::Shared()->GetId());
  folder.setParentId(pid);
  folder.setType(objType);
  folder.setTitle(title);
  folder.setVirtualContainerType(type);
  folder.setVirtualContainerPath(path);
  folder.setDevice(layout);

  /* todo set metadata for artist, composer or album folders
  switch(type) {
    case DbObject::Artist:
      break;
    case DbObject::Composer:
      break;
    case DbObject::Album:
      break;
    default:
      break;
  }*/

  folder.save();

  cout << "createFolderIfNotExists" << endl << DbObject::toString(&folder) << endl;
  
  return folder.objectId();
}


void VirtualContainerMgr::updateAudioFile(fuppes::DbObject* object, fuppes::ObjectDetails* oldDetails) // static
{
  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {  
    updateAudioFileForLayout(object, oldDetails, vfolders.at(i));
  }
}

void VirtualContainerMgr::updateAudioFileForLayout(fuppes::DbObject* object, fuppes::ObjectDetails* oldDetails, std::string layout) // static
{
  stringstream sql;
  SQLQuery qry;

  sql << "select * from OBJECTS where " <<
    "VREF_ID = " << object->objectId() << " and " <<
    "DEVICE = '" << layout << "'";
}


void VirtualContainerMgr::deleteFile(fuppes::DbObject* object) // static
{
  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {  
    deleteFileForLayout(object, vfolders.at(i));
  }
}

void VirtualContainerMgr::deleteDirectory(fuppes::DbObject* directory) // static
{
  stringstream sql;
  SQLQuery qry;
  DbObject* object;
               
  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {  

    // get all the items from within the directory path ...
    sql.str("");
    sql << "select * from OBJECTS where " <<
      "PATH like '" << directory->path() << "%' and " <<
      "TYPE > " << ITEM << " and " <<
      "DEVICE is NULL";
    qry.select(sql.str());

    // ... and remove them from each virtual layout
    while(!qry.eof()) {
      object = new DbObject(qry.result());
      deleteFileForLayout(object, vfolders.at(i));
      delete object;
      qry.next();
    }
  }  
}

void VirtualContainerMgr::deleteFileForLayout(fuppes::DbObject* object, std::string layout) // static
{
  stringstream sql;
  SQLQuery qry;

  sql << "select * from OBJECTS where "
    "VREF_ID = " << object->objectId() << " and " <<
    "DEVICE = '" << layout << "'";


cout << sql.str() << endl;
  
  DbObject* obj;
  DbObject* parent;
  qry.select(sql.str());
  object_id_t pid;
  while(!qry.eof()) {
    obj = new DbObject(qry.result());
    pid = obj->parentId();
    obj->remove();
    delete obj;
    
    // delete empty parent folders
    DbObject::VirtualContainerType type;
    do {
      parent = DbObject::createFromObjectId(pid, NULL, layout);
      type = parent->vcType();
      if(type >= DbObject::Genre) {
        deleteFolderIfEmpty(parent);
      }
      pid = parent->parentId();
      delete parent;
    } while(type >= DbObject::Genre);

    qry.next();
  }  
  
}

void VirtualContainerMgr::deleteFolderIfEmpty(DbObject* vfolder) // static
{
  stringstream sql;
  SQLQuery qry;

  sql << "select count(*) as COUNT from OBJECTS where "
    "PARENT_ID = " << vfolder->objectId() << " and " <<
    "DEVICE = '" << vfolder->device() << "'";

  qry.select(sql.str());
  if(qry.result()->asInt("COUNT") > 0) {
    return;
  }

  sql.str("");
  sql << "delete from OBJECT_DETAILS where ID = " << vfolder->detailId();
  qry.exec(sql.str());

  sql.str("");
  sql << "delete from OBJECTS where "
    "OBJECT_ID = " << vfolder->objectId() << " and " <<
    "DEVICE = '" << vfolder->device() << "'";
  qry.exec(sql.str());
}
