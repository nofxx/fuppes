/***************************************************************************
 *            SharedConfig.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include <iostream>

#include "SharedConfig.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/param.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#endif

#include "Common.h"
#include "UUID.h"

#include <sys/types.h>
#include <fcntl.h>
#include <fstream>
#include <sstream>
#include <sys/stat.h>
#ifdef WIN32
#else
#include <sys/utsname.h>
#endif

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN     64
#endif

/* transcoding */
#ifndef DISABLE_TRANSCODING

/* LAME is always needed */
#include "Transcoding/LameWrapper.h"

#ifndef DISABLE_VORBIS
#include "Transcoding/VorbisWrapper.h"
#endif

#ifndef DISABLE_MUSEPACK
#include "Transcoding/MpcWrapper.h"
#endif

#ifndef DISABLE_FLAC
#include "Transcoding/FlacWrapper.h"
#endif

#endif

/*===============================================================================
 CLASS CSharedConfig
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 INSTANCE
===============================================================================*/

CSharedConfig* CSharedConfig::m_Instance = 0;

CSharedConfig* CSharedConfig::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedConfig();
	return m_Instance;
}

/* <\PUBLIC> */

/* <PROTECTED> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CSharedConfig::CSharedConfig()
{
  m_sUUID     = GenerateUUID();  
  m_nHTTPPort = 0;
  
  m_nMaxFileNameLength = 0;
  
  m_pDoc = NULL;
  m_pSharedDirNode  = NULL;
  m_pContentDirNode = NULL;
  
  /* transcoding */
  #ifdef DISABLE_TRANSCODING
  m_bTranscodingEnabled = false;
  #else
  m_bTranscodingEnabled = true;
  #endif
  m_bLameAvailable     = false;
  m_bVorbisAvailable   = false;
  m_bMusePackAvailable = false;
  m_bFlacAvailable     = false;
  
  m_bTranscodeVorbis   = true;
  m_bTranscodeMusePack = true;
  m_bTranscodeFlac     = true;
  
  /* display settings */
  m_DisplaySettings.bShowTranscodingTypeInItemNames = true;
  
  m_DisplaySettings.bShowDirNamesInFirstLevel = true;
}

CSharedConfig::~CSharedConfig()
{
  xmlFreeDoc(m_pDoc);
}

/* <\PROTECTED> */

/* <PUBLIC> */

/*===============================================================================
 INIT
===============================================================================*/

bool CSharedConfig::SetupConfig()
{
  bool bResult = true;  
  
  /* read config file */
  bResult = ReadConfigFile(true);
  if(!bResult)
    return false;
  
  /* Network settings */
  if(bResult && !ResolveHostAndIP())
  {
    cout << "[ERROR] can't resolve hostname and address" << endl;
    return false;
  }
  
  /*cout << "hostname: " << GetHostname() << endl; 
  cout << "address : " << GetIPv4Address() << endl; 
  cout << endl;*/

  /* OS information */
  GetOSInfo();


  /* Transcoding */
  #ifndef DISABLE_TRANSCODING  
  CheckForTranscodingLibs();
  //PrintTranscodingSettings();
  #endif
  return bResult;
}

bool CSharedConfig::Refresh()
{
  bool bResult = false;
  
  /* reset all variables and containers ... */
  m_nMaxFileNameLength = 0;  
  m_vSharedDirectories.clear();  
  
  m_pSharedDirNode  = NULL;
  m_pContentDirNode = NULL;
  
  /*xmlFreeDoc(m_pDoc);
  m_pDoc = NULL;*/
  
  /* ... and read the config file */
  bResult = ReadConfigFile(false);
  
  return bResult;
}

/*===============================================================================
 GET
===============================================================================*/

string CSharedConfig::GetAppName()
{
  return "FUPPES";
}

string CSharedConfig::GetAppFullname()
{
  return "Free UPnP Entertainment Service";
}

string CSharedConfig::GetAppVersion()
{
	return "0.5.3a";
}

string CSharedConfig::GetHostname()
{
  return m_sHostname;
}

string CSharedConfig::GetUUID()
{
  return m_sUUID; 
}

string CSharedConfig::GetIPv4Address()
{
  return m_sIP;
}

std::string CSharedConfig::GetSharedDir(unsigned int p_nIndex)
{
  return m_vSharedDirectories[p_nIndex];
}

unsigned int CSharedConfig::SharedDirCount()
{
  return (unsigned int)m_vSharedDirectories.size();
}

std::string CSharedConfig::GetConfigDir()
{
  stringstream sResult;
  #ifdef WIN32
  sResult << getenv("APPDATA") << "\\Free UPnP Entertainment Service\\";
  #else
  sResult << getenv("HOME") << "/.fuppes/";
  #endif
  return sResult.str();
}

bool CSharedConfig::IsSupportedFileExtension(std::string p_sFileExtension)
{  
  /* TODO :: 
    - bei Bedarf '.' abschneiden
    - extensions + transcoding [ja|nein] aus der config lesen  */
  
  /* Audio */
  if((ToLower(p_sFileExtension).compare("mp3") == 0))
    return true;
  else if((ToLower(p_sFileExtension).compare("ogg") == 0) && ((m_bTranscodingEnabled && m_bVorbisAvailable) || !m_bTranscodeVorbis))
    return true;
  else if((ToLower(p_sFileExtension).compare("mpc") == 0) && ((m_bTranscodingEnabled && m_bMusePackAvailable) || !m_bTranscodeMusePack))
    return true;
  else if((ToLower(p_sFileExtension).compare("flac") == 0) && ((m_bTranscodingEnabled && m_bFlacAvailable) || !m_bTranscodeFlac))
    return true;
    
  /* Images */
  else if((ToLower(p_sFileExtension).compare("jpeg") == 0) || (ToLower(p_sFileExtension).compare("jpg") == 0))
    return true;
  else if(ToLower(p_sFileExtension).compare("png") == 0)
    return true;
  else if(ToLower(p_sFileExtension).compare("bmp") == 0)
    return true;
  else if(ToLower(p_sFileExtension).compare("gif") == 0)
    return true;
    
  /* Video */   
  else if((ToLower(p_sFileExtension).compare("mpeg") == 0) || (ToLower(p_sFileExtension).compare("mpg") == 0))
    return true;
  else if((ToLower(p_sFileExtension).compare("avi") == 0))
    return true;
  else if((ToLower(p_sFileExtension).compare("wmv") == 0))
    return true;
  else if((ToLower(p_sFileExtension).compare("vdr") == 0))
    return true;  
  else if((ToLower(p_sFileExtension).compare("vob") == 0))
    return true;  
  /*else if((ToLower(p_sFileExtension).compare("rm") == 0))
    return true;*/
  else
    return false;
}

FILE_KIND CSharedConfig::GetFileKindByExtension(std::string p_sFileExtension)
{
  FILE_KIND nResult = FILE_KIND_UNKNOWN;
  
  if(
     (ToLower(p_sFileExtension).compare("mp3") == 0) ||
     (ToLower(p_sFileExtension).compare("ogg") == 0) ||
     (ToLower(p_sFileExtension).compare("mpc") == 0) ||
     (ToLower(p_sFileExtension).compare("flac") == 0)
    )
    nResult = FILE_KIND_AUDIO;  
  
  return nResult;
}

bool CSharedConfig::IsTranscodingExtension(std::string p_sFileExt)
{  
  if((ToLower(p_sFileExt).compare("mp3") == 0))
    return false;
  else if((ToLower(p_sFileExt).compare("ogg") == 0) && m_bTranscodingEnabled && m_bVorbisAvailable && m_bTranscodeVorbis)
    return true;
  else if((ToLower(p_sFileExt).compare("mpc") == 0) && m_bTranscodingEnabled && m_bMusePackAvailable && m_bTranscodeMusePack)
    return true;
  else if((ToLower(p_sFileExt).compare("flac") == 0) && m_bTranscodingEnabled && m_bFlacAvailable && m_bTranscodeFlac)
    return true;  
  else
    return false;  
}

bool CSharedConfig::IsAllowedIP(std::string p_sIPAddress)
{
  bool bResult = (m_vAllowedIPs.size() == 0);
  for(unsigned int i = 0; i < m_vAllowedIPs.size(); i++)
  { 
    if(m_vAllowedIPs[i].compare(p_sIPAddress) == 0)
    {
      bResult = true;
      break;
    }
  }  
  return bResult;
}


bool CSharedConfig::AddSharedDirectory(std::string p_sDirectory)
{  
  unsigned char* szBuf = new unsigned char[4096];  
  int nSize = 4096;
  int nLength = p_sDirectory.length();
  isolat1ToUTF8(szBuf, &nSize, (const unsigned char*)p_sDirectory.c_str(), &nLength);
  szBuf[nSize] = '\0';  
  
  if(m_pSharedDirNode)
    xmlNewTextChild(m_pSharedDirNode, NULL, BAD_CAST "dir", BAD_CAST szBuf);
  
  delete[] szBuf;
  
  xmlSaveFormatFileEnc(m_sConfigFileName.c_str(), m_pDoc, "UTF-8", 1);    
  return this->Refresh();
}

bool CSharedConfig::RemoveSharedDirectory(unsigned int p_nIndex)
{
  
  
  return this->Refresh();
}

void CSharedConfig::SetMaxFileNameLength(unsigned int p_nMaxFileNameLenght)
{
  if(!m_pContentDirNode)
    return;
  
  xmlNode* pTmp = m_pContentDirNode->children;
  string sTmp;
  while(pTmp)
  {
    sTmp = (char*)pTmp->name;
    if(sTmp.compare("max_file_name_length") == 0)
    {      
      stringstream sMaxLen;
      sMaxLen << p_nMaxFileNameLenght;
      if(!pTmp->children)
        xmlNodeAddContent(pTmp, BAD_CAST sMaxLen.str().c_str());
      else
        xmlNodeSetContent(pTmp->children, BAD_CAST sMaxLen.str().c_str());
      cout << pTmp->children->content << endl;
      //pTmp->children->content = BAD_CAST sMaxLen.str().c_str();
      break;
    }    
    pTmp = pTmp->next;
  }
  
  xmlSaveFormatFileEnc(m_sConfigFileName.c_str(), m_pDoc, "UTF-8", 1);    
  this->Refresh();
}

/* <\PUBLIC> */
	
/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

bool CSharedConfig::ReadConfigFile(bool p_bIsInit)
{
  //xmlDocPtr pDoc    = NULL;  
  bool      bResult = true;
  
  if(!m_pDoc)
  {    
    if(m_sConfigFileName.length() == 0)
    {
      stringstream sFileName;
      stringstream sDir;
    
      #ifdef WIN32
      sDir << getenv("APPDATA") << "\\Free UPnP Entertainment Service\\";
      sFileName << sDir.str() << "fuppes.cfg";
      if(!DirectoryExists(sDir.str())) 
        CreateDirectory(sDir.str().c_str(), NULL);
      #else
      sDir << getenv("HOME") << "/.fuppes/";
      sFileName << sDir.str() << "fuppes.cfg";
      if(!DirectoryExists(sDir.str())) 
        mkdir(sDir.str().c_str(), S_IRWXU | S_IRWXG);
      #endif
    
      m_sConfigFileName = sFileName.str();   
    }    
    
    if(FileExists(m_sConfigFileName))
    { 
      m_pDoc = xmlReadFile(m_sConfigFileName.c_str(), NULL, XML_PARSE_NOBLANKS);
      if(m_pDoc != NULL)  
        bResult = true;
    }
    /* config does not exist 
       write default config, reload it and show
       "configure via webinterface"-hint */
    else
    { 
      //cout << endl << "[ERROR] no config file found" << endl;      
      /*cout << "wrote default config to \"" << sFileName.str() << "\"" << endl;
      cout << "please edit the config-file and restart FUPPES" << endl;*/
      
      this->WriteDefaultConfig(m_sConfigFileName);      
      bResult = this->ReadConfigFile(true);      
      if(bResult)
      {
        cout << "[INFORMATION]" << endl <<
                "  wrote configuration file to \"" << m_sConfigFileName << "\"." << endl <<
                "  You can now configure fuppes via the webinterface." << endl << 
                "  Thanks for using fuppes!" << endl << endl;        
        fflush(stdout);
      }
      else      
      {
        cout << "[ERROR] can not create config file \"" << m_sConfigFileName << "\"." << endl;
        fflush(stdout);
      }
        
      return bResult;
    }
  
  } /* if(!m_pDoc) */
  
  
  if(bResult)
  {
    xmlNode* pRootNode = NULL;  
    xmlNode* pTmpNode  = NULL;   
    pRootNode = xmlDocGetRootElement(m_pDoc);    
    
    //m_sConfigVersion
    
    for(pTmpNode = pRootNode->children->next; pTmpNode; pTmpNode = pTmpNode->next)
    { 
      string sName = (char*)pTmpNode->name;
      
      /* shared_directories */
      if(sName.compare("shared_directories") == 0)
      {
        m_pSharedDirNode = pTmpNode;        
        xmlNode* pTmp = m_pSharedDirNode->children;
        
        while(pTmp)
        {
          m_vSharedDirectories.push_back((char*)pTmp->children->content);
          pTmp = pTmp->next;
        }       

      }
      /* shared_dir */
      
      
      /* network_settings */
      else if(sName.compare("network_settings") == 0)
      {
        xmlNode* pNetNode = pTmpNode->children;
        
        while(pNetNode)
        {
          string sNet = (char*)pNetNode->name;      
          
          if(sNet.compare("ip_address") == 0)
          {            
            if(pNetNode->children)
            {
              string sIP = (char*)pNetNode->children->content;
              if(sIP.compare("0") != 0)
                m_sIP = sIP;
            }
          }
          else if(sNet.compare("http_port") == 0)
          {
            if(pNetNode->children)
            {
              string sPort = (char*)pNetNode->children->content;
              if(sPort.compare("0") != 0)
                m_nHTTPPort = atoi(sPort.c_str());              
            }
          }
          else if(sNet.compare("allowed_ips") == 0)
          {
            if(pNetNode->children)
            {
              xmlNode* pIPNode = pNetNode->children;
              while(pIPNode)
              {
                if(pIPNode->children)
                  m_vAllowedIPs.push_back((char*)pIPNode->children->content);                
                
                pIPNode = pIPNode->next;
              }
            }
          }
          
          
          pNetNode = pNetNode->next;
        }
      }
      /* end network_settings */
      
      
      /* content_directory */
      else if(sName.compare("content_directory") == 0)
      {
        m_pContentDirNode = pTmpNode;
        xmlNode* pTmp = m_pContentDirNode->children;
        
        string sContentDir;
        while(pTmp)
        {
          sContentDir = (char*)pTmp->name;        
          
          // max_file_name_length
          if(sContentDir.compare("max_file_name_length") == 0)
          {
            if(pTmp->children)
            {
              string sMaxFileNameLength = (char*)pTmp->children->content;              
              if(sMaxFileNameLength.compare("0") != 0)
                m_nMaxFileNameLength = atoi(sMaxFileNameLength.c_str());
            }
          }
          
          pTmp = pTmp->next;
        }    
      }
      /* end content_directory */      
      
    }   
    
  }  
  xmlCleanupParser();  
  return bResult;
}

bool CSharedConfig::ResolveHostAndIP()
{
  char name[MAXHOSTNAMELEN];
  int nRet = gethostname(name, MAXHOSTNAMELEN);
  if(0 == nRet)
  {    
    m_sHostname = name;
    
    if(((m_sIP == "") && !ResolveIPByHostname()) || m_sIP == "127.0.0.1")
    {
      return ResolveIPByInterface("eth0");    
    }
    else    
      return true;    
  }
  else 
    return false;
}

bool CSharedConfig::ResolveIPByHostname()
{
    in_addr* addr;
    struct hostent* host;
      
    host = gethostbyname(m_sHostname.c_str());
    if(host != NULL)
    {
      addr = (struct in_addr*)host->h_addr;
      m_sIP = inet_ntoa(*addr);
      return true;
    }
    else
      return false;
}

bool CSharedConfig::ResolveIPByInterface(std::string p_sInterfaceName)
{
  #ifdef WIN32
  return true;
  #else
  struct ifreq ifa;
  struct sockaddr_in *saddr;
  int       fd;
  
  strcpy (ifa.ifr_name, p_sInterfaceName.c_str());
  if(((fd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) || ioctl(fd, SIOCGIFADDR, &ifa))
      return false;
  saddr = (struct sockaddr_in*)&ifa.ifr_addr;
  m_sIP = inet_ntoa(saddr->sin_addr);
  return true;
  #endif
}

bool CSharedConfig::WriteDefaultConfig(std::string p_sFileName)
{
  xmlTextWriterPtr  pWriter;	
	std::stringstream sTmp;
	
	pWriter = xmlNewTextWriterFilename(p_sFileName.c_str(), 0);
  xmlTextWriterSetIndent(pWriter, 4);
	xmlTextWriterStartDocument(pWriter, NULL, "UTF-8", NULL);

	/* fuppes_config */
	xmlTextWriterStartElement(pWriter, BAD_CAST "fuppes_config");  
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "version", BAD_CAST "0.5"); 
	
    /* shared_directories */    
    xmlTextWriterWriteComment(pWriter, BAD_CAST "\r\n");
    xmlTextWriterStartElement(pWriter, BAD_CAST "shared_directories");
        
      /*xmlTextWriterStartElement(pWriter, BAD_CAST "dir");
      #ifdef WIN32
      xmlTextWriterWriteString(pWriter, BAD_CAST "C:\\Musik\\mp3s\\Marillion");      
      #else
      xmlTextWriterWriteString(pWriter, BAD_CAST "/mnt/musik/mp3s/Marillion");
      #endif
      xmlTextWriterEndElement(pWriter); 
      
      xmlTextWriterStartElement(pWriter, BAD_CAST "dir");
      #ifdef WIN32
      xmlTextWriterWriteString(pWriter, BAD_CAST "C:\\Musik\\mp3s\\Porcupine Tree");      
      #else
      xmlTextWriterWriteString(pWriter, BAD_CAST "/mnt/musik/mp3s/Porcupine Tree");
      #endif
      xmlTextWriterEndElement(pWriter); */
  
    /* end shared_directories */
    xmlTextWriterEndElement(pWriter);
    
    
    /* network_settings */
    xmlTextWriterWriteComment(pWriter, BAD_CAST "\r\n");
    xmlTextWriterStartElement(pWriter, BAD_CAST "network_settings");
        
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty or 0 = automatic detection");
      xmlTextWriterStartElement(pWriter, BAD_CAST "ip_address");
      xmlTextWriterEndElement(pWriter); 
      
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty or 0 = random port");
      xmlTextWriterStartElement(pWriter, BAD_CAST "http_port");
      xmlTextWriterEndElement(pWriter); 
  
      xmlTextWriterWriteComment(pWriter, BAD_CAST "list of ip addresses allowed to access fuppes. if empty all ips are allowed");
      xmlTextWriterStartElement(pWriter, BAD_CAST "allowed_ips");
        /*xmlTextWriterStartElement(pWriter, BAD_CAST "ip");
        xmlTextWriterWriteString(pWriter, BAD_CAST "192.168.0.1");
        xmlTextWriterEndElement(pWriter);*/
        xmlTextWriterWriteComment(pWriter, BAD_CAST "<ip>192.168.0.1</ip>");
      xmlTextWriterEndElement(pWriter); 
  
    /* end network_settings */
    xmlTextWriterEndElement(pWriter);


    /* content directory */
    xmlTextWriterWriteComment(pWriter, BAD_CAST "\r\n");
    xmlTextWriterStartElement(pWriter, BAD_CAST "content_directory");
    
      /* max_file_name_length */
      std::stringstream sComment;
         sComment << "specify the maximum length for file names." << endl;
         sComment << "      e.g. the Telegent TG 100 can handle file names up" << endl;
         sComment << "      to 101 characters. everything above leads to an error." << endl;
         sComment << "      if you leave the field empty or insert 0 the maximum" << endl;
         sComment << "      length is unlimited.";
      xmlTextWriterWriteComment(pWriter, BAD_CAST sComment.str().c_str());
      xmlTextWriterStartElement(pWriter, BAD_CAST "max_file_name_length");
      xmlTextWriterWriteString(pWriter, BAD_CAST "0");
      xmlTextWriterEndElement(pWriter);
    
    
    /* end content directory */
    xmlTextWriterEndElement(pWriter);
    
  
	/* end fuppes_config */
	xmlTextWriterEndElement(pWriter);	
	xmlTextWriterEndDocument(pWriter);
	xmlFreeTextWriter(pWriter);
	
  xmlCleanupParser();
  
  return true;
}

  
void CSharedConfig::CheckForTranscodingLibs()
{
  #ifndef DISABLE_TRANSCODING
    
  /* LAME */
  CLameWrapper* pLame = new CLameWrapper();
  m_bLameAvailable = pLame->LoadLib();
  if(m_bLameAvailable)
    m_sLameVersion = pLame->GetVersion();
  delete pLame;  
  
  if(m_bLameAvailable)
  { 
    /* Vorbis */
    #ifndef DISABLE_VORBIS
    CVorbisDecoder* pVorbis = new CVorbisDecoder();
    m_bVorbisAvailable = pVorbis->LoadLib();
    delete pVorbis;
    #endif
    
    /* MusePack */
    #ifndef DISABLE_MUSEPACK
    CMpcDecoder* pMuse = new CMpcDecoder();
    m_bMusePackAvailable = pMuse->LoadLib();
    delete pMuse;
    #endif
    
    /* FLAC */
    #ifndef DISABLE_FLAC
    CFLACDecoder* pFlac = new CFLACDecoder();
    m_bFlacAvailable = pFlac->LoadLib();
    delete pFlac;
    #endif
  }  
   
  #endif  
}

void CSharedConfig::PrintTranscodingSettings()
{
  #ifdef DISABLE_TRANSCODING 
  cout << "compiled without transcoding support" << endl;
  #else
  if(!m_bLameAvailable)
  {
    cout << endl;
    cout << "LAME not found. transcoding disabled!" << endl;
    #ifdef WIN32
    cout << "Get a copy of the lame_enc.dll and" << endl;
    cout << "put it in the application directory." << endl;
    #endif
    cout << endl;
    m_bTranscodingEnabled = false;
  }
  else
  {
    /* no decoder available */
    if(!m_bVorbisAvailable && !m_bMusePackAvailable && !m_bFlacAvailable)
    {
      m_bTranscodingEnabled = false;
      cout << endl;
      cout << "no decoding library found. transcoding disabled" << endl;
      cout << endl;
    }
    else
    {
      cout << "transcoding settings:" << endl;
      
      /* lame */
      cout << "  lame    : (version: " << m_sLameVersion << ")" << endl;
      
      /* vorbis */
      cout << "  vorbis  : ";
      #ifdef DISABLE_VORBIS
      cout << "compiled without vorbis support" << endl;
      #else
      if(m_bVorbisAvailable)
        cout << "enabled" << endl;
      else
        cout << "disabled" << endl;
      #endif      
    
      /* musepack */
      cout << "  musepack: ";
      #ifdef DISABLE_MUSEPACK
      cout << "compiled without MusePack support" << endl;
      #else
      if(m_bMusePackAvailable)
        cout << "enabled" << endl;
      else
        cout << "disabled" << endl;
      #endif
      
      /* flac */
      cout << "  flac    : ";
      #ifdef DISABLE_FLAC
      cout << "compiled without FLAC support" << endl;
      #else
      if(m_bFlacAvailable)
        cout << "enabled" << endl;
      else
        cout << "disabled" << endl;
      #endif
      
      cout << endl;      
    }
  }
  #endif  
    
}

std::string CSharedConfig::GetOSName()
{
  return m_sOSName;
}

std::string CSharedConfig::GetOSVersion()
{
  return m_sOSVersion;
}

bool CSharedConfig::GetOSInfo()
{
  bool bResult = true;
  
  #ifdef WIN32
  m_sOSName    = "Windows";
  m_sOSVersion = "3.11";
  #else  
  struct utsname sUtsName;
  uname(&sUtsName);  
  m_sOSName    = sUtsName.sysname;
  m_sOSVersion = sUtsName.release;
  /*cout << sUtsName.sysname << endl;
  cout << sUtsName.release << endl;
  cout << sUtsName.version << endl;
  cout << sUtsName.machine << endl;*/
  #endif   
  
  return bResult;
}

/* <\PRIVATE> */
