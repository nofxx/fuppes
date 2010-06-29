/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            metadata_taglib.cpp
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

#include <fileref.h>
#include <tfile.h>
#include <tag.h>
#include <mpegfile.h>
#include <id3v2tag.h>
#include <attachedpictureframe.h>

#include <iostream>
using namespace std;


int taglib_open_file(plugin_info* info, const char* fileName)
{
	info->user_data = new TagLib::FileRef(fileName);
	if(((TagLib::FileRef*)info->user_data)->isNull() ||
     (((TagLib::FileRef*)info->user_data)->audioProperties()) == NULL) {
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
	set_value(audio->title, sizeof(audio->title), sTmp.to8Bit(true).c_str());
}

void taglib_get_artist(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->artist();
	set_value(audio->artist, sizeof(audio->artist), sTmp.to8Bit(true).c_str());
}

void taglib_get_album(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->album();
	set_value(audio->album, sizeof(audio->album), sTmp.to8Bit(true).c_str());
}

void taglib_get_genre(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->genre();
	set_value(audio->genre, sizeof(audio->genre), sTmp.to8Bit(true).c_str());
}

void taglib_get_comment(plugin_info* plugin, metadata_t* audio)
{	
	TagLib::String sTmp =
		((TagLib::FileRef*)plugin->user_data)->tag()->comment();
	set_value(audio->description, sizeof(audio->description), sTmp.to8Bit(true).c_str());
}

void taglib_get_composer(plugin_info* plugin, metadata_t* audio)
{
	TagLib::MPEG::File* mpegFile = new TagLib::MPEG::File(((TagLib::FileRef*)plugin->user_data)->file()->name());
	if(mpegFile->isValid() == false || mpegFile->ID3v2Tag() == NULL) {
		delete mpegFile;
		return;
	}

	TagLib::ID3v2::Tag *tag = mpegFile->ID3v2Tag();
  const TagLib::ID3v2::FrameList frameList = tag->frameList("TCOM");
	if(frameList.isEmpty()) {
		delete mpegFile;
		return;
	}
	
	set_value(audio->composer, sizeof(audio->composer), frameList.front()->toString().to8Bit(true).c_str());
	delete mpegFile;
}

void taglib_get_duration(plugin_info* info, metadata_t* metadata)
{
	TagLib::String sTmp;

	long length = ((TagLib::FileRef*)info->user_data)->audioProperties()->length();
	metadata->duration_ms = length * 1000;
}

void taglib_get_channels(plugin_info* info, metadata_t* metadata)
{
	metadata->nr_audio_channels = 
		((TagLib::FileRef*)info->user_data)->audioProperties()->channels();
}

void taglib_get_track_no(plugin_info* info, metadata_t* metadata)
{
	metadata->track_number = 
		((TagLib::FileRef*)info->user_data)->tag()->track();
}

void taglib_get_year(plugin_info* info, metadata_t* metadata)
{
	metadata->year = 
		((TagLib::FileRef*)info->user_data)->tag()->year();
}

void taglib_get_bitrate(plugin_info* info, metadata_t* metadata)
{
	metadata->audio_bitrate = 
		(((TagLib::FileRef*)info->user_data)->audioProperties()->bitrate() * 1024);
}

void taglib_get_samplerate(plugin_info* info, metadata_t* metadata)
{
	metadata->audio_sample_frequency = 
		((TagLib::FileRef*)info->user_data)->audioProperties()->sampleRate();
}

void taglib_close_file(plugin_info* plugin)
{
	delete (TagLib::FileRef*)plugin->user_data;
	plugin->user_data = NULL;
}

void taglib_check_image(plugin_info* info, metadata_t* metadata)
{
	TagLib::MPEG::File* mpegFile = new TagLib::MPEG::File(((TagLib::FileRef*)info->user_data)->file()->name());
	if(mpegFile->isValid() == false || mpegFile->ID3v2Tag() == NULL) {
		delete mpegFile;
		return;
	}

	TagLib::ID3v2::Tag *tag = mpegFile->ID3v2Tag();
  const TagLib::ID3v2::FrameList frameList = tag->frameList("APIC");
	if(frameList.isEmpty()) {
		delete mpegFile;
		return;
	}
	
	TagLib::ID3v2::AttachedPictureFrame* picFrame;
	picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
  if(picFrame == NULL) {
		delete mpegFile;
		return;
	}
	
	//cout << "mime/type: " << picFrame->mimeType().toCString() << endl;
  
	metadata->has_image = 1;
	set_value(metadata->image_mime_type, sizeof(metadata->image_mime_type), picFrame->mimeType().toCString());

	delete mpegFile;
}

int taglib_read_image(plugin_info* info, char** mimeType, unsigned char** buffer, size_t* size)
{
	*size = 0;
	TagLib::MPEG::File* mpegFile = new TagLib::MPEG::File(((TagLib::FileRef*)info->user_data)->file()->name());
	if(mpegFile->isValid() == false || mpegFile->ID3v2Tag() == NULL) {
		delete mpegFile;
		return -1;
	}

	TagLib::ID3v2::Tag *tag = mpegFile->ID3v2Tag();
  const TagLib::ID3v2::FrameList frameList = tag->frameList("APIC");
	if(frameList.isEmpty()) {
		delete mpegFile;
		return -1;
	}
	
	TagLib::ID3v2::AttachedPictureFrame* picFrame;
	picFrame = dynamic_cast<TagLib::ID3v2::AttachedPictureFrame*>(frameList.front());
  if(picFrame == NULL) {
		delete mpegFile;
		return -1;
	}

	set_value_old(mimeType, picFrame->mimeType().toCString());
	
	const TagLib::ByteVector pic = picFrame->picture();
	*buffer = (unsigned char*)realloc(*buffer, pic.size());
  memcpy(*buffer, pic.data(), pic.size());
	*size = pic.size();
	
	delete mpegFile;
	return 0;
}


#ifdef __cplusplus
extern "C" {
#endif

void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "taglib");
	strcpy(info->plugin_author, "Ulrich Voelkel");
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
	taglib_get_composer(plugin, metadata);
	// original performer // TOPE
	taglib_check_image(plugin, metadata);
	
	metadata->audio_bits_per_sample = 0;

	return 0;
}

int fuppes_metadata_read_image(plugin_info* plugin, char** mimeType, unsigned char** buffer, size_t* size)
{
	return taglib_read_image(plugin, mimeType, buffer, size);
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	taglib_close_file(plugin);
}

void unregister_fuppes_plugin(plugin_info* plugin ) // __attribute__((unused))
{
}

#ifdef __cplusplus
}
#endif
