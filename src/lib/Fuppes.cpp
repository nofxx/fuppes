/***************************************************************************
 *            Fuppes.cpp
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

#include "Fuppes.h"

#include <iostream>
//#include <fstream>

#include "Common/Common.h"
#include "SharedLog.h"
#include "HTTP/HTTPMessage.h"
#include "MediaServer.h"
#include "UPnPDevice.h"
#include "GENA/SubscriptionMgr.h"

#include "ContentDirectory/ContentDatabase.h"
#include "ContentDirectory/VirtualContainerMgr.h"

using namespace std;

/** constructor
 *  @param  p_sIPAddress  IP-address of the network interface 
 *                        this instance should be started on
 *  @param  p_sUUID       UUID this instance should be started with
 */
CFuppes::CFuppes(std::string p_sIPAddress, std::string p_sUUID)
{
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "starting UPnP subsystem");
 
  // set member
  m_sIPAddress                  = p_sIPAddress;
  m_sUUID                       = p_sUUID;
  fuppesThreadInitMutex(&m_OnTimerMutex);
  fuppesThreadInitMutex(&m_RemoteDevicesMutex);

  // init database 
  bool bIsNewDB = false;   
  if(!CContentDatabase::Shared()->Init(&bIsNewDB)) {    
    throw EException("unable to create database file", __FILE__, __LINE__);
  }
  if(bIsNewDB) {
    CContentDatabase::Shared()->RebuildDB();
  }  
  
  /* init HTTP-server */
  try {
    m_pHTTPServer = new CHTTPServer(m_sIPAddress);
    m_pHTTPServer->SetReceiveHandler(this);
    m_pHTTPServer->Start();
  }
  catch(EException ex) {    
    throw;
  }
    
  /* init SSDP-controller */
  try {
    m_pSSDPCtrl = new CSSDPCtrl(m_sIPAddress, m_pHTTPServer->GetURL());
	  m_pSSDPCtrl->SetReceiveHandler(this);
    m_pSSDPCtrl->Start();
  }
  catch(EException ex) {    
    throw;
  }

  /* init SubscriptionMgr */
  try {
    CSubscriptionMgr::Shared();
  }
  catch(EException ex) {
    throw;
  }
	  
  /* Create MediaServer */
  m_pMediaServer = new CMediaServer(m_pHTTPServer->GetURL(), this);	  
  
  /* create ContentDirectory */
  try {
    m_pContentDirectory = new CContentDirectory(m_pHTTPServer->GetURL());   
	  m_pMediaServer->AddUPnPService(m_pContentDirectory);
  }
  catch(EException ex) {
    throw;
  }    

  /* create ConnectionManager */
  try {
    m_pConnectionManager = new CConnectionManager(m_pHTTPServer->GetURL()); 
    m_pMediaServer->AddUPnPService(m_pConnectionManager);
  }
  catch(EException ex) {
    throw;
  }
	
	// XMSMediaReceiverRegistrar
	try {
	  m_pXMSMediaReceiverRegistrar = new CXMSMediaReceiverRegistrar();
		m_pMediaServer->AddUPnPService(m_pXMSMediaReceiverRegistrar);
	}
	catch(EException ex) {
	  throw;
	}
  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "UPnP subsystem started");

  // init virtual containers
	CVirtualContainerMgr::Shared();
  
  /* if everything is up and running, multicast alive messages
     and search for other devices */       
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "multicasting alive messages");
  m_pSSDPCtrl->send_alive();
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "multicasting m-search");
  m_pSSDPCtrl->send_msearch();
  
  /* start alive timer */
  m_pMediaServer->GetTimer()->SetInterval(840);  // 900 sec = 15 min
  m_pMediaServer->GetTimer()->Start();
}

/** destructor
 */
CFuppes::~CFuppes()
{  
  CSharedLog::Shared()->Log(L_EXT, "deleting FUPPES instance", __FILE__, __LINE__);    
  
  /* multicast notify-byebye */
  CSharedLog::Shared()->Log(L_EXT, "multicasting byebye messages", __FILE__, __LINE__);
  m_pSSDPCtrl->send_byebye();  
  
  /* stop SSDP-controller */
	CSharedLog::Shared()->Log(L_EXT, "stopping SSDP controller", __FILE__, __LINE__);
  m_pSSDPCtrl->Stop();

  /* stop HTTP-server */
	CSharedLog::Shared()->Log(L_EXT, "stopping HTTP server", __FILE__, __LINE__);
  m_pHTTPServer->Stop();

  CleanupTimedOutDevices();  
  
  fuppesThreadDestroyMutex(&m_OnTimerMutex);
  fuppesThreadDestroyMutex(&m_RemoteDevicesMutex);
  
  /* destroy objects */
  delete m_pContentDirectory;
  delete m_pMediaServer;
  delete m_pSSDPCtrl;
  delete m_pHTTPServer;
}

void CFuppes::CleanupTimedOutDevices()
{  
  if(m_TimedOutDevices.size() == 0) {
    return;
  }
  
  // iterate device list ...
  for(m_TimedOutDevicesIterator = m_TimedOutDevices.begin(); m_TimedOutDevicesIterator != m_TimedOutDevices.end(); m_TimedOutDevicesIterator++)
  {
    // ... and delete timed out devices
    CUPnPDevice* pTimedOutDevice = *m_TimedOutDevicesIterator;      
    m_TimedOutDevicesIterator = m_TimedOutDevices.erase(m_TimedOutDevicesIterator);    
    delete pTimedOutDevice;    
  }  
}

void CFuppes::OnTimer(CUPnPDevice* pSender)
{
  fuppesThreadLockMutex(&m_OnTimerMutex);
  
  CleanupTimedOutDevices();
  
  // local device must send alive message
  if(pSender->GetIsLocalDevice()) {                                  
    
    CSharedLog::Shared()->Log(L_EXT, "device: " + pSender->GetUUID() + " send timed alive", __FILE__, __LINE__);
    m_pSSDPCtrl->send_alive();    
  }
  
  // remote device timed out
  else
  {  
	  if(!pSender->GetFriendlyName().empty()) {      
      CSharedLog::Shared()->Log(L_NORM, "device: " + pSender->GetFriendlyName() + " timed out", __FILE__, __LINE__);
		}

    fuppesThreadLockMutex(&m_RemoteDevicesMutex);
    
    m_RemoteDeviceIterator = m_RemoteDevices.find(pSender->GetUUID());      
    if(m_RemoteDeviceIterator != m_RemoteDevices.end()) { 
                              
      // remove device from list of remote devices
      m_RemoteDevices.erase(pSender->GetUUID());			
      // stop the device's timer and HTTP client and ...
      pSender->GetTimer()->Stop();
      // ... push it to the list containing timed out devices
      m_TimedOutDevices.push_back(pSender);      
    }
    
		fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);

  }
  
  fuppesThreadUnlockMutex(&m_OnTimerMutex);
}

/** Returns URL of the HTTP member
 * @return std::string
 */
std::string CFuppes::GetHTTPServerURL()
{
  return m_pHTTPServer->GetURL();
}

/** Returns IP address 
 *  @return std::string
 */
std::string CFuppes::GetIPAddress()
{
  return m_sIPAddress;
}

/** Returns a UPnP device
 *  @return std::vector<CUPnPDevice*>
 */
std::vector<CUPnPDevice*> CFuppes::GetRemoteDevices()
{
  std::vector<CUPnPDevice*> vResult;
  
  /* Iterate devices */
  for(m_RemoteDeviceIterator = m_RemoteDevices.begin(); m_RemoteDeviceIterator != m_RemoteDevices.end(); m_RemoteDeviceIterator++)
  {
    vResult.push_back((*m_RemoteDeviceIterator).second);
  }
  
  return vResult; 
}


void CFuppes::OnSSDPCtrlReceiveMsg(CSSDPMessage* pMessage)
{
//  cout << inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pMessage->GetRemoteEndPoint().sin_port) << endl;  
  CSharedLog::Log(L_DBG, __FILE__, __LINE__,
      "OnSSDPCtrlReceiveMsg\n%s", pMessage->GetMessage().c_str());
  
  if((m_sIPAddress.compare(inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr)) != 0) || (pMessage->GetUUID().compare(m_sUUID) != 0))
  { 
    switch(pMessage->GetMessageType())
    {
      case SSDP_MESSAGE_TYPE_NOTIFY_ALIVE:
        //CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_NOTIFY_ALIVE");
        HandleSSDPAlive(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_M_SEARCH_RESPONSE:
        //CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_M_SEARCH_RESPONSE");
        HandleSSDPAlive(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_NOTIFY_BYEBYE:
        //CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_NOTIFY_BYEBYE");
        HandleSSDPByeBye(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_M_SEARCH:
        /* m-search is handled by SSDPCtrl */
        break;
      case SSDP_MESSAGE_TYPE_UNKNOWN:
        /* this should never happen :) */
        break;
    }
  }
}

void CFuppes::HandleSSDPAlive(CSSDPMessage* pMessage)
{
  fuppesThreadLockMutex(&m_RemoteDevicesMutex);

  m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetUUID());    

  // known device
  if(m_RemoteDeviceIterator != m_RemoteDevices.end()) {
    
    m_RemoteDevices[pMessage->GetUUID()]->GetTimer()->Reset();
        
    CSharedLog::Shared()->Log(L_EXT, 
        "received \"Notify-Alive\" from known device id: " + pMessage->GetUUID(), __FILE__, __LINE__);
  }

  // new device
  else {    
    
    CSharedLog::Shared()->Log(L_EXT, 
        "received \"Notify-Alive\" from unknown device id: " + pMessage->GetUUID(), __FILE__, __LINE__);
        
    if((pMessage->GetLocation().compare("") == 0) ||
       (pMessage->GetUUID().compare("") == 0)) {
      fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
      return;
    }
      
    CUPnPDevice* pDevice = new CUPnPDevice(this, pMessage->GetUUID());
		m_RemoteDevices[pMessage->GetUUID()] = pDevice;
		pDevice->BuildFromDescriptionURL(pMessage->GetLocation());
		pDevice->GetTimer()->SetInterval(30);  // wait max. 30 sec. for description
		pDevice->GetTimer()->Start();
  }

  fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
}

void CFuppes::OnNewDevice(CUPnPDevice* pSender)
{     
  fuppesThreadLockMutex(&m_RemoteDevicesMutex);

  m_RemoteDeviceIterator = m_RemoteDevices.find(pSender->GetUUID());  
  if(m_RemoteDeviceIterator != m_RemoteDevices.end())	{	  
    
	  CSharedLog::Shared()->Log(L_NORM, 
      "new device: " + pSender->GetFriendlyName() + " :: " + pSender->GetUPnPDeviceTypeAsString(), __FILE__, __LINE__);	
	
	  pSender->GetTimer()->SetInterval(900); // 900 sec = 15 min
			
    CSharedLog::Shared()->UserNotify(pSender->GetFriendlyName(),                                    
        "new UPnP device:\n" + pSender->GetFriendlyName() + " (" + pSender->GetUPnPDeviceTypeAsString() + ")");	  
	}
	
  fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);   
}

void CFuppes::HandleSSDPByeBye(CSSDPMessage* pMessage)
{
  CSharedLog::Shared()->Log(L_EXT, "received \"Notify-ByeBye\" from device: " + pMessage->GetUUID(), __FILE__, __LINE__);

  fuppesThreadLockMutex(&m_RemoteDevicesMutex);

  m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetUUID());    
  if(m_RemoteDeviceIterator != m_RemoteDevices.end()) {    
    
    CSharedLog::Shared()->Log(L_NORM, 
        "received byebye from " + m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName() + " :: " + m_RemoteDevices[pMessage->GetUUID()]->GetUPnPDeviceTypeAsString(), __FILE__, __LINE__);
    
    CSharedLog::Shared()->UserNotify(m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName(),                                    
        "UPnP device gone:\n" + m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName() + " (" + m_RemoteDevices[pMessage->GetUUID()]->GetUPnPDeviceTypeAsString() + ")");	  
    
    delete m_RemoteDevices[pMessage->GetUUID()];
    m_RemoteDevices.erase(pMessage->GetUUID());
  }
  
  fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
}

bool CFuppes::OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  pMessageOut->SetVersion(pMessageIn->GetVersion());
  return HandleHTTPRequest(pMessageIn, pMessageOut);
}

bool CFuppes::HandleHTTPRequest(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  // Get request
  std::string strRequest = pMessageIn->GetRequest();

  // Root description
  if(ToLower(strRequest).compare("/description.xml") == 0) {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml"); // HTTP_CONTENT_TYPE_TEXT_XML
    pMessageOut->SetContent(m_pMediaServer->GetDeviceDescription(pMessageIn));
    return true;
  }

  return false;
}
