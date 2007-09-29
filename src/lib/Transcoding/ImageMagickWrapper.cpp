/***************************************************************************
 *            ImageMagickWrapper.cpp
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

#ifndef DISABLE_TRANSCODING

#include "ImageMagickWrapper.h"

#ifdef HAVE_IMAGEMAGICK

#include <iostream>
#include <sstream>
#include <Magick++.h>

#include "../SharedConfig.h"

#ifdef WIN32
#else
#include <stdlib.h>
#endif


using namespace std;

bool CImageMagickWrapper::Transcode(CFileSettings* pFileSettings, std::string p_sInFile, std::string* p_psOutFile)
{
  Magick::Image     image;
  Magick::Geometry  geometry;
  
  std::string sTmpFileName;
  
  
  stringstream sDcraw;
  
  // first run dcraw
  if(pFileSettings->pImageSettings->bDcraw) {
    sTmpFileName = CSharedConfig::Shared()->CreateTempFileName() + ".tiff";
    
    sDcraw << "dcraw " << pFileSettings->pImageSettings->sDcrawParams << " -T -c " << p_sInFile << " > " << sTmpFileName;
    
    #ifdef WIN32
    #warning todo: dcraw
    #else
    system(sDcraw.str().c_str());
    #endif
    
    p_sInFile = sTmpFileName;
  }
  
  
  if(sTmpFileName.empty()) {
    sTmpFileName = CSharedConfig::Shared()->CreateTempFileName();
  }
  sTmpFileName += "." + pFileSettings->Extension();
  *p_psOutFile = sTmpFileName;
  
  
  // and then convert/resize using imagemagick
  try {
    image.read(p_sInFile);
  }
  catch(Magick::WarningCorruptImage &ex) {
    cout << "WARNING: image \"" << p_sInFile << "\" corrupt" << endl;
    cout << ex.what() << endl << endl;
    return false;
  }
  catch(exception &ex) {
    cout << __FILE__ << " " << __LINE__ << " :: " << ex.what() << endl << endl;
    return false;
  }
  
  
  try {
    geometry.width(pFileSettings->pImageSettings->Width());
    geometry.height(pFileSettings->pImageSettings->Height());
   
    geometry.greater(pFileSettings->pImageSettings->Greater());
    geometry.less(pFileSettings->pImageSettings->Less());
    
    image.scale(geometry);
    
    image.write(*p_psOutFile);
  }
  catch (exception &ex) {
    cout << __FILE__ << " " << __LINE__ << " :: " << ex.what() << endl << endl;
    return false; 
  }

  return true;
}

#endif // HAVE_IMAGEMAGICK

#endif // DISABLE_TRANSCODING
