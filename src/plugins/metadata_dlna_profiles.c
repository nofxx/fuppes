/***************************************************************************
 *            metadata_dlna_profiles.c
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
#include "dlna/dlna_image_profiles.h"

#ifdef __cplusplus
extern "C" {
#endif

void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "dlna-profiles");
	strcpy(info->plugin_author, "Ulrich Voelkel");
	info->plugin_type = PT_DLNA;
}

int fuppes_dlna_get_image_profile(const char* ext, int width, int height, char* dlnaProfile, char* mimeType)
{
	if(width == 0 || height == 0) {
		return -1;
	}
	
	if((strcmp(ext, "jpeg") == 0) || (strcmp(ext, "jpg") == 0)) {
		
		if(dlna_get_image_profile_jpeg(width, height, dlnaProfile, mimeType) == 0) {
			return 0;
		}		
	}	
	else if(strcmp(ext, "png") == 0) {
		
		if(dlna_get_image_profile_png(width, height, dlnaProfile, mimeType) == 0) {
			return 0;
		}
	}
	
	// no other image formats are defined by dlna	
	return -1;
}

void unregister_fuppes_plugin(plugin_info* plugin __attribute__((unused)))
{
}

#ifdef __cplusplus
}
#endif
