/***************************************************************************
 *            UPnPItem.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
#ifndef _UPNPITEM_H
#define _UPNPITEM_H

#include <fstream>

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "UPnPObject.h"

/*===============================================================================
 CLASS CUPnPItem
===============================================================================*/

class CUPnPItem: public CUPnPObject
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
  *  @param  p_nUPnPObjectType  type of the UPnP object
  *  @param  p_sHTTPServerURL  URL of the HTTP server
  */
  CUPnPItem(UPNP_OBJECT_TYPE p_nUPnPObjectType, std::string p_sHTTPServerURL, std::string p_sMimeType):
      CUPnPObject(p_nUPnPObjectType, p_sHTTPServerURL)
  {
    m_sMimeType = p_sMimeType;
  }
      
/*===============================================================================
 GET
===============================================================================*/
  
  /** empty base class method
  *  @param  pWriter  XML container to write to
  */
  /*void GetDescription(xmlTextWriterPtr pWriter)
  {
  }*/

  virtual std::string GetMimeType() = 0;  
  virtual unsigned int GetSize() = 0;
/* <\PUBLIC> */

  protected:
    unsigned int m_nSize;  
    std::string  m_sMimeType;
  
};

#endif /* _UPNPITEM_H */
