/***************************************************************************
 *            UPnPActionFactory.h
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
 
#ifndef _UPNPACTIONFACTORY_H
#define _UPNPACTIONFACTORY_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <string>
#include "UPnPAction.h"
#include "UPnPActions/UPnPBrowse.h"

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
    *  @return returns the action object on success otherwise NULL
    *  @todo   Parse whole description
    */
    CUPnPAction* BuildActionFromString(std::string p_sContent);

/* <\PUBLIC> */

  private:
          
    bool ParseBrowseAction(CUPnPBrowse* pAction);

};

#endif /* _UPNPACTIONFACTORY_H */
