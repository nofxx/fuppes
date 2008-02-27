/***************************************************************************
 *            metadata_taglib.cpp
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

#include <fileref.h>
#include <tfile.h>
#include <tag.h>

#include <iostream>
using namespace std;


int taglib_open_file(plugin_info* info, const char* fileName)
{
	info->user_data = new TagLib::FileRef(fileName);
	if(((TagLib::FileRef*)info->user_data)->isNull()) {
		delete (TagLib::FileRef*)info->user_data;
		info->user_data = NULL;
		return -1;
	}
	return 0;
}

void taglib_get_title(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->title();
	set_value(&audio->title, sTmp.to8Bit(true).c_str());
}

void taglib_get_artist(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->artist();
	set_value(&audio->artist, sTmp.to8Bit(true).c_str());
}

void taglib_get_album(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->album();
	set_value(&audio->album, sTmp.to8Bit(true).c_str());
}

void taglib_get_genre(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->genre();
	set_value(&audio->genre, sTmp.to8Bit(true).c_str());
}

void taglib_get_comment(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->comment();
	set_value(&audio->description, sTmp.to8Bit(true).c_str());
}

void taglib_get_duration(plugin_info* info, metadata_t* metadata)
{
	TagLib::String sTmp;

	long length = ((TagLib::FileRef*)info->user_data)->audioProperties()->length();  
  int hours, mins, secs;
    
  secs  = length % 60;
  length /= 60;
  mins  = length % 60;
  hours = length / 60;  

  char szDuration[12];
	sprintf(szDuration, "%02d:%02d:%02d.00", hours, mins, secs);
	szDuration[11] = '\0';

	set_value(&metadata->duration, szDuration);
}

void taglib_get_channels(plugin_info* info, metadata_t* metadata)
{
	metadata->channels = 
		((TagLib::FileRef*)info->user_data)->audioProperties()->channels();
}

void taglib_get_track_no(plugin_info* info, metadata_t* metadata)
{
	metadata->track_no = 
		((TagLib::FileRef*)info->user_data)->tag()->track();
}

void taglib_get_year(plugin_info* info, metadata_t* metadata)
{
	metadata->year = 
		((TagLib::FileRef*)info->user_data)->tag()->year();
}

void taglib_get_bitrate(plugin_info* info, metadata_t* metadata)
{
	metadata->bitrate = 
		(((TagLib::FileRef*)info->user_data)->audioProperties()->bitrate() * 1024);
}

void taglib_get_samplerate(plugin_info* info, metadata_t* metadata)
{
	metadata->samplerate = 
		((TagLib::FileRef*)info->user_data)->audioProperties()->sampleRate();
}

void taglib_close_file(plugin_info* plugin)
{
	delete (TagLib::FileRef*)plugin->user_data;
	plugin->user_data = NULL;
}


#ifdef __cplusplus
extern "C" {
#endif

void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "taglib");
	info->plugin_type = PT_METADATA;

	/*add_extension(info, "mp3");
	add_extension(info, "ogg");
	add_extension(info, "flac");
	add_extension(info, "mpc");*/
}

int fuppes_metadata_file_open(plugin_info* plugin, const char* fileName)
{
	if(taglib_open_file(plugin, fileName) != 0)
		return -1;
		
	return 0;
}

int fuppes_metadata_read(plugin_info* plugin, metadata_t* metadata)
{
	metadata->type = MD_AUDIO;
	
	taglib_get_title(plugin, metadata);
	taglib_get_artist(plugin, metadata);
	taglib_get_album(plugin, metadata);
	taglib_get_genre(plugin, metadata);	
	taglib_get_comment(plugin, metadata);	
	taglib_get_duration(plugin, metadata);
	taglib_get_channels(plugin, metadata);
	taglib_get_track_no(plugin, metadata);
	taglib_get_year(plugin, metadata);
	taglib_get_bitrate(plugin, metadata);
	taglib_get_samplerate(plugin, metadata);

	metadata->bits_per_sample = 0;

	return 0;
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	taglib_close_file(plugin);
}

void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif
