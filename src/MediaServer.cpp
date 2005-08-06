/***************************************************************************
 *            MediaServer.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "MediaServer.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

/*===============================================================================
 CLASS CMediaServer
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CMediaServer::CMediaServer(std::string p_sHTTPServerURL):
  CUPnPDevice(UPNP_DEVICE_TYPE_MEDIA_SERVER, p_sHTTPServerURL)
{
  /* Set common data for FUPPES */

  stringstream stream;

  //stream << "Free UPnP Entertainment Service (" << CSharedConfig::Shared()->GetHostname() << ")";	
  stream << CSharedConfig::Shared()->GetAppName() << " ";
  stream << CSharedConfig::Shared()->GetAppVersion() << " (";
  stream << CSharedConfig::Shared()->GetHostname() << ")";  
  m_sFriendlyName = stream.str();
  stream.str("");
  stream.clear();

  m_sManufacturer     = "Ulrich Völkel";
  m_sManufacturerURL  = "http://www.ulrich-voelkel.de";      
  m_sModelDescription = "Free UPnP Media Server licensed under the terms of the GPL";

  stream << "Free UPnP Entertainment Service " << CSharedConfig::Shared()->GetAppVersion();
  m_sModelName = stream.str();
  stream.str("");
  stream.clear();

  stream << CSharedConfig::Shared()->GetAppVersion();   
  m_sModelNumber = stream.str();
  stream.str("");
  stream.clear();

  m_sModelURL        = "http://fuppes.sourceforge.net";
  m_sSerialNumber    = "012345678910";
  m_sUUID  			     = CSharedConfig::Shared()->GetUUID();
  m_sUPC				     = "";
  m_sPresentationURL = "index.html";      
}
  
CMediaServer::~CMediaServer()
{
}

/* <\PUBLIC> */
