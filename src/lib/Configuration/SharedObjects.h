/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ContentDatabase.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#ifndef _SHARED_OBJECTS_H
#define _SHARED_OBJECTS_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <vector>
#include <string>
#include "ConfigSettings.h"

class SharedObjects;

class SharedObject
{
  friend class SharedObjects;
  
  public:
    enum SharedObjectType {
      directory,
      itunes
    };

    SharedObjectType type() { return m_type; }
    std::string path() { return m_path; }
    
  private:
    SharedObjectType  m_type;
    std::string       m_path;

    // itunes settings
    std::string       m_rewritePathOld;
    std::string       m_rewritePathNew;
};

class SharedObjects : public ConfigSettings
{
  public:
    virtual bool Read(void);

    //  shared dir
    int SharedDirCount();
    std::string GetSharedDir(int p_nIdx);  
    void AddSharedDirectory(std::string p_sDirectory);
    void RemoveSharedDirectory(int p_nIdx);
    
    //  shared iTunes
    int SharedITunesCount() { return m_lSharedITunes.size(); }
    std::string GetSharedITunes(int p_nIdx) { return m_lSharedITunes[p_nIdx]; }
    void AddSharedITunes(std::string p_sITunes);
    void RemoveSharedITunes(int p_nIdx);

  private:
    std::vector<std::string>  m_lSharedDirs;
    std::vector<std::string>  m_lSharedITunes;
};

#endif // _SHARED_OBJECTS_H
