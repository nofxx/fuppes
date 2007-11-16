/***************************************************************************
 *            MediaServer.cpp
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
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
 
#include "MediaServer.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

CMediaServer::CMediaServer(std::string p_sHTTPServerURL, IUPnPDevice* pOnTimerHandler):
  CUPnPDevice(UPNP_DEVICE_MEDIA_SERVER, p_sHTTPServerURL, pOnTimerHandler)
{
  /*m_sFriendlyName = CSharedConfig::Shared()->FriendlyName();

	if(!CSharedConfig::Shared()->ConfigFile()->MediaServerSettings()->Manufacturer.empty()) {
  	m_sManufacturer = CSharedConfig::Shared()->ConfigFile()->MediaServerSettings()->Manufacturer;
	}
	else {
		m_sManufacturer = "Ulrich Voelkel";
	}
  m_sManufacturerURL  = "http://www.ulrich-voelkel.de";      
  m_sModelDescription = "Free UPnP Media Server licensed under the terms of the GPL";

  m_sModelName = "Free UPnP Entertainment Service " + CSharedConfig::Shared()->GetAppVersion();

  m_sModelNumber = CSharedConfig::Shared()->GetAppVersion();

  m_sModelURL        = "http://fuppes.sourceforge.net";
  m_sSerialNumber    = "012345678910";*/
  m_sUUID  			     = CSharedConfig::Shared()->GetUUID();
  //m_sUPC				     = "";
  m_sPresentationURL = "index.html";      
}
  
CMediaServer::~CMediaServer()
{
}
