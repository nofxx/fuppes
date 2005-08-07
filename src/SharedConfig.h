/***************************************************************************
 *            SharedConfig.h
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
 
#ifndef _SHAREDCONFIG_H
#define _SHAREDCONFIG_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>
#include <vector>

using namespace std;

/*===============================================================================
 CLASS CSharedConfig
===============================================================================*/

class CSharedConfig
{

/* <PROTECTED> */

protected:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

		CSharedConfig();

/* <\PROTECTED> */

/* <PUBLIC> */

	public:

/*===============================================================================
 INSTANCE
===============================================================================*/

  static CSharedConfig* Shared();

/*===============================================================================
 INIT
===============================================================================*/

  bool        SetupConfig();

/*===============================================================================
 GET
===============================================================================*/

  std::string GetAppName();
	std::string GetAppFullname();
	std::string GetAppVersion();
	
	std::string GetHostname();
	std::string GetUUID();
	
	std::string GetIPv4Address();
	
  std::string GetSharedDir(int);

  int         SharedDirCount();
  
/* <\PUBLIC> */
	
/* <PRIVATE> */

private:

/*===============================================================================
 INSTANCE
===============================================================================*/
    
  static CSharedConfig* m_Instance;

/*===============================================================================
 MEMBERS
===============================================================================*/

  std::string m_sHostname;
	std::string m_sIP;
  std::string m_sUUID;
  std::vector<std::string> m_vSharedDirectories;

/*===============================================================================
 HELPER
===============================================================================*/

  bool ReadConfigFile();
  bool ResolveHostAndIP();
  bool ResolveIPByHostname();
  bool ResolveIPByInterface(std::string p_sInterfaceName);
  bool FileExists(std::string p_sFileName);
  bool WriteDefaultConfig(std::string p_sFileName); 

/* <\PRIVATE> */

};

#endif /* _SHAREDCONFIG_H */
