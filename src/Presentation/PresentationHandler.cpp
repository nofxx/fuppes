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
#include "../Common.h"

#include "Images/fuppes_png.cpp"
#include "Images/fuppes_small_png.cpp"

#include <sstream>

#include <fstream>
#include <iostream>

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
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/fuppes.png") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send fuppes logo");
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(fuppes_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());  
  }    
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/fuppes-small.png") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send small fuppes logo");
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(fuppes_small_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());  
  } 
  
  if(nPresentationPage == PRESENTATION_BINARY_IMAGE)
  {
    pResult->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);    
    pResult->SetContentType(HTTP_CONTENT_TYPE_IMAGE_PNG);
  }  
  else if((nPresentationPage != PRESENTATION_BINARY_IMAGE) && (nPresentationPage != PRESENTATION_PAGE_UNKNOWN))
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
  
  /* header */
  sResult << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
  sResult << "<head>";  
  sResult << "<title>" << CSharedConfig::Shared()->GetAppName() << " - " << CSharedConfig::Shared()->GetAppFullname() << " " << CSharedConfig::Shared()->GetAppVersion() << endl;
  sResult << " (" << CSharedConfig::Shared()->GetHostname() << ")" << endl;
  sResult << "</title>" << endl;
  
  sResult << "<style type=\"text/css\">";
  if(p_nPresentationPage == PRESENTATION_PAGE_INDEX)
    sResult << GetStylesheet("presentation/images/");
  else
    sResult << GetStylesheet("images/");
  sResult << "</style>";  
  sResult << "</head>";
  /* header end */
  
  sResult << "<body>";
  
  /* title */
  sResult << "<div id=\"title\">" << endl;
  
  sResult << "<p style=\"font-size: large; margin-top: 10px; margin-left: 65px; margin-bottom: 0px; padding: 0px;\">" << endl <<
    "FUPPES - Free UPnP Entertainment Service<br />" << endl <<
    "<span style=\"font-size: small; margin-left: 10px; padding: 0px;\">" <<
    "Version: " << CSharedConfig::Shared()->GetAppVersion() << "&nbsp;&nbsp;&nbsp;" <<
    "Host: "    << CSharedConfig::Shared()->GetHostname() << "&nbsp;&nbsp;&nbsp;" <<
    "Address: " << CSharedConfig::Shared()->GetIPv4Address() <<
    "</span>"   << endl <<
    "</p>" << endl;
  
  sResult << "</div>" << endl;
  /* title end */
  
  /* menu */
  sResult << "<div id=\"menu\">" << endl;
  
  sResult << "<p style=\"background: #FF0000; margin: 0px; padding-bottom: 2px; padding-top: 0px; text-indent: 5px;\">" <<
    "&bull; Menu" <<
    "</p>" << endl;
  
  sResult << "<a href=\"/index.html\">Start</a>" << endl;
  sResult << "<br />";
  sResult << "<a href=\"/presentation/about.html\">About</a>" << endl;
  sResult << "<br />";
  sResult << "Options" << endl;
  sResult << "<br />";
  sResult << "Status" << endl;
  sResult << "<br />";
  sResult << "Debug" << endl;
  
  sResult << "</div>" << endl;  
  /* menu end */
  
  sResult << "<div id=\"content\">" << endl;
  
  return sResult.str();
}


std::string CPresentationHandler::GetPageFooter(PRESENTATION_PAGE p_nPresentationPage)
{
  std::stringstream sResult;
  
  sResult << "<p style=\"padding-top: 20pt; text-align: center;\"><small>copyright &copy; 2005 Ulrich V&ouml;lkel<!--<br />distributed under the GPL--></small></p>";

  sResult << "</div>" << endl;
  
  sResult << "</body>";
  sResult << "</html>";
  
  return sResult.str();
}

/* GetXHTMLHeader */
std::string CPresentationHandler::GetXHTMLHeader()
{
  std::stringstream sResult;
  
  sResult << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" << endl;
  sResult << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" " << endl;
  sResult << "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" << endl;
  
  return sResult.str();
}

/* GetIndexHTML */
std::string CPresentationHandler::GetIndexHTML()
{
  std::stringstream sResult;
  
  sResult << "<h2>Start</h2>" << endl;
  
  for (unsigned int i = 0; i < m_vFuppesInstances.size(); i++)
  {
    //sResult << "FUPPES Instance No. " << i + 1 << "<br />";    
    //sResult << "IP-Address: " << ((CFuppes*)m_vFuppesInstances[i])->GetIPAddress() << "<br />";
    //sResult << "HTTP-Server URL: " << ((CFuppes*)m_vFuppesInstances[i])->GetHTTPServerURL() << "<br />";
    sResult << "UUID: " << ((CFuppes*)m_vFuppesInstances[i])->GetUUID() << "<br />";    
    //sResult << "<br />";
    sResult << "<br />";
    
    sResult << "<h3>Remote Devices</h3>";
    sResult << BuildFuppesDeviceList((CFuppes*)m_vFuppesInstances[i]);
  }
  
  return sResult.str();
}

std::string CPresentationHandler::GetAboutHTML()
{
  std::stringstream sResult;
  
  sResult << "<h2>About</h2>" << endl;
  sResult << "<a href=\"http://sourceforge.net/projects/fuppes/\">http://sourceforge.net/projects/fuppes/</a><br />" << endl;
  
  sResult << __DATE__ << " " << __TIME__ "<br /><i>build with:</i> " << __VERSION__;  

  return sResult.str();
}

/*===============================================================================
 HELPER
===============================================================================*/

/* BuildFuppesDeviceList */
std::string CPresentationHandler::BuildFuppesDeviceList(CFuppes* pFuppes)
{
  stringstream sResult;

  for(unsigned int i = 0; i < pFuppes->GetRemoteDevices().size(); i++)
  {
    sResult << 
      "<table border=\"1\" style=\"font-size: 10pt;\">" <<
        "<tbody>";               
               
    CUPnPDevice* pDevice = pFuppes->GetRemoteDevices()[i];
    sResult << "<tr><th colspan=\"2\">"    << pDevice->GetFriendlyName() << "</th></tr>" << endl;
    sResult << "<tr><td>UUID</td><td>"     << pDevice->GetUUID() << "</td></tr>" << endl;
    sResult << "<tr><td>Time Out</td><td>" << pDevice->GetTimer()->GetCount() / 60 << "min. " << pDevice->GetTimer()->GetCount() % 60 << "sec.</td></tr>" << endl;
    //sResult << "<tr><td>Status</td><td>"   << "<i>todo</i>" << "</td></tr>" << endl;
    
    sResult << 
        "<tbody>" <<
      "</table><br />";    
  }
  
  return sResult.str();
}

/* <\PRIVATE> */
