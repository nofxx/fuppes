/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Plugin.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "../Common/Exception.h"
#include "../SharedLog.h"
#include "../SharedConfig.h"

#include "../HTTP/HTTPMessage.h"

#include "../ControlInterface/ControlInterface.h"

#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>

#include <iostream>
using namespace std;

CPluginMgr* CPluginMgr::m_instance = 0;

CPluginMgr::CPluginMgr()
{
	m_dlnaPlugin = NULL;
	m_presentationPlugin = NULL;
}

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
	
	//struct stat	Stat;
	string			fileName;
	string			ext;
	//size_t			pos;	
	CPlugin*		plugin = NULL;
	
	registerPlugin_t		regPlugin;
	unregisterPlugin_t	unregPlugin;
	
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
			pluginInfo.plugin_name[0] = '\0';
			pluginInfo.plugin_author[0] = '\0';
			pluginInfo.plugin_version[0] = '\0';
			pluginInfo.library_version[0] = '\0';
			
			fuppesLibHandle handle;
			
			//cout << "load: " << fileName << endl;
			
			handle = FuppesLoadLibrary(fileName);
			if(handle == NULL) {
				//cout << "error loading library: "  << fileName << endl;
				continue;
			}
			regPlugin = (registerPlugin_t)FuppesGetProcAddress(handle, "register_fuppes_plugin");	
			unregPlugin = (unregisterPlugin_t)FuppesGetProcAddress(handle, "unregister_fuppes_plugin");	
			
			if(regPlugin == NULL || unregPlugin == NULL) {
				//cout << "library: "  << fileName << " is no valid fuppes plugin" << endl;
				FuppesCloseLibrary(handle);
				continue;
			}

			regPlugin(&pluginInfo);

			switch(pluginInfo.plugin_type) {
				case PT_DLNA:
					plugin = new CDlnaPlugin(handle, &pluginInfo);
					if(plugin->initPlugin()) {
						m_instance->m_dlnaPlugin = (CDlnaPlugin*)plugin;
						CSharedLog::Log(L_EXT, __FILE__, __LINE__, "registered dlna profile plugin");
					}					
					break;
				case PT_PRESENTATION:
					plugin = new CPresentationPlugin(handle, &pluginInfo);
					if(plugin->initPlugin()) {
						m_instance->m_presentationPlugin = (CPresentationPlugin*)plugin;
						CSharedLog::Log(L_EXT, __FILE__, __LINE__, "registered presentation plugin");
					}
					break;					
				case PT_METADATA:
					plugin = new CMetadataPlugin(handle, &pluginInfo);
				  if(plugin->initPlugin()) {
						m_instance->m_metadataPlugins[ToLower(pluginInfo.plugin_name)] = (CMetadataPlugin*)plugin;
						CSharedLog::Log(L_EXT, __FILE__, __LINE__, "registered metadata plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
				case PT_AUDIO_DECODER:
					plugin = new CAudioDecoderPlugin(handle, &pluginInfo);
					if(plugin->initPlugin()) {
						m_instance->m_audioDecoderPlugins[ToLower(pluginInfo.plugin_name)] = (CAudioDecoderPlugin*)plugin;
						CSharedLog::Log(L_EXT, __FILE__, __LINE__, "registered audio decoder plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
				case PT_AUDIO_ENCODER:
					plugin = new CAudioEncoderPlugin(handle, &pluginInfo);
					if(plugin->initPlugin()) {
						m_instance->m_audioEncoderPlugins[ToLower(pluginInfo.plugin_name)] = (CAudioEncoderPlugin*)plugin;
						CSharedLog::Log(L_EXT, __FILE__, __LINE__, "registered audio encoder plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
				case PT_TRANSCODER:
				case PT_THREADED_TRANSCODER:
					plugin = new CTranscoderPlugin(handle, &pluginInfo);
          if(plugin->initPlugin()) {
						m_instance->m_transcoderPlugins[ToLower(pluginInfo.plugin_name)] = (CTranscoderPlugin*)plugin;
						CSharedLog::Log(L_EXT, __FILE__, __LINE__, "registered transcoder plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
				case PT_DATABASE_CONNECTION:
					plugin = new CDatabasePlugin(handle, &pluginInfo);
				  if(plugin->initPlugin()) {
						m_instance->m_databasePlugins[pluginInfo.plugin_name] = (CDatabasePlugin*)plugin;
						CSharedLog::Log(L_EXT, __FILE__, __LINE__, "registered database plugin \"%s\"", pluginInfo.plugin_name);
					}
					break;
	
				case PT_NONE:
					unregPlugin(&pluginInfo);
					FuppesCloseLibrary(handle);
					break;
					
			}
		}
		catch(fuppes::Exception ex) {
		}
		
	} // while

	closedir(dir);
}

CMetadataPlugin* CPluginMgr::metadataPlugin(std::string pluginName)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	CMetadataPlugin* plugin = NULL;
	
	pluginName = ToLower(pluginName);
	
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

	pluginName = ToLower(pluginName);	
	
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

	pluginName = ToLower(pluginName);
	
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
		
	pluginName = ToLower(pluginName);
	
	m_instance->m_audioEncoderPluginsIter =
		m_instance->m_audioEncoderPlugins.find(pluginName);
	
	if(m_instance->m_audioEncoderPluginsIter != m_instance->m_audioEncoderPlugins.end()) {
		plugin = new CAudioEncoderPlugin(m_instance->m_audioEncoderPlugins[pluginName]);
	}	

	fuppesThreadUnlockMutex(&m_instance->m_mutex);
	return plugin;
}

CDatabasePlugin* CPluginMgr::databasePlugin(const std::string pluginName)
{
	fuppesThreadLockMutex(&m_instance->m_mutex);
	CDatabasePlugin* plugin = NULL;
		
	//pluginName = ToLower(pluginName);
	
	m_instance->m_databasePluginsIter =
		m_instance->m_databasePlugins.find(pluginName);
	
	if(m_instance->m_databasePluginsIter != m_instance->m_databasePlugins.end()) {
		plugin = m_instance->m_databasePlugins[pluginName];
	}

	fuppesThreadUnlockMutex(&m_instance->m_mutex);
	return plugin;
}



std::string CPluginMgr::printInfo()
{
	stringstream result;
	CPlugin* plugin;
	
	result << "registered plugins" << endl;
	result << "database:" << endl;
	for(m_instance->m_databasePluginsIter = m_instance->m_databasePlugins.begin(); 
			m_instance->m_databasePluginsIter != m_instance->m_databasePlugins.end(); 
			m_instance->m_databasePluginsIter++) {
		plugin = m_instance->m_databasePluginsIter->second;
		result << "  \"" << plugin->name() << "\"\tlibrary version: " << plugin->libraryVersion() << endl <<
							"\t\t(author: " << plugin->author() << " version: " << plugin->pluginVersion() << ")" << endl;
	}
	result << endl;
 
	result << "misc:" << endl;
	if(m_instance->m_dlnaPlugin)
		result << "  " << m_instance->m_dlnaPlugin->name() << endl;
	if(m_instance->m_presentationPlugin)
		result << "  " << m_instance->m_presentationPlugin->name() << endl;
	result << endl;	
	
	result << "metadata:" << endl;
	for(m_instance->m_metadataPluginsIter = m_instance->m_metadataPlugins.begin(); 
			m_instance->m_metadataPluginsIter != m_instance->m_metadataPlugins.end(); 
			m_instance->m_metadataPluginsIter++) {
		plugin = m_instance->m_metadataPluginsIter->second;
		result << "  " << plugin->name() << " (" << plugin->author() << ")" << endl;
	}
	result << endl;
	
	result << "transcoder:" << endl;
	for(m_instance->m_transcoderPluginsIter = m_instance->m_transcoderPlugins.begin(); 
			m_instance->m_transcoderPluginsIter != m_instance->m_transcoderPlugins.end(); 
			m_instance->m_transcoderPluginsIter++) {
		plugin = m_instance->m_transcoderPluginsIter->second;
		result << "  " << plugin->name() << " (" << plugin->author() << ")" << endl;
	}
	result << endl;
	
	result << "audio decoder:" << endl;
	for(m_instance->m_audioDecoderPluginsIter = m_instance->m_audioDecoderPlugins.begin(); 
			m_instance->m_audioDecoderPluginsIter != m_instance->m_audioDecoderPlugins.end(); 
			m_instance->m_audioDecoderPluginsIter++) {
		plugin = m_instance->m_audioDecoderPluginsIter->second;
		result << "  " << plugin->name() << " (" << plugin->author() << ")" << endl;
	}
	result << endl;

	result << "audio encoder:" << endl;
	for(m_instance->m_audioEncoderPluginsIter = m_instance->m_audioEncoderPlugins.begin(); 
			m_instance->m_audioEncoderPluginsIter != m_instance->m_audioEncoderPlugins.end(); 
			m_instance->m_audioEncoderPluginsIter++) {
		plugin = m_instance->m_audioEncoderPluginsIter->second;
		result << "  " << plugin->name() << " (" << plugin->author() << ")" << endl;
	}
	result << endl;
	
	return result.str();
}



CPlugin::CPlugin(fuppesLibHandle handle, plugin_info* info)
{
	m_pluginInfo.plugin_type = info->plugin_type;
	strcpy(m_pluginInfo.plugin_name, info->plugin_name);
	strcpy(m_pluginInfo.plugin_author, info->plugin_author);
	
	strcpy(m_pluginInfo.plugin_version, info->plugin_version);
	strcpy(m_pluginInfo.library_version, info->library_version);
	
	m_pluginInfo.user_data = NULL;
	m_pluginInfo.log = &CPlugin::logCb;
	m_pluginInfo.ctrl = NULL;
	
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
	m_readImage = NULL;
	m_fileClose = NULL;

	m_fileOpen = (metadataFileOpen_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_file_open");
	if(m_fileOpen == NULL) {
		return false;
	}
	
	m_readData	= (metadataRead_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_read");
	if(m_readData == NULL) {
		return false;
	}

	m_readImage	= (metadataReadImage_t)FuppesGetProcAddress(m_handle, "fuppes_metadata_read_image");	

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

bool CMetadataPlugin::readImage(char** mimeType, unsigned char** buffer, size_t* size)
{
	if(m_readImage == NULL) {
		return false;
	}
	return (m_readImage(&m_pluginInfo, mimeType, buffer, size) == 0);
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
	m_transcodeImageMem = plugin->m_transcodeImageMem;
	m_transcodeImageFile = plugin->m_transcodeImageFile;
	m_transcodeStop = plugin->m_transcodeStop;
}

bool CTranscoderPlugin::initPlugin()
{
	m_transcodeVideo = NULL;
	m_transcodeImageMem = NULL;
	m_transcodeImageFile = NULL;
	m_transcodeStop = NULL;
	
	m_transcodeVideo = (transcoderTranscodeVideo_t)FuppesGetProcAddress(m_handle, "fuppes_transcoder_transcode");
	m_transcodeImageMem = (transcoderTranscodeImageMem_t)FuppesGetProcAddress(m_handle, "fuppes_transcoder_transcode_image_mem");
	m_transcodeImageFile = (transcoderTranscodeImageFile_t)FuppesGetProcAddress(m_handle, "fuppes_transcoder_transcode_image_file");
	m_transcodeStop = (transcoderStop_t)FuppesGetProcAddress(m_handle, "fuppes_transcoder_stop");
	
	if((m_transcodeVideo == NULL) && 
		 ((m_transcodeImageFile == NULL) || (m_transcodeImageMem == NULL))) {
		return false;
	}	
	
	return true;
}


bool CTranscoderPlugin::TranscodeFile(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile)
{  
	if((m_transcodeVideo == NULL) && 
		 (m_transcodeImageFile == NULL)) {
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
	else if(m_transcodeImageFile != NULL) {
		
		int less = 0;
		int greater = 0;
		if(pFileSettings->pImageSettings->Less()) {
			less = 1;
		}
		
		if(pFileSettings->pImageSettings->Greater()) {
			greater = 1;
		}
		
		return (m_transcodeImageFile(&m_pluginInfo, 
                      p_sInFile.c_str(), 
                      (*p_psOutFile).c_str(),
											pFileSettings->pImageSettings->Width(),
											pFileSettings->pImageSettings->Height(),
											less, greater) == 0);
	}

	return false;
}


bool CTranscoderPlugin::TranscodeMem(CFileSettings* pFileSettings, 
															const unsigned char** inBuffer, 
															size_t inSize, 
															unsigned char** outBuffer, 
															size_t* outSize)
{
	if(m_transcodeImageMem == NULL) {
		return false;
	}

	int less = 0;
	int greater = 0;
	if(pFileSettings->pImageSettings->Less()) {
		less = 1;
	}
	
	if(pFileSettings->pImageSettings->Greater()) {
		greater = 1;
	}
	
	return (m_transcodeImageMem(&m_pluginInfo, 
															inBuffer, inSize,
															outBuffer, outSize,
															pFileSettings->pImageSettings->Width(),
															pFileSettings->pImageSettings->Height(),
															less, greater) == 0);

}

void CTranscoderPlugin::stop()
{  
	if(m_transcodeVideo == NULL || m_transcodeStop == NULL) {
		return;
	}
	
	m_transcodeStop(&m_pluginInfo);
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
	m_getOutBufferSize = NULL;
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

	// optional
	m_getOutBufferSize = (audioDecoderGetOutBufferSize_t)FuppesGetProcAddress(m_handle, "fuppes_decoder_get_out_buffer_size");
	/*if(m_getOutBufferSize == NULL) {
		cout << "error load symbol 'fuppes_decoder_get_out_buffer_size'" << endl;
	}*/
		
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

int CAudioDecoderPlugin::getOutBufferSize()
{
	if(!m_getOutBufferSize) {
		return 0;
	}

	return m_getOutBufferSize(&m_pluginInfo);
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

int CAudioDecoderPlugin::decodeInterleaved(char* pcm, int bufferSize, int* bytesConsumed)
{
	if(!m_decodeInterleaved) {
		return -1;
	}
	
	return m_decodeInterleaved(&m_pluginInfo,pcm, bufferSize, bytesConsumed);
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
	m_setAudioSettings = plugin->m_setAudioSettings;
	m_encodeInterleaved = plugin->m_encodeInterleaved;
	m_getBuffer = plugin->m_getBuffer;
	m_guessContentLength = plugin->m_guessContentLength;
}

bool CAudioEncoderPlugin::initPlugin()
{	
	m_setAudioSettings = NULL;
	m_encodeInterleaved = NULL;
	m_getBuffer = NULL;
	m_guessContentLength = NULL;
	
	m_setAudioSettings = (audioEncoderSetAudioSettings_t)FuppesGetProcAddress(m_handle, "fuppes_encoder_set_audio_settings");
	if(m_setAudioSettings == NULL) {
		cout << "error load symbol 'fuppes_encoder_set_audio_settings'" << endl;
		return false;
	}

	m_encodeInterleaved = (audioEncoderEncodeInterleaved_t)FuppesGetProcAddress(m_handle, "fuppes_encoder_encode_interleaved");
	if(m_encodeInterleaved == NULL) {
		cout << "error load symbol 'fuppes_encoder_encode_interleaved'" << endl;
		return false;
	}	

	
	m_getBuffer = (audioEncoderGetBuffer_t)FuppesGetProcAddress(m_handle, "fuppes_encoder_get_buffer");
	if(m_getBuffer == NULL) {
		cout << "error load symbol 'fuppes_encoder_get_buffer'" << endl;
		return false;
	}

	m_guessContentLength = (audioEncoderGuessContentLength_t)FuppesGetProcAddress(m_handle, "fuppes_encoder_guess_content_length");
	if(m_guessContentLength == NULL) {
		cout << "error load symbol 'fuppes_encoder_guess_content_length'" << endl;
	}

	
	
	return true;
}	

void CAudioEncoderPlugin::SetAudioDetails(CAudioDetails* audioDetails)
{
	if(!m_setAudioSettings)
		return;
	
	audio_settings_t settings;
	init_audio_settings(&settings);

	settings.channels			= audioDetails->nNumChannels;
	settings.samplerate		= audioDetails->nSampleRate;
	settings.bitrate			= audioDetails->nBitRate;
	settings.num_samples	= audioDetails->nNumPcmSamples;

	m_setAudioSettings(&m_pluginInfo, &settings);
	free_audio_settings(&settings);
}

int CAudioEncoderPlugin::EncodeInterleaved(short int pcm[], int numSamples, int numBytes)
{
	if(!m_encodeInterleaved)
		return -1;


	return m_encodeInterleaved(&m_pluginInfo, (char*)pcm, numSamples, numBytes);	
}

int CAudioEncoderPlugin::Flush()
{
	return -1;
}

unsigned char* CAudioEncoderPlugin::GetEncodedBuffer()
{
	if(!m_getBuffer)
		return NULL;

	return (unsigned char*)m_getBuffer(&m_pluginInfo);
}

unsigned int CAudioEncoderPlugin::GuessContentLength(unsigned int numPcmSamples)
{
	if(!m_guessContentLength)
		return 0;

	return m_guessContentLength(&m_pluginInfo, numPcmSamples);
}


/**
 *  CPresentationPlugin
 */

CPresentationPlugin::CPresentationPlugin(fuppesLibHandle handle, plugin_info* info)
:CPlugin(handle, info)
{
	m_pluginInfo.ctrl = &CPresentationPlugin::ctrlAction;
}

int CPresentationPlugin::ctrlAction(const char* action, arg_list_t* args, arg_list_t* result)
{
//	cout << "ctrlAction: " << action << endl;

	stringList ctrlArgs;
	stringList ctrlResult;
	CControlInterface ctrl;

	while(args) {
		ctrlArgs[string(args->key)] = args->value;
		args = (arg_list_t*)args->next;
	}
	
	int ret = ctrl.action(action, &ctrlArgs, &ctrlResult);	
	if(ret != 0) {
		//cout << "ctrlAction: " << action << " error: " << ret << endl;
		return ret;
	}
	
	stringListIter iter = ctrlResult.begin();	
	arg_list_t* resultArgs = result;	
	while(iter != ctrlResult.end()) {

		set_value(&resultArgs->key, iter->first.c_str());
		set_value(&resultArgs->value, iter->second.c_str());
		
		iter++;
		if(iter != ctrlResult.end()) {
			resultArgs = arg_list_append(result);
		}
	}
	
	//cout << "ctrlAction: " << action << " DONE" << endl;
	return 0;	
}

bool CPresentationPlugin::initPlugin()
{
	m_handleRequest = NULL;
	
	m_handleRequest = (presentationHandleRequest_t)FuppesGetProcAddress(m_handle, "fuppes_presentation_handle_request");
	if(m_handleRequest == NULL) {
		cout << "error load symbol 'fuppes_presentation_handle_request'" << endl;
		return false;
	}
	
	return true;
}

bool CPresentationPlugin::handleRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
	if(!m_handleRequest)
		return false;
	
	
	pResponse->SetVersion(pRequest->GetVersion());
	
	
	/*char* url, char* get[], int getc, char* post[], int postc,
		int* error, char** mime_type, char** result, int* length */
	
	string url = pRequest->GetRequest();  
	
	int		error = 0;
	char* mimeType = (char*)malloc(sizeof(char));
	char* result = (char*)malloc(sizeof(char));
	int		length = 0;
	
	int ret = m_handleRequest(&m_pluginInfo, url.c_str(), NULL, NULL,
														&error, &mimeType, &result, &length);
														
	
	if(error == 200) {    
    pResponse->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);    
    pResponse->SetContentType(mimeType); //"text/html; charset=\"utf-8\""); // HTTP_CONTENT_TYPE_TEXT_HTML
    pResponse->SetBinContent(result, length);
	}
	
	free(result);
	free(mimeType);
	
	return (ret == 0);
}






/**
 *  CDatabasePlugin
 */

CDatabasePlugin::CDatabasePlugin(fuppesLibHandle handle, plugin_info* info)
:CPlugin(handle, info)
{
}

bool CDatabasePlugin::initPlugin()
{
	m_createConnection = NULL;
	
	m_createConnection = (createConnection_t)FuppesGetProcAddress(m_handle, "fuppes_plugin_create_db_connection");
	if(m_createConnection == NULL) {
		cout << "error load symbol 'fuppes_plugin_create_db_connection'" << endl;
		return false;
	}
	
	return true;
}

CDatabaseConnection* CDatabasePlugin::createConnection()
{
	if(!m_createConnection)
		return false;
	
	return m_createConnection(&m_pluginInfo);
}
