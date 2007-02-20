/***************************************************************************
 *            UPnPSearch.cpp
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
 
#include "UPnPSearch.h"
#include "../Common/Common.h"
#include "../Common/RegEx.h"

#include <sstream>
#include <iostream>

using namespace std;

CUPnPSearch::CUPnPSearch(std::string p_sMessage):
  CUPnPAction(UPNP_DEVICE_TYPE_CONTENT_DIRECTORY, UPNP_SEARCH, p_sMessage)
{
}                                     

CUPnPSearch::~CUPnPSearch()
{
}

/*unsigned int CUPnPSearch::GetObjectIDAsInt()
{
  return HexToInt(m_sObjectID);
}*/

std::string CUPnPSearch::BuildSQL()
{
  stringstream sSQL;
	sSQL << "select * from objects where ";
	
	string sTmp = m_sSearchCriteria;
	
	RegEx rxCrit("(\\([\\w| |:|]+\\)) *([and|or]*)", PCRE_CASELESS);
	if(rxCrit.Search(sTmp.c_str()))
	{
	  cout << "CRITERIA: " << rxCrit.Match(1) << endl;
		if(rxCrit.SubStrings() == 3)
	    cout << "OPERATOR: " << rxCrit.Match(1) << endl;		
	}
	else
	{
	  cout << "CRITERIA not found" << endl;
	}
	
	//(upnp:class contains "object.item.imageItem") and (dc:title contains "")
	

	

  return sSQL.str();
}
