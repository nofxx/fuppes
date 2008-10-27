/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
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
#include "../SharedConfig.h"

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
		fuppesThreadInitMutex(&m_instance->m_mutex);
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
			pluginInfo.log = &CPlugin::logCb;

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
				case PT_DLNA:
					plugin = new CDlnaPlugin(handle, &pluginInfo);
					if(plugin->initPlugin()) {
						m_instance->m_dlnaPlugin = (CDlnaPlugin*)plugin;
						CSharedLog::Log(L_NORM, __FILE__, __LINE__, "registered dlna profile plugin");
					}					
					break;
				case PT_METADATA:
					plugin = new CMetadataPlugin(handle, &pluginInfo);
				  if(plugin->initPlugin()) {
						m_instance->m_metadataPlugins[string(pluginInfo.plugin_name)] =	(CMetadataPlugin*)plugin;
						CSharedLog::Log(L_NORM, __FILE__, __LINE__, "registered metadata plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
				case PT_AUDIO_DECODER:
					plugin = new CAudioDecoderPlugin(handle, &pluginInfo);
					if(plugin->initPlugin()) {
						m_instance->m_audioDecoderPlugins[string(pluginInfo.plugin_name)] =	(CAudioDecoderPlugin*)plugin;
						CSharedLog::Log(L_NORM, __FILE__, __LINE__, "registered audio decoder plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
				case PT_AUDIO_ENCODER:
					plugin = new CAudioEncoderPlugin(handle, &pluginInfo);
					if(plugin->initPlugin()) {
						m_instance->m_audioEncoderPlugins[string(pluginInfo.plugin_name)] =	(CAudioEncoderPlugin*)plugin;
						CSharedLog::Log(L_NORM, __FILE__, __LINE__, "registered audio encoder plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
				case PT_TRANSCODER:
				case PT_THREADED_TRANSCODER:
					plugin = new CTranscoderPlugin(handle, &pluginInfo);
          if(plugin->initPlugin()) {
						m_instance->m_transcoderPlugins[string(pluginInfo.plugin_name)] =	(CTranscoderPlugin*)plugin;
						CSharedLog::Log(L_NORM, __FILE__, __LINE__, "registered transcoder plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
			}
		}
		catch(EException ex) {
		}
		
	} // while

	closedir(dir);
}

CMetadataPlugin* CPluginMgr::metadataPlugin(std::string pluginName)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	CMetadataPlugin* plugin = NULL;
	
	m_instance->m_metadataPluginsIter =
		m_instance->m_metadataPlugins.find(pluginName);
	
	if(m_instance->m_metadataPluginsIter != m_instance->m_metadataPlugins.end()) {
		plugin = m_instance->m_metadataPlugins[pluginName];
	}
	
	fuppesThreadUnlockMutex(&m_instance->m_mutex);
	return plugin;	
}


CTranscoderBase* CPluginMgr::transcoderPlugin(std::string pluginName)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	CTranscoderPlugin* plugin = NULL;
		
	m_instance->m_transcoderPluginsIter =
		m_instance->m_transcoderPlugins.find(pluginName);
	
	if(m_instance->m_transcoderPluginsIter != m_instance->m_transcoderPlugins.end()) {
		plugin = new CTranscoderPlugin(m_instance->m_transcoderPlugins[pluginName]);
	}	

	fuppesThreadUnlockMutex(&m_instance->m_mutex);
	return (CTranscoderBase*)plugin;
}

CAudioDecoderPlugin* CPluginMgr::audioDecoderPlugin(std::string pluginName)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	CAudioDecoderPlugin* plugin = NULL;
		
	m_instance->m_audioDecoderPluginsIter =
		m_instance->m_audioDecoderPlugins.find(pluginName);
	
	if(m_instance->m_audioDecoderPluginsIter != m_instance->m_audioDecoderPlugins.end()) {
		plugin = new CAudioDecoderPlugin(m_instance->m_audioDecoderPlugins[pluginName]);
	}	

	fuppesThreadUnlockMutex(&m_instance->m_mutex);
	return plugin;
}

CAudioEncoderPlugin* CPluginMgr::audioEncoderPlugin(std::string pluginName)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	CAudioEncoderPlugin* plugin = NULL;
		
	m_instance->m_audioEncoderPluginsIter =
		m_instance->m_audioEncoderPlugins.find(pluginName);
	
	if(m_instance->m_audioEncoderPluginsIter != m_instance->m_audioEncoderPlugins.end()) {
		plugin = new CAudioEncoderPlugin(m_instance->m_audioEncoderPlugins[pluginName]);
	}	

	fuppesThreadUnlockMutex(&m_instance->m_mutex);
	return plugin;
}



CPlugin::CPlugin(fuppesLibHandle handle, plugin_info* info)
{
	m_pluginInfo.plugin_type = info->plugin_type;
	strcpy(m_pluginInfo.plugin_name, info->plugin_name);
	strcpy(m_pluginInfo.plugin_author, info->plugin_author);
	m_pluginInfo.user_data = NULL;
	m_pluginInfo.log = &CPlugin::logCb;
	
	m_handle = handle;
}

CPlugin::~CPlugin()
{
  if(m_pluginInfo.plugin_type == PT_METADATA || 
		 m_pluginInfo.plugin_type == PT_DLNA) {
    FuppesCloseLibrary(m_handle);
  }
}

void CPlugin::logCb(int level, const char* file, int line, const char* format, ...)
{
	va_list args;	
  va_start(args, format);
	CSharedLog::LogArgs(level, file, line, format, args);	
  va_end(args);
}



/**
 *  CDlnaPlugin
 */

bool CDlnaPlugin::initPlugin()
{
	m_getImageProfile = NULL;

	m_getImageProfile = (dlnaGetImageProfile_t)FuppesGetProcAddress(m_handle, "fuppes_dlna_get_image_profile");
	if(m_getImageProfile == NULL) {
		return false;
	}
	
	return true;
}

bool CDlnaPlugin::getImageProfile(std::string ext, int width, int height, std::string* dlnaProfile, std::string* mimeType)
{
	char profile[256];
	char mime[256];
	if(m_getImageProfile(ext.c_str(), width, height, (char*)&profile, (char*)&mime) == 0) {
		*dlnaProfile = profile;
		*mimeType = mime;
		return true;
	}
	return false;																																		
}


/**
 *  CMetadataPlugin
 */

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


/**
 *  CTranscoderPlugin
 */

CTranscoderPlugin::CTranscoderPlugin(CTranscoderPlugin* plugin)
:CPlugin(plugin->m_handle, &plugin->m_pluginInfo) 
{
	m_transcodeVideo = plugin->m_transcodeVideo;
	m_transcodeImage = plugin->m_transcodeImage;
}

bool CTranscoderPlugin::initPlugin()
{
	m_transcodeVideo = NULL;
	m_transcodeImage = NULL;

	m_transcodeVideo = (transcoderTranscodeVideo_t)FuppesGetProcAddress(m_handle, "fuppes_transcoder_transcode");
	m_transcodeImage = (transcoderTranscodeImage_t)FuppesGetProcAddress(m_handle, "fuppes_transcoder_transcode_image");
	
	if((m_transcodeVideo == NULL) && 
		 (m_transcodeImage == NULL)) {
		return false;
	}	
	
	return true;
}


bool CTranscoderPlugin::Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile)
{  
	if((m_transcodeVideo == NULL) && 
		 (m_transcodeImage == NULL)) {
		return false;
	}	
	
  *p_psOutFile = CSharedConfig::Shared()->CreateTempFileName() + "." + pFileSettings->Extension(m_audioCodec, m_videoCodec);  

	if(m_transcodeVideo != NULL) {
		return (m_transcodeVideo(&m_pluginInfo, 
                      p_sInFile.c_str(), 
                      (*p_psOutFile).c_str(), 
                      pFileSettings->pTranscodingSettings->AudioCodec(m_audioCodec).c_str(), 
                      pFileSettings->pTranscodingSettings->VideoCodec(m_videoCodec).c_str(),
                      pFileSettings->pTranscodingSettings->VideoBitRate(),
                      pFileSettings->pTranscodingSettings->AudioBitRate(),
                      pFileSettings->pTranscodingSettings->AudioSampleRate(),                      
                      pFileSettings->pTranscodingSettings->FFmpegParams().c_str()) == 0);
	}
	else if(m_transcodeImage != NULL) {
		
		int less = 0;
		int greater = 0;
		if(pFileSettings->pImageSettings->Less()) {
			less = 1;
		}
		
		if(pFileSettings->pImageSettings->Greater()) {
			greater = 1;
		}
		
		return (m_transcodeImage(&m_pluginInfo, 
                      p_sInFile.c_str(), 
                      (*p_psOutFile).c_str(),
											pFileSettings->pImageSettings->Width(),
											pFileSettings->pImageSettings->Height(),
											less, greater) == 0);
	}
		
}


/**
 *  CAudioDecoderPlugin
 */

CAudioDecoderPlugin::CAudioDecoderPlugin(CAudioDecoderPlugin* plugin)
:CPlugin(plugin->m_handle, &plugin->m_pluginInfo) 
{
	//m_pluginInfo.user_data = malloc(1);
	
	m_fileOpen = plugin->m_fileOpen;
	m_setOutEndianess = plugin->m_setOutEndianess;
	m_totalSamples = plugin->m_totalSamples;
	m_decodeInterleaved = plugin->m_decodeInterleaved;
	m_fileClose = plugin->m_fileClose;	
}

bool CAudioDecoderPlugin::initPlugin()
{
	m_fileOpen = NULL;
	m_setOutEndianess = NULL;
	m_totalSamples = NULL;
	m_decodeInterleaved = NULL;
	m_fileClose = NULL;
	
	m_fileOpen = (audioDecoderFileOpen_t)FuppesGetProcAddress(m_handle, "fuppes_decoder_file_open");
	if(m_fileOpen == NULL) {
		cout << "error load symbol 'fuppes_decoder_file_open'" << endl;
		return false;
	}

	m_setOutEndianess = (audioDecoderSetOutEndianess_t)FuppesGetProcAddress(m_handle, "fuppes_decoder_set_out_endianess");
	if(m_setOutEndianess == NULL) {
		cout << "error load symbol 'fuppes_decoder_set_out_endianess'" << endl;
		return false;
	}
	
	m_totalSamples = (audioDecoderTotalSamples_t)FuppesGetProcAddress(m_handle, "fuppes_decoder_total_samples");
	if(m_totalSamples == NULL) {
		cout << "error load symbol 'fuppes_decoder_total_samples'" << endl;
		return false;
	}
	
	m_decodeInterleaved = (audioDecoderDecodeInterleaved_t)FuppesGetProcAddress(m_handle, "fuppes_decoder_decode_interleaved");
	if(m_decodeInterleaved == NULL) {
		cout << "error load symbol 'fuppes_decoder_decode_interleaved'" << endl;
		return false;
	}

	m_fileClose	= (audioDecoderFileClose_t)FuppesGetProcAddress(m_handle, "fuppes_decoder_file_close");
	if(m_fileClose == NULL) {
		cout << "error load symbol 'fuppes_decoder_file_close'" << endl;
		return false;
	}
	
	return true;
}

bool CAudioDecoderPlugin::openFile(std::string fileName, CAudioDetails* pAudioDetails)
{
	if(!m_fileOpen) {
		return false;
	}
	
	audio_settings_t settings;
	init_audio_settings(&settings);
	
	bool open = ((m_fileOpen(&m_pluginInfo, fileName.c_str(), &settings)) == 0);	
	if(open) {		
		pAudioDetails->nNumChannels		= settings.channels;
		pAudioDetails->nSampleRate		= settings.samplerate;
		pAudioDetails->nBitRate				= settings.bitrate;
		pAudioDetails->nNumPcmSamples = numPcmSamples();
		
		setOutEndianness(OutEndianess());
	}
	free_audio_settings(&settings);	

	return open;
}

void CAudioDecoderPlugin::setOutEndianness(ENDIANESS endianess) 
{
	if(!m_setOutEndianess)
		return;
	
	m_setOutEndianess(&m_pluginInfo, endianess);
}

int CAudioDecoderPlugin::numPcmSamples()
{
	if(!m_totalSamples)
		return 0;
	
	return m_totalSamples(&m_pluginInfo);
}

int CAudioDecoderPlugin::decodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead)
{
	if(!m_decodeInterleaved) {
		return -1;
	}
	
	return m_decodeInterleaved(&m_pluginInfo, p_PcmOut, p_nBufferSize, p_nBytesRead);
}

void CAudioDecoderPlugin::closeFile()
{
	if(m_fileClose) {
		m_fileClose(&m_pluginInfo);
	}
}


/**
 *  CAudioEncoderPlugin
 */

CAudioEncoderPlugin::CAudioEncoderPlugin(CAudioEncoderPlugin* plugin)
:CPlugin(plugin->m_handle, &plugin->m_pluginInfo) 
{
}

bool CAudioEncoderPlugin::initPlugin()
{ 
	
	return true;
}	
