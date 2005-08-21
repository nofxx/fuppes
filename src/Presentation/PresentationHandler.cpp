/***************************************************************************
 *            PresentationHandler.cpp
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

#include "PresentationHandler.h"
#include "Stylesheet.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"

#include <sstream>

const std::string LOGNAME = "PresentationHandler"; 

/*===============================================================================
 CLASS CPresentationHandler
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

/* constructor */
CPresentationHandler::CPresentationHandler()
{
}

/* destrcutor */
CPresentationHandler::~CPresentationHandler()
{
}

/*===============================================================================
 INSTANCE
===============================================================================*/

/* AddFuppesInstance */
void CPresentationHandler::AddFuppesInstance(CFuppes* pFuppes)
{
  /* Add the instance to the list */
  m_vFuppesInstances.push_back(pFuppes);
}

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 REQUESTS
===============================================================================*/



void CPresentationHandler::OnReceivePresentationRequest(CFuppes* pSender, CHTTPMessage* pMessage, CHTTPMessage* pResult)
{
  PRESENTATION_PAGE nPresentationPage = PRESENTATION_PAGE_UNKNOWN;
  string sContent;
  
  if((pMessage->GetRequest().compare("/") == 0) || (ToLower(pMessage->GetRequest()).compare("/index.html") == 0))
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send index.html");
    nPresentationPage = PRESENTATION_PAGE_INDEX;
    sContent = this->GetIndexHTML();
  }
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/about.html") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send about.html");
    nPresentationPage = PRESENTATION_PAGE_ABOUT;
    sContent = this->GetAboutHTML();
  }  
  
  
  if(nPresentationPage != PRESENTATION_PAGE_UNKNOWN)
  {
    stringstream sResult;   
    
    sResult << GetXHTMLHeader();
    sResult << GetPageHeader(nPresentationPage);    
    sResult << sContent;    
    sResult << GetPageFooter(nPresentationPage);    
    
    pResult->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);    
    pResult->SetContentType(HTTP_CONTENT_TYPE_TEXT_HTML);
    pResult->SetContent(sResult.str());
  }  
  else if(nPresentationPage == PRESENTATION_PAGE_UNKNOWN) 
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send 404");
    pResult->SetMessageType(HTTP_MESSAGE_TYPE_404_NOT_FOUND); 
    pResult->SetContentType(HTTP_CONTENT_TYPE_TEXT_HTML);
  }
}




/*===============================================================================
 GET
===============================================================================*/
std::string CPresentationHandler::GetPageHeader(PRESENTATION_PAGE p_nPresentationPage)
{
  std::stringstream sResult;
  
  sResult << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
  sResult << "<head>";  
  sResult << "<title>" << CSharedConfig::Shared()->GetAppName() << " - " << CSharedConfig::Shared()->GetAppFullname() << " " << CSharedConfig::Shared()->GetAppVersion() << "</title>";
  
  sResult << "<style type=\"text/css\">";
  sResult << GetStylesheet();
  sResult << "</style>";  
  sResult << "</head>";
  
  sResult << "<body>";
  
  sResult << "<h1>" << CSharedConfig::Shared()->GetAppName() << " - " << CSharedConfig::Shared()->GetAppFullname() << " " << CSharedConfig::Shared()->GetAppVersion() << "</h1>";
    
  sResult << "<p>" << endl;
  sResult << "<a href=\"/index.html\">Start</a>" << endl;
  sResult << " | ";
  sResult << "<a href=\"/presentation/about.html\">About</a>" << endl;
  sResult << "</p>" << endl;  
  
  return sResult.str();
}


std::string CPresentationHandler::GetPageFooter(PRESENTATION_PAGE p_nPresentationPage)
{
  std::stringstream sResult;
  
  sResult << "<p><small>copyright &copy; 2005 - the FUPPES development team<br />distributed under the GPL</small></p>";
  
  sResult << "</body>";
  sResult << "</html>";
  
  return sResult.str();
}

/* GetXHTMLHeader */
std::string CPresentationHandler::GetXHTMLHeader()
{
  std::stringstream sResult;
  
  sResult << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  sResult << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN" ;
  sResult << "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd";
  
  return sResult.str();
}

/* GetIndexHTML */
std::string CPresentationHandler::GetIndexHTML()
{
  std::stringstream sResult;
    
  for (unsigned int i = 0; i < m_vFuppesInstances.size(); i++)
  {
    sResult << "FUPPES Instance No. " << i + 1 << "<br />";    
    sResult << "IP-Address: " << ((CFuppes*)m_vFuppesInstances[i])->GetIPAddress() << "<br />";
    sResult << "HTTP-Server URL: " << ((CFuppes*)m_vFuppesInstances[i])->GetHTTPServerURL() << "<br />";
    sResult << "UUID: " << ((CFuppes*)m_vFuppesInstances[i])->GetUUID() << "<br />";    
    sResult << "<br />";
    sResult << "<br />";
    
    sResult << "<h3>Remote Devices</h3>";
    sResult << BuildFuppesDeviceList((CFuppes*)m_vFuppesInstances[i]);
  }
  
  return sResult.str();
}

std::string CPresentationHandler::GetAboutHTML()
{
  std::stringstream sResult;
  
  sResult << __DATE__ << " " << __TIME__ " - " << __VERSION__;  

  return sResult.str();
}

/*===============================================================================
 HELPER
===============================================================================*/

/* BuildFuppesDeviceList */
std::string CPresentationHandler::BuildFuppesDeviceList(CFuppes* pFuppes)
{
  stringstream sResult;

  /* Find devices and add them to the list */
  for(unsigned int i = 0; i < pFuppes->GetRemoteDevices().size(); i++)
  {
    CUPnPDevice* pDevice = pFuppes->GetRemoteDevices()[i];
    sResult << "No. " << i + 1 << "<br />";
    sResult << "Name: " << pDevice->GetFriendlyName() << "<br />";
    sResult << "UUID: " << pDevice->GetUUID() << "<br />";
    sResult << "Status: " << "<i>todo</i>" << "<br />";
  }

  return sResult.str();
}

/* <\PRIVATE> */
