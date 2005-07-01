/***************************************************************************
 *            SharedConfig.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 
#include <iostream>

#include "SharedConfig.h"

#ifndef WIN32
#include <unistd.h>
#include <sys/param.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include <sys/types.h>

#include <fstream>
#include <sstream>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include "win32.h"

#ifndef MAXHOSTNAMELEN
#define MAXHOSTNAMELEN     64
#endif

//using namespace std;

CSharedConfig* CSharedConfig::m_Instance = 0;

CSharedConfig* CSharedConfig::Shared()
{
	if (m_Instance == 0)
		m_Instance = new CSharedConfig();
	return m_Instance;
}

CSharedConfig::CSharedConfig()
{
}

bool CSharedConfig::SetupConfig()
{
  bool bResult = true;
  
  if(!ResolveHostAndIP())
  {
    cout << "[ERROR] can'r resolve hostname and address" << endl;
    bResult = false;
  }
  
  if(!ReadConfigFile())
    cout << "[WARNING] config file not found" << endl;
  
  return bResult;
}

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
	return "0.1.1a";
}

string CSharedConfig::GetHostname()
{
	return m_sHostname;
}

string CSharedConfig::GetIP()
{
	return m_sIP;
}

string CSharedConfig::GetUDN()
{	
	return "12345678-aabb-0000-ccdd-1234eeff0000";
}

void CSharedConfig::SetHTTPServerURL(string p_sURL)
{
  m_sHTTPServerURL = p_sURL;
}

string CSharedConfig::GetHTTPServerURL()
{
  return m_sHTTPServerURL;
}

std::string CSharedConfig::GetSharedDir(int p_nIndex)
{
  return m_vSharedDirectories[p_nIndex];
}

int CSharedConfig::SharedDirCount()
{
  return (int)m_vSharedDirectories.size();
}

bool CSharedConfig::ReadConfigFile()
{
  xmlDocPtr pDoc;  
  string    sFileName = "fuppes.cfg";
  bool      bResult   = false;
  
  if(FileExists(sFileName))
  {
    pDoc = xmlReadFile(sFileName.c_str(), NULL, 0);
    if(pDoc != NULL)  
      bResult = true;
  }
  
  if(bResult)
  {
    xmlNode* pRootNode = NULL;  
    xmlNode* pTmpNode  = NULL;   
    pRootNode = xmlDocGetRootElement(pDoc);
    
    for(pTmpNode = pRootNode; pTmpNode; pTmpNode = pTmpNode->next)
    {
      /*if(pTmpNode->type == XML_ELEMENT_NODE)
        cout << pTmpNode->name << endl;*/
      
      xmlNode* pDirsNode = NULL;
      pDirsNode = pTmpNode->children->next;
      //cout << pDirsNode->name << endl;
      
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
  in_addr* addr;

  char name[MAXHOSTNAMELEN];
  int nRet = gethostname(name, MAXHOSTNAMELEN);
  if(nRet == 0)
  {    
    m_sHostname = name;

    struct hostent* host;
    host = gethostbyname(name);
    addr = (struct in_addr*)host->h_addr;
    m_sIP = inet_ntoa(*addr);
    
    return true;
  }
  else
  {
    return false;
  }  
}

bool CSharedConfig::FileExists(std::string p_sFileName)
{
  ifstream fsTmp;
  bool     bResult = false;
  
  fsTmp.open(p_sFileName.c_str(),ios::in);  
  bResult = fsTmp.is_open();
  fsTmp.close();
  
  return bResult;
}
