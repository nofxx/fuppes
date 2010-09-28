/***************************************************************************
 *            dlna_audio_profiles.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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


int dlna_get_audio_profile_ac3(int channels, int bitrate, char* profile, char* mimeType)
{
  strcpy(mimeType, "audio/vnd.dolby.dd-raw");
  strcpy(profile, "AC3");
  return 0;
}

int dlna_get_audio_profile_atrac3plus(int channels, int bitrate, char* profile, char* mimeType)
{
  strcpy(mimeType, "audio/x-sony-oma");
  strcpy(profile, "ATRAC3plus");
  return 0;
}

int dlna_get_audio_profile_lpcm(int channels, int bitrate, char* profile, char* mimeType)
{
  strcpy(mimeType, "audio/L16");
  strcpy(profile, "LPCM");
  return 0;
}

int dlna_get_audio_profile_mp3(int channels, int bitrate, char* profile, char* mimeType)
{
  strcpy(mimeType, "audio/mpeg");
  strcpy(profile, "MP3");
  return 0;
}

int dlna_get_audio_profile_mpeg4(int channels, int bitrate, char* profile, char* mimeType)
{
  return -1;
}

int dlna_get_audio_profile_wma(int channels, int bitrate, char* profile, char* mimeType)
{
  strcpy(mimeType, "audio/x-ms-wma");
  
  if(channels == 2 && bitrate < (193 * 1024)) {
    strcpy(profile, "WMABASE");
    return 0;
  }
  else if(channels == 2) {
    strcpy(profile, "WMAFULL");
    return 0;
  }
  else if(channels > 2) {
    strcpy(profile, "WMAPRO");
    return 0;
  }
  
  return -1;
}

