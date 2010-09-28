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
#include "dlna/dlna_audio_profiles.h"
#include "dlna/dlna_video_profiles.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
  
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "dlna-profiles");
	strcpy(info->plugin_author, "Ulrich Voelkel");
	info->plugin_type = PT_DLNA;
}

int fuppes_dlna_get_image_profile(const char* ext, int width, int height, char* dlnaProfile, char* mimeType)
{

  printf("[dlna] get image profile for ext: %s, width: %d, height: %d\n", ext, width, height);
  
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


int fuppes_dlna_get_audio_profile(const char* ext, int channels, int bitrate, char* dlnaProfile, char* mimeType)
{
  printf("[dlna] get audio profile for ext: %s, channels: %d, bitrate: %d\n", ext, channels, bitrate);

  bitrate /= 1024; // kbits
  
	if(strcmp(ext, "mp3") == 0) {
		
		if(dlna_get_audio_profile_mp3(channels, bitrate, dlnaProfile, mimeType) == 0) {
			return 0;
		}		
	}	
	else if(strcmp(ext, "wma") == 0) {
		
		if(dlna_get_audio_profile_wma(channels, bitrate, dlnaProfile, mimeType) == 0) {
			return 0;
		}
	}
	else if(strcmp(ext, "m4a") == 0) {
		
		if(dlna_get_audio_profile_mpeg4(channels, bitrate, dlnaProfile, mimeType) == 0) {
			return 0;
		}
	}
	else if(strcmp(ext, "ac3") == 0) {
		
		if(dlna_get_audio_profile_ac3(channels, bitrate, dlnaProfile, mimeType) == 0) {
			return 0;
		}
	}
  
	// no other audio formats are defined by dlna	
	return -1;
}


int fuppes_dlna_get_video_profile(const char* ext, char* vcodec, char* acodec, char* dlnaProfile, char* mimeType)
{
  printf("[dlna] get video profile for ext: %s, vcodec: %s, acodec: %s\n", ext, vcodec, acodec);
  
	if((strcmp(ext, "mpg") == 0) || (strcmp(ext, "mpeg") == 0)) {

    if(strcmp(vcodec, "mpeg1video") == 0) {    
		  if(dlna_get_video_profile_mpeg1(vcodec, acodec, dlnaProfile, mimeType) == 0) {
			  return 0;
		  }
    }

    // mpeg2video
    // mpeg4
    // msmpeg4
    // msmpeg4v1
    // msmpeg4v2
    // h264
	}	
	else if(strcmp(ext, "avi") == 0) {		
	}
	else if(strcmp(ext, "wmv") == 0) {		
    // wmv1, wmv2, wmv3
	}
	else if(strcmp(ext, "mp4") == 0) {
	}
	else if(strcmp(ext, "mkv") == 0) {
	}

	return -1;
}


void unregister_fuppes_plugin(plugin_info* plugin __attribute__((unused)))
{
}

#ifdef __cplusplus
}
#endif
