/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            decoder_flac.c
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
		
#ifdef HAVE_FLAC_FILEDECODER // <= 1.1.2
#include <FLAC/file_decoder.h>
#else // >= 1.1.3
#include <FLAC/stream_decoder.h>
#endif
		
typedef struct flacData_t {
	ENDIANESS				outEndianess;
} flacData_t;
		
void register_fuppes_plugin(plugin_info* plugin)
{
	strcpy(plugin->plugin_name, "FLAC");
	plugin->plugin_type = PT_AUDIO_DECODER;
}

int fuppes_decoder_file_open(plugin_info* plugin, const char* fileName, audio_settings_t* settings)
{	
	plugin->user_data = malloc(sizeof(struct flacData_t));
	flacData_t* data = (flacData_t*)plugin->user_data;
	
	return 1;
}

int fuppes_decoder_set_out_endianess(plugin_info* plugin, ENDIANESS endianess)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	data->outEndianess = endianess;
	return 0;
}

int fuppes_decoder_total_samples(plugin_info* plugin)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	return 0;
}

int fuppes_decoder_decode_interleaved(plugin_info* plugin, char* pcmOut, int bufferSize, int* bytesRead)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	return -1;
}

void fuppes_decoder_file_close(plugin_info* plugin)
{
	flacData_t* data = (flacData_t*)plugin->user_data;
	
	
	free(plugin->user_data);
	plugin->user_data = NULL;
}
		
void unregister_fuppes_plugin(plugin_info* info)
{
}

		
#ifdef __cplusplus
}
#endif
