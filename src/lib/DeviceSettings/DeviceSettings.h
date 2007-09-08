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

struct CImageSettings {
  
  friend class CConfigFile;
  
  CImageSettings();
  CImageSettings(CImageSettings* pImageSettings);
  
  std::string  sExt;
  std::string  sMimeType;  
  
  // dcraw
  bool    bDcraw;
  std::string  sDcrawParams;

  std::string Extension() { return sExt; }
  std::string MimeType() { return sMimeType; }
  
  bool Enabled() { return bEnabled; }
  
  // ImageMagick
  bool bGreater;
	bool bLess;
	int  nWidth;
	int  nHeight;
	enum { resize, scale } nResizeMethod; // resize = better quality (lower) | scale = lower quality (faster)

  bool Greater() { return bGreater; }
  bool Less() { return bLess; }
  int  Width() { return nWidth; }
  int  Height() { return nHeight; }
  
  private:
    bool bEnabled;
};

typedef struct {
  bool bShowChildCountInTitle;
  int  nMaxFileNameLength;
} DisplaySettings_t;

typedef enum TRANSCODING_TYPE {
  TT_NONE,
  TT_THREADED_DECODER_ENCODER,
  TT_TRANSCODER,
  TT_THREADED_TRANSCODER  
} TRANSCODING_TYPE;

typedef enum TRANSCODER_TYPE {
  TTYP_NONE,
  TTYP_IMAGE_MAGICK,
  TTYP_FFMPEG
} TRANSCODER_TYPE;

typedef enum ENCODER_TYPE {
  ET_NONE,
  ET_LAME,
  ET_TWOLAME,
  ET_WAV,
  ET_PCM
} ENCODER_TYPE;

typedef enum DECODER_TYPE {
  DT_NONE,
  DT_OGG_VORBIS,
  DT_FLAC,
  DT_MUSEPACK,
  DT_FAAD
} DECODER_TYPE;

typedef enum TRANSCODING_HTTP_RESPONSE {
  RESPONSE_STREAM,
  RESPONSE_CHUNKED
} TRANSCODING_HTTP_RESPONSE;

struct CFFmpegSettings {
  
  private:
    std::string   sVcodec;
    std::string   sAcodec;
    int           nVBitRate;
    int           nVFrameRate;
    int           nABitRate;
    int           nASampleRate;
};

struct CTranscodingSettings {
  
    friend class CConfigFile;
  
    CTranscodingSettings();  
    CTranscodingSettings(CTranscodingSettings* pTranscodingSettings);
  
    std::string   sExt;
    std::string   sMimeType;
    std::string   sDLNA;
  
    /*std::string   sDecoder;     // vorbis | flac | mpc
    std::string   sEncoder;     // lame | twolame | pcm | wav
    std::string   sTranscoder;  // ffmpeg*/
  
    std::string   sOutParams;
  
    std::string MimeType() { return sMimeType; }
    std::string DLNA() { return sDLNA; }
    bool Enabled() { return bEnabled; }
  
    unsigned int BitRate() { return nBitRate; }
    unsigned int SampleRate() { return nSampleRate; }
  
    int LameQuality() { return nLameQuality; }
  
    std::string  Extension() { return sExt; }
    TRANSCODING_HTTP_RESPONSE   TranscodingHTTPResponse() { return nTranscodingResponse; }
  
    int ReleaseDelay() { return nReleaseDelay; }
  
    std::list<CFFmpegSettings*>   pFFmpegSettings;
  
    TRANSCODER_TYPE TranscoderType() { return nTranscoderType; }
    DECODER_TYPE    DecoderType() { return nDecoderType; }
    ENCODER_TYPE    EncoderType() { return nEncoderType; }
  
  private:
    bool          bEnabled;
  
    TRANSCODING_HTTP_RESPONSE   nTranscodingResponse;
    TRANSCODING_TYPE            nTranscodingType;
    TRANSCODER_TYPE             nTranscoderType;
    DECODER_TYPE                nDecoderType;
    ENCODER_TYPE                nEncoderType;
    int                         nReleaseDelay;
  
    unsigned int  nBitRate;
    unsigned int  nSampleRate;
  
    int           nLameQuality;
  
    std::string     sACodecCondition;
    std::string     sVCodecCondition;
};

struct CFileSettings {
  
  friend class CConfigFile;
  
  CFileSettings();
  CFileSettings(CFileSettings* pFileSettings);
  
  std::string   MimeType();
  std::string   DLNA();
  
  unsigned int  TargetSampleRate();
  unsigned int  TargetBitRate();
  
  std::string   Extension();
  
  TRANSCODING_HTTP_RESPONSE   TranscodingHTTPResponse();
  
  std::string   sExt;
  OBJECT_TYPE   nType;
  std::string   sMimeType;
  std::string   sDLNA;
  
  bool Enabled() { return bEnabled; }  
  bool ExtractMetadata() { return bExtractMetadata; }
  
  CTranscodingSettings* pTranscodingSettings;
  CImageSettings*       pImageSettings;
  
  OBJECT_TYPE   ObjectType() { return nType; }
  std::string   ObjectTypeAsStr();
  
  int ReleaseDelay();
  
  private:
    bool  bEnabled;
    bool  bExtractMetadata;
};

typedef std::map<std::string, CFileSettings*>::iterator FileSettingsIterator_t;

class CDeviceSettings
{
  friend class CConfigFile;
  friend class CDeviceIdentificationMgr;
  
  public:
	  CDeviceSettings(std::string p_sDeviceName);
    CDeviceSettings(std::string p_sDeviceName, CDeviceSettings* pSettings);
		
		bool HasUserAgent(std::string p_sUserAgent);
    bool HasIP(std::string p_sIPAddress);	
    /*std::list<std::string> m_slUserAgents;
		std::list<std::string> m_slIPAddresses;*/

    OBJECT_TYPE       ObjectType(std::string p_sExt);
    std::string       ObjectTypeAsStr(std::string p_sExt);

    bool              DoTranscode(std::string p_sExt, std::string p_sACodec = "", std::string p_sVCodec = "");
    TRANSCODING_TYPE  GetTranscodingType(std::string p_sExt);
    TRANSCODER_TYPE   GetTranscoderType(std::string p_sExt);
    DECODER_TYPE      GetDecoderType(std::string p_sExt);
    ENCODER_TYPE      GetEncoderType(std::string p_sExt);
  
    std::string   MimeType(std::string p_sExt);
    std::string   DLNA(std::string p_sExt);
  
    unsigned int  TargetSampleRate(std::string p_sExt);
    unsigned int  TargetBitRate(std::string p_sExt);
    
    bool          Exists(std::string p_sExt);
    std::string   Extension(std::string p_sExt);
    TRANSCODING_HTTP_RESPONSE TranscodingHTTPResponse(std::string p_sExt);
  
    int ReleaseDelay(std::string p_sExt);
    
    DisplaySettings_t* DisplaySettings() { return &m_DisplaySettings; }
  
    CFileSettings* FileSettings(std::string p_sExt);
    void AddExt(CFileSettings* pFileSettings, std::string p_sExt);
    
    bool         EnableDeviceIcon() { return m_bEnableDeviceIcon; }  
    bool         Xbox360Support() { return m_bXBox360Support; }    
		bool         ShowPlaylistAsContainer() { return m_bShowPlaylistAsContainer; }		
    bool         DLNAEnabled() { return m_bDLNAEnabled; }
    
    std::string  VirtualFolderDevice() { return m_sVirtualFolderDevice; }
  
  private:
    std::string m_sDeviceName;
    std::string m_sVirtualFolderDevice;
    
    DisplaySettings_t m_DisplaySettings;

		bool m_bShowPlaylistAsContainer;
		bool m_bXBox360Support;
    bool m_bDLNAEnabled;  
    bool m_bEnableDeviceIcon;
    
    std::map<std::string, CFileSettings*> m_FileSettings;
    std::map<std::string, CFileSettings*>::iterator m_FileSettingsIterator;
  
    int nDefaultReleaseDelay;
    
    std::list<std::string> m_slUserAgents;
		std::list<std::string> m_slIPAddresses;
};

#endif // _DEVICESETTINGS_H
