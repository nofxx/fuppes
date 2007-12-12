/***************************************************************************
 *            PlaylistParser.h
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
 
#ifndef _PLAYLISTPARSER_H
#define _PLAYLISTPARSER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include <list>

struct PlaylistEntry_t
{
  std::string   sFileName;
  bool          bIsLocalFile;
};

class CPlaylistParser
{
  public:
    CPlaylistParser();
    ~CPlaylistParser();
    
    bool LoadPlaylist(std::string p_sFileName);
  
    bool              Eof() { return m_bEof; }
    PlaylistEntry_t*  Entry();
    void              Next();
    
  private:
    bool                        m_bEof;
    int                         m_nPosition;
    std::string                 m_sListPath;
  
    std::list<PlaylistEntry_t*>            m_lEntries;
    std::list<PlaylistEntry_t*>::iterator  m_lEntriesIterator;
  
    std::string FormatFileName(std::string p_sValue);
  
    bool IsURL(std::string p_sValue);
    bool IsFile(std::string p_sValue);
  
    bool ParseM3U(std::string p_sContent);
    bool ParsePLS(std::string p_sContent);
};

#endif // _PLAYLISTPARSER_H
