/***************************************************************************
 *            Fuppes.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *  Copyright (C) 2005 Ulrich Völkel
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
 
#include "Fuppes.h"

#include "NotifyMsgFactory.h"
#include "SharedConfig.h"
#include "Common.h"

#include <iostream>
using namespace std;

CFuppes::CFuppes()
{
    m_SSDPCtrl.SetReceiveHandler(this);
    m_SSDPCtrl.Start();

    m_HTTPServer.SetReceiveHandler(this);
    m_HTTPServer.Start();
    CSharedConfig::Shared()->SetHTTPServerURL(m_HTTPServer.GetURL());

    CNotifyMsgFactory::shared()->SetHTTPServerURL(m_HTTPServer.GetURL());	

    m_MediaServer.SetHTTPServerURL(m_HTTPServer.GetURL());
    m_MediaServer.AddUPnPService((CUPnPService*)&m_ContentDirectory);	
}

CFuppes::~CFuppes()
{
}

CSSDPCtrl* CFuppes::GetSSDPCtrl()
{
	return &m_SSDPCtrl;
}

void CFuppes::OnSSDPCtrlReceiveMsg(CSSDPMessage* pSSDPMessage)
{
	cout << "[fuppes] OnSSDPCtrlReceiveMsg:" << endl;
	cout << pSSDPMessage->GetContent() << endl;
}

bool CFuppes::OnHTTPServerReceiveMsg(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
    cout << "[fuppes] OnHTTPServerReceiveMsg:" << endl;
    if(!pMessageIn)
        return false;
    if(!pMessageOut)
        return false;

    bool fRet = false;
    
    switch(pMessageIn->GetMessageType())
    {
    case http_get:
        fRet = HandleHTTPGet(pMessageIn, pMessageOut);
        break;
    case http_post:
        fRet = HandleHTTPPost(pMessageIn, pMessageOut);
        break;
    case http_200_ok:
        break;
    case http_404_not_found:
        break;
    }

    return fRet;
}

bool CFuppes::HandleHTTPGet(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
    if(!pMessageIn)
        return false;
    if(!pMessageOut)
        return false;
    
    // request == "/" => root description	
    if(pMessageIn->GetRequest().compare("/"))
    {
        pMessageOut->SetContent(m_MediaServer.GetDeviceDescription());
        return true;
    }
    else if(pMessageIn->GetRequest().compare("/UPnPServices/ContentDirectory/description.xml"))
    {
        pMessageOut->SetContent(m_ContentDirectory.GetServiceDescription());
        return true;
    }
    
    return false;
}

bool CFuppes::HandleHTTPPost(CHTTPMessage* pMessageIn, CHTTPMessage* pMessageOut)
{
    if(!pMessageIn)
        return false;
    if(!pMessageOut)
        return false;
    
    if(pMessageIn->GetAction() != NULL)
    {
        if(pMessageIn->GetAction()->m_TargetDevice == udtContentDirectory)
        {
            bool fRet = m_ContentDirectory.HandleUPnPAction(pMessageIn->GetAction(), pMessageOut);
            return fRet;
        }
    }

    return false;
}