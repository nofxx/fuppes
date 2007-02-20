/***************************************************************************
 *            DeviceSettings.cpp
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

#include "DeviceSettings.h"
#include "../Common/RegEx.h"
#include <iostream>

using namespace std;

CDeviceSettings::CDeviceSettings(std::string p_sDeviceName)
{
  m_sDeviceName = p_sDeviceName;
}

bool CDeviceSettings::HasUserAgent(std::string p_sUserAgent)
{
  bool   bResult = false;
	string sUserAgent;
	RegEx* pRxUserAgent;
	std::list<std::string>::const_iterator it;
	
	for(it = m_slUserAgents.begin(); it != m_slUserAgents.end(); it++)
	{
		sUserAgent = *it;
	
		pRxUserAgent = new RegEx(sUserAgent.c_str(), PCRE_CASELESS);
		if(pRxUserAgent->Search(p_sUserAgent.c_str())) {
			bResult = true;
			delete pRxUserAgent;
			break;
		}
		
		delete pRxUserAgent;
	}

	return bResult;
}

bool CDeviceSettings::HasIP(std::string p_sIPAddress)
{
  string sIP;
	std::list<std::string>::const_iterator it;
	for(it = m_slIPAddresses.begin(); it != m_slIPAddresses.end(); it++)
	{
	  //cout << p_sIPAddress << endl;
	
	  sIP = *it;
		if(sIP.compare(p_sIPAddress) == 0)
		  return true;
	}
	return false;
}