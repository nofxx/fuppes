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

#include "../Common/Common.h"

#include "../../../include/fuppes_plugin.h"

typedef void (*registerPlugin_t)(plugin_info* info);

class CPlugin;

class CPluginMgr
{
	public:
		static void init(std::string pluginDir);
	
	private:
		static CPluginMgr* m_instance;
};

class CPlugin
{
	protected:
		CPlugin(fuppesLibHandle handle, plugin_info* info);
		~CPlugin();
	
	public:
		PLUGIN_TYPE		pluginType() { return m_pluginInfo.plugin_type; }
		
		virtual bool 	initPlugin() = 0;
	
	private:
		fuppesLibHandle		m_handle;
		plugin_info				m_pluginInfo;
};

class CMetadataPlugin: public CPlugin
{
	public:
		CMetadataPlugin(fuppesLibHandle handle, plugin_info* info): 
			CPlugin(handle, info) {}
	
		bool initPlugin();
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
