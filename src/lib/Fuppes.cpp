/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Fuppes.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "Fuppes.h"

#include <iostream>
#include <cassert>
//#include <fstream>

#include "Common/Common.h"
#include "Common/Exception.h"
#include "SharedLog.h"
#include "HTTP/HTTPMessage.h"
#include "MediaServer.h"
#include "UPnPDevice.h"
#include "GENA/SubscriptionMgr.h"

#include "ContentDirectory/ContentDatabase.h"
#ifdef HAVE_VFOLDER
#include "ContentDirectory/VirtualContainerMgr.h"
#endif

using namespace std;

/** constructor
 *  @param  p_sIPAddress  IP-address of the network interface 
 *                        this instance should be started on
 *  @param  p_sUUID       UUID this instance should be started with
 */
CFuppes::CFuppes(std::string p_sIPAddress, std::string p_sUUID)
{
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "starting UPnP subsystem");


  m_pHTTPServer = NULL;
  m_pSSDPCtrl = NULL;
  m_pMediaServer = NULL;
  m_pContentDirectory = NULL;
  m_pConnectionManager = NULL;
  m_pXMSMediaReceiverRegistrar = NULL;
  
  // set member
  m_sIPAddress                  = p_sIPAddress;
  m_sUUID                       = p_sUUID;
  fuppesThreadInitMutex(&m_OnTimerMutex);
  fuppesThreadInitMutex(&m_RemoteDevicesMutex);

  // init database 
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "init database");
  bool bIsNewDB = false;   
  if(!CContentDatabase::Shared()->Init(&bIsNewDB)) {    
    throw fuppes::Exception(__FILE__, __LINE__, "unable to create database file");
  }
  if(bIsNewDB) {
    CContentDatabase::Shared()->RebuildDB();
  }

  // init file details
  try {
    CFileDetails::Shared();
  }
  catch(fuppes::Exception ex) {    
    throw;
  }  
  
  
  
  /* init HTTP-server */
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "init http-server");
  try {
    m_pHTTPServer = new CHTTPServer(m_sIPAddress);
    m_pHTTPServer->SetReceiveHandler(this);
    m_pHTTPServer->Start();
  }
  catch(fuppes::Exception ex) {    
    throw;
  }
    
  /* init SSDP-controller */
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "init ssdp-controller");
  try {
    m_pSSDPCtrl = new CSSDPCtrl(m_sIPAddress, m_pHTTPServer->GetURL());
	  m_pSSDPCtrl->SetReceiveHandler(this);
    m_pSSDPCtrl->Start();
  }
  catch(fuppes::Exception ex) {    
    throw;
  }
	
	while(!m_pSSDPCtrl->isStarted()) {
		fuppesSleep(10);
	}

  /* init SubscriptionMgr */
  try {
    CSubscriptionMgr::Shared();
    CSubscriptionCache::Shared();
  }
  catch(fuppes::Exception ex) {
    throw;
  }
	  
  /* Create MediaServer */
  m_pMediaServer = new CMediaServer(m_pHTTPServer->GetURL(), p_sUUID, this);	  
  
  /* create ContentDirectory */
  try {
    m_pContentDirectory = new CContentDirectory(m_pHTTPServer->GetURL());   
	  m_pMediaServer->AddUPnPService(m_pContentDirectory);
  }
  catch(fuppes::Exception ex) {
    throw;
  }    

  /* create ConnectionManager */
  try {
    m_pConnectionManager = new CConnectionManager(m_pHTTPServer->GetURL()); 
    m_pMediaServer->AddUPnPService(m_pConnectionManager);
		CConnectionManagerCore::init();
  }
  catch(fuppes::Exception ex) {
    throw;
  }
	
	// XMSMediaReceiverRegistrar
	try {
	  m_pXMSMediaReceiverRegistrar = new CXMSMediaReceiverRegistrar(m_pHTTPServer->GetURL());
		m_pMediaServer->AddUPnPService(m_pXMSMediaReceiverRegistrar);
	}
	catch(fuppes::Exception ex) {
	  throw;
	}
  
	// wait for ssdp listener
	while(!m_pSSDPCtrl->isStarted() || !m_pHTTPServer->isStarted()) {
		fuppesSleep(10);
	}
	
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "UPnP subsystem started");

  // init virtual containers
#ifdef HAVE_VFOLDER
	CVirtualContainerMgr::Shared();
#endif
	
	/* if everything is up and running, multicast alive messages
  and search for other devices */       
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "multicasting alive messages");
  m_pSSDPCtrl->send_alive();
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "multicasting m-search");
  m_pSSDPCtrl->send_msearch();
	
  /* start alive timer */
  m_pMediaServer->GetTimer()->SetInterval(840);  // 900 sec = 15 min
  m_pMediaServer->GetTimer()->start();

  m_startTime = fuppes::DateTime::now();
}

/** destructor
 */
CFuppes::~CFuppes()
{  
  CSharedLog::Log(L_EXT, __FILE__, __LINE__, "deleting FUPPES instance");



  
  cout << "delete SSDP" << endl;
  if(m_pSSDPCtrl) {
    /* multicast notify-byebye */
    CSharedLog::Shared()->Log(L_EXT, __FILE__, __LINE__, "multicasting byebye messages");
    m_pSSDPCtrl->send_byebye();  
    
    /* stop SSDP-controller */
    CSharedLog::Shared()->Log(L_EXT, __FILE__, __LINE__, "stopping SSDP controller");
    m_pSSDPCtrl->Stop();
  }

  cout << "stop HTTP Server" << endl;
  /* stop HTTP-server */
  if(m_pHTTPServer) {
    CSharedLog::Shared()->Log(L_EXT, __FILE__, __LINE__, "stopping HTTP server");
    m_pHTTPServer->stop(); // stop thread execution
  }



  
    cout << "delete devices" << endl;
  
  m_RemoteDeviceIterator = m_RemoteDevices.begin();
  while(m_RemoteDeviceIterator != m_RemoteDevices.end()) {
		delete m_RemoteDeviceIterator->second;
		m_RemoteDeviceIterator++;
  }
  m_RemoteDevices.clear();
  CleanupTimedOutDevices();
  
  
  fuppesThreadDestroyMutex(&m_OnTimerMutex);
  fuppesThreadDestroyMutex(&m_RemoteDevicesMutex);
  
  /* destroy objects */
  cout << "delete pConnectionManager" << endl;
  if(m_pConnectionManager)
    delete m_pConnectionManager;

  cout << "delete pXMSMediaReceiverRegistrar" << endl;
  if(m_pXMSMediaReceiverRegistrar)
    delete m_pXMSMediaReceiverRegistrar;

  cout << "delete pContentDirectory" << endl;
  if(m_pContentDirectory)
    delete m_pContentDirectory;

  cout << "delete pMediaServer" << endl;
  if(m_pMediaServer)
    delete m_pMediaServer;

  cout << "delete pSSDPCtrl" << endl;
  if(m_pSSDPCtrl)
    delete m_pSSDPCtrl;

  cout << "delete pHTTPServer" << endl;
  if(m_pHTTPServer) {
    m_pHTTPServer->Stop();
    delete m_pHTTPServer;
  }
  

  cout << "delete CSubscriptionMgr" << endl;
  CSubscriptionMgr::deleteInstance();

  cout << "delete CSubscriptionCache" << endl;
  CSubscriptionCache::deleteInstance();
  
  cout << "delete CContentDatabase" << endl;
  delete CContentDatabase::Shared();
#ifdef HAVE_FOLDER
  delete CVirtualContainerMgr::Shared();
#endif

  cout << "delete CConnectionManagerCore" << endl;
	CConnectionManagerCore::uninit();
  
  //cout << "delete CFileAlterationMgr" << endl;
  //CFileAlterationMgr::deleteInstance();
  cout << "CFileDetails CConnectionManagerCore" << endl;
  CFileDetails::deleteInstance();


  //delete CTranscodingMgr::Shared();
}

void CFuppes::CleanupTimedOutDevices()
{  
	MutexLocker2 locker(&m_RemoteDevicesMutex);
	
  if(m_TimedOutDevices.size() == 0) {
    return;
  }
  
  // iterate device list ...
  for(m_TimedOutDevicesIterator = m_TimedOutDevices.begin(); m_TimedOutDevicesIterator != m_TimedOutDevices.end(); m_TimedOutDevicesIterator++) {
    // ... and delete timed out devices
    delete *m_TimedOutDevicesIterator;
  }  
  m_TimedOutDevices.clear();
}

void CFuppes::OnTimer(CUPnPDevice* pSender)
{
  MutexLocker2 locker(&m_OnTimerMutex);
  
	CleanupTimedOutDevices();
	
  // local device must send alive message
  if(pSender->isLocalDevice()) {                                  
    
    CSharedLog::Log(L_EXT, __FILE__, __LINE__, "device: %s send timed alive",  + pSender->GetUUID().c_str());
    m_pSSDPCtrl->send_alive();    
  }
  
  // remote device timed out
  else
  {  
	  if(!pSender->GetFriendlyName().empty()) {      
      CSharedLog::Log(L_EXT, __FILE__, __LINE__, 
					"device: %s timed out", pSender->GetFriendlyName().c_str());
		}

    fuppesThreadLockMutex(&m_RemoteDevicesMutex);
    
    m_RemoteDeviceIterator = m_RemoteDevices.find(pSender->GetUUID());      
    if(m_RemoteDeviceIterator != m_RemoteDevices.end()) { 
                              
      // remove device from list of remote devices
      m_RemoteDevices.erase(pSender->GetUUID());			
      // stop the device's timer and HTTP client and ...
      pSender->GetTimer()->stop();
      // ... push it to the list containing timed out devices
      m_TimedOutDevices.push_back(pSender);      
    }
    
		fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);

  }
  
}

/** Returns URL of the HTTP member
 * @return std::string
 */
std::string CFuppes::GetHTTPServerURL()
{
  assert(m_pHTTPServer != NULL);
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
    
    m_RemoteDevices[pMessage->GetUUID()]->GetTimer()->reset();
    /*CSharedLog::Log(L_EXT, __FILE__, __LINE__,
        "received \"Notify-Alive\" from known device id: %s", pMessage->GetUUID().c_str());*/
  }
  // new device
  else {    
    CSharedLog::Log(L_EXT, __FILE__, __LINE__,
        "received \"Notify-Alive\" from unknown device id: %s", pMessage->GetUUID().c_str());
        
    if((pMessage->GetLocation().compare("") == 0) ||
       (pMessage->GetUUID().compare("") == 0)) {
      fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
      return;
    }
      
    CUPnPDevice* pDevice = new CUPnPDevice(this, pMessage->GetUUID());
		m_RemoteDevices[pMessage->GetUUID()] = pDevice;
		pDevice->GetTimer()->SetInterval(30);  // wait max. 30 sec. for description
		pDevice->GetTimer()->start();
		pDevice->BuildFromDescriptionURL(pMessage->GetLocation());
  }

  fuppesThreadUnlockMutex(&m_RemoteDevicesMutex);
}

void CFuppes::onUPnPDeviceDeviceReady(std::string uuid)
{

}

void CFuppes::HandleSSDPByeBye(CSSDPMessage* pMessage)
{
  CSharedLog::Shared()->Log(L_EXT, __FILE__, __LINE__,
														"received \"Notify-ByeBye\" from device: %s", pMessage->GetUUID().c_str());

  fuppesThreadLockMutex(&m_RemoteDevicesMutex);

  m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetUUID());    
  if(m_RemoteDeviceIterator != m_RemoteDevices.end()) {    
    
    CSharedLog::Log(L_EXT, __FILE__, __LINE__,
        "received byebye from %s :: %s", m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName().c_str(), m_RemoteDevices[pMessage->GetUUID()]->GetUPnPDeviceTypeAsString().c_str());
    
    /*CSharedLog::Shared()->UserNotify(m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName(),                                    
        "UPnP device gone:\n" + m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName() + " (" + m_RemoteDevices[pMessage->GetUUID()]->GetUPnPDeviceTypeAsString() + ")");*/

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
    pMessageOut->SetContent(m_pMediaServer->localDeviceDescription(pMessageIn));
		//cout << m_pMediaServer->GetDeviceDescription(pMessageIn) << endl;
    return true;
  }

  return false;
}
