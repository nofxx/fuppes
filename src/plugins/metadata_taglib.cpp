#include "../../include/fuppes_plugin.h"

#include <fileref.h>
#include <tfile.h>
#include <tag.h>

#include <iostream>
using namespace std;


int taglib_open_file(plugin_info* info, char* fileName)
{
	info->user_data = new TagLib::FileRef(fileName);
	if(((TagLib::FileRef*)info->user_data)->isNull()) {
		delete (TagLib::FileRef*)info->user_data;
		info->user_data = NULL;
		return -1;
	}
}

void taglib_get_title(plugin_info* info)
{	
	TagLib::String sTmp;

	sTmp = ((TagLib::FileRef*)info->user_data)->tag()->title();
	cout << "title: \"" << sTmp.to8Bit(true) << "\"" << endl;
}

void taglib_get_duration(plugin_info* info)
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

	cout << "duration: " << szDuration << endl;
}

	/*
	TagLib::String sTmp;
	unsigned int nTmp;
  
	// channels
	pMusicTrack->nNrAudioChannels = pFile.audioProperties()->channels();
	
	// bitrate
	pMusicTrack->nBitrate = (pFile.audioProperties()->bitrate() * 1024);
	
  pMusicTrack->nBitsPerSample = 0;
  
	// samplerate
	pMusicTrack->nSampleRate = pFile.audioProperties()->sampleRate();
	
  // size
  pMusicTrack->nSize = pFile.file()->length();
     
	// artist
  sTmp = pFile.tag()->artist();
  pMusicTrack->sArtist = TrimWhiteSpace(sTmp.to8Bit(true));  
  if(pMusicTrack->sArtist.empty()) {
    pMusicTrack->sArtist = "unknown";
  }   
    
  // album
  sTmp = pFile.tag()->album();
  pMusicTrack->sAlbum = TrimWhiteSpace(sTmp.to8Bit(true));  
  if(pMusicTrack->sAlbum.empty()) {
    pMusicTrack->sAlbum = "unknown";
  }  
  
  // genre
	sTmp = pFile.tag()->genre();
  pMusicTrack->sGenre = TrimWhiteSpace(sTmp.to8Bit(true));   
  if(pMusicTrack->sGenre.empty()) {
    pMusicTrack->sGenre = "unknown";
  }

  // description/comment
	sTmp = pFile.tag()->comment();
	pMusicTrack->sDescription = TrimWhiteSpace(sTmp.to8Bit(true));

  // track no.
	nTmp = pFile.tag()->track();
	pMusicTrack->nOriginalTrackNumber = nTmp;

  // date/year
	nTmp = pFile.tag()->year();
  stringstream sDate;
  sDate << nTmp;
  pMusicTrack->sDate = sDate.str(); */


#ifdef __cplusplus
extern "C" {
#endif

int fuppes_metadata_file_open(plugin_info* info, char* fileName);
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "taglib metadata");
	info->plugin_type = PT_METADATA;

	/*add_extension(info, "mp3");
	add_extension(info, "ogg");
	add_extension(info, "flac");
	add_extension(info, "mpc");*/
	
	//fuppes_metadata_file_open(info, "/home/ulrich/fuppes-test/mp3/Wilson Picket - Mustang Sally.mp3");
}

int fuppes_metadata_file_open(plugin_info* info, char* fileName)
{
	if(taglib_open_file(info, fileName) != 0)
		return -1;

	/*taglib_get_title(info);
	taglib_get_duration(info);*/
}

void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif
