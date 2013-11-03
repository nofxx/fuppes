/* -*- Mode: C; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            metadata_libavformat.c
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008-2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#if FFMPEG_VERSION >= 52 && !defined(OLD_INCLUDES_PATH)
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavcodec/avcodec.h>
#else
#include <avformat.h>
#include <avutil.h>
#include <avcodec.h>
#endif
  
#include <string.h>
#include <stdarg.h>


/*void av_log_callback(void* ptr, int* level, const char* fmt, va_list vl)
{
}*/
    
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "libavformat");
	strcpy(info->plugin_author, "Ulrich Voelkel");
	info->plugin_type = PT_METADATA;

  //av_log_set_callback(&av_log_callback);
#if LIBAVUTIL_VERSION_INT < (50 << 16)
  av_log_level = AV_LOG_QUIET;
#else
  av_log_set_level(AV_LOG_QUIET);
#endif

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

int fuppes_metadata_read(plugin_info* plugin, struct metadata_t* metadata)
{
  AVFormatContext* ctx = (AVFormatContext*)plugin->user_data;
  
	// duration
	if(((AVFormatContext*)plugin->user_data)->duration != AV_NOPTS_VALUE) {
	  int secs, us;
		secs = ((AVFormatContext*)plugin->user_data)->duration / AV_TIME_BASE;
		us   = ((AVFormatContext*)plugin->user_data)->duration % AV_TIME_BASE;
    metadata->duration_ms = secs * 1000;
	}
	
	// bitrate
	if(((AVFormatContext*)plugin->user_data)->bit_rate)
  	metadata->video_bitrate = ((AVFormatContext*)plugin->user_data)->bit_rate;

  // filesize  
	//pVideoItem->nSize = pFormatCtx->file_size;	
  
	//char codec_name[128];
	//char buf1[32];
	int i;
	for(i = 0; i < ((AVFormatContext*)plugin->user_data)->nb_streams; i++) {
	  
		AVStream* pStream = ((AVFormatContext*)plugin->user_data)->streams[i];
	  AVCodec* pCodec;
				
		pCodec = avcodec_find_decoder(pStream->codec->codec_id);
		if(!pCodec) {
			continue;
		}
		
		//strncpy(codec_name, (char*)pCodec->name, sizeof(codec_name));

    printf("codec %s*\n", pCodec->name);
    
		switch(pStream->codec->codec_type) {
			case AVMEDIA_TYPE_VIDEO:
				metadata->type		= MD_VIDEO;
				metadata->width 	= pStream->codec->width;
				metadata->height	= pStream->codec->height;
				set_value(metadata->video_codec, sizeof(metadata->video_codec), pCodec->name);
				break;
			case AVMEDIA_TYPE_AUDIO:
				set_value(metadata->audio_codec, sizeof(metadata->audio_codec), pCodec->name);
				if(metadata->type == MD_NONE) {
					metadata->type = MD_AUDIO;
				}
				break;
			case AVMEDIA_TYPE_DATA:
				break;
			case AVMEDIA_TYPE_SUBTITLE:
				break;
			default:
				break;
		} // switch (codec_type)
    
	}	// for


  // metadata
  // here is a nice list of possible metadata keys:
  // http://multimedia.cx/eggs/supplying-ffmpeg-with-metadata/
  if(ctx->metadata) {

    // convert tag names to generic ffmpeg names
    av_metadata_conv(ctx, NULL, ctx->iformat->metadata_conv);
    
    AVMetadataTag* tag = NULL;

    while((tag = av_metadata_get(ctx->metadata, "", tag, AV_METADATA_IGNORE_SUFFIX))) {      
      printf("%s => %s\n", tag->key, tag->value);
    }
    
    // title
    if((tag = av_metadata_get(ctx->metadata, "title", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: TITLE: %s\n", tag->value);
    	set_value(metadata->title, sizeof(metadata->title), tag->value);      
    }

    // genre
    if((tag = av_metadata_get(ctx->metadata, "genre", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: GENRE: %s\n", tag->value);
    	set_value(metadata->genre, sizeof(metadata->genre), tag->value);      
    }

    // track
    if((tag = av_metadata_get(ctx->metadata, "track", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: TRACK: %s\n", tag->value);
    	//set_value(&metadata->genre, tag->value);      
    }

    // comment
    if((tag = av_metadata_get(ctx->metadata, "comment", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: comment: %s\n", tag->value);
    	set_value(metadata->description, sizeof(metadata->description), tag->value);      
    }

    // composer
    if((tag = av_metadata_get(ctx->metadata, "composer", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: composer: %s\n", tag->value);
    	set_value(metadata->composer, sizeof(metadata->composer), tag->value);      
    }
    
    // date
    if((tag = av_metadata_get(ctx->metadata, "date", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: date: %s\n", tag->value);
    	set_value(metadata->date, sizeof(metadata->date), tag->value);      
    }

    // language
    if((tag = av_metadata_get(ctx->metadata, "language", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: language: %s\n", tag->value);
    	set_value(metadata->language, sizeof(metadata->language), tag->value);      
    }

    // performer
    if((tag = av_metadata_get(ctx->metadata, "performer", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: performer: %s\n", tag->value);
    	//set_value(&metadata->language, tag->value);      
    }

    // publisher
    if((tag = av_metadata_get(ctx->metadata, "publisher", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: publisher: %s\n", tag->value);
    	set_value(metadata->publisher, sizeof(metadata->publisher), tag->value);      
    }

    // show
    if((tag = av_metadata_get(ctx->metadata, "show", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: show: %s\n", tag->value);
    	set_value(metadata->series_title, sizeof(metadata->series_title), tag->value);      
    }

    // episode_id
    if((tag = av_metadata_get(ctx->metadata, "episode_id", NULL, AV_METADATA_IGNORE_SUFFIX))) {
      printf("libavformat metadata: episode_id: %s\n", tag->value);
    	set_value(metadata->program_title, sizeof(metadata->program_title), tag->value);      
    }
    
  }
  

  
	return 0;
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	av_close_input_file(plugin->user_data);
	plugin->user_data = NULL;
}

void unregister_fuppes_plugin(plugin_info* plugin __attribute__((unused)))
{
}

#ifdef __cplusplus
}
#endif
