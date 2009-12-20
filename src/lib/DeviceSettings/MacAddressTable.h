/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            MacAddressTable.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifndef _MACADDRESSTABLE_H
#define _MACADDRESSTABLE_H

#include <string>
#include <map>
#include "../Common/Thread.h"

namespace fuppes
{

class MacAddressTable
{
  public:
    static void     init();
    static void     uninit();
    static bool     mac(std::string ip, std::string& mac);
    static void     clear();

  private:
    static MacAddressTable*             instance();
    
    static MacAddressTable*               m_instance;
    std::map<std::string, std::string>    m_map;
    Mutex                                 m_mutex;  

    static std::string      getMac(std::string ip);
};

}

#endif // _MACADDRESSTABLE_H
