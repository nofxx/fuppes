/***************************************************************************
 *            fuppes_plugin_types.h
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


#ifdef __cplusplus
extern "C" {
#endif

typedef enum tagPLUGIN_TYPE {
	PT_NONE,
	PT_DLNA,
	PT_METADATA,
	PT_AUDIO_DECODER,
	PT_AUDIO_ENCODER,
	PT_TRANSCODER,
	PT_THREADED_TRANSCODER
} PLUGIN_TYPE;

typedef struct {
	char		ext[10];
	void*		next;
} file_ext;

typedef struct {
	char					plugin_name[200];
	PLUGIN_TYPE		plugin_type;
	void*					user_data;
} plugin_info;


// METADATA

typedef enum tagMETADATA_TYPE {
	MD_NONE,
	MD_AUDIO,
	MD_VIDEO,
	MD_IMAGE
} METADATA_TYPE;

typedef struct {
	METADATA_TYPE		type;
	char*						title;
	char* 					artist;
	char*						album;
	char*						genre;
	char*						duration;
	char*						description;
	int							track_no;
	int							year;
	int							channels;

	int							has_image;
	char*						image_mime_type;
	
	int							width;
	int							height;
	
	int							bitrate;
	int 						samplerate;
  int 						bits_per_sample;
	
	char*						audio_codec;
	char*						video_codec;
} metadata_t;

static void init_metadata(metadata_t* metadata)
{
	metadata->type = MD_NONE;
	
	metadata->title = (char*)malloc(sizeof(char));
	metadata->title[0] = '\0';
	metadata->artist = (char*)malloc(sizeof(char));
	metadata->artist[0] = '\0';
	metadata->album = (char*)malloc(sizeof(char));
	metadata->album[0] = '\0';
	metadata->genre = (char*)malloc(sizeof(char));
	metadata->genre[0] = '\0';
	metadata->duration = (char*)malloc(sizeof(char));
	metadata->duration[0] = '\0';
	metadata->description = (char*)malloc(sizeof(char));
	metadata->description[0] = '\0';
	
	metadata->track_no = 0;
	metadata->year = 0;
	metadata->channels = 0;
	
	metadata->has_image = 0;
	metadata->image_mime_type = (char*)malloc(sizeof(char));
	metadata->image_mime_type[0] = '\0';
	
	metadata->width = 0;
	metadata->height = 0;
	
	metadata->bitrate = 0;
	metadata->samplerate = 0;
  metadata->bits_per_sample = 0;
	
	metadata->audio_codec = (char*)malloc(sizeof(char));
	metadata->audio_codec[0] = '\0';	
	metadata->video_codec = (char*)malloc(sizeof(char));
	metadata->video_codec[0] = '\0';
}

static void free_metadata(metadata_t* metadata)
{
	free(metadata->title);
	free(metadata->artist);
	free(metadata->album);
	free(metadata->genre);
	free(metadata->duration);
	free(metadata->description);
	
	free(metadata->audio_codec);
	free(metadata->video_codec);
	
	free(metadata->image_mime_type);
}

#ifdef __cplusplus
}
#endif
