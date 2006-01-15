/***************************************************************************
 *            ImageItem.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _IMAGEITEM_H
#define _IMAGEITEM_H

#include "UPnPItem.h"

typedef enum tagIMAGE_TYPE
{
  IMAGE_TYPE_UNKNOWN  =  0,
  IMAGE_TYPE_PNG      =  1,
	IMAGE_TYPE_BMP      =  2,
  IMAGE_TYPE_JPEG     =  3
}IMAGE_TYPE;

class CImageItem: public CUPnPItem
{

/* <PUBLIC> */

  public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/
    
    /** constructor
     *  @param  p_sHTTPServerURL  URL of the HTTP server
     */
    CImageItem(std::string p_sHTTPServerURL, std::string p_sMimeType);

/*===============================================================================
 GET
===============================================================================*/

    /** writes the whole description of an audio item
     *  @param  pWriter  the XML container to write to
     */
    void GetDescription(xmlTextWriterPtr pWriter);
      
    std::string  GetMimeType();
    unsigned int GetSize();
/* <\PUBLIC> */  
  
  //private:      
    IMAGE_TYPE   m_nImageType;
};

#endif /* _IMAGEITEM_H */
