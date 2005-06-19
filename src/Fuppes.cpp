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

#include <iostream>
using namespace std;

CFuppes::CFuppes()
{
	m_pSSDPCtrl = new CSSDPCtrl();
	m_pSSDPCtrl->SetReceiveHandler(this);
	m_pSSDPCtrl->Start();
	
	m_pHTTPServer = new CHTTPServer();
	m_pHTTPServer->SetReceiveHandler(this);
	m_pHTTPServer->Start();
  CSharedConfig::Shared()->SetHTTPServerURL(m_pHTTPServer->GetURL());
	
	CNotifyMsgFactory::shared()->SetHTTPServerURL(m_pHTTPServer->GetURL());	
	
	m_pContentDirectory = new CContentDirectory();
		
	m_pMediaServer = new CMediaServer();
	m_pMediaServer->SetHTTPServerURL(m_pHTTPServer->GetURL());
	m_pMediaServer->AddUPnPService((CUPnPService*)m_pContentDirectory);	
}

CFuppes::~CFuppes()
{
	delete m_pMediaServer;
	delete m_pHTTPServer;
	delete m_pSSDPCtrl;
}

CSSDPCtrl* CFuppes::GetSSDPCtrl()
{
	return m_pSSDPCtrl;
}

void CFuppes::OnSSDPCtrlReceiveMsg(CSSDPMessage* pSSDPMessage)
{
	cout << "[fuppes] OnSSDPCtrlReceiveMsg:" << endl;
	cout << pSSDPMessage->GetContent() << endl;
}

CHTTPMessage* CFuppes::OnHTTPServerReceiveMsg(CHTTPMessage* pHTTPMessage)
{
	cout << "[fuppes] OnHTTPServerReceiveMsg:" << endl;
	
	CHTTPMessage* pResult = NULL;
	
	switch(pHTTPMessage->GetMessageType())
	{
		case http_get:
			pResult = HandleHTTPGet(pHTTPMessage);
		  break;
		case http_post:
			pResult = HandleHTTPPost(pHTTPMessage);
		  break;
		case http_200_ok:
			break;
		case http_404_not_found:
			break;
	}
		
	return pResult;
}

CHTTPMessage* CFuppes::HandleHTTPGet(CHTTPMessage* pHTTPMessage)
{
	CHTTPMessage* pResult = NULL;	
	// request == "/" => root description	
	if(pHTTPMessage->GetRequest().compare("/"))
	{
		pResult = new CHTTPMessage(http_200_ok, text_xml);
		pResult->SetContent(m_pMediaServer->GetDeviceDescription());
	}
	else if(pHTTPMessage->GetRequest().compare("/UPnPServices/ContentDirectory/description.xml"))
	{
		pResult = new CHTTPMessage(http_200_ok, text_xml);
		pResult->SetContent(m_pContentDirectory->GetServiceDescription());
  }
		
  delete pHTTPMessage;  
	return pResult;
}

CHTTPMessage* CFuppes::HandleHTTPPost(CHTTPMessage* pHTTPMessage)
{
  CHTTPMessage* pResult = NULL;	
  
  if(pHTTPMessage->GetAction() != NULL)
  {
    if(pHTTPMessage->GetAction()->m_TargetDevice == udtContentDirectory)
    {
      pResult = m_pContentDirectory->HandleUPnPAction(pHTTPMessage->GetAction());
    }
  }
  
  delete pHTTPMessage;  
  return pResult;
}
