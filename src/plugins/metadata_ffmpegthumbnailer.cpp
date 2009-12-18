/* -*- Mode: C++; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            metadata_ffmpegthumbnailer.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include <videothumbnailer.h>
#include <imagetypes.h>

#include <vector>
#include <iostream>
#include <exception>
using namespace std;


int read_image(const char* fileName, char** mimeType, unsigned char** buffer, size_t* size)
{
	std::vector<uint8_t>	data;

	try {
		VideoThumbnailer thumbnailer(160, false, true, 8, true);	
		//thumbnailer.setSeekPercentage(44);
		thumbnailer.generateThumbnail(fileName, Jpeg, data);
	}
	catch (std::exception ex) {
		return -1;
	}

	*size = sizeof(uint8_t) * data.size();
	*buffer = (unsigned char*)realloc(*buffer, *size);
	
	size_t offset = 0;
	//for(int i = 0; i < data.size(); i++) {
		memcpy(&(*buffer)[offset], &data.at(0), sizeof(uint8_t) * data.size());
		/*offset += sizeof(uint8_t);
	}	*/
	return 0;
}


#ifdef __cplusplus
extern "C" {
#endif


typedef struct {
	char* fileName;
} ffmpegthumbnailer_t;
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "ffmpegthumbnailer");
	strcpy(info->plugin_author, "Ulrich Voelkel");
	info->plugin_type = PT_METADATA;
}

int fuppes_metadata_file_open(plugin_info* plugin, const char* fileName)
{		
	plugin->user_data = malloc(sizeof(ffmpegthumbnailer_t));
	((ffmpegthumbnailer_t*)plugin->user_data)->fileName = (char*)malloc(strlen(fileName) + 1);
	strcpy(((ffmpegthumbnailer_t*)plugin->user_data)->fileName, fileName);
	return 0;
}

int fuppes_metadata_read(plugin_info* plugin, metadata_t* metadata)
{
	return -1;
}

int fuppes_metadata_read_image(plugin_info* plugin, char** mimeType, unsigned char** buffer, size_t* size)
{
	return read_image(((ffmpegthumbnailer_t*)plugin->user_data)->fileName, mimeType, buffer, size);
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	free(((ffmpegthumbnailer_t*)plugin->user_data)->fileName);
	free(plugin->user_data);
}

void unregister_fuppes_plugin(plugin_info* plugin __attribute__((unused)))
{
}

#ifdef __cplusplus
}
#endif
