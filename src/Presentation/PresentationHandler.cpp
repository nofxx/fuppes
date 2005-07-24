/***************************************************************************
 *            PresentationHandler.cpp
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
 
#include "PresentationHandler.h"
#include "Stylesheet.h"
#include "../SharedConfig.h"

#include <sstream>

CPresentationHandler::CPresentationHandler()
{
}

CPresentationHandler::~CPresentationHandler()
{
}

void CPresentationHandler::AddFuppesInstance(CFuppes* pFuppes)
{
  m_vFuppesInstances.push_back(pFuppes);
}

std::string CPresentationHandler::GetXHTMLHeader()
{
  std::stringstream sResult;
  
  sResult << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  sResult << "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN" ;
  sResult << "http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd";
  
  return sResult.str();
}

std::string CPresentationHandler::GetIndexHTML()
{
  std::stringstream sResult;
  sResult << this->GetXHTMLHeader();
  sResult << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
  sResult << "<head>";  
  sResult << "<title>" << CSharedConfig::Shared()->GetAppFullname() << " " << CSharedConfig::Shared()->GetAppVersion() << "</title>";
  
  sResult << "<style type=\"text/css\">";
  sResult << GetStylesheet();
  sResult << "</style>";  
  sResult << "</head>";
  
  sResult << "<body>";  
  
  sResult << "<h1>" << CSharedConfig::Shared()->GetAppFullname() << " " << CSharedConfig::Shared()->GetAppVersion() << "</h1>";
  
  for (unsigned int i = 0; i < m_vFuppesInstances.size(); i++)
  {
    sResult << "FUPPES Instance No. " << i + 1 << "<br />";    
    sResult << "IP-Address: " << ((CFuppes*)m_vFuppesInstances[i])->GetIPAddress() << "<br />";
    sResult << "HTTP-Server URL: " << ((CFuppes*)m_vFuppesInstances[i])->GetHTTPServerURL() << "<br />";
    sResult << "Status: " << "<i>todo</i>" << "<br />";
    sResult << "<br />";
    sResult << "<br />";
    
    sResult << "<h3>Remote Devices</h3>";
    sResult << this->BuildFuppesDeviceList((CFuppes*)m_vFuppesInstances[i]);
  }
  
  sResult << "</body>";
  sResult << "</html>";
  
  return sResult.str();
}

void CPresentationHandler::OnReceivePresentationRequest(CFuppes* pSender, CHTTPMessage* pMessage, CHTTPMessage* pResult)
{
  pResult->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);
  pResult->SetContentType(HTTP_CONTENT_TYPE_TEXT_HTML);
  pResult->SetContent(this->GetIndexHTML());
}


std::string CPresentationHandler::BuildFuppesDeviceList(CFuppes* pFuppes)
{
  stringstream sResult;
  
  for(unsigned int i = 0; i < pFuppes->GetRemoteDevices().size(); i++)
  {
    CUPnPDevice* pDevice = pFuppes->GetRemoteDevices()[i];
    sResult << "No. " << i + 1 << "<br />";
    sResult << "Name: " << pDevice->GetFriendlyName() << "<br />";
  }
  
  return sResult.str();
}
