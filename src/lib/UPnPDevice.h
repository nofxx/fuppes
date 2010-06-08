/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            UPnPDevice.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _UPNPDEVICE_H
#define _UPNPDEVICE_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <string>
#include <vector>
#include <libxml/xmlwriter.h>

#include "UPnPBase.h"
#include "UPnPService.h"
#include "Common/Timer.h"
#include "Common/Thread.h"
#include "HTTP/HTTPClient.h"
#include "DeviceSettings/DeviceSettings.h"

using namespace std;

class CUPnPDevice;
  
class IUPnPDevice
{
  public:
    virtual ~IUPnPDevice() {};
  
    virtual void OnTimer(CUPnPDevice* pSender) = 0;
		//virtual void OnNewDevice(CUPnPDevice* pSender) = 0;
    virtual void onUPnPDeviceDeviceReady(std::string uuid) = 0;
};


class CUPnPDevice: public CUPnPBase, fuppes::ITimer, IHTTPClient
{
  protected:

    /** constructor
     *  @param  nType  the device type
     *  @param  p_sHTTPServerURL  URL of the HTTP server
     */
    CUPnPDevice(UPNP_DEVICE_TYPE nType, std::string p_sHTTPServerURL, IUPnPDevice* pEventHandler);		


  public:

    CUPnPDevice(IUPnPDevice* pEventHandler, std::string p_sUUID);
    virtual ~CUPnPDevice();

    void OnTimer();

    fuppes::Timer* GetTimer() { return &m_timer; }

    CHTTPClient* GetHTTPClient() { return m_pHTTPClient; }

    bool isLocalDevice() { return m_bIsLocalDevice; }

    bool descriptionAvailable() { return m_descriptionAvailable; }
      
    
    /** gets a device description from a specific URL and builds the device
     *  @param  p_sDescriptionURL URL where we can get the device description message
     *  @return returns true on success otherwise false
     */
    void BuildFromDescriptionURL(std::string p_sDescriptionURL);

    void OnAsyncReceiveMsg(CHTTPMessage* pMessage);

    /** adds a UPnP service to this device
     *  @param  pService the service to add to the device
     */
    void AddUPnPService(CUPnPService* pService);


    /** returns the count of all services for this device
     *  @return returns the count or 0
     */
    int GetUPnPServiceCount();

    /** returns a pointer to a UPnP service
     *  @param  p_nIndex  index of the service to get
     *  @return  pointer to the service or NULL
     */
    CUPnPService* GetUPnPService(int p_nIndex);  
	
    /** returns the whole device description
     *  @return  the device descripition as string
     */
    std::string localDeviceDescription(CHTTPMessage* pRequest);		

    /** returns the friendly name of this device
     *  @return  name of the device
     */
    std::string GetFriendlyName() { return m_sFriendlyName; }
  
    /** returns the device's UUID
     *  @return the UUID
     */  
    std::string GetUUID() { return m_sUUID; }
  
   
    std::string manufacturer() { return m_sManufacturer; }
    
    std::string manufacturerUrl() { return m_sManufacturerURL; }
    
    std::string presentationUrl() { return m_sPresentationURL; }


    std::string macAddress() { return m_macAddress; }

    std::string descriptionUrl() { return m_descriptionUrl; }


    CDeviceSettings* deviceSettings() { return m_deviceSettings; }
    
  protected:
    std::string   m_sFriendlyName;
    std::string   m_sManufacturer;
    std::string   m_sManufacturerURL;
    std::string   m_sModelDescription;
    std::string   m_sModelName;
    std::string   m_sModelNumber;
    std::string   m_sModelURL;
    std::string   m_sSerialNumber;
    std::string   m_sUUID;
    std::string   m_sUPC;
    std::string   m_sPresentationURL;


	private:

    std::string     m_descriptionUrl;
    bool            m_descriptionAvailable;
    std::string     m_macAddress;
    
    bool            m_bIsLocalDevice;
    fuppes::Timer		m_timer;
    IUPnPDevice*    m_pEventHandler;
		CHTTPClient*    m_pHTTPClient;
    fuppes::Mutex		m_mutex;
    CDeviceSettings* m_deviceSettings;

    


	//UPNP_DEVICE_TYPE           m_nUPnPDeviceType;			
	std::vector<CUPnPService*> m_vUPnPServices;

  
  /** gets all information from a device description
  *  @param  p_sDescription string with the device description
  *  @return returns true on success otherwise false
  */
  bool ParseDescription(std::string p_sDescription);

};

#endif // _UPNPDEVICE_H
