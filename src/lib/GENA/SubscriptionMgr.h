/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            SubscriptionMgr.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _SUBSCRIPTIONMGR_H
#define _SUBSCRIPTIONMGR_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include <map>
#include "../Common/Thread.h"
#include "../HTTP/HTTPMessage.h"
#include "../HTTP/HTTPClient.h"
 
typedef enum tagSUBSCRIPTION_TYPE
{
  ST_SUBSCRIBE   = 0,
  ST_RENEW       = 1,
  ST_UNSUBSCRIBE = 2
} SUBSCRIPTION_TYPE;

class CSubscription
{
  public:
    CSubscription();  
    ~CSubscription();
  
    std::string GetSID() { return m_sSID; }
    void SetSID(std::string p_sSID) { m_sSID = p_sSID; }
    
    unsigned int GetTimeout() { return m_nTimeout; }
    void SetTimeout(unsigned int p_nTimeout) { m_nTimeout = p_nTimeout; m_nTimeLeft = m_nTimeout; }
        
    
    SUBSCRIPTION_TYPE GetType() { return m_nSubscriptionType; }
    void SetType(SUBSCRIPTION_TYPE p_nSubscriptionType) { m_nSubscriptionType = p_nSubscriptionType; }
    
    UPNP_DEVICE_TYPE GetSubscriptionTarget() { return m_nSubscriptionTarget; }
    void SetSubscriptionTarget(UPNP_DEVICE_TYPE p_nTargetDeviceType) { m_nSubscriptionTarget = p_nTargetDeviceType; }
    
    std::string GetCallback() { return m_sCallback; }
    void SetCallback(std::string p_sCallback) { m_sCallback = p_sCallback; }
    
    
    
    void Renew();
    void DecTimeLeft();
    unsigned int GetTimeLeft() { return m_nTimeLeft; }
    
    bool m_bHandled;
    
    
    void AsyncReply();
    
        
  private:
    std::string        m_sSID;
    unsigned int       m_nTimeout;
    unsigned int       m_nTimeLeft;
    std::string        m_sCallback;
    SUBSCRIPTION_TYPE  m_nSubscriptionType;  
    UPNP_DEVICE_TYPE   m_nSubscriptionTarget;
  
    CHTTPClient* GetHTTPClient();
    CHTTPClient* m_pHTTPClient;
};

class CSubscriptionCache
{
  public:
    static CSubscriptionCache* Shared();
  
    void AddSubscription(CSubscription* pSubscription);
    bool RenewSubscription(std::string pSID);
    bool DeleteSubscription(std::string pSID);    
  
    void Lock();
    void Unlock();
  
  private:
    static CSubscriptionCache* m_pInstance;
    fuppesThreadMutex  m_Mutex;
  
    CSubscriptionCache();
    ~CSubscriptionCache();
  
public:  
    std::map<std::string, CSubscription*>           m_Subscriptions;  
    std::map<std::string, CSubscription*>::iterator m_SubscriptionsIterator;  
};

class CSubscriptionMgr: private fuppes::Thread
{
  public:
    static CSubscriptionMgr* Shared();
    static void deleteInstance();
    ~CSubscriptionMgr();
  
    static bool HandleSubscription(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
  
  private:  
    CSubscriptionMgr();
    static CSubscriptionMgr* m_pInstance;   
  
    static void ParseSubscription(CHTTPMessage* pRequest, CSubscription* pSubscription);  

    std::map<std::string, CSubscription*>           m_Subscriptions;
    std::map<std::string, CSubscription*>::iterator m_SubscriptionsIterator;  

		void run();
};

#endif // _SUBSCRIPTIONMGR_H
