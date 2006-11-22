/***************************************************************************
 *            PresentationHandler.cpp
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

#include "PresentationHandler.h"
#include "Stylesheet.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Common.h"
#include "../ContentDirectory/ContentDatabase.h"
#include "../ContentDirectory/FileDetails.h"

//#include "Images/fuppes_png.cpp"
#include "Images/fuppes_small_png.cpp"
#include "Images/header_gradient_png.cpp"
#include "Images/header_gradient_small_png.cpp"

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

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 REQUESTS
===============================================================================*/



void CPresentationHandler::OnReceivePresentationRequest(CFuppes* pSender, CHTTPMessage* pMessage, CHTTPMessage* pResult)
{
  PRESENTATION_PAGE nPresentationPage = PRESENTATION_PAGE_UNKNOWN;
  string sContent;
 
  std::string sImgPath = "images/";
  std::string sPageName = "undefined";
    
  //cout << pMessage->GetRequest() << " "; // << endl;
  
  if((pMessage->GetRequest().compare("/") == 0) || (ToLower(pMessage->GetRequest()).compare("/index.html") == 0))
  {
    sImgPath = "presentation/images/";
    
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send index.html");
    nPresentationPage = PRESENTATION_PAGE_INDEX;
    sContent = this->GetIndexHTML(sImgPath);
    sPageName = "Start";
  }
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/about.html") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send about.html");
    nPresentationPage = PRESENTATION_PAGE_ABOUT;
    sContent = this->GetAboutHTML(sImgPath);
    sPageName = "About";
  }
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/options.html") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send options.html");
    nPresentationPage = PRESENTATION_PAGE_OPTIONS;
    sContent = this->GetOptionsHTML(sImgPath);
    sPageName = "Options";
  }
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/options.html?rebuild=db") == 0)
  {
    CSharedConfig::Shared()->Refresh();
    if(!CContentDatabase::Shared()->IsRebuilding())
      CContentDatabase::Shared()->BuildDB();
    
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send options.html");
    nPresentationPage = PRESENTATION_PAGE_OPTIONS;
    sContent = this->GetOptionsHTML(sImgPath);
    sPageName = "Options";
  }
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/status.html") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send status.html");
    nPresentationPage = PRESENTATION_PAGE_STATUS;
    sContent = this->GetStatusHTML(sImgPath);
    sPageName = "Status";
  }
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/config.html") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send config.html");
    nPresentationPage = PRESENTATION_PAGE_STATUS;
    sContent = this->GetConfigHTML(sImgPath, pMessage);
    sPageName = "Configuration";
    
    /*cout << pMessage->GetMessage() << endl;
    cout << "TYPE: " << pMessage->GetMessageType() << endl;*/
  }        
  
  
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/fuppes-small.png") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send small fuppes logo");
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(fuppes_small_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());  
    
    //cout << sImg.length() << endl;
  } 
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/header-gradient.png") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send header gradient");
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(header_gradient_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());  
    
    //cout << sImg.length() << endl;
  }
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/header-gradient-small.png") == 0)
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send header gradient small");
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(header_gradient_small_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());
    
    //cout << sImg.length() << endl;
  }
  
  
  if(nPresentationPage == PRESENTATION_BINARY_IMAGE)
  {
    pResult->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);    
    pResult->SetContentType("image/png"); // HTTP_CONTENT_TYPE_IMAGE_PNG
  }  
  else if((nPresentationPage != PRESENTATION_BINARY_IMAGE) && (nPresentationPage != PRESENTATION_PAGE_UNKNOWN))
  {   
    stringstream sResult;   
    
    sResult << GetXHTMLHeader();
    sResult << GetPageHeader(nPresentationPage, sImgPath, sPageName);    
    sResult << sContent;    
    sResult << GetPageFooter(nPresentationPage);
    
    pResult->SetMessageType(HTTP_MESSAGE_TYPE_200_OK);    
    pResult->SetContentType("text/html"); // HTTP_CONTENT_TYPE_TEXT_HTML
    pResult->SetContent(sResult.str());    
  }  
  else if(nPresentationPage == PRESENTATION_PAGE_UNKNOWN) 
  {
    CSharedLog::Shared()->ExtendedLog(LOGNAME, "send 404");
    pResult->SetMessageType(HTTP_MESSAGE_TYPE_404_NOT_FOUND); 
    pResult->SetContentType("text/html");
  }
}




/*===============================================================================
 GET
===============================================================================*/
std::string CPresentationHandler::GetPageHeader(PRESENTATION_PAGE p_nPresentationPage, std::string p_sImgPath, std::string p_sPageName)
{
  std::stringstream sResult; 

  
  /* header */
  sResult << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
  sResult << "<head>";  
  sResult << "<title>" << CSharedConfig::Shared()->GetAppName() << " - " << CSharedConfig::Shared()->GetAppFullname() << " " << CSharedConfig::Shared()->GetAppVersion() << endl;
  sResult << " (" << CSharedConfig::Shared()->GetHostname() << ")" << endl;
  sResult << "</title>" << endl;
  
  /* stylesheet */
  sResult << "<style type=\"text/css\">" << endl << GetStylesheet(p_sImgPath) << endl << "</style>";
  
  
  /*sResult << "<script type=\"text/javascript\">"
    "function Klappen(Id) {"
    "var KlappText = document.getElementById('Remote'+Id);"
    "var KlappBild = document.getElementById('Pic'+Id);"
    "var jetec_Minus=\"minus.gif\", jetec_Plus=\"plus.gif\";"
    "if (KlappText.style.display == 'none')"
    "{"
    "KlappText.style.display = '';"
    "KlappBild.src = jetec_Minus;"
    "}"
    "else"
    "{"
    "KlappText.style.display = 'none';"
    "KlappBild.src = jetec_Plus;"
    "}"
    "}"
    "</script>"; */
  
  sResult << "</head>";
  /* header end */
  
  sResult << "<body>";
  
  /* title */
  sResult << "<div id=\"title\">" << endl;
  sResult << "<img src=\"" << p_sImgPath << "fuppes-small.png\" style=\"float: left; margin-top: 7px; margin-left: 5px;\" />" << endl;
  
  sResult << "<p style=\"font-size: large; margin-top: 12px; margin-left: 65px; margin-bottom: 0px; padding: 0px;\">" << endl <<
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
  
  sResult << "<div style=\"background-image: url(" << p_sImgPath << "header-gradient-small.png);" << endl <<
             "background-repeat: repeat-x; color: #FFFFFF; height: 20px; font-weight: bold; text-align: center; \">" <<
             "Menu</div>" << endl;
    
  /*
  " <<  
             "margin: 0px; padding-bottom: 2px; padding-top: 0px; text-indent: 5px;
             */
  
  sResult << "<a href=\"/index.html\">Start</a>" << endl;
  sResult << "<br />";
  /*sResult << "<a href=\"/presentation/about.html\">About</a>" << endl;
  sResult << "<br />";*/
  sResult << "<a href=\"/presentation/options.html\">Options</a>" << endl;
  sResult << "<br />";
  sResult << "<a href=\"/presentation/status.html\">Status</a>" << endl;    
  sResult << "<br />";
  sResult << "<a href=\"/presentation/config.html\">Configuration</a>" << endl;    
  sResult << "<br />";
  sResult << "Debug" << endl;
  
  sResult << "</div>" << endl;  
  /* menu end */
  
  sResult << "<div id=\"mainframe\">" << endl;
  sResult << "<div style=\"background-image: url(" << p_sImgPath << "header-gradient-small.png);" << endl <<
             "background-repeat: repeat-x; color: #FFFFFF; height: 20px; font-weight: bold; text-align: center; margin-left: 0px; \">" <<
             p_sPageName << "</div>" << endl;     
  
  sResult << "<div id=\"content\">" << endl;
  
  return sResult.str();
}


std::string CPresentationHandler::GetPageFooter(PRESENTATION_PAGE p_nPresentationPage)
{
  std::stringstream sResult;
  
  sResult << "<p style=\"padding-top: 20pt; text-align: center;\"><small>copyright &copy; 2005, 2006 Ulrich V&ouml;lkel<!--<br />distributed under the GPL--></small></p>";

  sResult << "</div>" << endl; // #content
  sResult << "</div>" << endl; // #mainframe
  
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
std::string CPresentationHandler::GetIndexHTML(std::string p_sImgPath)
{
  std::stringstream sResult;
  
  sResult << "<h1>system information</h1>" << endl;  
  
  sResult << "<p>" << endl;
  sResult << "Version: " << CSharedConfig::Shared()->GetAppVersion() << "<br />" << endl;
  sResult << "Hostname: " << CSharedConfig::Shared()->GetHostname() << "<br />" << endl;
  sResult << "OS: " << CSharedConfig::Shared()->GetOSName() << " " << CSharedConfig::Shared()->GetOSVersion() << "<br />" << endl;
  sResult << "SQLite: " << CContentDatabase::Shared()->GetLibVersion() << endl;
  sResult << "</p>" << endl;
  
  sResult << "<p>" << endl;
  sResult << "build at: " << __DATE__ << "" << __TIME__ "<br />" << endl;
  sResult << "build with: " << __VERSION__ << endl;
  sResult << "</p>" << endl;
  
  sResult << "<p>" << endl;
  sResult << "<a href=\"http://sourceforge.net/projects/fuppes/\">http://sourceforge.net/projects/fuppes/</a><br />" << endl;
  sResult << "</p>" << endl;
  
  
  sResult << "<h1>remote devices</h1>";
  sResult << BuildFuppesDeviceList(CSharedConfig::Shared()->GetFuppesInstance(0), p_sImgPath);
  
  return sResult.str();
  
  //sResult << "<h2>Start</h2>" << endl;
  //*p_psImgPath = "Start";
  
  /*for (unsigned int i = 0; i < m_vFuppesInstances.size(); i++)
  {
    sResult << "FUPPES Instance No. " << i + 1 << "<br />";    
    sResult << "IP-Address: " << ((CFuppes*)m_vFuppesInstances[i])->GetIPAddress() << "<br />";
    sResult << "HTTP-Server URL: " << ((CFuppes*)m_vFuppesInstances[i])->GetHTTPServerURL() << "<br />";    
    sResult << "<br />";
    sResult << "<br />";    
    
  }*/
}

std::string CPresentationHandler::GetAboutHTML(std::string p_sImgPath)
{
  std::stringstream sResult;
  
  //sResult << "<h2>About</h2>" << endl;
  //*p_psImgPath = "About";

  
  return sResult.str();
}

std::string CPresentationHandler::GetOptionsHTML(std::string p_sImgPath)
{
  std::stringstream sResult;
  
  //sResult << "<h2>Options</h2>" << endl;
  //*p_psImgPath = "Options";
  /*sResult << "<a href=\"http://sourceforge.net/projects/fuppes/\">http://sourceforge.net/projects/fuppes/</a><br />" << endl; */

  //((CFuppes*)m_vFuppesInstances[0])->GetContentDirectory()->BuildDB();
  sResult << "<h1>database options</h1>" << endl;
  if(!CContentDatabase::Shared()->IsRebuilding())  
    sResult << "<a href=\"/presentation/options.html?rebuild=db\">rebuild database</a>" << endl;
  else
    sResult << "database rebuild in progress" << endl;
  
  return sResult.str();
}

std::string CPresentationHandler::GetStatusHTML(std::string p_sImgPath)
{
  std::stringstream sResult;  
  
  //sResult << "<h2>Status</h2>" << endl;
  //*p_psImgPath = "Status";
  
  //sResult
  CContentDatabase::Shared()->Lock();
  std::stringstream sSQL;
  sSQL << "select TYPE, count(*) as VALUE from OBJECTS group by TYPE;";
  CContentDatabase::Shared()->Select(sSQL.str());
  int nType = 0;  
  
  // Database status
  sResult << "<h1>database status</h1>" << endl;  
  sResult << 
    "<table rules=\"all\" style=\"font-size: 10pt; border-style: solid; border-width: 1px; border-color: #000000;\" cellspacing=\"0\" width=\"400\">" << endl <<
      "<thead>" << endl <<
        "<tr>" << endl <<        
          "<th style=\"background-image: url(" << p_sImgPath << "header-gradient-small.png); color: #FFFFFF;\">" <<           
          "Type" << "</th>" << 
          "<th style=\"background-image: url(" << p_sImgPath << "header-gradient-small.png); color: #FFFFFF;\">" <<           
          "Count" << 
          "</th>" << endl <<
        "</tr>" << endl <<
      "</thead>" << endl << 
      "<tbody>" << endl;  
  
  while (!CContentDatabase::Shared()->Eof())
  {
    sResult << "<tr>" << endl;
    
    nType = atoi(CContentDatabase::Shared()->GetResult()->GetValue("TYPE").c_str());
    sResult << "<td>" << CFileDetails::Shared()->GetObjectTypeAsString(nType) << "</td>" << endl;
    sResult << "<td>" << CContentDatabase::Shared()->GetResult()->GetValue("VALUE") << "</td>" << endl;
    CContentDatabase::Shared()->Next();
    
    sResult << "</tr>" << endl;
  }
  
  sResult <<
      "</tbody>" << endl <<   
    "</table>" << endl;
  
  CContentDatabase::Shared()->ClearResult();
  CContentDatabase::Shared()->Unlock();
  // end Database status
  
  // system status
  sResult << "<h1>system status</h1>" << endl;  
  
  sResult << "<p>" << endl;
  sResult << "UUID: " << CSharedConfig::Shared()->GetFuppesInstance(0)->GetUUID() << "<br />";    
  sResult << "</p>" << endl;
  
  // end system status
  
  return sResult.str();  
}


std::string CPresentationHandler::GetConfigHTML(std::string p_sImgPath, CHTTPMessage* pRequest)
{
  std::stringstream sResult;  
  
  //cout << pRequest->GetMessage() << endl;
  
  /* handle config changes */
  if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_POST)
  {
    /* remove shared dir(s) */    
    stringstream sVar;
    for(int i = CSharedConfig::Shared()->SharedDirCount() - 1; i >= 0; i--)
    {
      sVar << "shared_dir_" << i;      
      if(pRequest->PostVarExists(sVar.str()))      
        CSharedConfig::Shared()->RemoveSharedDirectory(i);      
      sVar.str("");
    }    
    
    /* add shared dir */
    if(pRequest->PostVarExists("new_dir") && (pRequest->GetPostVar("new_dir").length() > 0))
    {     
      CSharedConfig::Shared()->AddSharedDirectory(pRequest->GetPostVar("new_dir"));
    }

    /* local_charset */
    if(pRequest->PostVarExists("local_charset"))
    {
      CSharedConfig::Shared()->SetLocalCharset(pRequest->GetPostVar("local_charset"));
    }    
    
    /* playlist_representation */
    if(pRequest->PostVarExists("playlist_representation"))
    {
      CSharedConfig::Shared()->SetPlaylistRepresentation(pRequest->GetPostVar("playlist_representation"));
    }
    
    /* max_file_name_length */
    if(pRequest->PostVarExists("max_file_name_length") && (pRequest->GetPostVar("max_file_name_length").length() > 0))
    {
      int nMaxFileNameLength = atoi(pRequest->GetPostVar("max_file_name_length").c_str());      
      CSharedConfig::Shared()->SetMaxFileNameLength(nMaxFileNameLength);
    }
    
    /* ip address */
    if(pRequest->PostVarExists("ip_address") && (pRequest->GetPostVar("ip_address").length() > 0))
    {
      CSharedConfig::Shared()->SetIPv4Address(pRequest->GetPostVar("ip_address"));
    }
    
    /* http port */
    if(pRequest->PostVarExists("http_port") && (pRequest->GetPostVar("http_port").length() > 0))
    {
      int nHTTPPort = atoi(pRequest->GetPostVar("http_port").c_str());      
      CSharedConfig::Shared()->SetHTTPPort(nHTTPPort);
    }
    
    /* add allowed ip */
    if(pRequest->PostVarExists("new_allowed_ip") && (pRequest->GetPostVar("new_allowed_ip").length() > 0))
    {     
      CSharedConfig::Shared()->AddAllowedIP(pRequest->GetPostVar("new_allowed_ip"));
    }
    
    /* remove allowed ip */
    for(int i = CSharedConfig::Shared()->AllowedIPCount() - 1; i >= 0; i--)
    {
      sVar << "allowed_ip_" << i;      
      if(pRequest->PostVarExists(sVar.str()))      
        CSharedConfig::Shared()->RemoveAllowedIP(i);      
      sVar.str("");
    }        
  }
  
  
  /* show config page */
  sResult << "<h1>ContentDirectory settings</h1>" << endl;
  // shared dirs
  sResult << "<h2>shared directories</h2>" << endl;
  sResult << "<form method=\"POST\" action=\"/presentation/config.html\" accept-charset=\"UTF-8\" enctype=\"text/plain\">" << endl;
  
  // directory list
  sResult << "<p>" << endl <<  
             "<table>" << endl <<
               "<thead>" << endl <<
                 "<tr>" <<
                   "<th>Del</th>" <<
                   "<th>Directory</th>" <<
                 "</tr>" <<
               "</thead>" << endl <<
               "<tbody>" << endl;
  for(unsigned int i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++)
  {
    sResult << "<tr>" << endl;    
    sResult << "<td><input type=\"checkbox\" name=\"shared_dir_" << i << "\" value=\"remove\"></td>" << endl;   
    sResult << "<td>" << CSharedConfig::Shared()->GetSharedDir(i) << "</td>" << endl;
    sResult << "</tr>" << endl;
  }
  
  sResult <<   "</tbody>" << endl <<
             "</table>" << endl <<  
             "</p>" << endl;
  
  // "add new form" controls
  sResult << "<p>" <<  
               "Add directory: <input name=\"new_dir\" />" << endl <<
               "<br />" << endl <<
               "<input type=\"submit\" />" << endl <<             
             "</p>" << endl;  
  
  // playlist representation
  sResult << "<h2>playlist representation</h2>" << endl;
  sResult << "<p>Choose how playlist items are represented. <br />\"file\" sends playlists as real playlist files (m3u or pls)<br />" <<
             "\"container\" represents playlists as containers including the playlist items.<br />" << endl;
             if(CSharedConfig::Shared()->GetDisplaySettings().bShowPlaylistsAsContainers)  
             {
               sResult << "<input type=\"radio\" name=\"playlist_representation\" value=\"file\"> file<br />" << endl <<
                          "<input type=\"radio\" name=\"playlist_representation\" value=\"container\" checked=\"checked\"> container<br />" << endl;
             }
             else
             {
               sResult << "<input type=\"radio\" name=\"playlist_representation\" value=\"file\" checked=\"checked\"> file<br />" << endl <<
                          "<input type=\"radio\" name=\"playlist_representation\" value=\"container\"> container<br />" << endl;               
             }
             
  sResult << "<input type=\"submit\" />" << endl <<             
             "</p>" << endl; 
  
             
  // charset
  sResult << "<h2>character encoding</h2>" << endl;
  sResult << "<p>Set your local character encoding.<br />" <<
             "<a href=\"http://www.gnu.org/software/libiconv/\" target=\"blank\">http://www.gnu.org/software/libiconv/</a><br />" << endl <<
             "</p>" << endl;
             
  sResult << "<p>" << endl <<
             "<input name=\"local_charset\" value=\"" << CSharedConfig::Shared()->GetLocalCharset() << "\"/><br />" << endl;
  sResult << "<input type=\"submit\" />" << endl <<             
             "</p>" << endl;              
             
  
  // max filename length
  sResult << "<h2>max file name length</h2>" << endl;
  sResult << "<p>The \"max file name length\" option sets the maximum length for file names in the directory listings.<br />" <<
             "some devices can't handle an unlimited length.<br />" << endl <<
             "(e.g. the Telegent TG 100 crashes on receiving file names larger then 101 characters.)<br />" << endl <<
             "0 or empty means unlimited length. a value greater 0 sets the maximum number of characters to be sent.</p>" << endl;
  //sResult << "<p><strong>Max file name length: " << CSharedConfig::Shared()->GetMaxFileNameLength() << "</strong></p>" << endl;  
  
    sResult << "<p>" <<  
               "<input name=\"max_file_name_length\" value=\"" << CSharedConfig::Shared()->GetMaxFileNameLength() << "\" />" << endl <<
               "<br />" << endl <<
               "<input type=\"submit\" />" << endl <<             
             "</p>" << endl;  
  
  
  sResult << "<h1>Network settings</h1>" << endl;
  
  sResult << "<h2>IP address/HTTP port</h2>" << endl;  
  sResult << "<p>" <<  
               "<input name=\"ip_address\" value=\"" << CSharedConfig::Shared()->GetIPv4Address() << "\" />" << endl <<  
               "<input name=\"http_port\" value=\"" << CSharedConfig::Shared()->GetHTTPPort() << "\" />" << endl <<
               "<br />" << endl <<
               "<input type=\"submit\" />" << endl <<             
             "</p>" << endl;  
  
  sResult << "<h2>Allowed IP-addresses</h2>" << endl;
  
  // allowed ip list
  sResult << "<p>" << endl <<  
             "host address is always allowed to access." << endl <<
             "</p>" << endl <<
             "<p>" << endl <<
             "<table>" << endl <<
               "<thead>" << endl <<
                 "<tr>" <<
                   "<th>Del</th>" <<
                   "<th>IP-Address</th>" <<
                 "</tr>" <<
               "</thead>" << endl <<
               "<tbody>" << endl;
  for(unsigned int i = 0; i < CSharedConfig::Shared()->AllowedIPCount(); i++)
  {
    sResult << "<tr>" << endl;    
    sResult << "<td><input type=\"checkbox\" name=\"allowed_ip_" << i << "\" value=\"remove\"></td>" << endl;   
    sResult << "<td>" << CSharedConfig::Shared()->GetAllowedIP(i) << "</td>" << endl;
    sResult << "</tr>" << endl;
  }
  
  sResult <<   "</tbody>" << endl <<
             "</table>" << endl <<  
             "</p>" << endl;  
  
  sResult << "<p>" <<  
               "<input name=\"new_allowed_ip\" />" << endl <<
               "<br />" << endl <<
               "<input type=\"submit\" />" << endl <<             
             "</p>" << endl;    
  
  sResult << "</form>";
  
  return sResult.str();  
}

/*===============================================================================
 HELPER
===============================================================================*/

/* BuildFuppesDeviceList */
std::string CPresentationHandler::BuildFuppesDeviceList(CFuppes* pFuppes, std::string p_sImgPath)
{
  stringstream sResult;

  for(unsigned int i = 0; i < pFuppes->GetRemoteDevices().size(); i++)
  {
    CUPnPDevice* pDevice = pFuppes->GetRemoteDevices()[i];
    
    sResult << 
      "<table rules=\"all\" cellspacing=\"0\" width=\"400\">" <<
        "<thead>";
    /*sResult << "<tr><th colspan=\"2\"><a href=\"javascript:Klappen(" << i << ")\"><img src=\"plus.gif\" id=\"Pic" << i << "\" border=\"0\">x</a> ";
    sResult<< pDevice->GetFriendlyName() << "</th></tr>" << endl;*/
    sResult << "<tr>" << endl <<
               "<th colspan=\"2\" style=\"background-image: url(" << p_sImgPath << "header-gradient-small.png); " <<
               "color: #FFFFFF;\">" <<
               //"<a href=\"javascript:Klappen(" << i << ")\"><!--<img src=\"plus.gif\" id=\"Pic" << i << "\" border=\"0\">-->x</a> " <<
               pDevice->GetFriendlyName() << "</th></tr>" << endl;
    sResult << "</thead>" << endl;
    
    //sResult << "<tbody id=\"Remote" << i << "\" style=\"display: none;\">" << endl;
    sResult << "<tbody>" << endl;
    
    
    sResult << "<tr><td>Type</td><td>" << pDevice->GetDeviceTypeAsString() << "</td></tr>" << endl;
    sResult << "<tr><td>UUID</td><td>" << pDevice->GetUUID() << "</td></tr>" << endl;
    sResult << "<tr><td>Time Out</td><td style=\"border-style: solid; border-width: 1px;\">" << pDevice->GetTimer()->GetCount() / 60 << "min. " << pDevice->GetTimer()->GetCount() % 60 << "sec.</td></tr>" << endl;
    //sResult << "<tr><td>Status</td><td>"   << "<i>todo</i>" << "</td></tr>" << endl;
    
    sResult << 
        "<tbody>" <<
      "</table><br />"  << endl;    
  }
  
  return sResult.str();
}

/* <\PRIVATE> */
