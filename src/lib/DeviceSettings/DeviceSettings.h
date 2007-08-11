/***************************************************************************
 *            DeviceSettings.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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

#ifndef _DEVICESETTINGS_H
#define _DEVICESETTINGS_H

#include <string>
#include <list>
#include <map>

#include "../ContentDirectory/ContentDatabase.h"
#include "../Transcoding/WrapperBase.h"

struct CImageSettings {
  CImageSettings();
  CImageSettings(CImageSettings* pImageSettings);
  
  std::string  sExt;
  std::string  sMimeType;  
  
  // dcraw
  bool    bDcraw;
  std::string  sDcrawParams;

  // ImageMagick
  bool bGreater;
	bool bLower;
	int  nWidth;
	int  nHeight;
	enum { resize, scale } nResizeMethod; // resize = better quality (lower) | scale = lower quality (faster)

  private:
    bool bEnabled;
};

typedef struct {
  bool bShowChildCountInTitle;
  int  nMaxFileNameLength;
} DisplaySettings_t;

typedef enum TRANSCODING_HTTP_RESPONSE {
  RESPONSE_STREAM,
  RESPONSE_CHUNKED
} TRANSCODING_HTTP_RESPONSE;

struct CTranscodingSettings {
  
  CTranscodingSettings();  
  CTranscodingSettings(CTranscodingSettings* pTranscodingSettings);
  
  std::string   sExt;
  std::string   sMimeType;
  std::string   sDLNA;
  
  std::string   sDecoder;     // vorbis | flac | mpc
  std::string   sEncoder;     // lame | twolame | pcm | wav
  std::string   sTranscoder;  // ffmpeg
  
  std::string   sOutParams;
  
  std::string MimeType() { return sMimeType; }
  std::string DLNA() { return sDLNA; }
  bool Enabled() { return bEnabled; }
  
  unsigned int BitRate() { return nBitRate; }
  unsigned int SampleRate() { return nSampleRate; }
  
  std::string  Extension() { return sExt; }
  TRANSCODING_HTTP_RESPONSE   TranscodingHTTPResponse() { return nTranscodingResponse; }
  
  int ReleaseDelay() { return nReleaseDelay; }
  
  TRANSCODING_HTTP_RESPONSE   nTranscodingResponse;
  TRANSCODING_TYPE            nTranscodingType;
  int                nReleaseDelay;
  
    unsigned int  nBitRate;
    unsigned int  nSampleRate;
  
  private:
    bool          bEnabled;
};

struct CFileSettings {
  
  CFileSettings();
  CFileSettings(CFileSettings* pFileSettings);
  
  std::string   MimeType();
  std::string   DLNA();
  
  unsigned int  TargetSampleRate();
  unsigned int  TargetBitRate();
  
  std::string   Extension(std::string p_sExt);
  
  TRANSCODING_HTTP_RESPONSE   TranscodingHTTPResponse();
  
  //std::string   sExt;
  OBJECT_TYPE   nType;
  std::string   sMimeType;
  std::string   sDLNA;
  
  bool Enabled() { return bEnabled; }
  void Enabled(bool p_bEnabled) { bEnabled = p_bEnabled; }
  
  CTranscodingSettings* pTranscodingSettings;
  CImageSettings*       pImageSettings;
  
  OBJECT_TYPE   ObjectType() { return nType; }
  std::string   ObjectTypeAsStr();
  
  int ReleaseDelay();
  
  private:
    bool  bEnabled;
};

typedef std::map<std::string, CFileSettings*>::iterator FileSettingsIterator_t;

class CDeviceSettings
{
  public:
	  CDeviceSettings(std::string p_sDeviceName);
    CDeviceSettings(std::string p_sDeviceName, CDeviceSettings* pSettings);
		
		bool HasUserAgent(std::string p_sUserAgent);
		std::list<std::string> m_slUserAgents;
		bool HasIP(std::string p_sIPAddress);
		std::list<std::string> m_slIPAddresses;
		
    /** show playlist as container or file */
		bool m_bShowPlaylistAsContainer;
    /** XBox 360 fixes */
		bool m_bXBox360Support;
    /** DLNA */
    bool m_bDLNAEnabled;
  
    OBJECT_TYPE   ObjectType(std::string p_sExt);
    std::string   ObjectTypeAsStr(std::string p_sExt);
    bool          DoTranscode(std::string p_sExt);
    std::string   MimeType(std::string p_sExt);
    std::string   DLNA(std::string p_sExt);
  
    unsigned int  TargetSampleRate(std::string p_sExt);
    unsigned int  TargetBitRate(std::string p_sExt);
    
    bool          Exists(std::string p_sExt);
    std::string   Extension(std::string p_sExt);
    TRANSCODING_HTTP_RESPONSE TranscodingHTTPResponse(std::string p_sExt);
  
    int ReleaseDelay(std::string p_sExt);
    int nDefaultReleaseDelay;
		//ImageSettings_t   m_ImageSettings;
    
    DisplaySettings_t* DisplaySettings() { return &m_DisplaySettings; }

	  std::string m_sDeviceName;
    std::string m_sVirtualFolderDevice;

    
  
    CFileSettings* FileSettings(std::string p_sExt);
    void AddExt(CFileSettings* pFileSettings, std::string p_sExt);
  
    std::map<std::string, CFileSettings*> m_FileSettings;
    std::map<std::string, CFileSettings*>::iterator m_FileSettingsIterator;
  
  private:
    DisplaySettings_t m_DisplaySettings;
};

#endif // _DEVICESETTINGS_H
