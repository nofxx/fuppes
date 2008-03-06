/***************************************************************************
 *            UPnPSearch.cpp
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
 
#include "UPnPSearch.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../ContentDirectory/ContentDatabase.h"
#include "../ContentDirectory/VirtualContainerMgr.h"

#include <sstream>
#include <iostream>

using namespace std;

CUPnPSearch::CUPnPSearch(std::string p_sMessage):
  CUPnPBrowseSearchBase(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_SEARCH, p_sMessage)
{
  m_sParentIds = "";
	m_SQLselect = "";
	m_SQLtotalMatches = "";
}                                     

CUPnPSearch::~CUPnPSearch()
{
}

unsigned int CUPnPSearch::GetContainerIdAsUInt()
{
  return HexToInt(m_sContainerID);
}


void BuildParentIdList(CContentDatabase* pDb, std::string p_sIds, std::string p_sDevice, std::string* m_sParentIds)
{
  // read child ids of p_sIds
  stringstream sSql;
  sSql << 
    "select OBJECT_ID from MAP_OBJECTS " <<
    "where " <<
    "  PARENT_ID in (" << p_sIds << ") and " <<
    "  DEVICE " << p_sDevice;  
    
  pDb->Select(sSql.str());
  if(pDb->Eof()) {
    return;
  }
  
  string sResult = "";
  while(!pDb->Eof()) {    
    sResult += pDb->GetResult()->GetValue("OBJECT_ID") + ", ";
    pDb->Next();
  }

  // append to list
  (*m_sParentIds) += sResult;
    
  // remove trailing ", "
  sResult = sResult.substr(0, sResult.length() - 2);
    
  // recursively read child objects
  BuildParentIdList(pDb, sResult, p_sDevice, m_sParentIds);
}


bool CUPnPSearch::prepareSQL()
{
  /*std::string sTest;
	sTest = "(upnp:class contains \"object.item.imageItem\") and (dc:title = \"test \\\"dahhummm\\\" [xyz] §$%&(abc) titel\") or author exists true and (title exists false and (author = \"test\" or author = \"dings\"))";
*/

  string sOpenBr;
	string sProp;
	string sOp;
	string sVal;
	string sCloseBr;
	string sLogOp;
  string sPrevLog;
	bool   bNumericProp = false;
	bool   bLikeOp  = false;
  bool   bBuildOK = false;
  bool   bVirtualSearch = false;
  bool   bFirst = true;
  
  stringstream sSql;

  string sDevice = " is NULL ";
  
  unsigned int nContainerId = GetContainerIdAsUInt();
  if(nContainerId > 0) {
    if(CVirtualContainerMgr::Shared()->IsVirtualContainer(nContainerId, DeviceSettings()->VirtualFolderDevice())) {
      bVirtualSearch = true;
      sDevice = " = '" + DeviceSettings()->VirtualFolderDevice() + "' ";
    }
    
    if(m_sParentIds.length() == 0) {
      CContentDatabase* pDb = new CContentDatabase();       
      stringstream sIds;
      sIds << nContainerId;    
      BuildParentIdList(pDb, sIds.str(), sDevice, &m_sParentIds);      
      m_sParentIds = m_sParentIds + sIds.str();      
      delete pDb;
      
      //cout << "PARENT ID LIST: " << m_sParentIds << endl; fflush(stdout);
    }
  }
	else if(nContainerId == 0) {
		if(CVirtualContainerMgr::Shared()->HasVirtualChildren(nContainerId, DeviceSettings()->VirtualFolderDevice())) {
      bVirtualSearch = true;
      sDevice = " = '" + DeviceSettings()->VirtualFolderDevice() + "' ";
    }
	}
  
  
	m_SQLselect = "select * ";
	m_SQLtotalMatches  = "select count(*) as COUNT ";
	
  sSql <<
    "from " <<
    "  OBJECTS o, MAP_OBJECTS m " <<
    "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  o.DEVICE " << sDevice << " and " <<
    "  m.DEVICE " << sDevice << " and " <<
    "  o.OBJECT_ID = m.OBJECT_ID ";  
  
  if(m_sParentIds.length() > 0) {
    sSql << " and " <<
      "  m.PARENT_ID in (" << m_sParentIds << ") ";        
  }
  
	m_sSearchCriteria = StringReplace(m_sSearchCriteria, "&quot;", "\"");
  m_sSearchCriteria = StringReplace(m_sSearchCriteria, "@", ":");


  RegEx rxSearch("(\\(*) *([\\w+:*\\w*]+) ([=|!=|<|<=|>|>=|contains|doesNotContain|derivedfrom|exists]+) (\".*?[^\\\\]\"|true|false) *(\\)*) *([and|or]*)");
	if(rxSearch.Search(m_sSearchCriteria.c_str())) {
	  do {
		  //cout <<  rxSearch.Match(1) << " X " << rxSearch.Match(2) << " X " << rxSearch.Match(3) << " X " << rxSearch.Match(4) << " X " << rxSearch.Match(5) << " X " << rxSearch.Match(6) << endl;
		
		  sOpenBr  = rxSearch.Match(1);
			sProp    = rxSearch.Match(2);
		  sOp      = rxSearch.Match(3);
			sVal     = rxSearch.Match(4);
			sCloseBr = rxSearch.Match(5);
			sLogOp   = rxSearch.Match(6);
			
			if(sOp.compare("exists") == 0) {
				bBuildOK = false;

				if(sProp.compare(":refID") == 0) {
					bBuildOK = true;

					sProp = "o.REF_ID";
						
					if(sVal.compare("true") == 0)
					  sOp = "is not";
				  else if (sVal.compare("false") == 0)
						sOp = "is";
				
					sVal = "NULL";
				}
        
			}
			else {
				
				bBuildOK = true;
				
				// replace property
				if(sProp.compare("upnp:class") == 0) {
				  sProp = "o.TYPE";
					bNumericProp = true;
				}
				else if(sProp.compare("dc:title") == 0) {
				  sProp = "o.TITLE";
					bNumericProp = false;
				}
        else if(sProp.compare("upnp:artist") == 0) {
				  sProp = "d.A_ARTIST";
					bNumericProp = false;
				}
				else if(sProp.compare("upnp:genre") == 0) {
				  sProp = "d.A_GENRE";
					bNumericProp = false;
				}
        else if(sProp.compare("res:protocolInfo") == 0) {
				  sPrevLog = sLogOp;
					continue;
				}
				else {
				  bBuildOK = false;
				}
				
				
				// replace operator
				bLikeOp = false;
				if(sOp.compare("contains") == 0) {
				  if(bNumericProp)
					  sOp = "in";
					else
					  sOp = "like";
						
					bLikeOp = true;
				}
				else if(sOp.compare("derivedfrom") == 0) {
				  sOp = "in";
				}
        else if(sOp.compare("=") == 0) {          
        }
				else {
				  bBuildOK = false;
				}
				
				
				// trim value
				//cout << "Val: " << sVal << " => ";
			  sVal = sVal.substr(1, sVal.length() - 2);
				//cout << sVal << endl;
				
				// replace value
				if(sProp.compare("o.TYPE") == 0) { 
          sOp = "in";
          
          #warning todo: use values from ContentDatabase.h
				  /*if(sVal.compare("object.item") == 0)
						
					else*/ if(sVal.compare("object.item.imageItem") == 0)
					  sVal = "(110, 111)";
					else if(sVal.compare("object.item.audioItem") == 0)
					  sVal = "(120, 121, 122)";	
					else if(sVal.compare("object.item.videoItem") == 0)
					  sVal = "(130, 131, 132, 133)";
					else if(sVal.compare("object.container.person.musicArtist") == 0)
					  sVal = "(11)";          
					else if(sVal.compare("object.container.album.musicAlbum") == 0)
					  sVal = "(31)";
          else if(sVal.compare("object.container.genre.musicGenre") == 0)
            sVal = "(41)";
          else if (sVal.compare("object.container.playlistContainer") == 0)
            sVal = "(20)";
					else
					  bBuildOK = false;
				} 
				else if (!bNumericProp) {
				  if(bLikeOp)
				    sVal = "'%" + sVal + "%'";
					else
						sVal = "'" + sVal + "'";
				}
				
			} // != exists
		
		  if(bBuildOK) {
        if(bFirst) {
          sSql << " and ";
          bFirst = false;
        }        
  			sSql << sPrevLog << " " << sOpenBr << sProp << " " << sOp << " " << sVal << sCloseBr << " ";
        sPrevLog = sLogOp;
      }
			else {
			  cout << "error parsing search request!" << endl <<
          "please file a bugreport containing the following lines: " << endl << endl <<
          "=== CUT ===" << endl <<
          m_sSearchCriteria << endl <<
          "=== CUT ===" << endl;
      }
			  
			
		}	while (rxSearch.SearchAgain());
	}
	
	m_SQLselect += sSql.str();
	m_SQLtotalMatches += sSql.str();
	
	sSql.str("");
  
  // order by
  sSql << " order by " << m_sortCriteriaSQL << " ";

  // limit
	if((m_nRequestedCount > 0) || (m_nStartingIndex > 0)) {
    sSql << " limit " << m_nStartingIndex << ", ";
    if(m_nRequestedCount == 0)
      sSql << "-1";
    else
      sSql << m_nRequestedCount;
	}

  // order by and limit are not needed
  // in a count request  
	m_SQLselect += sSql.str();

  return true;
}
