/***************************************************************************
 *            MediaServer.cpp
 * 
 *  Copyright  2005  Ulrich Völkel
 *  mail@ulrich-voelkel.de
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
 
#include "MediaServer.h"
#include "SharedConfig.h"

#include <iostream>
#include <sstream>

using namespace std;

CMediaServer::CMediaServer(): CUPnPDevice(dt_media_server)
{
	cout << this->GetDeviceTypeAsString() << endl;
	
	stringstream stream;
	
	stream << "Free UPnP Entertainment Service (" << shared_config::shared()->get_hostname() << ")";	
	//stream << "FUPPES (" << shared_config::shared()->get_hostname() << ")";	
	m_sFriendlyName = stream.str();
	stream.str("");
	stream.clear();
	
	m_sManufacturer     = "Ulrich Voelkel";
  m_sManufacturerURL  = "http://www.ulrich-voelkel.de";      
	m_sModelDescription = "Free UPnP Media Server licensed under the terms of the GPL";

	stream << "Free UPnP Entertainment Service " << shared_config::shared()->get_app_version();
	m_sModelName = stream.str();
  stream.str("");
	stream.clear();
	
  stream << shared_config::shared()->get_app_version();   
  m_sModelNumber = stream.str();
	stream.str("");
	stream.clear();
	
	m_sModelURL        = "";
	m_sSerialNumber    = "012345678910";
  m_sUDN  			     = shared_config::shared()->get_udn();
	m_sUPC				     = "";
	m_sPresentationURL = "index.html";      
}
  
CMediaServer::~CMediaServer()
{
}
