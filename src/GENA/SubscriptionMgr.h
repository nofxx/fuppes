/***************************************************************************
 *            SubscriptionMgr.h
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
 
#ifndef _SUBSCRIPTIONMGR_H
#define _SUBSCRIPTIONMGR_H

#include <string>
#include <map>
#include "../HTTP/HTTPMessage.h"
 
#define GENA_OK           0;
#define GENA_PARSE_ERROR  1;
#define GENA_UUID_UNKNOWN 2;
 
typedef enum tagSUBSCRIPTION_TYPE
{
  ST_SUBSCRIBE   = 0,
  ST_RENEW       = 1,
  ST_UNSUBSCRIBE = 2
} SUBSCRIPTION_TYPE;
 
class CSubscription
{
  public:
    std::string GetSID() { return m_sSID; }
    void SetSID(std::string p_sSID) { m_sSID = p_sSID; }
    
    unsigned int GetTimeout() { return m_nTimeout; }
    void SetTimeout(unsigned int p_nTimeout) { m_nTimeout = p_nTimeout; }
    
    std::string GetCallback() { return m_sCallback; }
    void SetCallback(std::string p_sCallback) { m_sCallback = p_sCallback; }
    
    SUBSCRIPTION_TYPE GetType() { return m_nSubscriptionType; }
    void SetType(SUBSCRIPTION_TYPE p_nSubscriptionType) { m_nSubscriptionType = p_nSubscriptionType; }
    
    void Renew() {}
    
  private:
    std::string      m_sSID;
    unsigned int     m_nTimeout;
    std::string      m_sCallback;
    SUBSCRIPTION_TYPE  m_nSubscriptionType;
};
 
class CSubscriptionMgr
{
  public:
    static CSubscriptionMgr* Shared();

    bool HandleSubscription(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
  private:  
    static CSubscriptionMgr* m_pInstance;
  
    int ParseSubscription(CHTTPMessage* pRequest, CSubscription* pSubscription);
  
    void AddSubscription(CSubscription* pSubscription);
    bool RenewSubscription(CSubscription* pSubscription);
    bool DeleteSubscription(CSubscription* pSubscription);
  
    std::map<std::string, CSubscription*>           m_Subscriptions;
    std::map<std::string, CSubscription*>::iterator m_SubscriptionsIterator;  
    
};

#endif /* _SUBSCRIPTIONMGR_H */
