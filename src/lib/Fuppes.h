/***************************************************************************
 *            Fuppes.h
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

#ifndef __FUPPES_H
#define __FUPPES_H

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#include <map>
#include <vector>
#include <list>
#include "SSDP/SSDPCtrl.h"
#include "HTTP/HTTPServer.h"
#include "UPnPDevice.h"
#include "ContentDirectory/ContentDirectory.h"
#include "ConnectionManager/ConnectionManager.h"
#include "XMSMediaReceiverRegistrar.h"

class CFuppes;
class CHTTPMessage;
class CMediaServer;
//class CContentDirectory;
class CUPnPDevice;

class IFuppes
{

  public:
    virtual ~IFuppes() {};
        
    virtual void OnReceivePresentationRequest(
      CFuppes* pSender,
      CHTTPMessage* pMessage,
      CHTTPMessage* pResult) = 0;
};



class CFuppes: public ISSDPCtrl, IHTTPServer, IUPnPDevice
{

  public:
  
    CFuppes(std::string p_sIPAddress, std::string p_sUUID);
    virtual ~CFuppes();

    void OnTimer(CUPnPDevice* pSender);
	  void OnNewDevice(CUPnPDevice* pSender);


    CSSDPCtrl*                GetSSDPCtrl() { return m_pSSDPCtrl; }
    CContentDirectory*        GetContentDirectory() { return m_pContentDirectory; }
    std::string               GetHTTPServerURL();
    std::string               GetIPAddress();
    std::vector<CUPnPDevice*> GetRemoteDevices();
    std::string               GetUUID() { return m_sUUID; }

  private:

    CSSDPCtrl*            m_pSSDPCtrl;
    CHTTPServer*          m_pHTTPServer;
    CMediaServer*         m_pMediaServer;
    CContentDirectory*    m_pContentDirectory;    
    CConnectionManager*   m_pConnectionManager;
	  CXMSMediaReceiverRegistrar* m_pXMSMediaReceiverRegistrar;
    std::string           m_sIPAddress;
    std::string           m_sUUID;
    IFuppes*              m_pPresentationRequestHandler;
    fuppesThreadMutex     m_OnTimerMutex;  

    std::map<std::string, CUPnPDevice*>           m_RemoteDevices;
    std::map<std::string, CUPnPDevice*>::iterator m_RemoteDeviceIterator;
    fuppesThreadMutex     m_RemoteDevicesMutex;

    std::list<CUPnPDevice*>           m_TimedOutDevices;
    std::list<CUPnPDevice*>::iterator m_TimedOutDevicesIterator;

    void CleanupTimedOutDevices();

    void OnSSDPCtrlReceiveMsg(CSSDPMessage*);	
    void HandleSSDPAlive(CSSDPMessage* pMessage);
    void HandleSSDPByeBye(CSSDPMessage* pMessage);

    bool OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);
    bool HandleHTTPRequest(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);
    //bool HandleHTTPPostSOAPAction(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);  
};

#endif // __FUPPES_H
