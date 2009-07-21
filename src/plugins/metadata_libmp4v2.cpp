/***************************************************************************
 *            metadata_libmp4v2.cpp
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

#include <mp4.h>
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "mp4v2");
	strcpy(info->plugin_author, "Ulrich Voelkel");
	info->plugin_type = PT_METADATA;
}

int fuppes_metadata_file_open(plugin_info* plugin, const char* fileName)
{
	MP4FileHandle mp4file = MP4Read(fileName);
	if(!mp4file) {
		return -1;
	}	
	MP4Close(mp4file);

	plugin->user_data = malloc((strlen(fileName)+1) * sizeof(char));
	strcpy((char*)plugin->user_data, fileName);
	
	return 0;
}

int fuppes_metadata_read(plugin_info* plugin, metadata_t* metadata)
{
	MP4FileHandle mp4file = MP4Read((const char*)plugin->user_data);
	if(!mp4file) {
		return -1;
	}

	char* value;
	
	// title
	MP4GetMetadataName(mp4file, &value);
	if(value) {
		set_value(&metadata->title, value);
		free(value);
	}
	
	// duration
	u_int32_t scale = MP4GetTimeScale(mp4file);//scale is ticks in secs, used same value in duration.
	MP4Duration length = MP4GetDuration(mp4file);
	int hours, mins, secs;
	length = length /scale;
	secs = length % 60;
	length /= 60;
	mins = length % 60;
	hours = length / 60;

	char szDuration[12];
	sprintf(szDuration, "%02d:%02d:%02d.00", hours, mins, secs);
	szDuration[11] = '\0';
	
	set_value(&metadata->duration, szDuration);
	
	// channels
	metadata->channels = MP4GetTrackAudioChannels(mp4file, 
																		MP4FindTrackId(mp4file, 0, MP4_AUDIO_TRACK_TYPE));
	// bitrate
	metadata->bitrate = MP4GetTrackBitRate(mp4file, 
														MP4FindTrackId(mp4file, 0, MP4_AUDIO_TRACK_TYPE));
	metadata->bits_per_sample = 0;

	// artist
	MP4GetMetadataArtist(mp4file, &value);
	if(value) {
		set_value(&metadata->artist, value);
		free(value);
	}
	
	// genre
	MP4GetMetadataGenre(mp4file, &value);
	if(value) {
		set_value(&metadata->genre, value);
		free(value);
	}
	
	// Album
	MP4GetMetadataAlbum(mp4file, &value);
	if(value) {
		set_value(&metadata->album, value);
		free(value);
	}
	
	// description/comment
	MP4GetMetadataComment(mp4file, &value);
	if(value) {
		set_value(&metadata->description, value);
		free(value);
	}

	// track no.
	u_int16_t track, totaltracks;
	MP4GetMetadataTrack(mp4file, &track, &totaltracks);
	metadata->track_no = track;

	// date/year
	/*MP4GetMetadataYear(mp4file, &value);
	if(value)	{
		pMusicTrack->nYear = atoi(value);
		free(value);
	}*/

	MP4Close(mp4file);
	
	return 0;
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	free(plugin->user_data);
	plugin->user_data = NULL;
}

void unregister_fuppes_plugin(plugin_info* plugin __attribute__((unused)))
{
}

#ifdef __cplusplus
}
#endif
