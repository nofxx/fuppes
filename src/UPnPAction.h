/***************************************************************************
 *            UPnPAction.h
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
 
#ifndef _UPNPACTION_H
#define _UPNPACTION_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "UPnPBase.h"
#include <string>

/*===============================================================================
 DEFINITIONS
===============================================================================*/

typedef enum tagUPNP_ACTION_TYPE
{
  UPNP_ACTION_TYPE_BROWSE,
  UPNP_ACTION_TYPE_MAX
}UPNP_ACTION_TYPE;

/*===============================================================================
 CLASS CUPnPAction
===============================================================================*/

class CUPnPAction
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  /** constructor
  */
  /*CUPnPAction(UPNP_ACTION_TYPE nType);*/

  /** destructor
  */
  /*CUPnPAction(std::string);*/

/*===============================================================================
 MEMBERS
===============================================================================*/

  UPNP_DEVICE_TYPE m_nTargetDevice;
  UPNP_ACTION_TYPE m_nActionType;

/* <\PUBLIC> */

};

#endif /* _UPNPACTION_H */
