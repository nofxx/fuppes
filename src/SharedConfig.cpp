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
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
  m_sUUID = GenerateUUID();  
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

  if(bResult && !ResolveHostAndIP())
  {
    cout << "[ERROR] can't resolve hostname and address" << endl;
    bResult = false;
  }

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
	return "0.2";
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

std::string CSharedConfig::GetSharedDir(int p_nIndex)
{
  return m_vSharedDirectories[p_nIndex];
}

int CSharedConfig::SharedDirCount()
{
  return (int)m_vSharedDirectories.size();
}

/* <\PUBLIC> */
	
/* <PRIVATE> */

/*===============================================================================
 HELPER
===============================================================================*/

bool CSharedConfig::ReadConfigFile()
{
  xmlDocPtr pDoc;  
  bool      bResult   = false;
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
    
    for(pTmpNode = pRootNode; pTmpNode; pTmpNode = pTmpNode->next)
    { 
      xmlNode* pDirsNode = NULL;
      pDirsNode = pTmpNode->children->next;     
      
      xmlNode* pDirNode = NULL;
      pDirNode = pDirsNode->children;
      for(pDirNode = pDirNode->next; pDirNode; pDirNode = pDirNode->next)
      {
        if(pDirNode->type == XML_ELEMENT_NODE)
        {
          stringstream sDirName;
          sDirName << pDirNode->children->content;          
          m_vSharedDirectories.push_back(sDirName.str());
          sDirName.str("");
        }
      }
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
    
    if(!ResolveIPByHostname() || m_sIP == "127.0.0.1")
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
  
	/* end fuppes_config */
	xmlTextWriterEndElement(pWriter);	
	xmlTextWriterEndDocument(pWriter);
	xmlFreeTextWriter(pWriter);
	
  xmlCleanupParser();
  
  return true;
}

/* <\PRIVATE> */
