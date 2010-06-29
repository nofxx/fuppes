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
  SQLQuery qry;
  qry.select("select min(OBJECT_ID) as VALUE from OBJECTS where DEVICE is not NULL");
  if(qry.eof())
  	m_nIdCounter    = 0;
  else
  	m_nIdCounter    = qry.result()->asUInt("VALUE");
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
    
    string file = CSharedConfig::Shared()->pathFinder->findVFolderInPath(vfolders.at(i));
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
    VirtualContainerMgr::insertFile(obj);
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
  else if(node->Name().compare("split") == 0) {
    return "split";
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

std::string createVFolderPath(CXMLNode* node)
{
  string result;
  string type;
  bool final;

  cout << "create path for : " << node->name() << "*" << endl;


  
  if(node->name().compare("vfolder") == 0) {
    result = "folder | ";

    cout << "  path for vfolder node : " << node->attribute("name") << "*" << endl;

    // get parent folders
    CXMLNode* parent = node->parent();
    cout << "   vfolder node parent: " << parent->name() << "*" << endl;
    while(parent->name().compare("vfolder") == 0) {
      result += "folder | ";
      parent = parent->parent();
    }

    cout << "  result: " << result << endl;
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
  // create a single vfolder
  if(node->name().compare("vfolder") == 0) {

    std::string path = createVFolderPath(node);

    if(node->AttributeAsUInt("id") > 0)
      folder.setObjectId(node->AttributeAsUInt("id"));
    else
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

  // create the split layout
  else if(node->name().compare("split") == 0) {
    
    std::string path = createVFolderPath(node->parent());
    string folders[] = {
    "0-9", "ABC", "DEF", "GHI", "JKL",
    "MNO" , "PQR", "STU", "VWX", "YZ",
    "!?#",
    ""};      

    for(int i = 0; folders[i].length() > 0; i++) {
      folder.reset();
      folder.setObjectId(GetId());
      folder.setParentId(pid);
      folder.setType(CONTAINER_STORAGE_FOLDER);
      folder.setTitle(folders[i]);
      folder.setDevice(layout);
      folder.setVirtualContainerType(DbObject::Split);
      folder.setVirtualContainerPath(path);
      folder.save();      
    }

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
*/

/*
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

*/




void VirtualContainerMgr::insertFile(fuppes::DbObject* object) // static
{
  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {  
    insertFileForLayout(object, vfolders.at(i));
  }
}


/*
 pid          parent id of the ABC, DEF, ... folders
 object       the object to insert (REF_ID = 0 and DEVICE = NULL)
 childType    property type of the split children
 layout       the virtual layout
*/
object_id_t getSplitParent(object_id_t pid, DbObject* object, std::string childType, std::string layout)
{
  string title;
  if(childType.compare("genre") == 0) {    
    title = object->details()->genre();    
  }  
  else if(childType.compare("artist") == 0) {
    title = object->details()->artist();
  }
  else if(childType.compare("album") == 0) {
    title = object->details()->artist();
  }
  else if(childType.compare("composer") == 0) {
    title = object->details()->composer();
  }
  else {
    cout << "TODO: getSplitParent property: " << childType << endl;
  }
  
  if(title.length() == 0)
    title = "unknown";  
  title = ToUpper(title.substr(0,1));

  // check if the first character is a number
  if(isdigit(title.c_str()[0]) != 0)
    title = "0-9";
  
#warning todo: handle umlauts and other special characters
  
  SQLQuery qry;
  stringstream sql;
  sql << "select OBJECT_ID from OBJECTS where " <<
    "PARENT_ID = " << pid << " and " <<
    "TITLE like '%" << title << "%' and " <<
    "DEVICE = '" << layout << "'";

  cout << sql.str() << endl;
  qry.select(sql.str());


  // this is a quick fix to handle umlauts and other special characters
  // it's not a clean solution because umlauts will go into the '!?#' folder
  // instead of e.g A-C for the a-umlaut.
  if(qry.size() == 0) {

    sql.str("");
    sql << "select OBJECT_ID from OBJECTS where " <<
      "PARENT_ID = " << pid << " and " <<
      "TITLE like '%" << "#" << "%' and " <<
      "DEVICE = '" << layout << "'";

    qry.select(sql.str());
  }
  

  ASSERT(qry.size() == 1);

  return qry.result()->asUInt("OBJECT_ID");
}


void VirtualContainerMgr::insertFileForLayout(fuppes::DbObject* object, std::string layout) // static
{
  string path;
  switch(object->type()) {
    case ITEM_AUDIO_ITEM:
    case ITEM_AUDIO_ITEM_MUSIC_TRACK:
      path = "audioItem";
      break;
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
  
  // get all paths containing the item type
  StringList paths;
  qry.select("select distinct(VCONTAINER_PATH) from OBJECTS where VCONTAINER_PATH like '%" + path + "%' and DEVICE = '" + layout + "'");
  while(!qry.eof()) {
    paths.push_back(qry.result()->asString("VCONTAINER_PATH"));
    qry.next();
  }
  
  StringList parts;
  object_id_t pid;
  object_id_t refId = 0;
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
    ASSERT(qry.size() == 1);
    
    pid = qry.result()->asUInt("OBJECT_ID");
    
    // loop through the parts of the path
    // except for the first item (folder) and the last (audioItem)
    for(unsigned int j = 1; j < parts.size() - 1; j++) {

      DbObject::VirtualContainerType type = DbObject::None;
      if(parts.at(j) == "split") {
        type = DbObject::Split;
        pid = getSplitParent(pid, object, parts.at(j + 1), layout);
        ASSERT(pid != 0);
        continue;
      }
      else if(parts.at(j) == "genre")
        type = DbObject::Genre;
      else if(parts.at(j) == "artist")
        type = DbObject::Artist;
      else if(parts.at(j) == "composer")
        type = DbObject::Composer;
      else if(parts.at(j) == "album")
        type = DbObject::Album;
      else {
        cout << "TODO handle path : " << parts.at(j) << "*" << endl;
        continue;
      }

      pid = createFolderIfNotExists(object, pid, type, paths.at(i), layout);
    }

    // insert the file
    DbObject file(object);
    file.setObjectId(CVirtualContainerMgr::Shared()->GetId());
    file.setParentId(pid);
    file.setDevice(layout);
    file.setVirtualContainerPath(paths.at(i));
    file.setVirtualRefId(object->objectId());
    file.setRefId(refId);
    file.save();

    // we take the object id of the first inserted item
    // and use it as ref_id for the next ones (if any)
    if(refId == 0) {
      refId = file.objectId();
    }
    
    //cout << "insertFileForDevice" << endl << DbObject::toString(&file) << endl;
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
    case DbObject::Genre:
      title = object->details()->genre();
      objType = CONTAINER_GENRE_MUSIC_GENRE;  // video genre
      break;
    case DbObject::Artist:
      title = object->details()->artist();
      objType = CONTAINER_PERSON_MUSIC_ARTIST; // video artist
      break;
    case DbObject::Composer:
      title = object->details()->composer();
      break;
    case DbObject::Album:
      title = object->details()->album();
      objType = CONTAINER_ALBUM_MUSIC_ALBUM;  // image album
      break;
    default:
      ASSERT(true == false);
      break;
  }

  title = TrimWhiteSpace(title);
  if(title.length() == 0)
    title = "unknown";
  
  SQLQuery qry;
  stringstream sql;
  sql << "select OBJECT_ID from OBJECTS where "
    "PARENT_ID = " << pid << " and " <<
    "VCONTAINER_TYPE = " << type << " and " <<
    "VCONTAINER_PATH = '" << path << "' and " <<
    "TITLE = '" << SQLEscape(title) << "' and " <<
    "DEVICE = '" << layout << "'";
  qry.select(sql.str());

  ASSERT(qry.size() == 0 || qry.size() == 1);

  if(qry.size() == 1) {
    //cout << "FOLDER *" << title << "* exists : id: " << qry.result()->asUInt("OBJECT_ID") << endl;
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

  //cout << "createFolderIfNotExists" << endl << DbObject::toString(&folder) << endl;
  
  return folder.objectId();
}


void VirtualContainerMgr::updateFile(fuppes::DbObject* object, fuppes::ObjectDetails* oldDetails) // static
{
  StringList vfolders = CSharedConfig::Shared()->virtualFolders()->getEnabledFolders();
  for(unsigned int i = 0; i < vfolders.size(); i++) {  
    updateFileForLayout(object, oldDetails, vfolders.at(i));
  }
}

void VirtualContainerMgr::updateFileForLayout(fuppes::DbObject* object, fuppes::ObjectDetails* oldDetails, std::string layout) // static
{
  deleteFileForLayout(object, layout);
  insertFileForLayout(object, layout);
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
