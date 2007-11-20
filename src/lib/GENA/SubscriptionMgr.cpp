/***************************************************************************
 *            SubscriptionMgr.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006, 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "SubscriptionMgr.h"
#include "../Common/RegEx.h"
#include "../Common/UUID.h"
#include "../SharedLog.h"

CSubscription::CSubscription()
{ 
  m_bHandled   = false;
  m_pHTTPClient = NULL;
}

CSubscription::~CSubscription()
{
  if(m_pHTTPClient)
    delete m_pHTTPClient;
}


void CSubscription::Renew()
{
  m_nTimeLeft = m_nTimeout;
}

void CSubscription::DecTimeLeft()
{
  if(m_nTimeLeft > 0)
    m_nTimeLeft--;
}

CHTTPClient* CSubscription::GetHTTPClient()
{
  if(!m_pHTTPClient)
    m_pHTTPClient = new CHTTPClient();
  return m_pHTTPClient;
}



void CSubscription::AsyncReply()
{
  CEventNotification* pNotification = new CEventNotification();
  
  switch(m_nSubscriptionTarget)
  {
    case UPNP_SERVICE_CONTENT_DIRECTORY :
      
      pNotification->SetContent(
        "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">"
        "<e:property>"
        "<SystemUpdateID>1</SystemUpdateID>"
        "</e:property>"
        "<e:property>"
        "<ContainerUpdateIDs></ContainerUpdateIDs>"
        "</e:property>"
        "<e:property>"
        "<TransferIDs></TransferIDs>"
        "</e:property>"
        "</e:propertyset>");
    
     pNotification->SetCallback(m_sCallback);
     pNotification->SetSID(m_sSID);
/*
NOTIFY / HTTP/1.1
HOST: 192.168.0.3:49152
CONTENT-TYPE: text/xml
CONTENT-LENGTH: 260
NT: upnp:event
NTS: upnp:propchange
SID: uuid:1
SEQ: 0
User-Agent: SSDP CD Events
Cache-Control: no-cache

<e:propertyset xmlns:e="urn:schemas-upnp-org:event-1-0">
<e:property>
<SystemUpdateID>1</SystemUpdateID>
</e:property>
<e:property>
<ContainerUpdateIDs></ContainerUpdateIDs>
</e:property>
</e:propertyset>

HTTP/1.1 200 OK
SERVER: Linux/2.6.16.16, UPnP/1.0, Intel SDK for UPnP devices /1.2
CONNECTION: close
CONTENT-LENGTH: 41
CONTENT-TYPE: text/html

<html><body><h1>200 OK</h1></body></html>
*/
    
      break;
    case UPNP_SERVICE_CONNECTION_MANAGER :
      
      pNotification->SetContent(
        "<e:propertyset xmlns:e=\"urn:schemas-upnp-org:event-1-0\">"
        "<e:property>"
        "<SourceProtocolInfo>http-get:*:audio/mpeg:*,http-get:*:audio/mpegurl:*,http-get:*:image/jpeg:*</SourceProtocolInfo>"
        "</e:property>"
        "<e:property>"
        "<SinkProtocolInfo></SinkProtocolInfo>"
        "</e:property>"
        "<e:property>"
        "<CurrentConnectionIDs>0</CurrentConnectionIDs>"
        "</e:property>"
        "</e:propertyset>");
    
     pNotification->SetCallback(m_sCallback);
     pNotification->SetSID(m_sSID);      
      
/*
NOTIFY / HTTP/1.1
HOST: 192.168.0.3:49152
CONTENT-TYPE: text/xml
CONTENT-LENGTH: 356
NT: upnp:event
NTS: upnp:propchange
SID: uuid:0
SEQ: 0
User-Agent: SSDP CD Events
Cache-Control: no-cache

<e:propertyset xmlns:e="urn:schemas-upnp-org:event-1-0">
<e:property>
<SourceProtocolInfo>http-get:*:audio/mpeg:*,http-get:*:audio/mpegurl:*,http-get:*:image/jpeg:*</SourceProtocolInfo>
</e:property>
<e:property>
<SinkProtocolInfo></SinkProtocolInfo>
</e:property>
<e:property>
<CurrentConnectionIDs>0</CurrentConnectionIDs>
</e:property>
</e:propertyset>

HTTP/1.1 200 OK
SERVER: Linux/2.6.16.16, UPnP/1.0, Intel SDK for UPnP devices /1.2
CONNECTION: close
CONTENT-LENGTH: 41
CONTENT-TYPE: text/html

<html><body><h1>200 OK</h1></body></html>
*/    
    
      break;
    
    default:
      break;
  }
  
  this->GetHTTPClient()->AsyncNotify(pNotification);
  delete pNotification;
}


CSubscriptionCache* CSubscriptionCache::m_pInstance = 0;

CSubscriptionCache* CSubscriptionCache::Shared()
{
  if(m_pInstance == 0)
    m_pInstance = new CSubscriptionCache();
  return m_pInstance;
}


CSubscriptionCache::CSubscriptionCache()
{
  fuppesThreadInitMutex(&m_Mutex);
}

CSubscriptionCache::~CSubscriptionCache()
{
  fuppesThreadDestroyMutex(&m_Mutex);
}

void CSubscriptionCache::Lock()
{
  fuppesThreadLockMutex(&m_Mutex); 
}

void CSubscriptionCache::Unlock()
{
  fuppesThreadUnlockMutex(&m_Mutex);  
}

void CSubscriptionCache::AddSubscription(CSubscription* pSubscription)
{  
  string sSID = GenerateUUID();
  pSubscription->SetSID(sSID);
  
  fuppesThreadLockMutex(&m_Mutex);  
  m_Subscriptions[sSID] = pSubscription;
  fuppesThreadUnlockMutex(&m_Mutex);  
}

bool CSubscriptionCache::RenewSubscription(std::string pSID)
{
  CSharedLog::Shared()->Log(L_EXT, "renew subscription \"" + pSID + "\"", __FILE__, __LINE__);  
  
  bool bResult = false;
  
  fuppesThreadLockMutex(&m_Mutex);
  m_SubscriptionsIterator = m_Subscriptions.find(pSID);
  if(m_SubscriptionsIterator != m_Subscriptions.end())
  {    
    CSharedLog::Shared()->Log(L_EXT, "renew subscription \"" + pSID + "\" done", __FILE__, __LINE__);
    m_Subscriptions[pSID]->Renew();
    bResult = true;
  }
  else
  {
    CSharedLog::Shared()->Log(L_EXT, "renew subscription \"" + pSID + "\" failed", __FILE__, __LINE__);
    bResult = false;
  }
  
  fuppesThreadUnlockMutex(&m_Mutex);
  return bResult;  
}

bool CSubscriptionCache::DeleteSubscription(std::string pSID)
{
  CSharedLog::Shared()->Log(L_EXT, "delete subscription \"" + pSID + "\"", __FILE__, __LINE__);  
    
  bool bResult = false;
  CSubscription* pSubscription;
  
  fuppesThreadLockMutex(&m_Mutex);    
  m_SubscriptionsIterator = m_Subscriptions.find(pSID);  
  if(m_SubscriptionsIterator != m_Subscriptions.end())
  { 
    CSharedLog::Shared()->Log(L_EXT, "delete subscription \"" + pSID + "\" done", __FILE__, __LINE__);    
    pSubscription = (*m_SubscriptionsIterator).second;
    m_Subscriptions.erase(pSID);
    delete pSubscription;
    bResult = true;
  }
  else
  {
    CSharedLog::Shared()->Log(L_EXT, "delete subscription \"" + pSID + "\" faild", __FILE__, __LINE__);
    bResult = false;
  }
  
  fuppesThreadUnlockMutex(&m_Mutex);
  return bResult;  
}


fuppesThreadCallback MainLoop(void *arg);

CSubscriptionMgr* CSubscriptionMgr::m_pInstance = 0;

CSubscriptionMgr* CSubscriptionMgr::Shared()
{
  if(m_pInstance == 0)
    m_pInstance = new CSubscriptionMgr(true);
  return m_pInstance;
}

CSubscriptionMgr::CSubscriptionMgr(bool p_bSingelton)
{ 
  m_MainLoop = (fuppesThread)NULL;
  m_bDoLoop = true;
  
  // start main loop in singleton instance only
  if(p_bSingelton && !m_MainLoop) {
    CSharedLog::Shared()->Log(L_DBG, "start CSubscriptionMgr MainLoop", __FILE__, __LINE__);
    fuppesThreadStartArg(m_MainLoop, MainLoop, *this);
  }
}

CSubscriptionMgr::~CSubscriptionMgr()
{
  m_bDoLoop = false;
	
	if(m_MainLoop) {
    fuppesThreadCancel(m_MainLoop);
    fuppesThreadClose(m_MainLoop);
    m_MainLoop = (fuppesThread)NULL;  
  }
}

bool CSubscriptionMgr::HandleSubscription(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  CSharedLog::Shared()->Log(L_DBG, pRequest->GetHeader(), __FILE__, __LINE__); 
  
  CSubscription* pSubscription = new CSubscription();
  try {    
    this->ParseSubscription(pRequest, pSubscription);
  } 
  catch (EException ex) {
    CSharedLog::Shared()->Log(L_EXT, ex.What(), __FILE__, __LINE__);
    
    delete pSubscription;
    return false;
  }
    
  switch(pSubscription->GetType())
  {
    case ST_SUBSCRIBE:
      CSubscriptionCache::Shared()->AddSubscription(pSubscription);
      pResponse->SetGENASubscriptionID(pSubscription->GetSID());
      break;
    case ST_RENEW:
      CSubscriptionCache::Shared()->RenewSubscription(pSubscription->GetSID());      
      pResponse->SetGENASubscriptionID(pSubscription->GetSID());
      delete pSubscription;
      break;
    case ST_UNSUBSCRIBE:
      CSubscriptionCache::Shared()->DeleteSubscription(pSubscription->GetSID());
      delete pSubscription;
      break;
    
    default:
      delete pSubscription;
      return false;    
  }

  return true;
}


void CSubscriptionMgr::ParseSubscription(CHTTPMessage* pRequest, CSubscription* pSubscription)
{
  // subscription type
  RegEx rxSubscribe("([SUBSCRIBE|UNSUBSCRIBE]+)", PCRE_CASELESS);
  if(!rxSubscribe.Search(pRequest->GetHeader().c_str()))
    throw EException("parsing subscription", __FILE__, __LINE__);
  
  // subscription target
  RegEx rxTarget("[SUBSCRIBE|UNSUBSCRIBE] +(.+) +HTTP/1\\.[1|0]", PCRE_CASELESS);
  if(rxTarget.Search(pRequest->GetHeader().c_str()))
  {    
    if (ToLower(rxTarget.Match(1)).compare("/upnpservices/contentdirectory/event/") == 0) {
      pSubscription->SetSubscriptionTarget(UPNP_SERVICE_CONTENT_DIRECTORY);
    }
    else if (ToLower(rxTarget.Match(1)).compare("/upnpservices/connectionmanager/event/") == 0) {
      pSubscription->SetSubscriptionTarget(UPNP_SERVICE_CONNECTION_MANAGER);
    }
    else {
      throw EException("unknown subscription target", __FILE__, __LINE__);
    }    
  }
  else {
    throw EException("parsing subscription", __FILE__, __LINE__);    
  }
  
  // SID
  string sSID = "";
  RegEx rxSID("SID: *uuid:([A-Z|0-9|-]+)", PCRE_CASELESS);
  if(rxSID.Search(pRequest->GetHeader().c_str()))
    sSID = rxSID.Match(1);  
  
  string sSubscribe = ToLower(rxSubscribe.Match(1));
  if(sSubscribe.compare("subscribe") == 0)
  {
    if(sSID.length() == 0)
    {
      pSubscription->SetType(ST_SUBSCRIBE);
      
      RegEx rxCallback("CALLBACK: *<*(http://[A-Z|0-9|\\-|_|/|\\.|:]+)>*", PCRE_CASELESS);
      if(rxCallback.Search(pRequest->GetHeader().c_str())) {
                                                           
        string sCallback = rxCallback.Match(1);
        if(sCallback[sCallback.length() - 1] != '/')
          sCallback += "/";
        pSubscription->SetCallback(sCallback);
      }
      else {
        throw EException("parsing subscription callback", __FILE__, __LINE__);
      }      
    }
    else
    {
      pSubscription->SetType(ST_RENEW);
      pSubscription->SetSID(sSID);
    }
    
    pSubscription->SetTimeout(180);    
  }
  else if(sSubscribe.compare("unsubscribe") == 0)
  {
    pSubscription->SetType(ST_UNSUBSCRIBE);
    pSubscription->SetSID(sSID);
  }  
}


/**
 * the CSubscriptionMgr's MainLoop constantly
 * walks the subscriptions and handles them
 */
fuppesThreadCallback MainLoop(void *arg)
{
  CSubscriptionMgr* pMgr = (CSubscriptionMgr*)arg;
  CSubscription* pSubscr = NULL;
  std::map<std::string, CSubscription*>::iterator tmpIt;    
  
  while(pMgr->m_bDoLoop)
  {
    //cout << "CSubscriptionMgr::MainLoop" << endl;
    
    CSubscriptionCache::Shared()->Lock();
    
    // walk subscriptions and decrement timeout
    for(CSubscriptionCache::Shared()->m_SubscriptionsIterator  = CSubscriptionCache::Shared()->m_Subscriptions.begin();
        CSubscriptionCache::Shared()->m_SubscriptionsIterator != CSubscriptionCache::Shared()->m_Subscriptions.end();)
    {
      if(CSubscriptionCache::Shared()->m_Subscriptions.size() == 0) {
        break;
      }      
        
      pSubscr = (*CSubscriptionCache::Shared()->m_SubscriptionsIterator).second;
      
      //cout << "  subscr: " << pSubscr->GetSID() << endl;
      //cout << "    time left: " << pSubscr->GetTimeLeft() << endl;
      
      pSubscr->DecTimeLeft();      
      
      if(!pSubscr->m_bHandled)
      {        
        #warning todo: max send freq 0.5hz for "SystemUpdateID" and "ContainerUpdateIDs"
        
        pSubscr->m_bHandled = true;        
        pSubscr->AsyncReply();
      }
      
      if(pSubscr->GetTimeLeft() == 0) {        
        tmpIt = CSubscriptionCache::Shared()->m_SubscriptionsIterator;
        ++tmpIt;
        CSubscriptionCache::Shared()->m_Subscriptions.erase(pSubscr->GetSID());        
        CSubscriptionCache::Shared()->m_SubscriptionsIterator = tmpIt;
        delete pSubscr;
      }
      else {
        CSubscriptionCache::Shared()->m_SubscriptionsIterator++;
      }
    }
    
    CSubscriptionCache::Shared()->Unlock();    
    
    fuppesSleep(1000);
  }
  
  fuppesThreadExit();
}

/* SUBSCRIBE
  
SUBSCRIBE /UPnPServices/ContentDirectory/event/ HTTP/1.1
HOST: 192.168.0.3:48756
NT: upnp:event
TIMEOUT: Second-180
User-Agent: POSIX, UPnP/1.0, Intel MicroStack/1.0.1423
CALLBACK: http://192.168.0.23:54877/UPnPServices/ContentDirectory/event/
*/
  
/* RENEW

SUBSCRIBE /UPnPServices/ConnectionManager/event/ HTTP/1.1
HOST: 192.168.0.3:48756
SID: uuid:b7e3b9b4-7059-472e-95ba-5401708fa2de
TIMEOUT: Second-180
User-Agent: POSIX, UPnP/1.0, Intel MicroStack/1.0.1423
*/

/* UNSUBSCRIBE

UNSUBSCRIBE /UPnPServices/ContentDirectory/event/ HTTP/1.1
HOST: 192.168.0.3:58642
SID: uuid:b5117436-a4ef-4944-9fb0-30190a83e2aa
USER-AGENT: Linux/2.6.16.16, UPnP/1.0, Intel SDK for UPnP devices /1.2
*/


/*
NOTIFY / HTTP/1.1
HOST: 192.168.0.3:49152
CONTENT-TYPE: text/xml
CONTENT-LENGTH: 356
NT: upnp:event
NTS: upnp:propchange
SID: uuid:0
SEQ: 0
User-Agent: SSDP CD Events
Cache-Control: no-cache

<e:propertyset xmlns:e="urn:schemas-upnp-org:event-1-0">
<e:property>
<SourceProtocolInfo>http-get:*:audio/mpeg:*,http-get:*:audio/mpegurl:*,http-get:*:image/jpeg:*</SourceProtocolInfo>
</e:property>
<e:property>
<SinkProtocolInfo></SinkProtocolInfo>
</e:property>
<e:property>
<CurrentConnectionIDs>0</CurrentConnectionIDs>
</e:property>
</e:propertyset>

HTTP/1.1 200 OK
SERVER: Linux/2.6.16.16, UPnP/1.0, Intel SDK for UPnP devices /1.2
CONNECTION: close
CONTENT-LENGTH: 41
CONTENT-TYPE: text/html

<html><body><h1>200 OK</h1></body></html>
*/


/*
NOTIFY / HTTP/1.1
HOST: 192.168.0.3:49152
CONTENT-TYPE: text/xml
CONTENT-LENGTH: 260
NT: upnp:event
NTS: upnp:propchange
SID: uuid:1
SEQ: 0
User-Agent: SSDP CD Events
Cache-Control: no-cache

<e:propertyset xmlns:e="urn:schemas-upnp-org:event-1-0">
<e:property>
<SystemUpdateID>1</SystemUpdateID>
</e:property>
<e:property>
<ContainerUpdateIDs></ContainerUpdateIDs>
</e:property>
<e:property>
<TransferIDs></TransferIDs>
</e:property>
</e:propertyset>

HTTP/1.1 200 OK
SERVER: Linux/2.6.16.16, UPnP/1.0, Intel SDK for UPnP devices /1.2
CONNECTION: close
CONTENT-LENGTH: 41
CONTENT-TYPE: text/html

<html><body><h1>200 OK</h1></body></html>
*/
