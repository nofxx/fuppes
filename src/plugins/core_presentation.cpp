/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            core_presentation.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#include "../../include/fuppes_plugin.h"

#include <string>
#include <sstream>
#include <iostream>

class Presentation
{
	public:
		Presentation(plugin_info* pluginInfo) {
			plugin = pluginInfo;
			
			getCommonValues();
		}
		
		int toString(char** szResult);
	
	private:
		plugin_info*	plugin;

		std::string		dataDir;
		
		std::string		version;
		std::string		friendlyName;
		std::string		hostname;
		std::string		ipAddress;
		
		void getCommonValues();
};


void Presentation::getCommonValues()
{
	int ret;
	arg_list_t* resultArgs = create_arg_list();
	
	ret = plugin->ctrl("get_version", NULL, resultArgs);
	if(ret == 0) {
		version = resultArgs->value;
	}
	
	ret = plugin->ctrl("get_friendly_name", NULL, resultArgs);
	if(ret == 0) {
		friendlyName = resultArgs->value;
	}

	ret = plugin->ctrl("get_data_dir", NULL, resultArgs);
	if(ret == 0) {
		dataDir = resultArgs->value;
	}

	ret = plugin->ctrl("get_hostname", NULL, resultArgs);
	if(ret == 0) {
		hostname = resultArgs->value;
	}
	
	ret = plugin->ctrl("get_ip_address", NULL, resultArgs);
	if(ret == 0) {
		ipAddress = resultArgs->value;
	}
	
	free_arg_list(resultArgs);
}


int Presentation::toString(char** szResult)
{
	std::stringstream result;
	
	result << 
			"<?xml version=\"1.0\" encoding=\"UTF-8\"?>" <<
			"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">" <<
			"<html xmlns=\"http://www.w3.org/1999/xhtml\">" <<
			"<head>" <<
		
			"<meta http-equiv=\"content-type\" content=\"text/html; charset=UTF-8\" />" <<
			"<meta http-equiv=\"Content-Style-Type\" content=\"text/css\" />" <<
			"<link href=\"/presentation/style.css\" rel=\"stylesheet\" type=\"text/css\" media=\"screen\" />" <<
		
			"</head>" <<
		"<body>";		
	
	/* title */
  result << 
		"<div id=\"title\">" <<
		"<img src=\"/presentation/fuppes-small.png\" style=\"float: left; margin-top: 10px; margin-left: 5px;\" />" <<
		"<p>" <<
    "FUPPES - Free UPnP Entertainment Service<br />" <<
    "<span>" <<
    "Version: " << version << " &bull; " <<
    "Host: "    << hostname << " &bull; " <<
    "Address: " << ipAddress <<
    "</span>" <<
    "</p>"<<
		"</div>" << std::endl;
  /* title end */
  
  /* menu */
  result << 
		"<div id=\"menu\">" <<
		"<div id=\"framehead\">Menu</div>" <<
    "<ul>" <<
      "<li><a href=\"/index.html\">Start</a></li>" <<
      "<li><a href=\"/presentation/options.html\">Options</a></li>" <<
      "<li><a href=\"/presentation/status.html\">Status</a></li>" <<
      "<li><a href=\"/presentation/config.html\">Configuration</a></li>" <<
    "</ul>" <<
		"</div>" << std::endl;
  /* menu end */


  result << "<div id=\"mainframe\">" << std::endl;
  result << "<div id=\"framehead\">" << "PAGENAME" << "</div>" << std::endl;  
  result << "<div id=\"content\">" << std::endl;	
	
	
	result << "<p>version: " << version << "</p>";
	result << "<p>friendlyName: " << friendlyName << "</p>";	
	result << "<p>dataDir: " << dataDir << "</p>";		
		
	
	result << 
		"<p style=\"padding-top: 20pt; text-align: center;\"><small>copyright &copy; 2005-2009 Ulrich V&ouml;lkel</small></p>" <<
		"</div>" << // #content
		"</div>" << std::endl; // #mainframe
  
	result <<	"</body>" <<
		"</html>";
	
	//std::cout << result.str() << std::endl;	
	return set_value(szResult, result.str().c_str());
}



void handle_request(plugin_info* plugin,
									 const std::string url, arg_list_t* get, arg_list_t* post,
									 int* error, char** mime_type, char** result, int* length)
{
	Presentation pres(plugin);
	
	*error = 200;
	if(url.compare("/") == 0 || url.compare("/index.html") == 0) {
		*length = pres.toString(result);
		set_value(mime_type, "text/html; charset=\"utf-8\"");
	}	
	/*else if(url.compare("/presentation/style.css")) {
		set_value(mime_type, "text/css");
	}*/
	else {
		*error = 404;
	}
	
}


#ifdef __cplusplus
extern "C" {
#endif

void register_fuppes_plugin(plugin_info* plugin)
{
	strcpy(plugin->plugin_name, "presentation");
	plugin->plugin_type = PT_PRESENTATION;
}

int fuppes_presentation_handle_request(plugin_info* plugin,
									 const char* url, arg_list_t* get, arg_list_t* post,
									 int* error, char** mime_type, char** result, int* length)
{
	//plugin->log(0, __FILE__, __LINE__, "request URL: %s", url);	
	
	handle_request(plugin, url, get, post,
								 error, mime_type, result, length);	
	
	if((*error) == 200) {
		//printf("RETURN 0 %s\n", url);
		return 0;
	}
	//printf("RETURN 1 %s\n", url);
	return 1;
}

void unregister_fuppes_plugin(plugin_info* plugin)
{
}
	
#ifdef __cplusplus
}
#endif
