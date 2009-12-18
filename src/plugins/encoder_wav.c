/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            encoder_wav.c
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
	
} wavSettings_t;

void register_fuppes_plugin(plugin_info* plugin)
{
	strcpy(plugin->plugin_name, "wav");
	strcpy(plugin->plugin_author, "Ulrich Voelkel");
	plugin->plugin_type = PT_AUDIO_ENCODER;
}

void unregister_fuppes_plugin(plugin_info* plugin)
{
	if(plugin->user_data == NULL)
		return;
	
	if(((wavSettings_t*)plugin->user_data)->buffer) {
		free(((wavSettings_t*)plugin->user_data)->buffer);
		((wavSettings_t*)plugin->user_data)->buffer = NULL;
		((wavSettings_t*)plugin->user_data)->bufferSize = 0;
	}
}

int fuppes_plugin_init_instance(plugin_info* plugin)
{
	return 1;
}

void fuppes_plugin_uninit_instance(plugin_info* plugin)
{
}


void fuppes_encoder_set_audio_settings(plugin_info* plugin, audio_settings_t* settings)
{
	if(plugin->user_data == NULL) {
		plugin->user_data = malloc(sizeof(wavSettings_t));

		((wavSettings_t*)plugin->user_data)->buffer = NULL;
		((wavSettings_t*)plugin->user_data)->bufferSize = 0;

	}

	wavSettings_t* tmp = (wavSettings_t*)plugin->user_data;	
	
	tmp->samplerate = settings->samplerate;
	tmp->channels		= settings->channels;
	tmp->numSamples = settings->num_samples;
}


/*
  most of the following lines are taken from oggdec.c
  from the vorbis-tools-1.1.1 package (GPLv2)  
*/

#define WRITE_U32(buf, x) *(buf)     = (unsigned char)((x)&0xff);\
                          *((buf)+1) = (unsigned char)(((x)>>8)&0xff);\
                          *((buf)+2) = (unsigned char)(((x)>>16)&0xff);\
                          *((buf)+3) = (unsigned char)(((x)>>24)&0xff);

#define WRITE_U16(buf, x) *(buf)     = (unsigned char)((x)&0xff);\
                          *((buf)+1) = (unsigned char)(((x)>>8)&0xff);

void create_wav_header(unsigned char* header, wavSettings_t* settings)
{
  int bits = 16;  
  unsigned int size = -1;
  int channels    = settings->channels;
  int samplerate  = settings->samplerate;  
  int bytespersec = channels * samplerate * bits / 8;
  int align       = channels * bits / 8;
  int samplesize  = bits;
  
  unsigned int knownlength = settings->numSamples;
  
  if(knownlength && knownlength*bits/8*channels < size)
    size = (unsigned int)(knownlength*bits/8*channels+44);
  
  memcpy(header, "RIFF", 4);
  WRITE_U32(header+4, size-8);
  memcpy(header+8, "WAVE", 4);
  memcpy(header+12, "fmt ", 4);
  WRITE_U32(header+16, 16);
  WRITE_U16(header+20, 1); /* format */
  WRITE_U16(header+22, channels);
  WRITE_U32(header+24, samplerate);
  WRITE_U32(header+28, bytespersec);
  WRITE_U16(header+32, align);
  WRITE_U16(header+34, samplesize);
  memcpy(header+36, "data", 4);
  WRITE_U32(header+40, size - 44);  
}



int fuppes_encoder_encode_interleaved(plugin_info* plugin, char* pcm, int numSamples, int numBytes)
{
	wavSettings_t* tmp = (wavSettings_t*)plugin->user_data;

	int offset = 0;

	// first call. let's create the buffer and write the wav header
	if(tmp->buffer == NULL) {
    tmp->buffer = (unsigned char*)malloc(numBytes * sizeof(unsigned char*) + 44);
		tmp->bufferSize = numBytes + 44;

		create_wav_header(tmp->buffer, tmp);		
    offset = 44;
	}
	else {
		if(tmp->bufferSize < numBytes) {
      tmp->buffer = (unsigned char*)realloc(tmp->buffer, numBytes * sizeof(unsigned char*));
      tmp->bufferSize = numBytes;
    }
	}

  memcpy(&tmp->buffer[offset], pcm, numBytes);  
  return numBytes;	
}



char* fuppes_encoder_get_buffer(plugin_info* plugin)
{
	wavSettings_t* tmp = (wavSettings_t*)plugin->user_data;
	return tmp->buffer;
}

fuppes_off_t fuppes_encoder_guess_content_length(plugin_info* plugin, unsigned int numSamples)
{
	return 0;
}


#ifdef __cplusplus
}
#endif

