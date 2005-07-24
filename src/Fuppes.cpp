/***************************************************************************
 *            Fuppes.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
 *  Copyright (C) 2005 Thomas Schnitzler
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
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
#include "HTTPMessage.h"
#include "MediaServer.h"
#include "UPnPDevice.h"
#include "ContentDirectory/AudioItem.h"
#include "ContentDirectory/ContentDirectory.h"

using namespace std;

/*===============================================================================
 CONSTANTS
===============================================================================*/

const string LOGNAME = "FUPPES";

/*===============================================================================
 CLASS CFuppes
===============================================================================*/

// <PUBLIC>

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

CFuppes::CFuppes(std::string p_sIPAddress, std::string p_sUUID, IFuppes* pPresentationRequestHandler)
{
  ASSERT(NULL != pPresentationRequestHandler);
  
  // Set members
  m_sIPAddress                  = p_sIPAddress;
  m_sUUID                       = p_sUUID;
  m_pPresentationRequestHandler = pPresentationRequestHandler;

  CSharedLog::Shared()->Log(LOGNAME, "initializing devices");
  
  // Init HTTP server
  m_pHTTPServer = new CHTTPServer(m_sIPAddress);
  m_pHTTPServer->SetReceiveHandler(this);
	m_pHTTPServer->Start();
  
  // Init SSDP receiver
  m_pSSDPCtrl = new CSSDPCtrl(m_sIPAddress, m_pHTTPServer->GetURL());
	m_pSSDPCtrl->SetReceiveHandler(this);
	m_pSSDPCtrl->Start();

  // Create MediaServer
  m_pMediaServer = new CMediaServer(m_pHTTPServer->GetURL());	  
  
  // Create ContentDirectory
  m_pContentDirectory = new CContentDirectory(m_pHTTPServer->GetURL());  
  
  // Add ContentDirectory to MediaServers service-list
	m_pMediaServer->AddUPnPService(m_pContentDirectory);

  // Logging
  CSharedLog::Shared()->Log(LOGNAME, "done");
  CSharedLog::Shared()->Log(LOGNAME, "multicasting alive messages");  
  //m_pSSDPCtrl->send_alive();  
  CSharedLog::Shared()->Log(LOGNAME, "done");
  CSharedLog::Shared()->Log(LOGNAME, "multicasting m-search");  
  //m_pSSDPCtrl->send_msearch();  
  CSharedLog::Shared()->Log(LOGNAME, "done");
}

CFuppes::~CFuppes()
{
  // Logging
  CSharedLog::Shared()->Log(LOGNAME, "shutting down");
  CSharedLog::Shared()->Log(LOGNAME, "multicasting byebye messages");  
  
  // Send ByeBye message
  ASSERT(NULL != m_pSSDPCtrl);
  if(NULL != m_pSSDPCtrl)
  {
    m_pSSDPCtrl->send_byebye();  
    m_pSSDPCtrl->Stop();
  }
  CSharedLog::Shared()->Log(LOGNAME, "done");

  // Stop HTTPServer
  ASSERT(NULL != m_pHTTPServer);
  if(NULL != m_pHTTPServer)
    m_pHTTPServer->Stop();

  // Destroy objects
  SAFE_DELETE(m_pContentDirectory);
  SAFE_DELETE(m_pMediaServer);
  SAFE_DELETE(m_pSSDPCtrl);
  SAFE_DELETE(m_pHTTPServer);
}

/*===============================================================================
 GET
===============================================================================*/

/*! \fn     CSSDPCtrl* CFuppes::GetSSDPCtrl()
 *  \brief  Returns a pointer to the CSSDPCtrl member
 *  \param  none
 *  \return CSSDPCtrl*
 */
CSSDPCtrl* CFuppes::GetSSDPCtrl()
{
  return m_pSSDPCtrl;
}

/*! \fn     std::string CFuppes::GetHTTPServerURL()
 *  \brief  Returns URL of the HTTP member
 *  \param  none
 *  \return std::string
 */
std::string CFuppes::GetHTTPServerURL()
{
  ASSERT(NULL != m_pHTTPServer);
  if(NULL != m_pHTTPServer)
    return m_pHTTPServer->GetURL();
  else return "";
}

/*! \fn     std::string CFuppes::GetIPAddress()
 *  \brief  Returns IP address
 *  \param  none
 *  \return std::string
 */
std::string CFuppes::GetIPAddress()
{
  return m_sIPAddress;
}

/*! \fn     std::vector<CUPnPDevice*> CFuppes::GetRemoteDevices()
 *  \brief  Returns a UPnP device
 *  \param  none
 *  \return std::vector<CUPnPDevice*>
 */
std::vector<CUPnPDevice*> CFuppes::GetRemoteDevices()
{
  std::vector<CUPnPDevice*> vResult;
  
  // Iterate devices
  for(m_RemoteDeviceIterator = m_RemoteDevices.begin(); m_RemoteDeviceIterator != m_RemoteDevices.end(); m_RemoteDeviceIterator++)
  {
    vResult.push_back((*m_RemoteDeviceIterator).second);
  }
  
  return vResult; 
}

// <\PUBLIC>

// <PRIVATE>

/*===============================================================================
 MESSAGE HANDLING
===============================================================================*/

void CFuppes::OnSSDPCtrlReceiveMsg(CSSDPMessage* pMessage)
{
//  cout << inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr) << ":" << ntohs(pMessage->GetRemoteEndPoint().sin_port) << endl;
  
  if((m_sIPAddress.compare(inet_ntoa(pMessage->GetRemoteEndPoint().sin_addr)) != 0) || (pMessage->GetUUID().compare(m_sUUID) != 0))
  { 
    m_RemoteDeviceIterator = m_RemoteDevices.find(pMessage->GetDeviceID());
    if(m_RemoteDeviceIterator != m_RemoteDevices.end())
    {
      std::stringstream sMsg;
      sMsg << "known device: " << pMessage->GetDeviceID();
      CSharedLog::Shared()->Log(LOGNAME, sMsg.str());      
    }
    else
    {
      std::stringstream sMsg;
      sMsg << "new device id: " << pMessage->GetDeviceID();      
      CSharedLog::Shared()->Log(LOGNAME, sMsg.str());
      
      if((pMessage->GetLocation().compare("")) == 0)
        return;
      
      CUPnPDevice* pDevice = new CUPnPDevice();
      if(pDevice->BuildFromDescriptionURL(pMessage->GetLocation()))
        m_RemoteDevices[pMessage->GetDeviceID()] = pDevice;      
      else      
        delete pDevice;     
    }    
  }
}

bool CFuppes::OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
	ASSERT(NULL != pMessageIn);
  if(NULL == pMessageIn)
    return false;
  ASSERT(NULL != pMessageOut);
  if(NULL == pMessageOut)
    return false;
  
  bool fRet = true;

  // Handle message
  HTTP_MESSAGE_TYPE nMsgType = pMessageIn->GetMessageType();
  switch(nMsgType)
  {
  case HTTP_MESSAGE_TYPE_GET:
    fRet = HandleHTTPGet(pMessageIn, pMessageOut);
    break;
  case HTTP_MESSAGE_TYPE_POST:
    fRet = HandleHTTPPost(pMessageIn, pMessageOut);
    break;
  case HTTP_MESSAGE_TYPE_200_OK:
    break;
  case HTTP_MESSAGE_TYPE_404_NOT_FOUND:
    break;
  default: break;
  }

  return fRet;
}

bool CFuppes::HandleHTTPGet(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
	ASSERT(NULL != pMessageIn);
  if(NULL == pMessageIn)
    return false;
  ASSERT(NULL != pMessageOut);
  if(NULL == pMessageOut)
    return false;

  // Get request
  std::string strRequest = pMessageIn->GetRequest();

  // Root description
  if(0 == strRequest.compare("/"))
  {
    ASSERT(NULL != m_pMediaServer);
    if(NULL == m_pMediaServer)
      return false;
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, HTTP_CONTENT_TYPE_TEXT_XML);
    pMessageOut->SetContent(m_pMediaServer->GetDeviceDescription());
    return true;
  }
  
  // ContentDirectory description
  else if(0 == strRequest.compare("/UPnPServices/ContentDirectory/description.xml"))
  {
    ASSERT(NULL != m_pContentDirectory);
    if(NULL == m_pContentDirectory)
      return false;
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, HTTP_CONTENT_TYPE_TEXT_XML);
    pMessageOut->SetContent(m_pContentDirectory->GetServiceDescription());
    return true;
  }
  
  // Presentation
  else if(0 == ToLower(strRequest).compare("/index.html"))
  {
    ASSERT(NULL != m_pPresentationRequestHandler);
    if(NULL == m_pPresentationRequestHandler)
      return false;
    m_pPresentationRequestHandler->OnReceivePresentationRequest(this, pMessageIn, pMessageOut);
    return true;
  }
  
  // AudioItem
  else if((strRequest.length() > 24)  &&
          ((strRequest.length() > 24) && (strRequest.substr(24).compare("/MediaServer/AudioItems/")))
         )
  {
    ASSERT(NULL != m_pContentDirectory);
    if(NULL == m_pContentDirectory)
      return false;
    string sItemObjId = pMessageIn->GetRequest().substr(24, pMessageIn->GetRequest().length());
    CAudioItem* pItem = (CAudioItem*)m_pContentDirectory->GetItemFromObjectID(sItemObjId);

    if(pItem && FileExists(pItem->GetFileName()))
    {
      pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, HTTP_CONTENT_TYPE_AUDIO_MPEG);
      pMessageOut->LoadContentFromFile(pItem->GetFileName());
    }    
    cout << "[FUPPES] sending audio file " << pItem->GetFileName() << endl;    
    return true;
  }
  
  return false;
}

bool CFuppes::HandleHTTPPost(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
  ASSERT(NULL != pMessageIn);
  if(NULL == pMessageIn)
    return false;
  ASSERT(NULL != pMessageOut);
  if(NULL == pMessageOut)
    return false;
  
  // Get UPnP action
  CUPnPBrowse UPnPBrowse;
  bool fRet = pMessageIn->GetAction(&UPnPBrowse);
  ASSERT(true == fRet);
  if(false == fRet)
    return false;
  
  // Handle UPnP action
  if(UPnPBrowse.m_TargetDevice == udtContentDirectory)
       fRet = m_pContentDirectory->HandleUPnPAction((CUPnPAction*)&UPnPBrowse, pMessageOut);
  
  return fRet;
}

// <\PRIVATE>
