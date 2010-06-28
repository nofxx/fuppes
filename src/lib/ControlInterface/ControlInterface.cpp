/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ControlInterface.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "ControlInterface.h"

#include <iostream>
#include <sstream>
using namespace std;

#include "../SharedConfig.h"

ControlInterface* ControlInterface::m_instance = NULL;

void ControlInterface::init() // static
{
	ASSERT(m_instance == NULL);
  m_instance = new ControlInterface();
}

void ControlInterface::uninit() // static
{
	ASSERT(m_instance != NULL);
  delete m_instance;
  m_instance = NULL;
}

ControlInterface::ControlInterface()
{
}


void exec(FUPPES_CONTROL_ACTION action) // static
{
  switch(action) {
  }
}


/*
int	CControlInterface::action(std::string action, stringList* args, stringList* result)
{
	//CFuppes* fuppes = CSharedConfig::Shared()->GetFuppesInstance(0);
	
	if(action.compare("get_version") == 0) {		
		(*result)["version"] = CSharedConfig::Shared()->GetAppVersion();
	}
	else if(action.compare("get_friendly_name") == 0) {		
		(*result)["friendly_name"] = CSharedConfig::Shared()->globalSettings->GetFriendlyName();
	}
	else if(action.compare("get_data_dir") == 0) {		
		(*result)["data_dir"] = CSharedConfig::Shared()->dataDir();
	}
	else if(action.compare("get_hostname") == 0) {		
		(*result)["hostname"] = CSharedConfig::Shared()->networkSettings->GetHostname();
	}
	else if(action.compare("get_ip_address") == 0) {
		(*result)["ip_address"] = CSharedConfig::Shared()->networkSettings->GetIPv4Address();
	}
	
	else {
		return 1;
	}
	
	return 0;
}
*/
