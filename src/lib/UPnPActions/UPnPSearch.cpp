/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            UPnPSearch.cpp
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
 
#include "UPnPSearch.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"
#include "../ContentDirectory/UPnPObjectTypes.h"
#include "../ContentDirectory/DatabaseConnection.h"

#include <sstream>
#include <iostream>

using namespace std;

CUPnPSearch::CUPnPSearch(std::string message)
:CUPnPBrowseSearchBase(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_SEARCH, message)
{
}                                     

std::string CUPnPSearch::getQuery(bool count /*= false*/) 
{
	if(!prepareSQL()) {
		return "";
	}

	if(count) 
		return m_queryCount;
	else
		return m_query;
}



void BuildParentIdList(SQLQuery* qry, std::string p_sIds, std::string p_sDevice, std::string* m_sParentIds)
{
  // read child ids of p_sIds
/*  stringstream sSql;
  sSql << 
    "select OBJECT_ID from OBJECTS " <<
    "where " <<
    "  PARENT_ID in (" << p_sIds << ") and " <<
    "  DEVICE " << p_sDevice;  */

  string sql = qry->build(SQL_SEARCH_GET_CHILDREN_OBJECT_IDS, p_sIds, p_sDevice);  
  qry->select(sql);
  if(qry->eof()) {
    return;
  }
  
  string sResult = "";
  while(!qry->eof()) {    
    sResult += qry->result()->asString("OBJECT_ID") + ", ";
    qry->next();
  }

  // append to list
  (*m_sParentIds) += sResult;
    
  // remove trailing ", "
  sResult = sResult.substr(0, sResult.length() - 2);
    
  // recursively read child objects
  BuildParentIdList(qry, sResult, p_sDevice, m_sParentIds);
}

bool CUPnPSearch::prepareSQL()
{
	if(m_query.length() > 0 && m_queryCount.length() > 0) {
		return true;
	}

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
  SQLQuery qry;
  
  string sDevice = virtualFolderLayout();
  bVirtualSearch = !sDevice.empty();

  
  unsigned int nContainerId = GetObjectIDAsUInt(); //GetContainerIdAsUInt();
  if(nContainerId > 0) {
    
    if(m_sParentIds.length() == 0) {
      stringstream sIds;
      sIds << nContainerId;    
      BuildParentIdList(&qry, sIds.str(), sDevice, &m_sParentIds);      
      m_sParentIds = m_sParentIds + sIds.str();      
      //cout << "PARENT ID LIST: " << m_sParentIds << endl; fflush(stdout);
    }
  }
  
  
	m_query = qry.build(SQL_SEARCH_PART_SELECT_FIELDS, 0, sDevice); //"select * ";
	m_queryCount  = qry.build(SQL_SEARCH_PART_SELECT_COUNT, 0, sDevice); //"select count(*) as COUNT ";


  
  /*sSql <<
    "from " <<
    "  MAP_OBJECTS m, OBJECTS o " <<
    "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  o.DEVICE " << sDevice << " and " <<
    "  m.DEVICE " << sDevice << " and " <<
    "  o.OBJECT_ID = m.OBJECT_ID "; */


  sSql << qry.build(SQL_SEARCH_PART_FROM, 0, sDevice);
  
  if(m_sParentIds.length() > 0) {
    sSql << " and " <<
      "  PARENT_ID in (" << m_sParentIds << ") ";        
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

					sProp = "REF_ID";
						
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
				  sProp = "TYPE";
					bNumericProp = true;
				}
				else if(sProp.compare("dc:title") == 0) {
				  sProp = "TITLE";
					bNumericProp = false;
				}
        else if(sProp.compare("upnp:artist") == 0) {
				  sProp = "A_ARTIST";
					bNumericProp = false;
				}
				else if(sProp.compare("upnp:genre") == 0) {
				  sProp = "A_GENRE";
					bNumericProp = false;
				}
				else if(sProp.compare("upnp:album") == 0) {
				  sProp = "A_ALBUM";
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
				if(sProp.compare("TYPE") == 0) { 
          sOp = "in";
          
          #warning todo: use values from UPnPObjectTypes.h
				  if(sVal.compare("object.item") == 0) {
						sOp = ">=";
						sVal = "100";
					}
					else if(sVal.compare("object.item.imageItem") == 0)
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
	
	m_query += sSql.str();
	m_queryCount += sSql.str();
	
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
	m_query += sSql.str();

  //cout << m_query << endl;
  
  return true;
}
