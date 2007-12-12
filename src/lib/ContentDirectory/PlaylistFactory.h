/***************************************************************************
 *            PlaylistFactory.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _PLAYLISTFACTORY_H
#define _PLAYLISTFACTORY_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>

class CPlaylistFactory
{
  public:
    std::string BuildPlaylist(std::string p_sObjectId);
  
  private:
    std::string BuildPLS(std::string p_sObjectId);
    std::string BuildM3U(std::string p_sObjectId);
};

#endif /* _PLAYLISTFACTORY_H */
