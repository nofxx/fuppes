/***************************************************************************
 *            UPnPActionFactory.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
 
#ifndef _UPNPACTIONFACTORY_H
#define _UPNPACTIONFACTORY_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>

/*===============================================================================
 FORWARD DECLARATIONS
===============================================================================*/

class CUPnPBrowse;

/*===============================================================================
 CLASS CUPnPActionFactory
===============================================================================*/

class CUPnPActionFactory
{

/* <PUBLIC> */

public:

/*===============================================================================
 UPNP ACTIONS
===============================================================================*/

  /** builds an UPnP action from a string
  *  @param  p_sContent  the string to build th message from
  *  @param  pBrowse  the built UPnP action
  *  @return returns true on success otherwise false
  */
  bool BuildActionFromString(
    std::string  p_sContent,
    CUPnPBrowse* pBrowse
    );

/* <\PUBLIC> */

};

#endif /* _UPNPACTIONFACTORY_H */
