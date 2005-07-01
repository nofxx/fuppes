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
  
  // init SSDP receiver	
	m_SSDPCtrl.SetReceiveHandler(this);
	m_SSDPCtrl.Start();
	
  // init HTTP server
	m_HTTPServer.SetReceiveHandler(this);
	m_HTTPServer.Start();  
  CSharedConfig::Shared()->SetHTTPServerURL(m_HTTPServer.GetURL());	  
	CNotifyMsgFactory::shared()->SetHTTPServerURL(m_HTTPServer.GetURL());	
	
  // create media server ...
	m_MediaServer.SetHTTPServerURL(m_HTTPServer.GetURL());
  // ... and add the content dir to its services
	m_MediaServer.AddUPnPService((CUPnPService*)&m_ContentDirectory);	  
  
  cout << "[FUPPES] done" << endl;   
  cout << "[FUPPES] multicasting alive messages" << endl;
  m_SSDPCtrl.send_alive();  
  cout << "[FUPPES] done" << endl; 
}

CFuppes::~CFuppes()
{
  cout << "[FUPPES] shutting down" << endl;	
  cout << "[FUPPES] multicasting byebye messages" << endl;
  m_SSDPCtrl.send_byebye();
  cout << "[FUPPES] done" << endl; 
}

CSSDPCtrl* CFuppes::GetSSDPCtrl()
{
	return &m_SSDPCtrl;
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
		pResult->SetContent(m_MediaServer.GetDeviceDescription());
	}
  
  // content dir description
	else if(pHTTPMessage->GetRequest().compare("/UPnPServices/ContentDirectory/description.xml") == 0)
	{
		pResult = new CHTTPMessage(http_200_ok, text_xml);
		pResult->SetContent(m_ContentDirectory.GetServiceDescription());
  }
  
  // presentation
  else if(ToLower(pHTTPMessage->GetRequest()).compare("/index.html") == 0)
  {
    pResult = new CHTTPMessage(http_200_ok, text_html);
		pResult->SetContent(m_PresentationHandler.GetIndexHTML());    
  }
    
  else if((pHTTPMessage->GetRequest().length() > 24) &&
          ((pHTTPMessage->GetRequest().length() > 24) && 
         (pHTTPMessage->GetRequest().substr(24).compare("/MediaServer/AudioItems/"))))
  {
    string sItemObjId = pHTTPMessage->GetRequest().substr(24, pHTTPMessage->GetRequest().length());
    string sFileName  = m_ContentDirectory.GetFileNameFromObjectID(sItemObjId);
    
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
      pResult = m_ContentDirectory.HandleUPnPAction(pHTTPMessage->GetAction());
    }
  }
  
  delete pHTTPMessage;  
  return pResult;
}
