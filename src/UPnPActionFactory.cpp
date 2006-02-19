/***************************************************************************
 *            UPnPActionFactory.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "Common.h"
#include "UPnPActionFactory.h"
#include "UPnPActions/UPnPBrowse.h"
#include "RegEx.h"

#include <iostream>
#include <libxml/parser.h>
#include <libxml/tree.h>

using namespace std;

/*===============================================================================
 CLASS CUPnPActionFactory
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 UPNP ACTIONS
===============================================================================*/

CUPnPAction* CUPnPActionFactory::BuildActionFromString(std::string p_sContent)
{  
  //BOOL_CHK_RET_POINTER(pAction);  
  //cout << p_sContent << endl;
  
  xmlDocPtr pDoc = NULL;
  pDoc = xmlReadMemory(p_sContent.c_str(), p_sContent.length(), "", NULL, 0);
  if(!pDoc)
    cout << "error parsing action" << endl;
   
  xmlNode* pRootNode = NULL;  
  xmlNode* pTmpNode  = NULL;   
  pRootNode = xmlDocGetRootElement(pDoc);   
  
  pTmpNode = pRootNode->children->next;
  string sName = (char*)pTmpNode->name;
  cout << sName << endl;

  pTmpNode = pTmpNode->children->next;
  sName = (char*)pTmpNode->name;
  cout << sName << endl;
  
  if(sName.compare("Browse") == 0)
  {
  }
  else if(sName.compare("GetSearchCapabilities") == 0)
  {
  }
  else if(sName.compare("GetSortCapabilities") == 0)
  {
  }
  else if(sName.compare("GetSystemUpdateID") == 0)
  {
  }  
  

  /* T.S.TODO: We have to parse the whole description here */
/*<?xml version="1.0" encoding="utf-8"?>
  <s:Envelope s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/" xmlns:s="http://schemas.xmlsoap.org/soap/envelope/">
  <s:Body>
  <u:Browse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1">
  <ObjectID>0</ObjectID>
  <BrowseFlag>BrowseDirectChildren</BrowseFlag>
  <Filter>*</Filter>
  <StartingIndex>0</StartingIndex>
  <RequestedCount>10</RequestedCount>
  <SortCriteria />
  </u:Browse>
  </s:Body>
  </s:Envelope>*/
  CUPnPBrowse* pAction = new CUPnPBrowse(p_sContent);
  
  /* Object ID */
  RegEx rxObjId("<ObjectID>(.+)</ObjectID>");
  if (rxObjId.Search(p_sContent.c_str()))  
    pAction->m_sObjectID = rxObjId.Match(1);
  
  /* Browse flag */
  pAction->m_nBrowseFlag = UPNP_BROWSE_FLAG_DIRECT_CHILDREN;
  
  /* Filter */
  pAction->m_sFilter    = "*";

  /* Starting index */
  RegEx rxStartIdx("<StartingIndex>(.+)</StartingIndex>");
  if(rxStartIdx.Search(p_sContent.c_str()))  
    pAction->m_nStartingIndex = atoi(rxStartIdx.Match(1));

  /* Requested count */
  RegEx rxReqCnt("<RequestedCount>(.+)</RequestedCount>");
  if(rxReqCnt.Search(p_sContent.c_str()))  
    pAction->m_nRequestedCount = atoi(rxReqCnt.Match(1));  
  
  /* Sort */
  pAction->m_sSortCriteria = "";

  /* Target */
  pAction->m_nTargetDevice = UPNP_DEVICE_TYPE_CONTENT_DIRECTORY;
  
  /*cout << "[UPnPActionFactory] Browse Action:" << endl;
  cout << "\tObjectID: " << ((CUPnPBrowse*)pResult)->m_sObjectID << endl;
  cout << "\tStartingIndex: " << ((CUPnPBrowse*)pResult)->m_nStartingIndex << endl;
  cout << "\tRequestedCount: " << ((CUPnPBrowse*)pResult)->m_nRequestedCount << endl;*/  
  
  xmlFreeDoc(pDoc);
  xmlCleanupParser();  
  
  return pAction;
}

/* <\PUBLIC> */
