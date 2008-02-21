/***************************************************************************
 *            Plugin.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "Plugin.h"

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <iostream>
using namespace std;

CPluginMgr* CPluginMgr::m_instance = 0;

void CPluginMgr::init(std::string pluginDir)
{
	if(m_instance == 0) {
		m_instance = new CPluginMgr();
	}
	
	DIR*    dir;
  dirent* dirEnt;
  
  dir = opendir(pluginDir.c_str());
	if(dir == NULL) {
		return;
	}
	
	struct stat	Stat;
	string			fileName;
	string			ext;
	size_t			pos;	
	CPlugin*		plugin = NULL;
	
	registerPlugin_t regPlugin;
	
  while((dirEnt = readdir(dir)) != NULL) {
		
		fileName = pluginDir + dirEnt->d_name;		
		
		if(!IsFile(fileName)) {
			continue;
		}
		
		ext = ExtractFileExt(fileName);
		if(ext.empty()) {
			continue;
		}
		
		#ifdef WIN32
		if(ext.compare("dll") != 0) {
			continue;
		}
		#else
		if((ext.compare("so") != 0) &&
			 (ext.compare("dylib") != 0)) {
			continue;
		}
		#endif
		
		try {
			plugin_info pluginInfo;
			pluginInfo.plugin_type = PT_NONE;

			fuppesLibHandle handle;
			
			//cout << "load: " << fileName << endl;
			
			handle = FuppesLoadLibrary(fileName);
			regPlugin = (registerPlugin_t)FuppesGetProcAddress(handle, "register_fuppes_plugin");	
	
			if(regPlugin == NULL) {
				FuppesCloseLibrary(handle);
				continue;
			}

			regPlugin(&pluginInfo);
			
			//cout << "name:" << pluginInfo.plugin_name << endl;
			
			switch(pluginInfo.plugin_type) {
				case PT_METADATA:
					plugin = new CMetadataPlugin(handle, &pluginInfo);
					break;
				case PT_AUDIO_DECODER:
					//plugin = new CAudioDecoderPlugin();
					break;
				case PT_AUDIO_ENCODER:
					//plugin = new CAudioEncoderPlugin();
					break;
				case PT_TRANSCODER:
				case PT_THREADED_TRANSCODER:
					//plugin = new CTranscoderPlugin();
					break;
			}

			if(plugin) {
				plugin->initPlugin();
			}
		}
		catch(EException ex) {
		}
		
	} // while
	
}

CPlugin::CPlugin(fuppesLibHandle handle, plugin_info* info)
{
	strcpy(m_pluginInfo.plugin_name, info->plugin_name);
	m_pluginInfo.plugin_type = info->plugin_type;
	m_pluginInfo.user_data = NULL;
	
	m_handle = handle;
}

CPlugin::~CPlugin()
{
}

bool CMetadataPlugin::initPlugin()
{
	return true;
}
