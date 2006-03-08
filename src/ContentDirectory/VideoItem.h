/***************************************************************************
 *            VideoItem.h
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
 
#ifndef _VIDEOITEM_H
#define _VIDEOITEM_H

#include "UPnPItem.h"

/*typedef enum tagVIDEO_TYPE
{
  VIDEO_TYPE_UNKNOWN  =  0,
  VIDEO_TYPE_AVI      =  1,
	VIDEO_TYPE_MPEG     =  2
}VIDEO_TYPE;*/

class CVideoItem: public CUPnPItem
{

/* <PUBLIC> */

  public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/
    
    /** constructor
     *  @param  p_sHTTPServerURL  URL of the HTTP server
     */
    CVideoItem(std::string p_sHTTPServerURL, std::string p_sMimeType);

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
    //VIDEO_TYPE   m_nVideoType;
};

#endif /* _VIDEOITEM_H */
