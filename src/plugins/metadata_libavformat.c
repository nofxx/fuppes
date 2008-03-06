/***************************************************************************
 *            metadata_libavformat.c
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

#include <avformat.h>
#include <avcodec.h>
#include <string.h>
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "libavformat");
	info->plugin_type = PT_METADATA;
	
	av_register_all();
}

int fuppes_metadata_file_open(plugin_info* plugin, const char* fileName)
{
	plugin->user_data = NULL;
	
	if(av_open_input_file(&plugin->user_data, fileName, NULL, 0, NULL) != 0) {
		return -1;
	}
		
	if(av_find_stream_info((AVFormatContext*)plugin->user_data) < 0) {
		av_close_input_file((AVFormatContext*)plugin->user_data);
	  return -1;
	}
	
	return 0;
}

int fuppes_metadata_read(plugin_info* plugin, metadata_t* metadata)
{
	// duration
	if(((AVFormatContext*)plugin->user_data)->duration != AV_NOPTS_VALUE) {
 
	  int hours, mins, secs, us;
		secs = ((AVFormatContext*)plugin->user_data)->duration / AV_TIME_BASE;
		us   = ((AVFormatContext*)plugin->user_data)->duration % AV_TIME_BASE;
		mins = secs / 60;
		secs %= 60;
		hours = mins / 60;
		mins %= 60;
	
		char szDuration[12];
	  sprintf(szDuration, "%02d:%02d:%02d.%02d", hours, mins, secs, (10 * us) / AV_TIME_BASE);
	  szDuration[11] = '\0';
		
		set_value(&metadata->duration, szDuration);
	}
	
	// bitrate
	if(((AVFormatContext*)plugin->user_data)->bit_rate)
  	metadata->bitrate = ((AVFormatContext*)plugin->user_data)->bit_rate / 8;

  // filesize  
	//pVideoItem->nSize = pFormatCtx->file_size;	
  
	char codec_name[128];
	char buf1[32];
	int i;
	for(i = 0; i < ((AVFormatContext*)plugin->user_data)->nb_streams; i++) {
	  
		AVStream* pStream = ((AVFormatContext*)plugin->user_data)->streams[i];
	  AVCodec* pCodec;
				
		pCodec = avcodec_find_decoder(pStream->codec->codec_id);
		if(!pCodec) {
			continue;
		}
		
		strncpy(codec_name, (char*)pCodec->name, sizeof(codec_name));
		
/*    if(pStream->codec->codec_id == CODEC_ID_MP3) {
    
			if(pStream->codec->sub_id == 2)
				strncpy(codec_name, "mp2", sizeof(codec_name));
      else if (pStream->codec->sub_id == 1)
				strncpy(codec_name, "mp1", sizeof(codec_name));
    }
    else if(pStream->codec->codec_id == CODEC_ID_MPEG2TS) {
      // fake mpeg2 transport stream codec (currently not registered)
			strncpy(codec_name, "mpeg2ts", sizeof(codec_name));
    } 
		else if(pStream->codec->codec_name[0] != '\0') {
			strncpy(codec_name, pStream->codec->codec_name, sizeof(codec_name));
    }
		else {
      // output avi tags 
      if(isprint(pStream->codec->codec_tag&0xFF) && 
				 isprint((pStream->codec->codec_tag>>8)&0xFF) &&
				 isprint((pStream->codec->codec_tag>>16)&0xFF) &&
				 isprint((pStream->codec->codec_tag>>24)&0xFF)) {
           
					 snprintf(buf1, sizeof(buf1), "%c%c%c%c / 0x%04X",
                    pStream->codec->codec_tag & 0xff,
                    (pStream->codec->codec_tag >> 8) & 0xff,
                    (pStream->codec->codec_tag >> 16) & 0xff,
                    (pStream->codec->codec_tag >> 24) & 0xff,
                     pStream->codec->codec_tag);
      } 
			else {
         snprintf(buf1, sizeof(buf1), "0x%04x", pStream->codec->codec_tag);
      }
			strncpy(codec_name, buf1, sizeof(codec_name));
		} */

		switch(pStream->codec->codec_type) {
			case CODEC_TYPE_VIDEO:
				metadata->type		= MD_VIDEO;
				metadata->width 	= pStream->codec->width;
				metadata->height	= pStream->codec->height;
				set_value(&metadata->video_codec, codec_name);
				break;
			case CODEC_TYPE_AUDIO:
				set_value(&metadata->audio_codec, codec_name);
				if(metadata->type == MD_NONE) {
					metadata->type = MD_AUDIO;
				}
				break;
			case CODEC_TYPE_DATA:
				break;
			case CODEC_TYPE_SUBTITLE:
				break;
			default:
				break;
		} // switch (codec_type)
		
	}	// for

	return 0;
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	av_close_input_file(plugin->user_data);
	plugin->user_data = NULL;
}

void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif
