/***************************************************************************
 *            SubscriptionMgr.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "SubscriptionMgr.h"
#include "../Common/RegEx.h"
#include "../Common/UUID.h"

void CSubscription::Renew()
{
  m_nTimeLeft = m_nTimeout;
}

void CSubscription::DecTimeLeft()
{
  if(m_nTimeLeft > 0)
    m_nTimeLeft--;
}



fuppesThreadMutex  g_SubscriptionsMutex;
fuppesThreadCallback MainLoop(void *arg);

CSubscriptionMgr* CSubscriptionMgr::m_pInstance = 0;

CSubscriptionMgr* CSubscriptionMgr::Shared()
{
  if(m_pInstance == 0)
    m_pInstance = new CSubscriptionMgr();
  return m_pInstance;
}

CSubscriptionMgr::CSubscriptionMgr()
{  
  fuppesThreadInitMutex(&g_SubscriptionsMutex);
  m_MainLoop = (fuppesThread)NULL;
  m_bDoLoop = true;
  
  fuppesThreadStartArg(m_MainLoop, MainLoop, *this);
}

CSubscriptionMgr::~CSubscriptionMgr()
{
  m_bDoLoop = false;
  
  int nExitCode;
  fuppesThreadCancel(m_MainLoop, nExitCode);
  fuppesThreadClose(m_MainLoop);
  m_MainLoop = (fuppesThread)NULL;  
  
  fuppesThreadDestroyMutex(&g_SubscriptionsMutex);
}

bool CSubscriptionMgr::HandleSubscription(CHTTPMessage* pRequest, CHTTPMessage* pResponse)
{
  CSubscription* pSubscription = new CSubscription();
  if(!this->ParseSubscription(pRequest, pSubscription))
  {
    delete pSubscription;
    return false;
  }
  
  cout << "SWITCH TYPE: " << pSubscription->GetType() << endl;
  
  switch(pSubscription->GetType())
  {
    case ST_SUBSCRIBE:
      AddSubscription(pSubscription);
      pResponse->SetGENASubscriptionID(pSubscription->GetSID());
      break;
    case ST_RENEW:
      RenewSubscription(pSubscription);
      cout << " pResponse->SetGENASubscriptionID(pSubscription->GetSID());" <<  endl << pSubscription->GetSID() << endl;
      pResponse->SetGENASubscriptionID(pSubscription->GetSID());
      delete pSubscription;
      break;
    case ST_UNSUBSCRIBE:
      DeleteSubscription(pSubscription);
      delete pSubscription;
      break;
    
    default:
      delete pSubscription;
      return false;    
  }
  
  
  return true;
}


bool CSubscriptionMgr::ParseSubscription(CHTTPMessage* pRequest, CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::ParseSubscription" << endl;
  //cout << pRequest->GetHeader() << endl;
  cout << endl << "SUBSCR COUNT: " << m_Subscriptions.size() << endl;
  
  RegEx rxSubscribe("([SUBSCRIBE|UNSUBSCRIBE]+)", PCRE_CASELESS);
  if(!rxSubscribe.Search(pRequest->GetHeader().c_str()))
    return false;
  
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
      cout << "SUBSCRIPTION" << endl;
    }
    else
    {
      pSubscription->SetType(ST_RENEW);
      pSubscription->SetSID(sSID);
      cout << "RENEW" << endl;
    }
    
    pSubscription->SetTimeout(180);    
  }
  else if(sSubscribe.compare("unsubscribe") == 0)
  {
    pSubscription->SetType(ST_UNSUBSCRIBE);
    pSubscription->SetSID(sSID);
    cout << "UNSUBSCRIPTION" << endl;
  }
  return true;      
}

void CSubscriptionMgr::AddSubscription(CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::AddSubscription" << endl;
  
  string sSID = GenerateUUID();
  pSubscription->SetSID(sSID);
  
  fuppesThreadLockMutex(&g_SubscriptionsMutex);  
  m_Subscriptions[sSID] = pSubscription;
  fuppesThreadUnlockMutex(&g_SubscriptionsMutex);
}

bool CSubscriptionMgr::RenewSubscription(CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::RenewSubscription" << endl;
  fuppesThreadLockMutex(&g_SubscriptionsMutex);  
  
  bool bResult = false;
  
  m_SubscriptionsIterator = m_Subscriptions.find(pSubscription->GetSID());
  if(m_SubscriptionsIterator != m_Subscriptions.end())
  {
    cout << "RENEW found" << endl;
    m_Subscriptions[pSubscription->GetSID()]->Renew();
    bResult = true;
  }
  else
  {
    cout << "RENEW NOT found" << endl;      
    bResult = false;
  }
  
  fuppesThreadUnlockMutex(&g_SubscriptionsMutex);
  return bResult;
}

bool CSubscriptionMgr::DeleteSubscription(CSubscription* pSubscription)
{
  cout << "CSubscriptionMgr::DeleteSubscription" << endl;
    
  fuppesThreadLockMutex(&g_SubscriptionsMutex);  
  
  bool bResult = false;
  
  m_SubscriptionsIterator = m_Subscriptions.find(pSubscription->GetSID());
  /* found device */
  if(m_SubscriptionsIterator != m_Subscriptions.end())
  { 
    m_Subscriptions.erase(pSubscription->GetSID());
    bResult = true;
  }
  else
  {
    bResult = false;
  }
  
  fuppesThreadUnlockMutex(&g_SubscriptionsMutex);
  return bResult;
}

fuppesThreadCallback MainLoop(void *arg)
{
  CSubscriptionMgr* pMgr = (CSubscriptionMgr*)arg;
  CSubscription* pSubscr = NULL;
  
  while(pMgr->m_bDoLoop)
  {
    //cout << "CSubscriptionMgr::MainLoop" << endl;
    
    fuppesThreadLockMutex(&g_SubscriptionsMutex);    
    
    for(pMgr->m_SubscriptionsIterator = pMgr->m_Subscriptions.begin();
        pMgr->m_SubscriptionsIterator != pMgr->m_Subscriptions.end();
        pMgr->m_SubscriptionsIterator++)
    {     
      pSubscr = (*pMgr->m_SubscriptionsIterator).second;
      
      //cout << "  subscr: " << pSubscr->GetSID() << endl;
      //cout << "    time left: " << pSubscr->GetTimeLeft() << endl;
      
      pSubscr->DecTimeLeft();      
      
      if(pSubscr->GetTimeLeft() == 0)
      {
        pMgr->m_Subscriptions.erase(pSubscr->GetSID());        
        delete pSubscr;
      }    
    }
    
    fuppesThreadUnlockMutex(&g_SubscriptionsMutex);
    
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
