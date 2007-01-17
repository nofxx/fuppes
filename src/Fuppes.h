/***************************************************************************
 *            Fuppes.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef _FUPPES_H
#define _FUPPES_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include <map>
#include <vector>
#include <list>
#include "SSDP/SSDPCtrl.h"
#include "HTTP/HTTPServer.h"
#include "UPnPDevice.h"
#include "ContentDirectory/ContentDirectory.h"
#include "ConnectionManager/ConnectionManager.h"

/*===============================================================================
 FORWARD DECLARATIONS
===============================================================================*/

class CFuppes;
class CHTTPMessage;
class CMediaServer;
//class CContentDirectory;
class CUPnPDevice;

/*===============================================================================
 CLASS IFuppes
===============================================================================*/

class IFuppes
{

/* <PUBLIC> */

public:

  virtual void OnReceivePresentationRequest(
    CFuppes* pSender,
    CHTTPMessage* pMessage,
    CHTTPMessage* pResult) = 0;

/* <\PUBLIC> */

};

/*===============================================================================
 CLASS CFuppes
===============================================================================*/

class CFuppes: public ISSDPCtrl, IHTTPServer, IUPnPDevice
{

/* <PUBLIC> */

public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/
  
  CFuppes(
    std::string p_sIPAddress,
    std::string p_sUUID,
    IFuppes*    pPresentationRequestHandler
    );
  virtual ~CFuppes();

  void OnTimer(CUPnPDevice* pSender);

/*==============================================================================
 GET
===============================================================================*/

  CSSDPCtrl*                GetSSDPCtrl() { return m_pSSDPCtrl; }
  CContentDirectory*        GetContentDirectory() { return m_pContentDirectory; }
  std::string               GetHTTPServerURL();
  std::string               GetIPAddress();
  std::vector<CUPnPDevice*> GetRemoteDevices();
  std::string               GetUUID() { return m_sUUID; }

/* <\PUBLIC> */

/* <PRIVATE> */

private:

/*===============================================================================
 MEMBERS
===============================================================================*/
  
  CSSDPCtrl*            m_pSSDPCtrl;
  CHTTPServer*          m_pHTTPServer;
  CMediaServer*         m_pMediaServer;
  CContentDirectory*    m_pContentDirectory;    
  CConnectionManager*   m_pConnectionManager;
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

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

  void OnSSDPCtrlReceiveMsg(CSSDPMessage*);
  bool OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);

  bool HandleHTTPRequest(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);
  bool HandleHTTPPostSOAPAction(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut);
	
  void HandleSSDPAlive(CSSDPMessage* pMessage);
  void HandleSSDPByeBye(CSSDPMessage* pMessage);
  
/* <\PRIVATE> */

};

#endif /* _FUPPES_H */
