/***************************************************************************
 *            UPnPSearch.h
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
 
#ifndef _UPNPSEARCH_H
#define _UPNPSEARCH_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "UPnPAction.h"

class CUPnPSearch: public CUPnPBrowseSearchBase
{
  public:
	  CUPnPSearch(std::string p_sMessage);
		~CUPnPSearch();
		
		std::string BuildSQL(bool p_bCount = false);
	  unsigned int GetContainerIdAsUInt();
  
  
    std::string      m_sContainerID;
    std::string      m_sSearchCriteria; 
  
  private:
    std::string      m_sParentIds;
};

#endif // _UPNPSEARCH_H
