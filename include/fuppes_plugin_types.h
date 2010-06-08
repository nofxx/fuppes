/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            fuppes_plugin_types.h
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


#ifdef __cplusplus
extern "C" {
#endif

#ifndef _FUPPES_PLUGIN_TYPES_H
#define _FUPPES_PLUGIN_TYPES_H



typedef enum PLUGIN_TYPE {
	PT_NONE,
	PT_DLNA,
	PT_METADATA,
	PT_AUDIO_DECODER,
	PT_AUDIO_ENCODER,
	PT_TRANSCODER,
	PT_THREADED_TRANSCODER,
	PT_PRESENTATION,
	PT_DATABASE_CONNECTION
} PLUGIN_TYPE;

typedef struct {
	char		ext[10];
	void*		next;
} file_ext;


// stores a list for key/value pairs
// it's used to represent a single dataset
// e.g. the description of a shared directory
// idx, type, path
typedef struct {
	char*		key;
	char*		value;
	
	void*		next;
} arg_list_t;

// stores a list of arg_list_t instances
// if the args store the description of a single shared dir
// this one holds the list of all shared dirs
typedef struct {
	arg_list_t* arg;

	void* next;
} result_set_t;




typedef int (*ctrl_action_t)(const char* action, arg_list_t* args, arg_list_t* result);


typedef void (*log_t)(void* plugin, int level, const char* file, int line, const char* format, ...);

typedef struct {
	log_t					log;
	ctrl_action_t	ctrl;
} plugin_callbacks;

typedef struct {
	char							plugin_name[200];
	char							plugin_author[1000];
	PLUGIN_TYPE				plugin_type;
	char							plugin_version[100];
	char							library_version[100];
	void*							user_data;
	plugin_callbacks	cb;
} plugin_info;



// METADATA

typedef enum METADATA_TYPE {
	MD_NONE,
	MD_AUDIO,
	MD_VIDEO,
	MD_IMAGE
} METADATA_TYPE;

typedef enum METADATA_PLUGIN_CAPABILITIES {
	MDC_UNKNOWN		= 0,
	MDC_METADATA	= 1 << 0,
	MDC_IMAGE			= 1 << 1
} METADATA_PLUGIN_CAPABILITIES;

typedef struct {
	METADATA_TYPE		type;
	char*						title;
	char* 					artist;
	char*						album;
	char*						genre;
	unsigned int		duration_ms;
	char*						description;
	int							track_no;
	int							year;
	int							channels;

	int							has_image;
	char*						image_mime_type;
	
	int							width;
	int							height;
	char*						date;
    	
	int							bitrate;
	int 						samplerate;
  int 						bits_per_sample;
	
	char*						audio_codec;
	char*						video_codec;
} metadata_t;


// AUDIO

typedef enum ENDIANESS {
  E_LITTLE_ENDIAN = 0,
  E_BIG_ENDIAN = 1
} ENDIANESS;


typedef struct {	
	int					channels;
  int					samplerate;
  int    			bitrate;
  int					num_samples;	
} audio_settings_t;


// PRESENTATION



#endif // _FUPPES_PLUGIN_TYPES_H

#ifdef __cplusplus
}
#endif
