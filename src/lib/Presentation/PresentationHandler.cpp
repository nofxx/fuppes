/***************************************************************************
 *            PresentationHandler.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */
 
#include "PresentationHandler.h"
#include "Stylesheet.h"
#include "../SharedConfig.h"
#include "../SharedLog.h"
#include "../Common/Common.h"
#include "../ContentDirectory/ContentDatabase.h"
#include "../ContentDirectory/VirtualContainerMgr.h"
#include "../ContentDirectory/FileDetails.h"
#include "../Transcoding/TranscodingMgr.h"
#include "../HTTP/HTTPParser.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"

//#include "Images/fuppes_png.cpp"
#include "Images/fuppes_small_png.cpp"
#include "Images/header_gradient_png.cpp"
#include "Images/header_gradient_small_png.cpp"
#include "Images/device_type_unknown_png.cpp"
#include "Images/device_type_media_server_png.cpp"

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <sstream>

#include <fstream>
#include <iostream>

const std::string LOGNAME = "PresentationHandler"; 


CPresentationHandler::CPresentationHandler()
{
}


CPresentationHandler::~CPresentationHandler()
{
}



void CPresentationHandler::OnReceivePresentationRequest(CHTTPMessage* pMessage, CHTTPMessage* pResult)
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
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/options.html") == 0) {
    nPresentationPage = PRESENTATION_PAGE_OPTIONS;
    sContent = this->GetOptionsHTML(sImgPath);
    sPageName = "Options";
  }
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/options.html?db=rebuild") == 0) {
    CSharedConfig::Shared()->Refresh();
    if(!CContentDatabase::Shared()->IsRebuilding())
      CContentDatabase::Shared()->RebuildDB();

    nPresentationPage = PRESENTATION_PAGE_OPTIONS;
    sContent = this->GetOptionsHTML(sImgPath);
    sPageName = "Options";
  }
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/options.html?db=update") == 0) {
    CSharedConfig::Shared()->Refresh();
    if(!CContentDatabase::Shared()->IsRebuilding() && !CVirtualContainerMgr::Shared()->IsRebuilding())
      CContentDatabase::Shared()->UpdateDB();

    nPresentationPage = PRESENTATION_PAGE_OPTIONS;
    sContent = this->GetOptionsHTML(sImgPath);
    sPageName = "Options";
  }
	else if(ToLower(pMessage->GetRequest()).compare("/presentation/options.html?vcont=rebuild") == 0) {
    CSharedConfig::Shared()->Refresh();
    if(!CContentDatabase::Shared()->IsRebuilding() && !CVirtualContainerMgr::Shared()->IsRebuilding())
      CVirtualContainerMgr::Shared()->RebuildContainerList();

    nPresentationPage = PRESENTATION_PAGE_OPTIONS;
    sContent = this->GetOptionsHTML(sImgPath);
    sPageName = "Options";
  }
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/status.html") == 0) {
    nPresentationPage = PRESENTATION_PAGE_STATUS;
    sContent = this->GetStatusHTML(sImgPath);
    sPageName = "Status";
  }
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/config.html") == 0) {
    nPresentationPage = PRESENTATION_PAGE_STATUS;
    sContent = this->GetConfigHTML(sImgPath, pMessage);
    sPageName = "Configuration";
  }        
  
  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/fuppes-small.png") == 0) {
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(fuppes_small_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());
  }  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/header-gradient.png") == 0) {
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(header_gradient_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());
  }  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/header-gradient-small.png") == 0) {
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(header_gradient_small_png);    
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());
  }  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/device-type-unknown.png") == 0) {
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(device_type_unknown_png);
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());
  }  
  else if(ToLower(pMessage->GetRequest()).compare("/presentation/images/device-type-media-server.png") == 0) {
    nPresentationPage = PRESENTATION_BINARY_IMAGE;
    string sImg = Base64Decode(device_type_media_server_png);
    pResult->SetBinContent((char*)sImg.c_str(), sImg.length());
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
    pResult->SetContentType("text/html; charset=\"utf-8\""); // HTTP_CONTENT_TYPE_TEXT_HTML
    pResult->SetContent(sResult.str());    
  }  
  else if(nPresentationPage == PRESENTATION_PAGE_UNKNOWN) 
  {
    pResult->SetMessageType(HTTP_MESSAGE_TYPE_404_NOT_FOUND); 
    pResult->SetContentType("text/html");
  }
}


std::string CPresentationHandler::GetPageHeader(PRESENTATION_PAGE p_nPresentationPage, std::string p_sImgPath, std::string p_sPageName)
{
  std::stringstream sResult; 
	 
  /* header */
  sResult << "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n";
  sResult << "<head>";  
  sResult << "<title>" << CSharedConfig::Shared()->GetAppName() << " - " << CSharedConfig::Shared()->GetAppFullname() << " " << CSharedConfig::Shared()->GetAppVersion();
  sResult << " (" << CSharedConfig::Shared()->GetHostname() << ")";
  sResult << "</title>" << endl;

	//sResult << "<meta http-equiv=\"Content-Script-Type\" content=\"text/javascript\">" << endl;
	sResult << "<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\">" << endl;
  sResult << "<meta http-equiv=\"Content-Style-Type\" content=\"text/css\">" << endl;  

	
  // stylesheet
	sResult << "<style type=\"text/css\">" << endl << GetStylesheet(p_sImgPath) << endl << "</style>";

	
  
  /*sResult << "<script type=\"text/javascript\"> \r\n"
    "function Toggle(Id) { \r\n"
    "  var remote = document.getElementById('Remote'+Id);"
    "  var img    = document.getElementById('Pic'+Id);\r\n"
    "  if (remote.style.display == 'none') { \r\n"
    "    remote.style.display = '';"
    "    img.src = \"minus.gif\";"
    "  }"
    "  else {"
    "    remote.style.display = 'none';"
    "    img.src = \"plus.gif\";"
    "  }"
    "} \r\n";
    
  sResult << 
    "function CloseAll(Count) { "
    "  var i = 0; "
    "  for(i = 0; i < Count; i++) { "
    "    Toggle(i); "
    "  } "
    "}"; */
    
  //sResult << "</script>";  
  sResult << "</head>";
  /* header end */
  
    
  sResult << "<body>";
  
  //pFuppes->GetRemoteDevices().size()
  
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
  
  sResult << "<p style=\"padding-top: 20pt; text-align: center;\"><small>copyright &copy; 2005 - 2007 Ulrich V&ouml;lkel<!--<br />distributed under the GPL--></small></p>";

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
  sResult << "build at: " << __DATE__ << " - " << __TIME__ "<br />" << endl;
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
  if(!CContentDatabase::Shared()->IsRebuilding() && !CVirtualContainerMgr::Shared()->IsRebuilding())  {
    sResult << "<a href=\"/presentation/options.html?db=rebuild\">rebuild database</a><br />" << endl;
    sResult << "<a href=\"/presentation/options.html?db=update\">update database</a><br />" << endl;
		sResult << "<a href=\"/presentation/options.html?vcont=rebuild\">rebuild virtual container</a>" << endl;
  }
  else {
		if(CContentDatabase::Shared()->IsRebuilding())
			sResult << "database rebuild/update in progress" << endl;
		else if(CVirtualContainerMgr::Shared()->IsRebuilding())
			sResult << "virtual container rebuild in progress" << endl;
	}
  
  return sResult.str();
}

std::string CPresentationHandler::GetStatusHTML(std::string p_sImgPath)
{
  std::stringstream sResult;  
  
  //sResult << "<h2>Status</h2>" << endl;
  //*p_psImgPath = "Status";
  
  //sResult
  CContentDatabase* pDb = new CContentDatabase();
  std::stringstream sSQL;
  sSQL << "select TYPE, count(*) as VALUE from OBJECTS group by TYPE;";
  pDb->Select(sSQL.str());
  OBJECT_TYPE nType = OBJECT_TYPE_UNKNOWN;  
  
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
  
  while (!pDb->Eof())
  {
    sResult << "<tr>" << endl;
    
    nType = (OBJECT_TYPE)atoi(pDb->GetResult()->GetValue("TYPE").c_str());
    sResult << "<td>" << CFileDetails::Shared()->GetObjectTypeAsStr(nType) << "</td>" << endl;
    sResult << "<td>" << pDb->GetResult()->GetValue("VALUE") << "</td>" << endl;
    pDb->Next();
    
    sResult << "</tr>" << endl;
  }
  
  sResult <<
      "</tbody>" << endl <<   
    "</table>" << endl;

  delete pDb;
  // end Database status
  
  
  string sTranscoding;
  sResult << "<h1>transcoding</h1>";
  CTranscodingMgr::Shared()->PrintTranscodingSettings(&sTranscoding);
  sResult << sTranscoding;
  
  
  
  sResult << "<h1>build options</h1>" <<
  "<table>" <<    
    "<tr>" <<
      "<th>option</th>" <<
      "<th>enabled</th>" <<
    "</tr>" <<
    "<tr>" <<
      "<td>iconv</td>" <<
      #ifdef HAVE_ICONV
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif
    "</tr>" <<
    #ifndef WIN32
    "<tr>" <<
      "<td>uuid</td>" <<
      #ifdef HAVE_UUID
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif
    "</tr>" <<
    #endif
    "<tr>" <<
      "<td>taglib</td>" <<
      #ifdef HAVE_TAGLIB
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif     
    "</tr>" <<
    "<tr>" <<
      "<td>imageMagick</td>" <<
      #ifdef HAVE_IMAGEMAGICK
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif      
    "</tr>" <<
    "<tr>" <<
      "<td>libavformat (ffmpeg)</td>" <<
      #ifdef HAVE_LIBAVFORMAT
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif    
    "</tr>" <<   
    "<tr>" <<
      "<td>video transcoding (experimental)</td>" <<
      #ifdef ENABLE_VIDEO_TRANSCODING
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif    
    "</tr>" <<   
    "<tr>" <<
      "<td>lame</td>" <<
      #ifdef HAVE_LAME
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif      
    "</tr>" << 
    "<tr>" <<
      "<td>twolame</td>" <<
      #ifdef HAVE_TWOLAME
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif     
    "</tr>" <<  
    "<tr>" <<
      "<td>ogg/vorbis</td>" <<
      #ifdef HAVE_VORBIS
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif     
    "</tr>" << 
    "<tr>" <<
      "<td>musepack</td>" <<
      #ifdef HAVE_MUSEPACK
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif     
    "</tr>" <<
    "<tr>" <<
      "<td>flac</td>" <<
      #ifdef HAVE_FLAC
      "<td>true</td>" <<
      #else
      "<td>false</td>" <<
      #endif     
    "</tr>" <<    
    "<tr>" <<
      "<td>flac</td>" <<
      #ifdef HAVE_FAAD
      #ifdef HAVE_MP4FF_H
      "<td>true (aac/mp4)</td>" <<
      #else
      "<td>true (aac/NO mp4)</td>" <<
      #endif
      #else
      "<td>false</td>" <<
      #endif     
    "</tr>" <<    
  "</table>";    
  
  // system status
  sResult << "<h1>system status</h1>" << endl;  
  
  sResult << "<p>" << endl;
  sResult << "UUID: " << CSharedConfig::Shared()->GetFuppesInstance(0)->GetUUID() << "<br />";    
  sResult << "</p>" << endl;
  // end system status
  
  
  // device settings
  sResult << "<h1>device settings</h1>" << endl;
  string sDeviceSettings;
  CDeviceIdentificationMgr::Shared()->PrintSettings(&sDeviceSettings);
  sResult << sDeviceSettings << endl;
  // end device settings
  
  return sResult.str();  
}


std::string CPresentationHandler::GetConfigHTML(std::string p_sImgPath, CHTTPMessage* pRequest)
{
  std::stringstream sResult;  

  // handle config changes
  if(pRequest->GetMessageType() == HTTP_MESSAGE_TYPE_POST)
  {
  	//cout << pRequest->GetMessage() << endl;
	
		CHTTPParser* pParser = new CHTTPParser();
		pParser->ConvertURLEncodeContentToPlain(pRequest);
		delete pParser;	
	
    // remove shared objects(s)
    stringstream sVar;
    for(int i = CSharedConfig::Shared()->SharedDirCount() - 1; i >= 0; i--) {
      sVar << "shared_dir_" << i;      
      if(pRequest->PostVarExists(sVar.str()))      
        CSharedConfig::Shared()->RemoveSharedDirectory(i);      
      sVar.str("");
    }    
    
    for(int i = CSharedConfig::Shared()->SharedITunesCount() - 1; i >= 0; i--) {
      sVar << "shared_itunes_" << i;      
      if(pRequest->PostVarExists(sVar.str()))      
        CSharedConfig::Shared()->RemoveSharedITunes(i);      
      sVar.str("");
    }    
    
    // add shared object
    if(pRequest->PostVarExists("new_obj") && (pRequest->GetPostVar("new_obj").length() > 0))
    {     
      if(pRequest->GetPostVar("new_obj_type").compare("dir") == 0) {
        CSharedConfig::Shared()->AddSharedDirectory(pRequest->GetPostVar("new_obj"));
      }
      else if(pRequest->GetPostVar("new_obj_type").compare("itunes") == 0) {
        CSharedConfig::Shared()->AddSharedITunes(pRequest->GetPostVar("new_obj"));
      }
    }

    // local_charset
    if(pRequest->PostVarExists("local_charset")) {
      CSharedConfig::Shared()->SetLocalCharset(pRequest->GetPostVar("local_charset"));
    }    
    
    /* playlist_representation */
    /*if(pRequest->PostVarExists("playlist_representation"))
    {
      CSharedConfig::Shared()->SetPlaylistRepresentation(pRequest->GetPostVar("playlist_representation"));
    }*/
    
    /* max_file_name_length */
    /*if(pRequest->PostVarExists("max_file_name_length") && (pRequest->GetPostVar("max_file_name_length").length() > 0))
    {
      int nMaxFileNameLength = atoi(pRequest->GetPostVar("max_file_name_length").c_str());      
      CSharedConfig::Shared()->SetMaxFileNameLength(nMaxFileNameLength);
    }*/
    
    // ip address
    if(pRequest->PostVarExists("net_interface") && (pRequest->GetPostVar("net_interface").length() > 0)) {
      CSharedConfig::Shared()->SetNetInterface(pRequest->GetPostVar("net_interface"));
    }
    
    // http port
    if(pRequest->PostVarExists("http_port") && (pRequest->GetPostVar("http_port").length() > 0)) {
      int nHTTPPort = atoi(pRequest->GetPostVar("http_port").c_str());      
      CSharedConfig::Shared()->SetHTTPPort(nHTTPPort);
    }
    
    // add allowed ip
    if(pRequest->PostVarExists("new_allowed_ip") && (pRequest->GetPostVar("new_allowed_ip").length() > 0)) {     
      CSharedConfig::Shared()->AddAllowedIP(pRequest->GetPostVar("new_allowed_ip"));
    }
    
    // remove allowed ip
    for(int i = CSharedConfig::Shared()->AllowedIPCount() - 1; i >= 0; i--) {
      sVar << "allowed_ip_" << i;      
      if(pRequest->PostVarExists(sVar.str()))      
        CSharedConfig::Shared()->RemoveAllowedIP(i);      
      sVar.str("");
    }        
  }
  
  
  sResult << "<strong>Device and file settings are currently not configurable via this webinterface.<br />";
  sResult << "Please edit the file: " << CSharedConfig::Shared()->GetConfigFileName() << "</strong>" << endl;
  
  /* show config page */
  sResult << "<h1>ContentDirectory settings</h1>" << endl;
  sResult << "<form method=\"POST\" action=\"/presentation/config.html\" enctype=\"text/plain\" accept-charset=\"UTF-8\">" << endl;  //  
  
  // shared objects
  sResult << "<h2>shared objects</h2>" << endl;
  
  // object list
  sResult << "<p>" << endl <<  
             "<table>" << endl <<
               "<thead>" << endl <<
                 "<tr>" <<
                   "<th>del</th>" <<
                   "<th>type</th>" <<
                   "<th>object</th>" <<
                 "</tr>" <<
               "</thead>" << endl <<
               "<tbody>" << endl;
  
  // dirs
  int i;
  for(i = 0; i < CSharedConfig::Shared()->SharedDirCount(); i++) {
    sResult << "<tr>" << endl;    
    sResult << "<td><input type=\"checkbox\" name=\"shared_dir_" << i << "\" value=\"remove\"></td>" << endl;
    sResult << "<td>dir</td>" << endl;
    sResult << "<td>" << CSharedConfig::Shared()->GetSharedDir(i) << "</td>" << endl;
    sResult << "</tr>" << endl;
  }
  
  // itunes
  for(i = 0; i < CSharedConfig::Shared()->SharedITunesCount(); i++) {
    sResult << "<tr>" << endl;    
    sResult << "<td><input type=\"checkbox\" name=\"shared_itunes_" << i << "\" value=\"remove\"></td>" << endl;   
    sResult << "<td>iTunes</td>" << endl;    
    sResult << "<td>" << CSharedConfig::Shared()->GetSharedITunes(i) << "</td>" << endl;
    sResult << "</tr>" << endl;
  }  
  
  sResult <<   "</tbody>" << endl <<
             "</table>" << endl <<  
             "</p>" << endl;
  
  // "add new form" controls
  sResult << "<p>" <<  
               "Add objects: <input name=\"new_obj\" /><br />" << endl <<
               "<input type=\"radio\" name=\"new_obj_type\" value=\"dir\" checked=\"checked\" />directory " <<
               "<input type=\"radio\" name=\"new_obj_type\" value=\"itunes\" />iTunes db<br />" <<    
               "<input type=\"submit\" />" << endl <<             
             "</p>" << endl;
  
  // playlist representation
  /*sResult << "<h2>playlist representation</h2>" << endl;
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
             "</p>" << endl; */
  
             
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
  /*sResult << "<h2>max file name length</h2>" << endl;
  sResult << "<p>The \"max file name length\" option sets the maximum length for file names in the directory listings.<br />" <<
             "some devices can't handle an unlimited length.<br />" << endl <<
             "(e.g. the Telegent TG 100 crashes on receiving file names larger then 101 characters.)<br />" << endl <<
             "0 or empty means unlimited length. a value greater 0 sets the maximum number of characters to be sent.</p>" << endl;
  //sResult << "<p><strong>Max file name length: " << CSharedConfig::Shared()->GetMaxFileNameLength() << "</strong></p>" << endl;  
  
    sResult << "<p>" <<  
               "<input name=\"max_file_name_length\" value=\"" << CSharedConfig::Shared()->GetMaxFileNameLength() << "\" />" << endl <<
               "<br />" << endl <<
               "<input type=\"submit\" />" << endl <<             
             "</p>" << endl;  */
  
  
  sResult << "<h1>Network settings</h1>" << endl;
  
  sResult << "<h2>IP address or network interface name (e.g. eth0, wlan1, ...)/HTTP port</h2>" << endl;  
  sResult << "<p>" <<  
               "<input name=\"net_interface\" value=\"" << CSharedConfig::Shared()->GetNetInterface() << "\" />" << endl <<  
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
  for(unsigned int i = 0; i < CSharedConfig::Shared()->AllowedIPCount(); i++) {
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
    sResult << "<tr><th colspan=\"2\" style=\"background-image: url(" << p_sImgPath << "header-gradient-small.png); color: #FFFFFF;\">" <<
                 
                 /*"<div style=\"float: left;\">";
                 switch(pDevice->GetDeviceType())
                 {
                   case UPNP_DEVICE_TYPE_MEDIA_SERVER:
                     sResult << "<img src=\"" << p_sImgPath << "device-type-media-server.png\" />";
                     break;
                   default:
                     sResult << "<img src=\"" << p_sImgPath << "device-type-unknown.png\" />";                  
                 }                      
                 sResult << "</div>" <<  */       
    
                 /*"<div style=\"float: right;\">" <<
                 "<a href=\"javascript:Toggle(" << i << ")\"><!--<img src=\"plus.gif\" id=\"Pic" << i << "\" border=\"0\">-->x</a> " <<
                 "</div>" << */
    
                 pDevice->GetFriendlyName() <<
               "</th></tr>" << endl;
    sResult << "</thead>" << endl;
    
    sResult << "<tbody id=\"Remote" << i << "\">" << endl; // style=\"display: none;\"
    //sResult << "<tbody>" << endl;
    
    
    sResult << "<tr><td>Type</td><td>" << pDevice->GetUPnPDeviceTypeAsString() << "</td></tr>" << endl;
    sResult << "<tr><td>UUID</td><td>" << pDevice->GetUUID() << "</td></tr>" << endl;
    sResult << "<tr><td>Time Out</td><td style=\"border-style: solid; border-width: 1px;\">" << pDevice->GetTimer()->GetCount() / 60 << "min. " << pDevice->GetTimer()->GetCount() % 60 << "sec.</td></tr>" << endl;
    //sResult << "<tr><td>Status</td><td>"   << "<i>todo</i>" << "</td></tr>" << endl;
    
    sResult << 
        "<tbody>" <<
      "</table><br />"  << endl;    
  }
  
  return sResult.str();
}
