/***************************************************************************
 *            ConfigFile.cpp
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
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "ConfigFile.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"

#include <libxml/xmlwriter.h>
#include <sstream>
#include <iostream>

using namespace std;

CConfigFile::CConfigFile()
{
  m_pDoc = new CXMLDocument();
  
  m_sIpAddress = "";
  m_nHttpPort  = 0;

  m_sLocalCharset   = "UTF-8";
  m_bUseImageMagick = false;
  m_bUseTaglib      = false;
  m_bUseLibAvFormat = false;
  
  m_bTranscodeFlac     = true;
  m_bTranscodeVorbis   = true;
  m_bTranscodeMusePack = true;
}

CConfigFile::~CConfigFile()
{
  if(m_pDoc != NULL) {
    delete m_pDoc;
  }    
}

int CConfigFile::Load(std::string p_sFileName, std::string* p_psErrorMsg)
{
  if(!m_pDoc->Load(p_sFileName)) {    
    *p_psErrorMsg = "parse error";
    return CF_PARSE_ERROR;
  }

  if(m_pDoc->RootNode() == NULL) {
    *p_psErrorMsg = "parse error";
    return CF_PARSE_ERROR;
  }
  
  string sVersion = m_pDoc->RootNode()->Attribute("version");
  if(sVersion.compare(NEEDED_CONFIGFILE_VERSION) != 0) {
    *p_psErrorMsg = "configuration deprecated";
    return CF_CONFIG_DEPRECATED;
  }
  
  CXMLNode* pTmpNode;
  int i;
  
  // shared objects
  ReadSharedObjects();
  
  // network
  ReadNetworkSettings();


  // content_directory
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("content_directory", false);
  if(pTmpNode != NULL) {
    for(i = 0; i < pTmpNode->ChildCount(); i++) {
      
      if(pTmpNode->ChildNode(i)->Name().compare("local_charset") == 0) {
        m_sLocalCharset = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("use_imagemagick") == 0) {
        m_bUseImageMagick = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("use_taglib") == 0) {
        m_bUseTaglib = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }      
      else if(pTmpNode->ChildNode(i)->Name().compare("use_libavformat") == 0) {
        m_bUseLibAvFormat = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }     
    }
  }
  // end content_directory

  // transcoding
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("transcoding", false);
  if(pTmpNode != NULL) {
    for(i = 0; i < pTmpNode->ChildCount(); i++) {    
      
      if(pTmpNode->ChildNode(i)->Name().compare("audio_encoder") == 0) {
        m_sAudioEncoder = pTmpNode->ChildNode(i)->Value();
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("transcode_vorbis") == 0) {
        m_bTranscodeVorbis = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("transcode_musepack") == 0) {
        m_bTranscodeMusePack = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("transcode_flac") == 0) {
        m_bTranscodeFlac = (pTmpNode->ChildNode(i)->Value().compare("true") == 0);
      }      
    }
  }
  // end transcoding
  
  // device_settings
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("device_settings", false);
  if(pTmpNode != NULL) {
    SetupDeviceIdentificationMgr(pTmpNode);
  }
  // end device_settings
  
  return CF_OK;
}

void CConfigFile::ReadSharedObjects()
{
  CXMLNode* pTmpNode;
  int i;  
  
  m_lSharedDirs.clear();    
  m_lSharedITunes.clear();
  
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("shared_objects", false);
  if(pTmpNode != NULL) {
    for(i = 0; i < pTmpNode->ChildCount(); i++) {
      if(pTmpNode->ChildNode(i)->Name().compare("dir") == 0) {
        m_lSharedDirs.push_back(pTmpNode->ChildNode(i)->Value());
      }
      else if(pTmpNode->ChildNode(i)->Name().compare("itunes") == 0) {
        m_lSharedITunes.push_back(pTmpNode->ChildNode(i)->Value());
      }
    }
  } 
}

void CConfigFile::ReadNetworkSettings()
{
  CXMLNode* pTmpNode;
  int i;
  int j;  
  
  m_lAllowedIps.clear();
  
  pTmpNode = m_pDoc->RootNode()->FindNodeByName("network", false);
  if(pTmpNode == NULL) {
    return;
  }
  
  for(i = 0; i < pTmpNode->ChildCount(); i++) {
      
    if(pTmpNode->ChildNode(i)->Name().compare("ip_address") == 0) {
      if(pTmpNode->ChildNode(i)->Value().length() > 0) {
        m_sIpAddress = pTmpNode->ChildNode(i)->Value();
      }        
    }
    else if(pTmpNode->ChildNode(i)->Name().compare("http_port") == 0) {
      if(pTmpNode->ChildNode(i)->Value().length() > 0) {
        m_nHttpPort = atoi(pTmpNode->ChildNode(i)->Value().c_str());
      } 
    }
    else if(pTmpNode->ChildNode(i)->Name().compare("allowed_ips") == 0) {
      for(j = 0; j < pTmpNode->ChildNode(i)->ChildCount(); j++) {
        if(pTmpNode->ChildNode(i)->ChildNode(j)->Name().compare("ip") == 0) {
          m_lAllowedIps.push_back(pTmpNode->ChildNode(i)->ChildNode(j)->Value());
        }
      }          
    }     
    
  }  
}

void CConfigFile::SetupDeviceIdentificationMgr(CXMLNode* pDeviceSettingsNode)
{
  int i;
  int j;
  CXMLNode* pDevice;
  CXMLNode* pTmp;
  CDeviceSettings* pSettings;
  
  for(i = 0; i < pDeviceSettingsNode->ChildCount(); i++) {
    
    pDevice = pDeviceSettingsNode->ChildNode(i);
    if(pDevice->Attribute("name").compare("default") != 0 && pDevice->Attribute("enabled").compare("true") != 0) {
      continue;
    }    
    
    pSettings = CDeviceIdentificationMgr::Shared()->GetSettingsForInitialization(pDevice->Attribute("name"));
    
    // virtual device
    if(pDevice->Attribute("virtual").length() > 0) {
      pSettings->m_sVirtualFolderDevice = pDevice->Attribute("virtual");
    }
    
    // settings
    for(j = 0; j < pDevice->ChildCount(); j++) {
      pTmp = pDevice->ChildNode(j);
      
      // user_agent
      if(pTmp->Name().compare("user_agent") == 0) {        
        pSettings->m_slUserAgents.push_back(pTmp->Value());
      }
      // ip
      else if(pTmp->Name().compare("ip") == 0) {
        pSettings->m_slIPAddresses.push_back(pTmp->Value());
      }
      // playlist_style
      else if(pTmp->Name().compare("playlist_style") == 0) {
        pSettings->m_bShowPlaylistAsContainer = (pTmp->Value().compare("container") == 0);
      }
      // max_file_name_length
      else if(pTmp->Name().compare("max_file_name_length") == 0) {
        pSettings->m_nMaxFileNameLength = atoi(pTmp->Value().c_str());
      }
      // show_childcount_in_title
      else if(pTmp->Name().compare("show_childcount_in_title") == 0) {
        pSettings->m_DisplaySettings.bShowChildCountInTitle = (pTmp->Value().compare("true") == 0);
      }
      // xbox360
      else if(pTmp->Name().compare("xbox360") == 0) {
        pSettings->m_bXBox360Support = (pTmp->Value().compare("true") == 0);
      }     
    }
    
  }
}

void CConfigFile::AddSharedDir(std::string p_sDirName)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  if(pTmp != NULL) {
    pTmp->AddChild("dir", p_sDirName);
    ReadSharedObjects();
    m_pDoc->Save();
  }
}

void CConfigFile::RemoveSharedDir(int p_nIdx)
{
  int i;
  int nIdx = 0;
  
  CXMLNode* pObj = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  CXMLNode* pTmp;
  if(pObj == NULL) {
    return;    
  }
  
  for(i = 0; i < pObj->ChildCount(); i++) {
    pTmp = pObj->ChildNode(i);
    if(pTmp->Name().compare("dir") == 0) {
      if(nIdx == p_nIdx) {
        pObj->RemoveChild(i);
        ReadSharedObjects();
        m_pDoc->Save();
        break;
      }      
      nIdx++;
    }    
  }
}

void CConfigFile::AddSharedITunes(std::string p_sITunesName)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  if(pTmp != NULL) {
    pTmp->AddChild("itunes", p_sITunesName); 
    ReadSharedObjects();
    m_pDoc->Save();
  }  
}

void CConfigFile::RemoveSharedITunes(int p_nIdx)
{
  int i;
  int nIdx = 0;
  
  CXMLNode* pObj = m_pDoc->RootNode()->FindNodeByName("shared_objects");
  CXMLNode* pTmp;
  if(pObj == NULL) {
    return;    
  }
  
  for(i = 0; i < pObj->ChildCount(); i++) {
    pTmp = pObj->ChildNode(i);
    if(pTmp->Name().compare("itunes") == 0) {
      if(nIdx == p_nIdx) {
        pObj->RemoveChild(i);
        ReadSharedObjects();
        m_pDoc->Save();
        break;
      }      
      nIdx++;
    }    
  }  
}


void CConfigFile::IpAddress(std::string p_sIpAddress)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("ip_address", true);
  if(pTmp) {
    pTmp->Value(p_sIpAddress);
    m_pDoc->Save();
    m_sIpAddress = p_sIpAddress;
  }  
}

void CConfigFile::HttpPort(int p_nHttpPort)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("http_port", true);
  if(pTmp) {
    pTmp->Value(p_nHttpPort);
    m_pDoc->Save();  
    m_nHttpPort = p_nHttpPort;
  }  
}

void CConfigFile::LocalCharset(std::string p_sLocalCharset)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("local_charset", true);
  if(pTmp) {
    pTmp->Value(p_sLocalCharset);
    m_pDoc->Save();  
    m_sLocalCharset = p_sLocalCharset;
  } 
}


void CConfigFile::AddAllowedIp(std::string p_sIpAddress)
{
  CXMLNode* pTmp = m_pDoc->RootNode()->FindNodeByName("allowed_ips", true);
  if(pTmp != NULL) {
    pTmp->AddChild("ip", p_sIpAddress);
    ReadNetworkSettings();
    m_pDoc->Save();
  }  
}

void CConfigFile::RemoveAllowedIp(int p_nIdx)
{
  int i;  
  int nIdx = 0;
  
  CXMLNode* pObj = m_pDoc->RootNode()->FindNodeByName("allowed_ips", true);
  CXMLNode* pTmp;
  if(pObj == NULL) {
    return;    
  }
  
  for(i = 0; i < pObj->ChildCount(); i++) {
    pTmp = pObj->ChildNode(i);
    
    if(pTmp->Name().compare("ip") == 0) {    
      if(nIdx == p_nIdx) {
        pObj->RemoveChild(i);
        ReadNetworkSettings();
        m_pDoc->Save();
        break;
      }        
    }
  }  
}


bool CConfigFile::WriteDefaultConfig(std::string p_sFileName)
{
  xmlTextWriterPtr  pWriter;
	
	pWriter = xmlNewTextWriterFilename(p_sFileName.c_str(), 0);
  if(!pWriter)
	  return false;
  
	xmlTextWriterSetIndent(pWriter, 4);
	xmlTextWriterStartDocument(pWriter, NULL, "UTF-8", NULL);

	// fuppes_config
	xmlTextWriterStartElement(pWriter, BAD_CAST "fuppes_config");  
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "version", BAD_CAST NEEDED_CONFIGFILE_VERSION.c_str()); 
	
    // shared_objects
    xmlTextWriterStartElement(pWriter, BAD_CAST "shared_objects");
      
      #ifdef WIN32
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<dir>C:\\Music\\</dir>");
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<itunes>C:\\Documents and Settings\\...\\iTunes.xml</itunes>");  
      #else 
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<dir>/mnt/music</dir>");
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<itunes>/Users/.../iTunes.xml</itunes>");  
      #endif
  
    // end shared_objects
    xmlTextWriterEndElement(pWriter);
    
    
    // network
    xmlTextWriterStartElement(pWriter, BAD_CAST "network");
        
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty or 0 = automatic detection");
      xmlTextWriterStartElement(pWriter, BAD_CAST "ip_address");
      xmlTextWriterEndElement(pWriter); 
      
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty or 0 = random port");
      xmlTextWriterStartElement(pWriter, BAD_CAST "http_port");
      xmlTextWriterEndElement(pWriter); 
  
      xmlTextWriterWriteComment(pWriter, BAD_CAST "list of ip addresses allowed to access fuppes. if empty all ips are allowed");
      xmlTextWriterStartElement(pWriter, BAD_CAST "allowed_ips");        
        xmlTextWriterWriteComment(pWriter, BAD_CAST "<ip>192.168.0.1</ip>");
      xmlTextWriterEndElement(pWriter); 
  
    // end network
    xmlTextWriterEndElement(pWriter);


    // content directory
    xmlTextWriterStartElement(pWriter, BAD_CAST "content_directory");
    
      std::stringstream sComment;
      
      // charset
      sComment << "a list of possible charsets can be found under:" << endl << "      http://www.gnu.org/software/libiconv/";
      xmlTextWriterWriteComment(pWriter, BAD_CAST sComment.str().c_str());
      sComment.str("");
      xmlTextWriterStartElement(pWriter, BAD_CAST "local_charset");
      xmlTextWriterWriteString(pWriter, BAD_CAST "UTF-8");
      xmlTextWriterEndElement(pWriter); 
    
      // libs for metadata extraction
      xmlTextWriterWriteComment(pWriter, BAD_CAST "libs used for metadata extraction when building the database. [true|false]");
      xmlTextWriterStartElement(pWriter, BAD_CAST "use_imagemagick");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true");
      xmlTextWriterEndElement(pWriter);  
      xmlTextWriterStartElement(pWriter, BAD_CAST "use_taglib");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true");
      xmlTextWriterEndElement(pWriter);
      xmlTextWriterStartElement(pWriter, BAD_CAST "use_libavformat");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true");
      xmlTextWriterEndElement(pWriter); 
  
    // end content directory
    xmlTextWriterEndElement(pWriter);
    
    
    // transcoding 
    xmlTextWriterStartElement(pWriter, BAD_CAST "transcoding");
      
      // audio_encoder
      xmlTextWriterWriteComment(pWriter, BAD_CAST "[lame|twolame]");
      xmlTextWriterStartElement(pWriter, BAD_CAST "audio_encoder");
      xmlTextWriterWriteString(pWriter, BAD_CAST "lame"); // [lame|twolame]
      xmlTextWriterEndElement(pWriter);
      
      // transcode_vorbis
      xmlTextWriterWriteComment(pWriter, BAD_CAST "[true|false]");
      xmlTextWriterStartElement(pWriter, BAD_CAST "transcode_vorbis");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true"); // [true|false]
      xmlTextWriterEndElement(pWriter);
      
      // transcode_musepack
      xmlTextWriterStartElement(pWriter, BAD_CAST "transcode_musepack");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true"); // [true|false]
      xmlTextWriterEndElement(pWriter);
      
      // transcode_flac
      xmlTextWriterStartElement(pWriter, BAD_CAST "transcode_flac");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true"); // [true|false]
      xmlTextWriterEndElement(pWriter);
    
    // end transcoding
    xmlTextWriterEndElement(pWriter);
    
  
    // device_settings
    xmlTextWriterStartElement(pWriter, BAD_CAST "device_settings");
      
      // device (default)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "default");   
  
        // max_file_name_length
        sComment << "specify the maximum length for file names." << endl;
           sComment << "      e.g. the Telegent TG 100 can handle file names up" << endl;
           sComment << "      to 101 characters. everything above leads to an error." << endl;
           sComment << "      if you leave the field empty or insert 0 the maximum" << endl;
           sComment << "      length is unlimited.";
        xmlTextWriterWriteComment(pWriter, BAD_CAST sComment.str().c_str());
        sComment.str("");
        xmlTextWriterStartElement(pWriter, BAD_CAST "max_file_name_length");
        xmlTextWriterWriteString(pWriter, BAD_CAST "0");
        xmlTextWriterEndElement(pWriter);
    
        // playlist_style
        xmlTextWriterWriteComment(pWriter, BAD_CAST "[file|container]");
        xmlTextWriterStartElement(pWriter, BAD_CAST "playlist_style");
        xmlTextWriterWriteString(pWriter, BAD_CAST "file"); // [file|container]
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "show_childcount_in_title");
        xmlTextWriterWriteString(pWriter, BAD_CAST "false");
        xmlTextWriterEndElement(pWriter);  
  
      // end device (default)
      xmlTextWriterEndElement(pWriter);
  
  
      // device (Xbox 360)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "Xbox 360");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "virtual", BAD_CAST "Xbox 360");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
  
        // user_agent
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "Xbox/2.0.\\\\d+.\\\\d+ UPnP/1.0 Xbox/2.0.\\\\d+.\\\\d+");
        xmlTextWriterEndElement(pWriter);
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "Xenon");
        xmlTextWriterEndElement(pWriter);
  
        // xbox 360
        xmlTextWriterStartElement(pWriter, BAD_CAST "xbox360");
        xmlTextWriterWriteString(pWriter, BAD_CAST "true");
        xmlTextWriterEndElement(pWriter);
  
      // end device (Xbox 360)
      xmlTextWriterEndElement(pWriter);  
  
  
      // device (Noxon audio)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "Noxon audio");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "virtual", BAD_CAST "default");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
  
        xmlTextWriterWriteComment(pWriter, BAD_CAST "<ip></ip>");
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "playlist_style");
        xmlTextWriterWriteString(pWriter, BAD_CAST "container");
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "show_childcount_in_title");
        xmlTextWriterWriteString(pWriter, BAD_CAST "true");
        xmlTextWriterEndElement(pWriter);  
  
      // end device (Noxon audio)
      xmlTextWriterEndElement(pWriter);   
  
      // device (Telegent TG 100)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "Telegent TG 100");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "virtual", BAD_CAST "default");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
  
        xmlTextWriterWriteComment(pWriter, BAD_CAST "<ip></ip>");
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "playlist_style");
        xmlTextWriterWriteString(pWriter, BAD_CAST "file");
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "max_file_name_length");
        xmlTextWriterWriteString(pWriter, BAD_CAST "101");
        xmlTextWriterEndElement(pWriter);  
  
      // end device (Telegent TG 100)
      xmlTextWriterEndElement(pWriter);  
  
  
    // end device_settings
    xmlTextWriterEndElement(pWriter);
  
	// end fuppes_config
	xmlTextWriterEndElement(pWriter);	
	xmlTextWriterEndDocument(pWriter);
	xmlFreeTextWriter(pWriter);
	
  //xmlCleanupParser(); 
  
  CXMLDocument* pDoc = new CXMLDocument();
  pDoc->Load(p_sFileName);
  pDoc->Save();
  delete pDoc;
  
  return true;
}
