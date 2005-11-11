/***************************************************************************
 *            Fuppes.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
/*===============================================================================
 INCLUDES
===============================================================================*/

#include "Fuppes.h"

#include <iostream>
#include <fstream>
#include <sstream>

#include "Common.h"
#include "SharedLog.h"
#include "HTTPMessage.h"
#include "MediaServer.h"
#include "UPnPDevice.h"
#include "ContentDirectory/AudioItem.h"
#include "ContentDirectory/ImageItem.h"
#include "ContentDirectory/ContentDirectory.h"

using namespace std;

/*===============================================================================
 CONSTANTS
===============================================================================*/

const string LOGNAME = "FUPPES";

/*===============================================================================
 CLASS CFuppes
===============================================================================*/

/* <PUBLIC> */

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

/** constructor
 *  @param  p_sIPAddress  IP-address of the network interface 
 *                        this instance should be started on
 *  @param  p_sUUID       UUID this instance should be started with
 *  @param  pPresentationRequestHandler pointer to an object implementing the request handler
 */
CFuppes::CFuppes(std::string p_sIPAddress, std::string p_sUUID, IFuppes* pPresentationRequestHandler)
{
  ASSERT(NULL != pPresentationRequestHandler);
  
  /* Set members */
  m_sIPAddress                  = p_sIPAddress;
  m_sUUID                       = p_sUUID;
  m_pPresentationRequestHandler = pPresentationRequestHandler;
  fuppesThreadInitMutex(&m_OnTimerMutex);
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "initializing devices");
  
  /* init HTTP-server */
  m_pHTTPServer = new CHTTPServer(m_sIPAddress);
  m_pHTTPServer->SetReceiveHandler(this);
	m_pHTTPServer->Start();
  
  /* init SSDP-controller */
  m_pSSDPCtrl = new CSSDPCtrl(m_sIPAddress, m_pHTTPServer->GetURL());
	m_pSSDPCtrl->SetReceiveHandler(this);
  m_pSSDPCtrl->Start();

  /* Create MediaServer */
  m_pMediaServer = new CMediaServer(m_pHTTPServer->GetURL(), this);	  
  
  /* Create ContentDirectory */
  m_pContentDirectory = new CContentDirectory(m_pHTTPServer->GetURL());  
  
  /* Add ContentDirectory to MediaServers service-list */
	m_pMediaServer->AddUPnPService(m_pContentDirectory);

  CSharedLog::Shared()->ExtendedLog(LOGNAME, "done");
  
  /* if everything is up and running,
     multicast alive messages and search
     for other devices */
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "multicasting alive messages");  
  m_pSSDPCtrl->send_alive();
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "multicasting m-search");  
  m_pSSDPCtrl->send_msearch();
  
  /* start alive timer */
  m_pMediaServer->GetTimer()->SetInterval(840);  // 900 sec = 15 min
  m_pMediaServer->GetTimer()->Start();
}

/** destructor
 */
CFuppes::~CFuppes()
{
  /* logging */
  CSharedLog::Shared()->Log(LOGNAME, "shutting down");
  
  /* multicast notify-byebye */
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "multicasting byebye messages");  
  m_pSSDPCtrl->send_byebye();  
  
  /* stop SSDP-controller */
  m_pSSDPCtrl->Stop();

  /* stop HTTP-server */
  m_pHTTPServer->Stop();

  CleanupTimedOutDevices();  
  
  fuppesThreadDestroyMutex(&m_OnTimerMutex);
  
  /* destroy objects */
  SAFE_DELETE(m_pContentDirectory);
  SAFE_DELETE(m_pMediaServer);
  SAFE_DELETE(m_pSSDPCtrl);
  SAFE_DELETE(m_pHTTPServer);
}


void CFuppes::CleanupTimedOutDevices()
{  
  /* iterate device list */            
  for(m_TimedOutDevicesIterator = m_TimedOutDevices.begin(); m_TimedOutDevicesIterator != m_TimedOutDevices.end(); m_TimedOutDevicesIterator++)
  {
    /* and delete timed out devices */
    CUPnPDevice* pTimedOutDevice = *m_TimedOutDevicesIterator;      
    m_TimedOutDevicesIterator = m_TimedOutDevices.erase(m_TimedOutDevicesIterator);
    delete pTimedOutDevice;
    //m_TimedOutDevicesIterator--;    
  }  
}

void CFuppes::OnTimer(CUPnPDevice* pSender)
{
  fuppesThreadLockMutex(&m_OnTimerMutex);
  
  CleanupTimedOutDevices();
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "OnTimer()");
  /* local device must send alive message */
  if(pSender->GetIsLocalDevice())
  {
    stringstream sLog;
    sLog << "device: " << pSender->GetUUID() << " send timed alive";
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());    
    m_pSSDPCtrl->send_alive();    
  }
  /* remote device timed out */
  else
  {
    stringstream sLog;
    sLog << "device: " << pSender->GetFriendlyName() << " timed out";
    CSharedLog::Shared()->Log(LOGNAME, sLog.str());

    m_RemoteDeviceIterator = m_RemoteDevices.find(pSender->GetUUID());  
    /* found device */
    if(m_RemoteDeviceIterator != m_RemoteDevices.end())
    { 
      /* remove device from list of remote devices */
      m_RemoteDevices.erase(pSender->GetUUID());
      
      /* stop the deivce's timer and */
      pSender->GetTimer()->Stop();      
      /* push it to the list containing timed out devices */
      m_TimedOutDevices.push_back(pSender);
    }
  }
  
  fuppesThreadUnlockMutex(&m_OnTimerMutex);
}

/*===============================================================================
 GET
===============================================================================*/

/** Returns a pointer to the CSSDPCtrl member 
 *  @return CSSDPCtrl*
 */
CSSDPCtrl* CFuppes::GetSSDPCtrl()
{
  return m_pSSDPCtrl;
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

/* <\PUBLIC> */

/* <PRIVATE> */

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

void CFuppes::OnSSDPCtrlReceiveMsg(CSSDPMessage* pMessage)
{
//  cout << inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pMessage->GetRemoteEndPoint().sin_port) << endl;
  
  CSharedLog::Shared()->ExtendedLog(LOGNAME, "OnSSDPCtrlReceiveMsg()");
  
  if((m_sIPAddress.compare(inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr)) != 0) || (pMessage->GetUUID().compare(m_sUUID) != 0))
  { 
    switch(pMessage->GetMessageType())
    {
      case SSDP_MESSAGE_TYPE_NOTIFY_ALIVE:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_NOTIFY_ALIVE");
        HandleSSDPAlive(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_M_SEARCH_RESPONSE:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_M_SEARCH_RESPONSE");
        HandleSSDPAlive(pMessage);
        break;
      case SSDP_MESSAGE_TYPE_NOTIFY_BYEBYE:
        CSharedLog::Shared()->ExtendedLog(LOGNAME, "SSDP_MESSAGE_TYPE_NOTIFY_BYEBYE");
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

bool CFuppes::OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  bool fRet = true;

  /* Handle message */
  HTTP_MESSAGE_TYPE nMsgType = pMessageIn->GetMessageType();
  switch(nMsgType)
  {
    case HTTP_MESSAGE_TYPE_GET:
      fRet = HandleHTTPGetOrHead(pMessageIn, pMessageOut);
      break;
    case HTTP_MESSAGE_TYPE_HEAD:
      fRet = HandleHTTPGetOrHead(pMessageIn, pMessageOut);
      break;
    case HTTP_MESSAGE_TYPE_POST:
      fRet = HandleHTTPPost(pMessageIn, pMessageOut);
      break;
    case HTTP_MESSAGE_TYPE_200_OK:
      break;
    case HTTP_MESSAGE_TYPE_404_NOT_FOUND:
      break;
    default:
        fRet = false;
      break;
  }

  return fRet;
}

bool CFuppes::HandleHTTPGetOrHead(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  /* Get request */
  std::string strRequest = pMessageIn->GetRequest();

  /* Root description */
  if(ToLower(strRequest).compare("/description.xml") == 0)
  {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml"); //HTTP_CONTENT_TYPE_TEXT_XML
    pMessageOut->SetContent(m_pMediaServer->GetDeviceDescription());
    return true;
  }
  
  /* ContentDirectory description */
  else if(strRequest.compare("/UPnPServices/ContentDirectory/description.xml") == 0)
  {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml");
    pMessageOut->SetContent(m_pContentDirectory->GetServiceDescription());
    return true;
  }
  
  /* Presentation */
  else if((strRequest.compare("/") == 0) || (ToLower(strRequest).compare("/index.html") == 0))
  {
    m_pPresentationRequestHandler->OnReceivePresentationRequest(this, pMessageIn, pMessageOut);
    return true;
  }  
  else if((strRequest.length() > 14) && (ToLower(strRequest).substr(0, 14).compare("/presentation/") == 0))
  {
    m_pPresentationRequestHandler->OnReceivePresentationRequest(this, pMessageIn, pMessageOut);
    return true;
  }
  
  /* AudioItem */
  else if((strRequest.length() > 24) && (strRequest.substr(0, 24).compare("/MediaServer/AudioItems/") == 0))
  {
    string sItemObjId = pMessageIn->GetRequest().substr(24, pMessageIn->GetRequest().length());
    CAudioItem* pItem = (CAudioItem*)m_pContentDirectory->GetItemFromObjectID(sItemObjId);

    if(pItem && FileExists(pItem->GetFileName()))
    {
      pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, pItem->GetMimeType());  //HTTP_CONTENT_TYPE_AUDIO_MPEG
      if (!pItem->GetDoTranscode())
        pMessageOut->LoadContentFromFile(pItem->GetFileName());
      else
        pMessageOut->TranscodeContentFromFile(pItem->GetFileName());     
      stringstream sLog;
      sLog << "sending audio file \"" << pItem->GetName() << "\""; 
      CSharedLog::Shared()->Log(LOGNAME, sLog.str()); 
      return true; 
    }
    else
    {
      if(!pItem)
        CSharedLog::Shared()->Error(LOGNAME, "HandleHTTPGet() :: pItem is NULL");
      else if(!FileExists(pItem->GetFileName()))
      {
        stringstream sLog;
        sLog << "requested file: \"" << pItem->GetFileName() << "\" not found";
        CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
      }      
      
      return false;
    }

  }
  
  /* ImageItem */
  else if((strRequest.length() > 24) && (strRequest.substr(0, 24).compare("/MediaServer/ImageItems/") == 0))
  {
    string sItemObjId = pMessageIn->GetRequest().substr(24, pMessageIn->GetRequest().length());
    CImageItem* pItem = (CImageItem*)m_pContentDirectory->GetItemFromObjectID(sItemObjId);

    if(pItem && FileExists(pItem->GetFileName()))
    {
      pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, pItem->GetMimeType()); 
      pMessageOut->LoadContentFromFile(pItem->GetFileName());
      
      stringstream sLog;
      sLog << "sending image file \"" << pItem->GetName() << "\""; 
      CSharedLog::Shared()->Log(LOGNAME, sLog.str()); 
      return true; 
    }
    else
    {
      if(!pItem)
        CSharedLog::Shared()->Error(LOGNAME, "HandleHTTPGet() :: pItem is NULL");
      else if(!FileExists(pItem->GetFileName()))
      {
        stringstream sLog;
        sLog << "requested file: \"" << pItem->GetFileName() << "\" not found";
        CSharedLog::Shared()->Warning(LOGNAME, sLog.str());
      }      
      return false;
    }
  }  
  
  return false;
}

bool CFuppes::HandleHTTPPost(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  /* Get UPnP action */
  CUPnPBrowse UPnPBrowse;
  bool fRet = pMessageIn->GetAction(&UPnPBrowse);  
  if(false == fRet)
    return false;
  
  /* Handle UPnP action */
  if(UPnPBrowse.m_nTargetDevice == UPNP_DEVICE_TYPE_CONTENT_DIRECTORY)
    fRet = m_pContentDirectory->HandleUPnPAction((CUPnPAction*)&UPnPBrowse, pMessageOut);
  
  return fRet;
}

void CFuppes::HandleSSDPAlive(CSSDPMessage* pMessage)
{
  m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetUUID());
  
  /* known device */
  if(m_RemoteDeviceIterator != m_RemoteDevices.end())
  {
    m_RemoteDevices[pMessage->GetUUID()]->GetTimer()->Reset();
    
    std::stringstream sMsg;
    sMsg << "received \"Notify-Alive\" from known device id: " << pMessage->GetUUID();      
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());
  }
  
  /* new device */
  else
  {  
    std::stringstream sMsg;
    sMsg << "received \"Notify-Alive\" from unknown device id: " << pMessage->GetUUID();      
    CSharedLog::Shared()->ExtendedLog(LOGNAME, sMsg.str());
    sMsg.str("");
        
    if((pMessage->GetLocation().compare("")) == 0)
      return;
      
    CUPnPDevice* pDevice = new CUPnPDevice(this);
    if(pDevice->BuildFromDescriptionURL(pMessage->GetLocation()))
    {
      sMsg << "new device: " << pDevice->GetFriendlyName();
      CSharedLog::Shared()->Log(LOGNAME, sMsg.str());      
      
      m_RemoteDevices[pMessage->GetUUID()] = pDevice;
      pDevice->GetTimer()->SetInterval(900);  // 900 sec = 15 min
      pDevice->GetTimer()->Start();
    }
    else      
      delete pDevice;
  
  }
}

void CFuppes::HandleSSDPByeBye(CSSDPMessage* pMessage)
{
  std::stringstream sLog;
  sLog << "received \"Notify-ByeBye\" from device: " << pMessage->GetUUID();
  CSharedLog::Shared()->ExtendedLog(LOGNAME, sLog.str());
  sLog.str("");
  
  m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetUUID());  
  /* found device */
  if(m_RemoteDeviceIterator != m_RemoteDevices.end())
  {
    sLog << "received byebye from " << m_RemoteDevices[pMessage->GetUUID()]->GetFriendlyName();
    CSharedLog::Shared()->Log(LOGNAME, sLog.str());
    
    delete m_RemoteDevices[pMessage->GetUUID()];
    m_RemoteDevices.erase(pMessage->GetUUID());
  }
}

/* <\PRIVATE> */
