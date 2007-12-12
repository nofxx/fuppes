/***************************************************************************
 *            DefaultConfig.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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

#ifndef _DEFAULT_CONFIG_H
#define _DEFAULT_CONFIG_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <libxml/xmlwriter.h>
#include <string>
#include <sstream>
#include "../Common/XMLParser.h"

using namespace std;

const std::string NEEDED_CONFIGFILE_VERSION = "0.7.2.3";

bool WriteDefaultConfigFile(std::string p_sFileName)
{
  xmlTextWriterPtr  pWriter;
	
	pWriter = xmlNewTextWriterFilename(p_sFileName.c_str(), 0);
  if(!pWriter)
	  return false;
  
	xmlTextWriterSetIndent(pWriter, 4);
	xmlTextWriterStartDocument(pWriter, NULL, "UTF-8", NULL);

	// fuppes_config
	xmlTextWriterStartElement(pWriter, BAD_CAST "fuppes_config");  
  xmlTextWriterWriteAttribute(pWriter, BAD_CAST "version", BAD_CAST NEEDED_CONFIGFILE_VERSION.c_str()); 
	
    // shared_objects
    xmlTextWriterStartElement(pWriter, BAD_CAST "shared_objects");
      
      #ifdef WIN32
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<dir>C:\\Music\\</dir>");
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<itunes>C:\\Documents and Settings\\...\\iTunes.xml</itunes>");  
      #else 
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<dir>/mnt/music</dir>");
      xmlTextWriterWriteComment(pWriter, BAD_CAST "<itunes>/Users/.../iTunes.xml</itunes>");  
      #endif
  
    // end shared_objects
    xmlTextWriterEndElement(pWriter);
    
    
    // network
    xmlTextWriterStartElement(pWriter, BAD_CAST "network");
        
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty = automatic detection");
      xmlTextWriterStartElement(pWriter, BAD_CAST "interface");
      xmlTextWriterEndElement(pWriter); 
      
      xmlTextWriterWriteComment(pWriter, BAD_CAST "empty or 0 = random port");
      xmlTextWriterStartElement(pWriter, BAD_CAST "http_port");
      xmlTextWriterEndElement(pWriter); 
  
      xmlTextWriterWriteComment(pWriter, BAD_CAST "list of ip addresses allowed to access fuppes. if empty all ips are allowed");
      xmlTextWriterStartElement(pWriter, BAD_CAST "allowed_ips");        
        xmlTextWriterWriteComment(pWriter, BAD_CAST "<ip>192.168.0.1</ip>");
      xmlTextWriterEndElement(pWriter); 
  
    // end network
    xmlTextWriterEndElement(pWriter);


    // content directory
    xmlTextWriterStartElement(pWriter, BAD_CAST "content_directory");
    
      std::stringstream sComment;
      
      // charset
      sComment << "a list of possible charsets can be found under:" << endl << "      http://www.gnu.org/software/libiconv/";
      xmlTextWriterWriteComment(pWriter, BAD_CAST sComment.str().c_str());
      sComment.str("");
      xmlTextWriterStartElement(pWriter, BAD_CAST "local_charset");
      xmlTextWriterWriteString(pWriter, BAD_CAST "UTF-8");
      xmlTextWriterEndElement(pWriter); 
    
      // libs for metadata extraction
      xmlTextWriterWriteComment(pWriter, BAD_CAST "libs used for metadata extraction when building the database. [true|false]");
      xmlTextWriterStartElement(pWriter, BAD_CAST "use_imagemagick");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true");
      xmlTextWriterEndElement(pWriter);  
      xmlTextWriterStartElement(pWriter, BAD_CAST "use_taglib");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true");
      xmlTextWriterEndElement(pWriter);
      xmlTextWriterStartElement(pWriter, BAD_CAST "use_libavformat");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true");
      xmlTextWriterEndElement(pWriter); 
  
    // end content directory
    xmlTextWriterEndElement(pWriter);
    
    
    // transcoding 
    xmlTextWriterStartElement(pWriter, BAD_CAST "transcoding");
      
      // audio_encoder
      xmlTextWriterWriteComment(pWriter, BAD_CAST "[lame|twolame]");
      xmlTextWriterStartElement(pWriter, BAD_CAST "audio_encoder");
      xmlTextWriterWriteString(pWriter, BAD_CAST "lame"); // [lame|twolame]
      xmlTextWriterEndElement(pWriter);
      
      // transcode_vorbis
      xmlTextWriterWriteComment(pWriter, BAD_CAST "[true|false]");
      xmlTextWriterStartElement(pWriter, BAD_CAST "transcode_vorbis");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true"); // [true|false]
      xmlTextWriterEndElement(pWriter);
      
      // transcode_musepack
      xmlTextWriterStartElement(pWriter, BAD_CAST "transcode_musepack");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true"); // [true|false]
      xmlTextWriterEndElement(pWriter);
      
      // transcode_flac
      xmlTextWriterStartElement(pWriter, BAD_CAST "transcode_flac");
      xmlTextWriterWriteString(pWriter, BAD_CAST "true"); // [true|false]
      xmlTextWriterEndElement(pWriter);
    
    // end transcoding
    xmlTextWriterEndElement(pWriter);
    
  
    // device_settings
    xmlTextWriterStartElement(pWriter, BAD_CAST "device_settings");
      
      xmlTextWriterWriteComment(pWriter, BAD_CAST "\"default\" settings are inhertied by specific devices and can be overwritten");
			xmlTextWriterWriteComment(pWriter, BAD_CAST "do NOT remove the \"default\" device settings");
		
      // device (default)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "default");   
  
        // max_file_name_length        
        xmlTextWriterWriteComment(pWriter, BAD_CAST "specify the maximum length for file names (0 or empty = unlimited)");
        xmlTextWriterStartElement(pWriter, BAD_CAST "max_file_name_length");
        xmlTextWriterWriteString(pWriter, BAD_CAST "0");
        xmlTextWriterEndElement(pWriter);
    
        // playlist_style
        xmlTextWriterWriteComment(pWriter, BAD_CAST "[file|container]");
        xmlTextWriterStartElement(pWriter, BAD_CAST "playlist_style");
        xmlTextWriterWriteString(pWriter, BAD_CAST "file"); // [file|container]
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "show_childcount_in_title");
        xmlTextWriterWriteString(pWriter, BAD_CAST "false");
        xmlTextWriterEndElement(pWriter);  
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "enable_dlna");
        xmlTextWriterWriteString(pWriter, BAD_CAST "false");
        xmlTextWriterEndElement(pWriter);  

        xmlTextWriterStartElement(pWriter, BAD_CAST "transcoding_release_delay");
        xmlTextWriterWriteString(pWriter, BAD_CAST "4");
        xmlTextWriterEndElement(pWriter); 
  
        // file_settings (default)
        xmlTextWriterStartElement(pWriter, BAD_CAST "file_settings");
  
          xmlTextWriterWriteComment(pWriter, BAD_CAST "audio files");
  
          // mp3
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "mp3");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "AUDIO_ITEM");
            xmlTextWriterEndElement(pWriter); 
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "audio/mpeg");
            xmlTextWriterEndElement(pWriter);         
            xmlTextWriterStartElement(pWriter, BAD_CAST "dlna");
              xmlTextWriterWriteString(pWriter, BAD_CAST "MP3");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);

          // ogg
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "ogg");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "AUDIO_ITEM");
            xmlTextWriterEndElement(pWriter); 
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "application/octet-stream");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "transcode");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "true");
              xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
                xmlTextWriterWriteString(pWriter, BAD_CAST "mp3");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
                xmlTextWriterWriteString(pWriter, BAD_CAST "audio/mpeg");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "dlna");
                xmlTextWriterWriteString(pWriter, BAD_CAST "MP3");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "http_encoding");
                xmlTextWriterWriteString(pWriter, BAD_CAST "chunked");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "decoder");
                xmlTextWriterWriteString(pWriter, BAD_CAST "vorbis");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "encoder");
                xmlTextWriterWriteString(pWriter, BAD_CAST "lame");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "bitrate");
                xmlTextWriterWriteString(pWriter, BAD_CAST "192");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "samplerate");
                xmlTextWriterWriteString(pWriter, BAD_CAST "44100");
              xmlTextWriterEndElement(pWriter);
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);
  
          // mpc
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "mpc");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "AUDIO_ITEM");
            xmlTextWriterEndElement(pWriter); 
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "application/octet-stream");
            xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "transcode");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "true");
              xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
                xmlTextWriterWriteString(pWriter, BAD_CAST "mp3");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
                xmlTextWriterWriteString(pWriter, BAD_CAST "audio/mpeg");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "dlna");
                xmlTextWriterWriteString(pWriter, BAD_CAST "MP3");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "http_encoding");
                xmlTextWriterWriteString(pWriter, BAD_CAST "chunked");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "decoder");
                xmlTextWriterWriteString(pWriter, BAD_CAST "musepack");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "encoder");
                xmlTextWriterWriteString(pWriter, BAD_CAST "lame");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "bitrate");
                xmlTextWriterWriteString(pWriter, BAD_CAST "192");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "samplerate");
                xmlTextWriterWriteString(pWriter, BAD_CAST "44100");
              xmlTextWriterEndElement(pWriter);
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);                        
                                  
          // wav
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "wav");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "AUDIO_ITEM");
            xmlTextWriterEndElement(pWriter); 
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "audio/x-wav");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);                        

          // flac
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "flac");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "AUDIO_ITEM");
            xmlTextWriterEndElement(pWriter); 
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "audio/x-flac");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "transcode");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "true");
              xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
                xmlTextWriterWriteString(pWriter, BAD_CAST "mp3");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
                xmlTextWriterWriteString(pWriter, BAD_CAST "audio/mpeg");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "dlna");
                xmlTextWriterWriteString(pWriter, BAD_CAST "MP3");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "http_encoding");
                xmlTextWriterWriteString(pWriter, BAD_CAST "chunked");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "decoder");
                xmlTextWriterWriteString(pWriter, BAD_CAST "flac");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "encoder");
                xmlTextWriterWriteString(pWriter, BAD_CAST "lame");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "bitrate");
                xmlTextWriterWriteString(pWriter, BAD_CAST "192");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "samplerate");
                xmlTextWriterWriteString(pWriter, BAD_CAST "44100");
              xmlTextWriterEndElement(pWriter);
            xmlTextWriterEndElement(pWriter);  
          xmlTextWriterEndElement(pWriter);                      
  
          // wma
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "wma");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "AUDIO_ITEM");
            xmlTextWriterEndElement(pWriter); 
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "audio/x-ms-wma");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "dlna");
              xmlTextWriterWriteString(pWriter, BAD_CAST "WMAFULL");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);  				

    
          xmlTextWriterWriteComment(pWriter, BAD_CAST "image files");
          
          // jpeg
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "jpg");
            xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
              xmlTextWriterWriteString(pWriter, BAD_CAST "jpeg");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "IMAGE_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "image/jpeg");
            xmlTextWriterEndElement(pWriter);
            
            xmlTextWriterStartElement(pWriter, BAD_CAST "convert");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
              xmlTextWriterWriteComment(pWriter, BAD_CAST "<dcraw enabled=\"true\">-q 0</dcraw>");
              xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
                xmlTextWriterWriteString(pWriter, BAD_CAST "png");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
                xmlTextWriterWriteString(pWriter, BAD_CAST "image/png");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "height");
                xmlTextWriterWriteString(pWriter, BAD_CAST "0");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "width");
                xmlTextWriterWriteString(pWriter, BAD_CAST "0");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterWriteComment(pWriter, BAD_CAST "set \"greater\" to \"true\" if you only want to resize images greater than \"height\" or \"width\"");
              xmlTextWriterStartElement(pWriter, BAD_CAST "greater");
                xmlTextWriterWriteString(pWriter, BAD_CAST "false");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterWriteComment(pWriter, BAD_CAST "set \"less\" to \"true\" if you only want to resize images less than \"height\" or \"width\"");
              xmlTextWriterStartElement(pWriter, BAD_CAST "less");
                xmlTextWriterWriteString(pWriter, BAD_CAST "false");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterWriteComment(pWriter, BAD_CAST "set \"less\" and \"greater\" to \"false\" if you always want to resize");
            xmlTextWriterEndElement(pWriter);
  
          xmlTextWriterEndElement(pWriter);        
  
          // bmp
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "bmp");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "IMAGE_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "image/bmp");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);        
  
          // png
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "png");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "IMAGE_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "image/png");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);        
  
          // gif
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "gif");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "IMAGE_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "image/gif");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);        
  

  
          xmlTextWriterWriteComment(pWriter, BAD_CAST "video files");
  
          // mpeg
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "mpg");
            xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
              xmlTextWriterWriteString(pWriter, BAD_CAST "mpeg");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "video/mpeg");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);  
    
          // mp4
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "mp4");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "video/mp4");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);  
    
          // avi
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "avi");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "video/x-msvideo");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);
  
          // wmv
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "wmv");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "video/x-ms-wmv");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);
  
          // vob
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "vob");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "video/x-ms-vob");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);
  
          // vdr
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "vdr");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "video/x-extension-vdr");
            xmlTextWriterEndElement(pWriter);
  
            xmlTextWriterStartElement(pWriter, BAD_CAST "transcode");
              xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "true");
              xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
                xmlTextWriterWriteString(pWriter, BAD_CAST "vob");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
                xmlTextWriterWriteString(pWriter, BAD_CAST "video/x-ms-vob");
              xmlTextWriterEndElement(pWriter);
            xmlTextWriterEndElement(pWriter);
  
          xmlTextWriterEndElement(pWriter);
  
          // flv
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "flv");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "application/x-flash-video");
            xmlTextWriterEndElement(pWriter);
  
            /*xmlTextWriterStartElement(pWriter, BAD_CAST "transcode");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "true");
              xmlTextWriterStartElement(pWriter, BAD_CAST "ext");
                xmlTextWriterWriteString(pWriter, BAD_CAST "mpg");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
                xmlTextWriterWriteString(pWriter, BAD_CAST "video/mpeg");
              xmlTextWriterEndElement(pWriter);
              xmlTextWriterStartElement(pWriter, BAD_CAST "transcoder");
                xmlTextWriterWriteString(pWriter, BAD_CAST "ffmpeg");
              xmlTextWriterEndElement(pWriter);              
              xmlTextWriterStartElement(pWriter, BAD_CAST "transcoder_settings");
                xmlTextWriterWriteComment(pWriter, BAD_CAST "run \"ffmpeg -formats\" to see available video-/audio codecs");
                xmlTextWriterStartElement(pWriter, BAD_CAST "video_codec");
                  xmlTextWriterWriteString(pWriter, BAD_CAST "mpeg1video");
                xmlTextWriterEndElement(pWriter);                
                xmlTextWriterStartElement(pWriter, BAD_CAST "audio_codec");
                  xmlTextWriterWriteString(pWriter, BAD_CAST "mp2");
                xmlTextWriterEndElement(pWriter);
                xmlTextWriterStartElement(pWriter, BAD_CAST "audio_bitrate");
                  xmlTextWriterWriteString(pWriter, BAD_CAST "192000");
                xmlTextWriterEndElement(pWriter);
                xmlTextWriterStartElement(pWriter, BAD_CAST "audio_samplerate");
                  xmlTextWriterWriteString(pWriter, BAD_CAST "44100");
                xmlTextWriterEndElement(pWriter);
                xmlTextWriterWriteComment(pWriter, BAD_CAST "additional output parameters for ffpmeg (e.g. force stereo)");
                xmlTextWriterStartElement(pWriter, BAD_CAST "out_params");
                  xmlTextWriterWriteString(pWriter, BAD_CAST "-ac 2");
                xmlTextWriterEndElement(pWriter);
              xmlTextWriterEndElement(pWriter);*/
              
            //xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);  
  
          // asf
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "asf");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "VIDEO_ITEM");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "video/x-ms-asf");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);

          xmlTextWriterWriteComment(pWriter, BAD_CAST "playlists");
  
          // pls
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "pls");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "PLAYLIST");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "audio/x-scpls");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);
  
          // m3u
          xmlTextWriterStartElement(pWriter, BAD_CAST "file");
            xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "m3u");
            xmlTextWriterStartElement(pWriter, BAD_CAST "type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "PLAYLIST");
            xmlTextWriterEndElement(pWriter);
            xmlTextWriterStartElement(pWriter, BAD_CAST "mime_type");
              xmlTextWriterWriteString(pWriter, BAD_CAST "audio/x-mpegurl");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);  
  
        // end file_settings (default)
        xmlTextWriterEndElement(pWriter);  
        
  
      // end device (default)
      xmlTextWriterEndElement(pWriter);
  
  
      xmlTextWriterWriteComment(pWriter, BAD_CAST "If you have more than one device it is a good idea to set the ip address manually as some devices may have conflicting \"user agents\".");
			xmlTextWriterWriteComment(pWriter, BAD_CAST "It is safe to remove unneeded devices");
		
      // device (PS3)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "PS3");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
        
        // user_agent
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "UPnP/1.0 DLNADOC/1.00");
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "PLAYSTATION3");
        xmlTextWriterEndElement(pWriter);
  
  
        xmlTextWriterWriteComment(pWriter, BAD_CAST "<ip></ip>");  
  
        // enable_dlna
        xmlTextWriterStartElement(pWriter, BAD_CAST "enable_dlna");
        xmlTextWriterWriteString(pWriter, BAD_CAST "true");
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "transcoding_release_delay");
        xmlTextWriterWriteString(pWriter, BAD_CAST "50");
        xmlTextWriterEndElement(pWriter); 
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "file_settings");

        // mp3
        xmlTextWriterStartElement(pWriter, BAD_CAST "file");
          xmlTextWriterWriteAttribute(pWriter, BAD_CAST "ext", BAD_CAST "ogg");
          xmlTextWriterStartElement(pWriter, BAD_CAST "type");
            xmlTextWriterWriteString(pWriter, BAD_CAST "AUDIO_ITEM_MUSIC_TRACK");
          xmlTextWriterEndElement(pWriter);          
          xmlTextWriterStartElement(pWriter, BAD_CAST "transcode");
          xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "true");
            xmlTextWriterStartElement(pWriter, BAD_CAST "http_encoding")>
              xmlTextWriterWriteString(pWriter, BAD_CAST "stream");
            xmlTextWriterEndElement(pWriter);
          xmlTextWriterEndElement(pWriter);
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterEndElement(pWriter);   
  
      // end device (PS3)
      xmlTextWriterEndElement(pWriter);      
  
      // device (Xbox 360)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "Xbox 360");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "virtual", BAD_CAST "Xbox 360");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
  
        // user_agent
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "Xbox/2.0.\\d+.\\d+ UPnP/1.0 Xbox/2.0.\\d+.\\d+");
        xmlTextWriterEndElement(pWriter);
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "Xenon");
        xmlTextWriterEndElement(pWriter);
  
        // xbox 360
        xmlTextWriterStartElement(pWriter, BAD_CAST "xbox360");
        xmlTextWriterWriteString(pWriter, BAD_CAST "true");
        xmlTextWriterEndElement(pWriter);
  
      // end device (Xbox 360)
      xmlTextWriterEndElement(pWriter);  
  
  
      // device (Noxon audio)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "Noxon audio");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "virtual", BAD_CAST "default");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
  
        xmlTextWriterWriteComment(pWriter, BAD_CAST "Please enter the address of your Noxon. Automatic detection is impossible because the Noxon does not send a \"user-agent\" in it's requests");
        xmlTextWriterWriteComment(pWriter, BAD_CAST "<ip></ip>");
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "playlist_style");
        xmlTextWriterWriteString(pWriter, BAD_CAST "container");
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "show_childcount_in_title");
        xmlTextWriterWriteString(pWriter, BAD_CAST "true");
        xmlTextWriterEndElement(pWriter);  
  
      // end device (Noxon audio)
      xmlTextWriterEndElement(pWriter);   
  
      // device (Telegent TG 100)
      xmlTextWriterStartElement(pWriter, BAD_CAST "device");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "name", BAD_CAST "Telegent TG 100");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "virtual", BAD_CAST "default");
      xmlTextWriterWriteAttribute(pWriter, BAD_CAST "enabled", BAD_CAST "false");
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "dma/1.0 \\(http://www.cybertan.com.tw/\\)");
        xmlTextWriterEndElement(pWriter);
        
        xmlTextWriterStartElement(pWriter, BAD_CAST "user_agent");
        xmlTextWriterWriteString(pWriter, BAD_CAST "UPnP/1.0 DLNADOC/1.00");
        xmlTextWriterEndElement(pWriter);  
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "playlist_style");
        xmlTextWriterWriteString(pWriter, BAD_CAST "file");
        xmlTextWriterEndElement(pWriter);
  
        xmlTextWriterStartElement(pWriter, BAD_CAST "max_file_name_length");
        xmlTextWriterWriteString(pWriter, BAD_CAST "101");
        xmlTextWriterEndElement(pWriter);  
  
      // end device (Telegent TG 100)
      xmlTextWriterEndElement(pWriter);  
  
  
    // end device_settings
    xmlTextWriterEndElement(pWriter);
  
	// end fuppes_config
	xmlTextWriterEndElement(pWriter);	
	xmlTextWriterEndDocument(pWriter);
	xmlFreeTextWriter(pWriter);
	
  //xmlCleanupParser(); 
  
  CXMLDocument* pDoc = new CXMLDocument();
  pDoc->Load(p_sFileName);
  pDoc->Save();
  delete pDoc;
  
  return true;
}
 
#endif // _DEFAULT_CONFIG_H
