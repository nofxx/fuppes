/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            DLNA.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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


#include "DLNA.h"

#include "dlna_image_profiles.h"
#include "dlna_audio_profiles.h"
#include "dlna_video_profiles.h"

bool DLNA::getImageProfile(std::string ext, int width, int height, std::string& dlnaProfile, std::string& mimeType) // static
{
	if(width == 0 || height == 0) {
		return false;
	}

  bool result = false;
	if((ext.compare("jpeg") == 0) || (ext.compare("jpg") == 0)) {
		result = dlna_get_image_profile_jpeg(width, height, dlnaProfile, mimeType);
	}	
	else if(ext.compare("png") == 0) {
		result = dlna_get_image_profile_png(width, height, dlnaProfile, mimeType);
	}
  
  return result;
}

bool DLNA::getAudioProfile(std::string ext, int channels, int bitrate, std::string& dlnaProfile, std::string& mimeType) // static
{
  bitrate /= 1024; // kbits

  bool result = false;
  
	if(ext.compare("mp3") == 0) {
		result = dlna_get_audio_profile_mp3(channels, bitrate, dlnaProfile, mimeType);
	}	
	else if(ext.compare("wma") == 0) {
		result = dlna_get_audio_profile_wma(channels, bitrate, dlnaProfile, mimeType);
	}
	else if(ext.compare("m4a") == 0) {
    result = dlna_get_audio_profile_mpeg4(channels, bitrate, dlnaProfile, mimeType);
	}
	else if(ext.compare("ac3") == 0) {
		result = dlna_get_audio_profile_ac3(channels, bitrate, dlnaProfile, mimeType);
	}
  
  return result;
}

bool DLNA::getVideoProfile(std::string ext, std::string vcodec, std::string acodec, std::string& dlnaProfile, std::string& mimeType) // static
{
  bool result = false;

	if((ext.compare("mpg") == 0) || (ext.compare("mpeg") == 0)) {

    if(vcodec.compare("mpeg1video") == 0) {    
		  result = dlna_get_video_profile_mpeg1(vcodec, acodec, dlnaProfile, mimeType);
    }

    // mpeg2video
    // mpeg4
    // msmpeg4
    // msmpeg4v1
    // msmpeg4v2
    // h264
	}	
	else if(ext.compare("avi") == 0) {		
	}
	else if(ext.compare("wmv") == 0) {		
    // wmv1, wmv2, wmv3
	}
	else if(ext.compare("mp4") == 0) {
	}
	else if(ext.compare("mkv") == 0) {
	}

  return result;
}


