/***************************************************************************
 *            transcoder_ffmpeg.c
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

#include "FFmpegWrapper.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <string.h>
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "ffmpeg");
	info->plugin_type = PT_TRANSCODER;
	
	av_register_all();
}


int fuppes_transcoder_transcode(plugin_info* plugin, 
																const char* audioCodec, 
																const char* videoCodec
																)
{
	return -1;
}


/*bool Init(std::string p_sACodec, std::string p_sVCodec);    
    bool Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile);
    bool Threaded() { return true; }*/


void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif
