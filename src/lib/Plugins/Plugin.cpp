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
#include "../SharedLog.h"

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
  
	cout << "read plugins from: " << pluginDir << endl;
	
  dir = opendir(pluginDir.c_str());
	if(dir == NULL) {
		cout << "error opening: " << pluginDir << endl;
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
		
		cout << "read file: " << fileName << endl;
		
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

			switch(pluginInfo.plugin_type) {
				case PT_METADATA:
					plugin = new CMetadataPlugin(handle, &pluginInfo);
				  if(plugin->initPlugin()) {
						m_instance->m_metadataPlugins[string(pluginInfo.plugin_name)] =	(CMetadataPlugin*)plugin;
						CSharedLog::Log(L_NORM, __FILE__, __LINE__, "registered metadata plugin \"%s\"", pluginInfo.plugin_name);
					}
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
		}
		catch(EException ex) {
		}
		
	} // while
	
	cout << "end read plugins" << endl;
	closedir(dir);
}

CMetadataPlugin* CPluginMgr::metadataPlugin(std::string pluginName)
{
	m_instance->m_metadataPluginsIter =
		m_instance->m_metadataPlugins.find(pluginName);
	
	if(m_instance->m_metadataPluginsIter != m_instance->m_metadataPlugins.end()) {
		return m_instance->m_metadataPlugins[pluginName];
	}
	else {
		return NULL;
	}
}


CTranscoderBase* CPluginMgr::transcoderPlugin(std::string pluginName)
{
#warning todo lock

	m_instance->m_transcoderPluginsIter =
		m_instance->m_transcoderPlugins.find(pluginName);
	
	if(m_instance->m_transcoderPluginsIter != m_instance->m_transcoderPlugins.end()) {
		CTranscoderPlugin* plugin = new CTranscoderPlugin(m_instance->m_transcoderPlugins[pluginName]);
		return (CTranscoderBase*)plugin;
	}
	else {
		return NULL;
	}
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
	FuppesCloseLibrary(m_handle);
}

bool CMetadataPlugin::initPlugin()
{
	m_fileOpen = NULL;
	m_readData = NULL;	
	m_fileClose = NULL;

	m_fileOpen = (metadataFileOpen_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_file_open");
	if(m_fileOpen == NULL) {
		return false;
	}
	
	m_readData	= (metadataRead_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_read");
	if(m_readData == NULL) {
		return false;
	}
	
	m_fileClose	= (metadataFileClose_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_file_close");
	
	return true;
}

bool CMetadataPlugin::openFile(std::string fileName)
{	
	if(m_fileOpen == NULL) {
		return false;
	}
	return ((m_fileOpen(&m_pluginInfo, fileName.c_str())) == 0);
}

bool CMetadataPlugin::readData(metadata_t* metadata)
{	
	if(m_readData == NULL) {
		return false;
	}
	return (m_readData(&m_pluginInfo, metadata) == 0);
}

void CMetadataPlugin::closeFile()
{
	if(m_fileClose) {
		m_fileClose(&m_pluginInfo);
	}
}


CTranscoderPlugin::CTranscoderPlugin(CTranscoderPlugin* plugin):
CPlugin(plugin->m_handle, &plugin->m_pluginInfo) 
{
	m_init = plugin->m_init;
}

bool CTranscoderPlugin::initPlugin()
{
	/*m_fileOpen = NULL;
	m_readData = NULL;	
	m_fileClose = NULL;

	m_fileOpen = (metadataFileOpen_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_file_open");
	if(m_fileOpen == NULL) {
		return false;
	}
	
	m_readData	= (metadataRead_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_read");
	if(m_readData == NULL) {
		return false;
	}
	
	m_fileClose	= (metadataFileClose_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_file_close");
	*/
	return true;
}


bool CTranscoderPlugin::Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile)
{
}
