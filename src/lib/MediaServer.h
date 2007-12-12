/***************************************************************************
 *            MediaServer.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _MEDIASERVER_H
#define _MEDIASERVER_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include "UPnPDevice.h"

class CMediaServer: public CUPnPDevice
{
  public:
    CMediaServer(std::string p_sHTTPServerURL, IUPnPDevice* pOnTimerHandler);
    ~CMediaServer();
};

#endif // _MEDIASERVER_H
