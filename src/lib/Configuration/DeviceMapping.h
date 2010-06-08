/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            DeviceMapping.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2010 Robert Massaioli <robertmassaioli@gmail.com>
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

#ifndef __DEVICEMAPPING_H
#define __DEVICEMAPPING_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <vector>
#include <string>
#include <set>
#include "../Common/Common.h"
#include "ConfigSettings.h"
#include "../DeviceSettings/DeviceSettings.h"
#include "../DeviceSettings/DeviceIdentificationMgr.h"


struct VirtualFolder
{
  std::string name;
  bool enabled;
};

class VirtualFolders : public ConfigSettings
{
  public:
    virtual bool Read(void);

    fuppes::StringList  getEnabledFolders();

    bool enabled() { return m_enabled; }
    
  private:
    std::vector<struct VirtualFolder> m_folderSettings;
    bool m_enabled;
};


struct mapping {
  std::string value;
  std::string vfolder;
  CDeviceSettings* device;
};

class DeviceMapping : public ConfigSettings
{
  public:
    friend class CDeviceIdentificationMgr;

    virtual bool Read(void);

    //void requiredVirtualDevices(std::set<std::string>* vfolders);

  private:
    // device mappings
    std::vector<struct mapping> macAddrs;
    std::vector<struct mapping> ipAddrs;
};

#endif
