/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            MacAddressTable.cpp
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

#include "MacAddressTable.h"
#include "../Common/File.h"
#include "../Common/RegEx.h"

using namespace fuppes;

#define MAC_UNKNOWN "00:00:00:00:00:00"

MacAddressTable* MacAddressTable::m_instance = NULL;

void MacAddressTable::init() // static
{
  if(m_instance == NULL)
    m_instance = new MacAddressTable();
}

MacAddressTable* MacAddressTable::instance() // static
{
  init();
  return m_instance;
}

void MacAddressTable::uninit() // static
{
  if(m_instance == NULL)
    return;

  clear();
  delete m_instance;
}


bool MacAddressTable::mac(std::string ip, std::string& mac) // static
{
  MutexLocker locker(&instance()->m_mutex);

  std::map<std::string, std::string>::iterator it;
  it = instance()->m_map.find(ip);
  if(it != instance()->m_map.end()) {
    mac = it->second;
    return true;
  }

  mac = instance()->getMac(ip);
  if(mac != MAC_UNKNOWN) {
    instance()->m_map[ip] = mac;
    return true;
  }
  return false;
}

void MacAddressTable::clear() // static
{
  MutexLocker locker(&instance()->m_mutex);
  instance()->m_map.clear();
}


std::string MacAddressTable::getMac(std::string ip) // static
{
#ifdef WIN32
	// GetIpNetTable
	// http://msdn.microsoft.com/en-us/library/aa365956(VS.85).aspx
  /*
  The IP Helper APIs available for ARP on Windows are listed below:

  GetIpNetTable: Retrieves address resolution table information.
  SetIpNetEntry: Adds entry to the ARP table.
  DeleteIpNetEntry: Deletes entry from the ARP table.
  CreateIpNetEntry: Creates an entry in the ARP table.
  FlushIpNetTable: Deletes all ARP entries for the specified interface from the ARP table
  SendARP: Sends an ARP request to obtain the physical address that corresponds to the specified destination IP address
  The structures available in IP Helper APIs for ARP follow:

  MIB_IPNETTABLE: Contains a table of ARPentries.
  PMIB_IPNETTABLE: Pointer to MIB_IPNETTABLE structure.
  MIB_IPNETROW: Contains information for an ARPtable entry.
  PMIB_IPNETROW: Pointer to MIB_IPNETROW structure.
  */
  
  return MAC_UNKNOWN;
#else
  /*File file("/proc/net/arp");
  if(file.open(File::Read)) {
    std::string line;
    RegEx rx("([\\d|\\.]+) +0x\\d +0x\\d +([\\d|\\w|:]+)");
    while(file.getline(line)) {
      if(!rx.search(line))
        continue;

      if(rx.match(1) == ip) {
        file.close();
        return rx.match(2);
      }
    }
    file.close();
  }*/

  return MAC_UNKNOWN;
#endif  
}

