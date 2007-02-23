/***************************************************************************
 *            UPnPBrowse.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "UPnPBrowse.h"
#include "../Common/Common.h"

CUPnPBrowse::CUPnPBrowse(std::string p_sMessage):
  CUPnPAction(UPNP_SERVICE_CONTENT_DIRECTORY, UPNP_BROWSE, p_sMessage)
{
}                                     

CUPnPBrowse::~CUPnPBrowse()
{
}

unsigned int CUPnPBrowse::GetObjectIDAsInt()
{
  return HexToInt(m_sObjectID);
}
