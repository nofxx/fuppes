/***************************************************************************
 *            UPnPAction.cpp
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

#include "UPnPAction.h"

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
