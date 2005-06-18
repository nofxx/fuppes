/***************************************************************************
 *            SharedConfig.h
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
 
#ifndef _SHAREDCONFIG_H
#define _SHAREDCONFIG_H

#include <string>
using namespace std;

class shared_config
{
	public:
		static shared_config* shared();
	
		std::string get_app_name();
	  std::string get_app_fullname();
	  std::string get_app_version();
	
	  std::string get_hostname();
	  std::string get_udn();
	
		std::string get_ip();
	
	protected:
		shared_config();
	
	private:
		static shared_config* instance;
	
	  std::string hostname;
	  std::string ip;
};

#endif /* _SHAREDCONFIG_H */
