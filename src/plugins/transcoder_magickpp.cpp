/***************************************************************************
 *            transcoder_magickpp.cpp
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

#include <Magick++.h>
#include <string>
#include <iostream>

int magickScale(const char* inputFile,
								const char* outputFile,
								int width, int height,
								int less,	int greater)
{
	Magick::Image     image;
  Magick::Geometry  geometry;  
  
  // and then convert/resize using imagemagick
  try {
    image.read(inputFile);
  }
  catch(Magick::WarningCorruptImage &ex) {
    std::cout << "WARNING: image \"" << inputFile << "\" corrupt" << std::endl;
    std::cout << ex.what() << std::endl << std::endl;
    return -1;
  }
  catch(std::exception &ex) {
    std::cout << __FILE__ << " " << __LINE__ << " :: " << ex.what() << std::endl << std::endl;
    return -1;
  }

  try {
    geometry.width(width);
    geometry.height(height);
   
    geometry.greater((greater == 1));
    geometry.less((less == 1));
    
    image.scale(geometry);
    
    image.write(outputFile);
  }
  catch (std::exception &ex) {
    std::cout << __FILE__ << " " << __LINE__ << " :: " << ex.what() << std::endl << std::endl;
    return -1; 
  }

	return 0;
}

#ifdef __cplusplus
extern "C" {
#endif
	
void register_fuppes_plugin(plugin_info* info)
{
	strcpy(info->plugin_name, "Magick++");
	info->plugin_type = PT_TRANSCODER;
}
  
int fuppes_transcoder_transcode_image(plugin_info* plugin,
                                const char* inputFile,
                                const char* outputFile,
																int width, int height,
																int less,	int greater)
{
  return magickScale(inputFile, outputFile, width, height, less, greater);
}

void unregister_fuppes_plugin(plugin_info* info)
{
}

#ifdef __cplusplus
}
#endif
