/***************************************************************************
 *            UPnPAction.cpp
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

#include "UPnPAction.h"
//#include "UPnPActionFactory.h"

CUPnPBrowseSearchBase::CUPnPBrowseSearchBase(UPNP_DEVICE_TYPE p_nTargetDeviceType, int p_nActionType, std::string p_sContent)
  :CUPnPAction(p_nTargetDeviceType, p_nActionType, p_sContent)
{
}

bool CUPnPBrowseSearchBase::IncludeProperty(std::string p_sProperty)
{
  if(m_sFilter.compare("*") == 0) {
    return true;
  }

  if(m_sFilter.find(p_sProperty) != std::string::npos) {
    return true;
  }
 
  return false;
}

std::string CUPnPBrowseSearchBase::getSortOrder()
{
	if(m_isSupportedSort) {
		return m_sortCriteriaSQL;
	}
	else {
		return " o.TITLE asc ";
	}
}
