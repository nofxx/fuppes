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
#include <fstream>
using namespace std;

CFuppes::CFuppes()
{
  cout << "[FUPPES] initializing devices" << endl;
  
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
  cout << "[FUPPES] done" << endl; 
  
  cout << "[FUPPES] multicasting alive messages" << endl;
  m_pSSDPCtrl->send_alive();  
  cout << "[FUPPES] done" << endl; 
}

CFuppes::~CFuppes()
{
  cout << "[FUPPES] multicasting byebye messages" << endl;
  m_pSSDPCtrl->send_byebye();
  cout << "[FUPPES] done" << endl; 
 
  cout << "[FUPPES] shutting down" << endl;		  
	delete m_pMediaServer;
  delete m_pContentDirectory;
	delete m_pHTTPServer;
	delete m_pSSDPCtrl;
  cout << "[FUPPES] done" << endl; 
}

CSSDPCtrl* CFuppes::GetSSDPCtrl()
{
	return m_pSSDPCtrl;
}

void CFuppes::OnSSDPCtrlReceiveMsg(CSSDPMessage* pSSDPMessage)
{
}

CHTTPMessage* CFuppes::OnHTTPServerReceiveMsg(CHTTPMessage* pHTTPMessage)
{
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
	if(pHTTPMessage->GetRequest().compare("/") == 0)
	{
		pResult = new CHTTPMessage(http_200_ok, text_xml);
		pResult->SetContent(m_pMediaServer->GetDeviceDescription());
	}
	else if(pHTTPMessage->GetRequest().compare("/UPnPServices/ContentDirectory/description.xml") == 0)
	{
		pResult = new CHTTPMessage(http_200_ok, text_xml);
		pResult->SetContent(m_pContentDirectory->GetServiceDescription());
  }
  else if((pHTTPMessage->GetRequest().length() > 24) &&
          ((pHTTPMessage->GetRequest().length() > 24) && 
         (pHTTPMessage->GetRequest().substr(24).compare("/MediaServer/AudioItems/"))))
  {
    string sItemObjId = pHTTPMessage->GetRequest().substr(24, pHTTPMessage->GetRequest().length());
    string sFileName  = m_pContentDirectory->GetFileNameFromObjectID(sItemObjId);
    
    if(FileExists(sFileName))
    {
      pResult = new CHTTPMessage(http_200_ok, audio_mpeg);
      pResult->LoadContentFromFile(sFileName);
    }    
    cout << "[FUPPES] sending audio file " << sFileName << endl;    
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
