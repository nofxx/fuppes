/***************************************************************************
 *            metadata_magickwand.c
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
#include <wand/magick-wand.h>

void magick_set_date(MagickWand* wand, char** dateMetadata)
{
	char* date = MagickGetImageProperty(wand, "exif:DateTimeOriginal");
	
	if(date == 0)
	{
		date = MagickGetImageProperty(wand, "exif:DateTimeDigitized");
		
		if(date == 0)
			return;
	}
	
    const int dateLen = strlen(date);
    
	if(dateLen > 0)
	{
	    *dateMetadata = (char*)relloc(*dateMetadata, (dateLen + 1) * sizeof(char));
	    strcpy(*dateMetadata, date);
	    
	    char* dateStr = *dateMetadata;
	    
	    if(dateLen > 4 && dateStr[4] != '-')
		    dateStr[4] = '-';
		    
	    if(dateLen > 6 && dateStr[7] != '-')
	        dateStr[7] = '-';
	        
	    if(dateLen > 10 && dateStr[10] != 'T')
	        dateStr[10] = 'T';
	}
	
    MagickRelinquishMemory(date);
}


#ifdef __cplusplus
extern "C" {
#endif
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "magickWand");
	info->plugin_type = PT_METADATA;
	
	#ifdef WIN32
	MagickWandGenesis();
	#endif
}

int fuppes_metadata_file_open(plugin_info* plugin, const char* fileName)
{
	MagickBooleanType status;
  
  plugin->user_data = NewMagickWand();	
  status = MagickReadImage(plugin->user_data, fileName);
	
	if(status == MagickFalse) {

		/*ExceptionType severity; 
		char* description;
		description = MagickGetException(plugin->user_data, &severity);
		(void)fprintf(stderr,"%s %s %lu %s\n", GetMagickModule(), description);
		description = (char*)MagickRelinquishMemory(description);	*/
	
		plugin->user_data = DestroyMagickWand(plugin->user_data);
		return -1;
	}

	return 0;
}

int fuppes_metadata_read(plugin_info* plugin, metadata_t* metadata)
{
	metadata->type = MD_IMAGE;
	
	metadata->width  = MagickGetImageWidth(plugin->user_data);
	metadata->height = MagickGetImageHeight(plugin->user_data);	
	
	magick_set_date(plugin->user_data, &metadata->date);
	
	return 0;
}

void fuppes_metadata_file_close(plugin_info* plugin)
{
	plugin->user_data = DestroyMagickWand(plugin->user_data);
	plugin->user_data = NULL;
}

void unregister_fuppes_plugin(plugin_info* info)
{
	MagickWandTerminus();
}

#ifdef __cplusplus
}
#endif
