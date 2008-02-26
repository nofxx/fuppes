/***************************************************************************
 *            metadata_simage.c
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifdef __cplusplus
extern "C" {
#endif

#include <simage.h>
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "simage");
	info->plugin_type = PT_METADATA;
}

int fuppes_metadata_file_open(plugin_info* plugin, const char* fileName)
{
	
	if(simage_check_supported(fileName) == 1) {	
		plugin->user_data = malloc((strlen(fileName)+1) * sizeof(char));
		strcpy(plugin->user_data, fileName);
		return 0;
	}
	else {
		return -1;
	}
}

int fuppes_metadata_read(plugin_info* plugin, metadata_t* metadata)
{
	metadata->type = MD_IMAGE;
	
	unsigned char* img;
	int width;
	int height;
	int numComponents;
		
	img = simage_read_image((const char*)plugin->user_data, &width, &height, &numComponents);
	if(img == NULL) {
		return -1;
	}
	simage_free_image(img);
	
	metadata->width  = width;
	metadata->height = height;
	
	return 0;
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	free(plugin->user_data);
	plugin->user_data = NULL;
}

void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif
