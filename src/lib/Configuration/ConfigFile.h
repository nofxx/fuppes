/***************************************************************************
 *            ConfigFile.h
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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _CONFIGFILE_H
#define _CONFIGFILE_H

#include "../Common/XMLParser.h"
#include <vector>

const std::string NEEDED_CONFIGFILE_VERSION = "0.7.2";

typedef enum CONFIG_FILE_ERROR {
  CF_OK,
  CF_FILE_NOT_FOUND,
  CF_PARSE_ERROR,
  CF_CONFIG_DEPRECATED  
} CONFIG_FILE_ERROR;

class CConfigFile
{
  public:
    CConfigFile();
    ~CConfigFile();

    int Load(std::string p_sFileName, std::string* p_psErrorMsg);
  
    // shared objects
    int SharedDirCount() { return m_lSharedDirs.size(); }
    std::string SharedDir(int p_nIdx) { return m_lSharedDirs[p_nIdx]; }
  
    int SharedITunesCount() { return m_lSharedITunes.size(); }
    std::string SharedITunes(int p_nIdx) { return m_lSharedITunes[p_nIdx]; }
  
    // network
    std::string IpAddress() { return m_sIpAddress; }
    void        IpAddress(std::string p_sIpAddress);
    int         HttpPort() { return m_nHttpPort; }
    void        HttpPort(int p_nHttpPort);
  
    int         AllowedIpsCount() { return m_lAllowedIps.size(); }
    std::string AllowedIp(int p_nIdx) { return m_lAllowedIps[p_nIdx]; }
    
    // content_directory
    std::string LocalCharset() { return m_sLocalCharset; }
    void        LocalCharset(std::string p_sLocalCharset);
    bool        UseImageMagick() { return m_bUseImageMagick; }
    bool        UseTaglib() { return m_bUseTaglib; }
    bool        UseLibAvFormat() { return m_bUseLibAvFormat; }
  
    // transcoding  
    std::string  AudioEncoder() { return m_sAudioEncoder; }
    bool         TranscodeVorbis() { return m_bTranscodeVorbis; }
    bool         TranscodeMusePack() { return m_bTranscodeMusePack; }
    bool         TranscodeFlac() { return m_bTranscodeFlac; }
  
    bool WriteDefaultConfig(std::string p_sFileName);
  
  private:
    CXMLDocument* m_pDoc;
  
    void SetupDeviceIdentificationMgr(CXMLNode* pDeviceSettingsNode);
  
    // shared objects
    std::vector<std::string>  m_lSharedDirs;
    std::vector<std::string>  m_lSharedITunes;
  
    // network
    std::string               m_sIpAddress;    
    int                       m_nHttpPort;
    std::vector<std::string>  m_lAllowedIps;
  
    // content_directory
    std::string               m_sLocalCharset;
    bool                      m_bUseImageMagick;
    bool                      m_bUseTaglib;
    bool                      m_bUseLibAvFormat;

    // transcoding
    std::string               m_sAudioEncoder;
    bool                      m_bTranscodeVorbis;
    bool                      m_bTranscodeMusePack;
    bool                      m_bTranscodeFlac;  
};

#endif // _CONFIGFILE_H
