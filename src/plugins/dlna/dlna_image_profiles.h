/***************************************************************************
 *            dlna_image_profiles.h
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

int dlna_get_image_profile_jpeg(int width, int height, char* profile, char* mimeType)
{
  if(width <= 0 && height <= 0) {
    return -1;
  }
  
	strcpy(mimeType, "image/jpeg");

  if(width <= 48 && height <= 48) {
		strcpy(profile, "JPEG_SM_ICO");
		return 0;
  }
  else if(width <= 120 && height <= 120) {
		strcpy(profile, "JPEG_LRG_ICO");
		return 0;
  }
  else if(width <= 160 && height <= 160) {
		strcpy(profile, "JPEG_TN");
		return 0;
  }
  else if(width <= 640 && height <= 480) {
		strcpy(profile, "JPEG_SM");
		return 0;
  }
  else if(width <= 1024 && height <= 768) {
    strcpy(profile, "JPEG_MED");
		return 0;
  }
  else if(width <= 4096 && height <= 4096) {
    strcpy(profile, "JPEG_LRG");
		return 0;
  }
  else {
    return -1;
  }
}

int dlna_get_image_profile_png(int width, int height, char* profile, char* mimeType)
{
  if(width <= 0 && height <= 0) {
    return -1;
  }
  
	return -1;
}
