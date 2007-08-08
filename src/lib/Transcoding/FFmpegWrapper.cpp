/***************************************************************************
 *            FFmpegWrapper.cpp
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

#include "FFmpegWrapper.h"

#ifdef HAVE_LIBAVFORMAT
#ifdef ENABLE_VIDEO_TRANSCODING

CFFmpegWrapper::~CFFmpegWrapper()
{
  this->Break();
}

bool CFFmpegWrapper::Transcode(std::string p_sInFileParams, std::string p_sInFile, std::string p_sOutFileParams, std::string* p_psOutFile)
{

  /*char* szArgs[] = {"ffmpeg", "-y",
    "-i", "/home/ulrich/Desktop/Mary Fahl - Going Home.wmv",
    "-vcodec", "mpeg1video", //"libxvid",
    "-acodec", "mp2", //"libmp3lame",
    //"-ar", "44100",
    //"-ab", "192000",
    "/tmp/fuppes.mpg"};*/
    
  char* szArgs[14];
  
  szArgs[0] = (char*)malloc(strlen("ffmpeg") * sizeof(char));
  strcpy(szArgs[0], "ffmpeg");
  
  szArgs[1] = (char*)malloc(strlen("-y") * sizeof(char));
  strcpy(szArgs[1], "-y");
  
  szArgs[2] = (char*)malloc(strlen("-i") * sizeof(char));
  strcpy(szArgs[2], "-i");
  
  szArgs[3] = (char*)malloc(p_sInFile.length() * sizeof(char));
  strcpy(szArgs[3], p_sInFile.c_str());
  
  szArgs[4] = (char*)malloc(strlen("-vcodec") * sizeof(char));
  strcpy(szArgs[4], "-vcodec");
  
  szArgs[5] = (char*)malloc(strlen("mpeg1video") * sizeof(char));
  strcpy(szArgs[5], "mpeg1video");
  
  szArgs[6] = (char*)malloc(strlen("-acodec") * sizeof(char));
  strcpy(szArgs[6], "-acodec");
  
  szArgs[7] = (char*)malloc(strlen("mp2") * sizeof(char));
  strcpy(szArgs[7], "mp2");
  
  
  szArgs[8] = (char*)malloc(strlen("-ac") * sizeof(char));
  strcpy(szArgs[8], "-ac");
  
  szArgs[9] = (char*)malloc(strlen("2") * sizeof(char));
  strcpy(szArgs[9], "2");
  
  
  
  szArgs[10] = (char*)malloc(strlen("-ar") * sizeof(char));
  strcpy(szArgs[10], "-ar");
  
  szArgs[11] = (char*)malloc(strlen("32000") * sizeof(char));
  strcpy(szArgs[11], "32000");
  
  
  
  szArgs[12] = (char*)malloc(strlen("/tmp/fuppes.mpg") * sizeof(char));
  strcpy(szArgs[12], "/tmp/fuppes.mpg");
  
  szArgs[13] = (char*)malloc(sizeof(char));
  szArgs[13] = '\0';
  
  ffmpeg_main(13, szArgs);
  
   return true;  
}

void CFFmpegWrapper::Break()
{
  ffmpeg_break();
}

#endif // ENABLE_VIDEO_TRANSCODING
#endif // HAVE_LIBAVFORMAT
