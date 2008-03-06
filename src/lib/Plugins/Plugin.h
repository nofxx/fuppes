/***************************************************************************
 *            Plugin.h
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

#ifndef _PLUGIN_H
#define _PLUGIN_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <map>

#include "../Common/Common.h"
#include "../Transcoding/WrapperBase.h"

#include "../../../include/fuppes_plugin.h"

typedef void (*registerPlugin_t)(plugin_info* info);

class CMetadataPlugin;
class CTranscoderPlugin;

class CPluginMgr
{
	public:
		static void init(std::string pluginDir);
	
		static CMetadataPlugin* metadataPlugin(std::string pluginName);
		static CTranscoderBase* transcoderPlugin(std::string pluginName);
	
	private:
		static CPluginMgr* m_instance;
	
		std::map<std::string, CMetadataPlugin*> m_metadataPlugins;
		std::map<std::string, CMetadataPlugin*>::iterator m_metadataPluginsIter;
		
		std::map<std::string, CTranscoderPlugin*> m_transcoderPlugins;
		std::map<std::string, CTranscoderPlugin*>::iterator m_transcoderPluginsIter;
};

class CPlugin
{
	protected:
		CPlugin(fuppesLibHandle handle, plugin_info* info);
	
	public:
		virtual ~CPlugin();
		PLUGIN_TYPE		pluginType() { return m_pluginInfo.plugin_type; }
		
		virtual bool 	initPlugin() = 0;
		virtual bool  openFile(std::string) { return false; }
		virtual bool	readData(metadata_t*) { return false; }
		virtual void	closeFile() {};
	
	protected:
		fuppesLibHandle		m_handle;
		plugin_info				m_pluginInfo;
};


typedef int		(*metadataFileOpen_t)(plugin_info* plugin, const char* fileName);
typedef int		(*metadataRead_t)(plugin_info* plugin, metadata_t* audio);
typedef void	(*metadataFileClose_t)(plugin_info* plugin);

class CMetadataPlugin: public CPlugin
{
	public:
		CMetadataPlugin(fuppesLibHandle handle, plugin_info* info): 
			CPlugin(handle, info) {}
	
		bool initPlugin();
	
		bool openFile(std::string fileName);
		bool readData(metadata_t* metadata);
		void closeFile();
	
	private:
		metadataFileOpen_t				m_fileOpen;
		metadataRead_t						m_readData;
		metadataFileClose_t				m_fileClose;
};

typedef int		(*transcoderInit_t)(plugin_info* info, const char* audioCodec, const char* videoCodec);

class CTranscoderPlugin: public CPlugin, CTranscoderBase
{
	public:
		CTranscoderPlugin(fuppesLibHandle handle, plugin_info* info): 
			CPlugin(handle, info) {}
			
		CTranscoderPlugin(CTranscoderPlugin* plugin);
			
		// from CPlugin
		bool 	initPlugin();		
			
		// from CTranscoderBase
		bool Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile);
    bool Threaded() { return true; }
			
		private:
			transcoderInit_t		m_init;		
};




/*class CAudioDecoderPlugin: public CPlugin
{
};

class CAudioEncoderPlugin: public CPlugin
{
};

class CTranscoderPlugin: public CPlugin
{
};*/

#endif // _PLUGIN_H
