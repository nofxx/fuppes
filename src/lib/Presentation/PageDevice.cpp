/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            PageDevice.cpp
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

#include "PageDevice.h"

#include "../UPnPDevice.h"
#include "../SharedConfig.h"
#include "../Fuppes.h"
#include <list>

std::string PageDevice::content()
{
  std::stringstream result;


  result << "<h1>" << title() << "</h1>" << endl;  
  

  // sort the devices
  std::list<CUPnPDevice*> devices;
  for(unsigned int i = 0; i < CSharedConfig::Shared()->GetFuppesInstance(0)->GetRemoteDevices().size(); i++) {

    CUPnPDevice* pDevice = CSharedConfig::Shared()->GetFuppesInstance(0)->GetRemoteDevices()[i];
    if(pDevice->descriptionAvailable())
      devices.push_front(pDevice);
    else
      devices.push_back(pDevice);
  }


  std::list<CUPnPDevice*>::iterator iter;
  int count = 0;
  for(iter = devices.begin(); iter != devices.end(); iter++) {
    
    CUPnPDevice* pDevice = *iter;

    result << "<div class=\"remote-device\">" << endl;

    // icon
    result << "<div class=\"remote-device-icon\">";
      if(pDevice->descriptionAvailable()) {
        result << "device icon";
      }
      else {
       result << "icon loading";
      }
    result << "</div>" << endl;

 
    // friendly name    
    result << "<div class=\"remote-device-friendly-name\">";
    result << pDevice->GetFriendlyName();
    result << "</div>" << endl;


    result << "<div class=\"remote-device-info\">";
    result << pDevice->GetUPnPDeviceTypeAsString() << " : " << pDevice->GetUPnPDeviceVersion();    
    result << "</div>" << endl;

    result << "<a href=\"javascript:deviceDetails(" << count << ");\">";
    result << "details";
    result << "</a>" << endl;

    // details
    result << "<div class=\"remote-device-details\" id=\"remote-device-details-" << count << "\">";


    result << "<table>" << endl;

    result << "<tr><th colspan=\"2\">details</th></tr>" << endl;
    
    // uuid
    result << "<tr><td>uuid</td><td>";
    result << pDevice->GetUUID();
    result << "</td></tr>" << endl;
    
    // timeout
    result << "<tr><td>timeout</td><td>";
    result << pDevice->GetTimer()->GetCount() / 60 << "min. " << pDevice->GetTimer()->GetCount() % 60 << "sec.";
    result << "</td></tr>" << endl;
    
    // ip : port

    // presentation url
    result << "<tr><td>presentation</td><td>";
    result << pDevice->presentationUrl() << "<br />";
    result << "</td></tr>" << endl;
    
    // manufacturer
    result << "<tr><td>manufacturer</td><td>";
    result << pDevice->manufacturer() << "<br />";
    result << "</td></tr>" << endl;
    
    // manufacturerUrl
    result << "<tr><td>manufacturerUrl</td><td>";
    result << pDevice->manufacturerUrl() << "<br />";
    result << "</td></tr>" << endl;



    // descriptionUrl
    result << "<tr><td>descriptionUrl</td><td>";
    result << pDevice->descriptionUrl() << "<br />";
    result << "</td></tr>" << endl;


    // mac
    result << "<tr><td>mac</td><td>";
    result << pDevice->macAddress();
    result << "</td></tr>" << endl;


    // device settings
    if(pDevice->deviceSettings()) {  
      result << "<tr><td>device settings</td><td>";
      result << pDevice->deviceSettings()->name();
      result << "</td></tr>" << endl;
    }
    
    
    result << "</table>" << endl;
    
    result << "</div>" << endl; //details
    

    result << "</div>" << endl; // remote-device

    count++;
  }

  result << "<div id=\"remote-device-count\" style=\"display: none;\">" << count << "</div>";
  
  return result.str();  
}

