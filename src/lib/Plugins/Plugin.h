/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Plugin.h
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

#ifndef _PLUGIN_H
#define _PLUGIN_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <map>

#include "../Common/Common.h"
#include "../Transcoding/WrapperBase.h"

#include "../../../include/fuppes_plugin.h"
#include "../../../include/fuppes_plugin_types.h"
#include "../../../include/fuppes_db_connection_plugin.h"

typedef void (*registerPlugin_t)(plugin_info* info);
typedef void (*unregisterPlugin_t)(plugin_info* info);

class CDlnaPlugin;
class CMetadataPlugin;
class CTranscoderPlugin;
class CAudioDecoderPlugin;
class CAudioEncoderPlugin;
class CPresentationPlugin;
class CDatabasePlugin;


class CHTTPMessage;

class CPluginMgr
{
	public:
		static void init(std::string pluginDir);
	
		// returns a reference to a single plugin instance
		static CDlnaPlugin*						dlnaPlugin() { return m_instance->m_dlnaPlugin; }
		static CMetadataPlugin*				metadataPlugin(std::string pluginName);
		static CPresentationPlugin*		presentationPlugin() { return m_instance->m_presentationPlugin; }
		static CDatabasePlugin*				databasePlugin(const std::string pluginName);
		
		// returns a new instance of the plugin that must be deleted by caller
		static CTranscoderBase*				transcoderPlugin(std::string pluginName);
		static CAudioDecoderPlugin*		audioDecoderPlugin(std::string pluginName);
		static CAudioEncoderPlugin*		audioEncoderPlugin(std::string pluginName);

		static std::string						printInfo();
		
	private:
		CPluginMgr();
		static CPluginMgr* m_instance;
		fuppesThreadMutex	 m_mutex;
		
		std::map<std::string, CMetadataPlugin*> m_metadataPlugins;
		std::map<std::string, CMetadataPlugin*>::iterator m_metadataPluginsIter;
		
		std::map<std::string, CTranscoderPlugin*> m_transcoderPlugins;
		std::map<std::string, CTranscoderPlugin*>::iterator m_transcoderPluginsIter;
			
		std::map<std::string, CAudioDecoderPlugin*> m_audioDecoderPlugins;
		std::map<std::string, CAudioDecoderPlugin*>::iterator m_audioDecoderPluginsIter;

		std::map<std::string, CAudioEncoderPlugin*> m_audioEncoderPlugins;
		std::map<std::string, CAudioEncoderPlugin*>::iterator m_audioEncoderPluginsIter;
		
		CDlnaPlugin*					m_dlnaPlugin;		
		CPresentationPlugin*	m_presentationPlugin;
		
		std::map<std::string, CDatabasePlugin*> m_databasePlugins;
		std::map<std::string, CDatabasePlugin*>::iterator m_databasePluginsIter;
};

// typedef void Log(int p_nLogLevel, const std::string p_sFileName, int p_nLineNumber, const char* p_szFormat, ...);
class CPlugin
{
	protected:
		CPlugin(fuppesLibHandle handle, plugin_info* info);
	
	public:
		virtual ~CPlugin();
		PLUGIN_TYPE		pluginType() { return m_pluginInfo.plugin_type; }
		virtual bool 	initPlugin() = 0;
		
		std::string		name() { return std::string(m_pluginInfo.plugin_name); }
		std::string		author() { return std::string(m_pluginInfo.plugin_author); }

		static void		logCb(int level, const char* file, int line, const char* format, ...);
		
	protected:
		fuppesLibHandle		m_handle;
		plugin_info				m_pluginInfo;
};


typedef int (*dlnaGetImageProfile_t)(const char* ext, int width, int height, char* dlnaProfile, char* mimeType);
class CDlnaPlugin: public CPlugin
{
	public:
		CDlnaPlugin(fuppesLibHandle handle, plugin_info* info): 
			CPlugin(handle, info) {}
	
		bool initPlugin();
		bool getImageProfile(std::string ext, int width, int height, std::string* dlnaProfile, std::string* mimeType);
			
	private:
		dlnaGetImageProfile_t			m_getImageProfile;
};


typedef int		(*metadataFileOpen_t)(plugin_info* plugin, const char* fileName);
typedef int		(*metadataRead_t)(plugin_info* plugin, metadata_t* audio);
typedef int		(*metadataReadImage_t)(plugin_info* plugin, char** mimeType, unsigned char** buffer, size_t* size);
typedef void	(*metadataFileClose_t)(plugin_info* plugin);

class CMetadataPlugin: public CPlugin
{
	public:
		CMetadataPlugin(fuppesLibHandle handle, plugin_info* info): 
			CPlugin(handle, info) {}
	
		bool initPlugin();
	
		bool openFile(std::string fileName);
		bool readData(metadata_t* metadata);
		bool readImage(char** mimeType, unsigned char** buffer, size_t* size);
		void closeFile();
	
	private:
		metadataFileOpen_t				m_fileOpen;
		metadataRead_t						m_readData;
		metadataReadImage_t				m_readImage;
		metadataFileClose_t				m_fileClose;
};


typedef int		(*transcoderTranscodeVideo_t)(plugin_info* plugin,
                                      const char* inputFile,
                                      const char* outputFile,
                                      const char* audioCodec, 
                                      const char* videoCodec,
                                      int videoBitrate,
                                      int audioBitrate,
                                      int audioSamplerate,
                                      const char* ffmpegParams);

typedef int		(*transcoderTranscodeImageFile_t)(plugin_info* plugin,
                                      const char* inputFile,
                                      const char* outputFile,
                                      int width, int height,
																			int less,	int greater);

typedef int	 (*transcoderTranscodeImageMem_t)(plugin_info* plugin,
	                                const unsigned char** inBuffer,
																	size_t inSize,
		                              unsigned char** outBuffer,
																	size_t* outSize,
																	int width, int height,
																	int less,	int greater);

class CTranscoderPlugin: public CPlugin, CTranscoderBase
{
	public:
		CTranscoderPlugin(fuppesLibHandle handle, plugin_info* info): 
			CPlugin(handle, info) {}
			
		CTranscoderPlugin(CTranscoderPlugin* plugin);
  
		// from CPlugin
		bool 	initPlugin();		
			
		// from CTranscoderBase
    bool Init(std::string audioCodec, std::string videoCodec) 
      {
        m_audioCodec = audioCodec;
        m_videoCodec = videoCodec;
        return true;
      }

		bool TranscodeMem(CFileSettings* pFileSettings, 
															const unsigned char** inBuffer, 
															size_t inSize, 
															unsigned char** outBuffer, 
															size_t* outSize);
		bool TranscodeFile(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile);
		bool Threaded() { return true; }
			
		private:
			transcoderTranscodeVideo_t			m_transcodeVideo;
			transcoderTranscodeImageFile_t	m_transcodeImageFile;
			transcoderTranscodeImageMem_t		m_transcodeImageMem;
      std::string             m_audioCodec;
      std::string             m_videoCodec;
};



typedef int		(*audioDecoderFileOpen_t)(plugin_info* plugin, const char* fileName, audio_settings_t* settings);
typedef int		(*audioDecoderSetOutEndianess_t)(plugin_info* plugin, ENDIANESS endianess);
typedef int		(*audioDecoderGetOutBufferSize_t)(plugin_info* plugin);
typedef int		(*audioDecoderTotalSamples_t)(plugin_info* plugin);
typedef int		(*audioDecoderDecodeInterleaved_t)(plugin_info* plugin, char* pcmOut, int bufferSize, int* bytesRead);
typedef void	(*audioDecoderFileClose_t)(plugin_info* plugin);

class CAudioDecoderPlugin: public CPlugin, public CAudioDecoderBase
{
  
  public:
    CAudioDecoderPlugin(fuppesLibHandle handle, plugin_info* info): 		
			CPlugin(handle, info) {}
		
		CAudioDecoderPlugin(CAudioDecoderPlugin* plugin);
    
		// from CPlugin
		bool 	initPlugin();
		
    bool  openFile(std::string fileName, CAudioDetails* pAudioDetails);  
		void	setOutEndianness(ENDIANESS endianess);
		int		getOutBufferSize();
    int   numPcmSamples();  
    int   decodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead);  
    void  closeFile();
		
		// CAudioDecoderBase
		bool LoadLib() { return true; }
    bool OpenFile(std::string p_sFileName, CAudioDetails* pAudioDetails) {
			return openFile(p_sFileName, pAudioDetails);
		}
		
    void CloseFile() {
			closeFile();
		}

    long DecodeInterleaved(char* p_PcmOut, int p_nBufferSize, int* p_nBytesRead) {
			return decodeInterleaved(p_PcmOut, p_nBufferSize, p_nBytesRead);
		}
		
    unsigned int NumPcmSamples() {
			return numPcmSamples();
		}
		
	private:
		audioDecoderFileOpen_t						m_fileOpen;
		audioDecoderSetOutEndianess_t			m_setOutEndianess;
		audioDecoderGetOutBufferSize_t		m_getOutBufferSize;
		audioDecoderTotalSamples_t				m_totalSamples;
		audioDecoderDecodeInterleaved_t		m_decodeInterleaved;
		audioDecoderFileClose_t						m_fileClose;
};

class CAudioEncoderPlugin: public CPlugin, public CAudioEncoderBase
{	
	public:
		CAudioEncoderPlugin(fuppesLibHandle handle, plugin_info* info): 		
			CPlugin(handle, info) {}
		CAudioEncoderPlugin(CAudioEncoderPlugin* plugin);
	
		// from CPlugin
		bool 	initPlugin();	
	
		// from CAudioEncoderBase
		bool LoadLib() { return true; }
    void SetAudioDetails(CAudioDetails* /*pAudioDetails*/) { }
    void SetTranscodingSettings(CTranscodingSettings* /*pTranscodingSettings*/) { }
		void Init() { }    
    int  EncodeInterleaved(short int /*p_PcmIn*/[], int /*p_nNumSamples*/, int /*p_nBytesRead*/) {
			return -1;
		}
		int  Flush() {
			return -1;
		}
		unsigned char* GetEncodedBuffer() {
			return NULL;
		}
		
    unsigned int GuessContentLength(unsigned int /*p_nNumPcmSamples*/) {
			return 0;
		}
		
};



typedef int (*presentationHandleRequest_t)(plugin_info* plugin, 
																					 const char* url, arg_list_t* get, arg_list_t* post,
																					 int* error, char** mime_type, char** result, int* length);
class CPresentationPlugin: public CPlugin
{
	public:
		CPresentationPlugin(fuppesLibHandle handle, plugin_info* info);
		
		// from CPlugin
		bool 	initPlugin();
		
		bool	handleRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse);		

		static int ctrlAction(const char* action, arg_list_t* args, arg_list_t* result);
		
	private:
		presentationHandleRequest_t	 m_handleRequest;

};


typedef CDatabaseConnection* (*createConnection_t)(plugin_info* plugin);
class CDatabasePlugin: public CPlugin
{
	public:
		CDatabasePlugin(fuppesLibHandle handle, plugin_info* info);
		
		// from CPlugin
		bool 	initPlugin();
		
		CDatabaseConnection*	createConnection();		
	private:
		createConnection_t  m_createConnection;	
};


#endif // _PLUGIN_H
