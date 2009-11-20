/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            encoder_pcm.c
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
#include "../../include/fuppes_types.h"


#ifdef __cplusplus
extern "C" {
#endif
		
#include <string.h>

typedef struct {
	
	int	 samplerate;
	int	 channels;
	int	 numSamples;

	char* buffer;
	int		bufferSize;
	
} pcmSettings_t;

void register_fuppes_plugin(plugin_info* plugin)
{
	strcpy(plugin->plugin_name, "pcm");
	strcpy(plugin->plugin_author, "Ulrich Voelkel");
	plugin->plugin_type = PT_AUDIO_ENCODER;
}

void unregister_fuppes_plugin(plugin_info* plugin)
{
	if(((pcmSettings_t*)plugin->user_data)->buffer) {
		free(((pcmSettings_t*)plugin->user_data)->buffer);
		((pcmSettings_t*)plugin->user_data)->buffer = NULL;
		((pcmSettings_t*)plugin->user_data)->bufferSize = 0;
	}
}


void fuppes_encoder_set_audio_settings(plugin_info* plugin, audio_settings_t* settings)
{
	if(plugin->user_data == NULL) {
		plugin->user_data = malloc(sizeof(pcmSettings_t));

		((pcmSettings_t*)plugin->user_data)->buffer = NULL;
		((pcmSettings_t*)plugin->user_data)->bufferSize = 0;

	}

	pcmSettings_t* tmp = (pcmSettings_t*)plugin->user_data;	
	
	tmp->samplerate = settings->samplerate;
	tmp->channels		= settings->channels;
	tmp->numSamples = settings->num_samples;
}

int fuppes_encoder_encode_interleaved(plugin_info* plugin, char* pcm, int numSamples, int numBytes)
{
	pcmSettings_t* tmp = (pcmSettings_t*)plugin->user_data;

	if(tmp->buffer == NULL) {
    tmp->buffer = (unsigned char*)malloc(numBytes * sizeof(unsigned char*));
		tmp->bufferSize = numBytes;
	}
	else {
		if(tmp->bufferSize < numBytes) {
      tmp->buffer = (unsigned char*)realloc(tmp->buffer, numBytes * sizeof(unsigned char*));
      tmp->bufferSize = numBytes;
    }
	}

  memcpy(tmp->buffer, pcm, numBytes);  
  return numBytes;	
}



char* fuppes_encoder_get_buffer(plugin_info* plugin)
{
	pcmSettings_t* tmp = (pcmSettings_t*)plugin->user_data;
	return tmp->buffer;
}

fuppes_off_t fuppes_encoder_guess_content_length(plugin_info* plugin, unsigned int numSamples)
{
	return 0;
}


#ifdef __cplusplus
}
#endif

