/***************************************************************************
 *            UPnPBrowse.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
 
#ifndef _UPNPBROWSE_H
#define _UPNPBROWSE_H

#include "../UPnPAction.h"

typedef enum tagUPNP_BROWSE_FLAG
{
  UPNP_BROWSE_FLAG_DIRECT_CHILDREN,
  UPNP_BROWSE_FLAG_MAX
}UPNP_BROWSE_FLAG;

class CUPnPBrowse: public CUPnPAction
{
  public:
    std::string     m_sObjectID;
    UPNP_BROWSE_FLAG m_nBrowseFlag;
    std::string     m_sFilter;
    unsigned int    m_nStartingIndex;
    unsigned int    m_nRequestedCount;
    std::string     m_sSortCriteria;
};

#endif /* _UPNPBROWSE_H */
