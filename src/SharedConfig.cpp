/***************************************************************************
 *            SharedConfig.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

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
  
  /* display settings */
  m_DisplaySettings.bShowTranscodingTypeInItemNames = true;
  
  m_DisplaySettings.bShowDirNamesInFirstLevel = true;
}

/* <\PROTECTED> */

/* <PUBLIC> */

/*===============================================================================
 INIT
===============================================================================*/

bool CSharedConfig::SetupConfig()
{
  bool bResult = true;  
  bResult = ReadConfigFile();

  /* Network settings */
  if(bResult && !ResolveHostAndIP())
  {
    cout << "[ERROR] can't resolve hostname and address" << endl;
    return false;
  }
  
  cout << "hostname: " << GetHostname() << endl; 
  cout << "address : " << GetIPv4Address() << endl; 
  cout << endl;  
  
  /* Transcoding */
  #ifndef DISABLE_TRANSCODING  
  CheckForTranscodingLibs();
  if(!m_bLameAvailable)
  {
    cout << endl;
    cout << "LAME not found. transcoding disabled!" << endl;
    #ifdef WIN32
    //cout << "Go to http://sourceforge.net/projects/fuppes/," << endl;
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
      cout << "no decoding library found. Transcoding disabled" << endl;
      cout << endl;
    }
    else
    {
      cout << "  transcoding" << endl;
      
      /* vorbis */
      cout << "   vorbis  : ";
      #ifdef DISABLE_VORBIS
      cout << "compiled without vorbis support" << endl;
      #else
      if(m_bVorbisAvailable)
        cout << "yes" << endl;
      else
        cout << "no" << endl;
      #endif      
    
      /* musepack */
      cout << "   musepack: ";
      #ifdef DISABLE_MUSEPACK
      cout << "compiled without MusePack support" << endl;
      #else
      if(m_bMusePackAvailable)
        cout << "yes" << endl;
      else
        cout << "no" << endl;
      #endif
      
      /* flac */
      cout << "   flac    : ";
      #ifdef DISABLE_FLAC
      cout << "compiled without FLAC support" << endl;
      #else
      cout << "coming soon" << endl;
      /*if(m_bMusePackAvailable)
        cout << "yes" << endl;
      else
        cout << "no" << endl;*/
      #endif
      
      cout << endl;      
    }
  }  
  #endif  
  
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
	return "0.3.2";
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
  
  if((ToLower(p_sFileExtension).compare("mp3") == 0))
    return true;
  else if((ToLower(p_sFileExtension).compare("ogg") == 0) && m_bTranscodingEnabled && m_bVorbisAvailable)
    return true;
  else if((ToLower(p_sFileExtension).compare("mpc") == 0) && m_bTranscodingEnabled && m_bMusePackAvailable)
    return true;
  else if((ToLower(p_sFileExtension).compare("flac") == 0) && m_bTranscodingEnabled && m_bFlacAvailable)
    return true;
  else if((ToLower(p_sFileExtension).compare("jpeg") == 0) || (ToLower(p_sFileExtension).compare("jpg") == 0))
    return true;
  else if((ToLower(p_sFileExtension).compare("mpeg") == 0) || (ToLower(p_sFileExtension).compare("mpg") == 0))
    return true;
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

/* <\PUBLIC> */
	
/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

bool CSharedConfig::ReadConfigFile()
{
  xmlDocPtr pDoc    = NULL;  
  bool      bResult = false;
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

  if(FileExists(sFileName.str()))
  {
    pDoc = xmlReadFile(sFileName.str().c_str(), NULL, 0);
    if(pDoc != NULL)  
      bResult = true;
  }
  else if(FileExists("fuppes.cfg"))
  {
    pDoc = xmlReadFile("fuppes.cfg", NULL, 0);
    if(pDoc != NULL)  
      bResult = true;
  }  
  else
  { 
    cout << endl << "[ERROR] no config file found" << endl;
    WriteDefaultConfig(sFileName.str());
    cout << "wrote default config to \"" << sFileName.str() << "\"" << endl;
    cout << "please edit the config-file and restart FUPPES" << endl;
    bResult = false;
    #ifdef WIN32
    fuppesSleep(4000);
    #endif
  }
  
  if(bResult)
  {
    xmlNode* pRootNode = NULL;  
    xmlNode* pTmpNode  = NULL;   
    pRootNode = xmlDocGetRootElement(pDoc);
    
    
    for(pTmpNode = pRootNode->children->next; pTmpNode; pTmpNode = pTmpNode->next)
    { 
      string sName = (char*)pTmpNode->name;
      
      /* shared_directories */
      if(sName.compare("shared_directories") == 0)
      {
        xmlNode* pDirNode = pTmpNode->children;
        for(pDirNode = pDirNode->next; pDirNode; pDirNode = pDirNode->next)
        {
          if(pDirNode->type == XML_ELEMENT_NODE)
          {
            string sDirName = (char*)pDirNode->children->content;          
            m_vSharedDirectories.push_back(sDirName);
          }        
        }
      }
      
      /* network_settings */
      else if(sName.compare("network_settings") == 0)
      {
        xmlNode* pNetNode = NULL;
        for(pNetNode = pTmpNode->children->next; pNetNode; pNetNode = pNetNode->next)
        {
          string sNet = (char*)pNetNode->name;
          
          /* ip address */
          if(sNet.compare("ip_address") == 0)
          {
            if(pNetNode->children)
            {
              string sIP = (char*)pNetNode->children->content;
              if(sIP.compare("0") != 0)
                m_sIP = sIP;
            }           
          }
          
          /* http_port */
          else if(sNet.compare("http_port") == 0)
          {
            if(pNetNode->children)
            {
              string sPort = (char*)pNetNode->children->content;
              if(sPort.compare("0") != 0)
                m_nHTTPPort = atoi(sPort.c_str());
            }
          }

          
        }
      }
      /* end network_settings */
      
    }   
    xmlFreeDoc(pDoc);
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

bool CSharedConfig::FileExists(std::string p_sFileName)
{
  ifstream fsTmp;
  bool     bResult = false;
  
  /* Try to open the file to check it exists */
  fsTmp.open(p_sFileName.c_str(),ios::in);  
  bResult = fsTmp.is_open();
  fsTmp.close();
  
  return bResult;
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
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "version", BAD_CAST "0.1"); 
	
    /* shared_directories */
    xmlTextWriterStartElement(pWriter, BAD_CAST "shared_directories");
        
      xmlTextWriterStartElement(pWriter, BAD_CAST "dir");
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
      xmlTextWriterEndElement(pWriter); 
  
    /* end shared_directories */
    xmlTextWriterEndElement(pWriter);
    
    /* network_settings */
    xmlTextWriterStartElement(pWriter, BAD_CAST "network_settings");
        
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty or 0 = automatic detection");
      xmlTextWriterStartElement(pWriter, BAD_CAST "ip_address");
      xmlTextWriterEndElement(pWriter); 
      
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty or 0 = random port");
      xmlTextWriterStartElement(pWriter, BAD_CAST "http_port");
      xmlTextWriterEndElement(pWriter); 
  
    /* end network_settings */
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
  }  
   
  #endif  
}

/* <\PRIVATE> */
