/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            SoapControl.cpp
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "SoapControl.h"

#include "ControlInterface.h"
#include "../SharedConfig.h"

#include <iostream>
#include <sstream>
using namespace std;


#include "../ContentDirectory/ContentDatabase.h"


SoapControl::SoapControl(std::string HTTPServerURL):
CUPnPService(FUPPES_SOAP_CONTROL, 1, HTTPServerURL)
{
}

std::string SoapControl::GetServiceDescription()
{
	return "";
}

void SoapControl::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
	FuppesCtrlAction* action = (FuppesCtrlAction*)pUPnPAction;
	
	cout << "SoapControl:: handleAction : " << pUPnPAction->GetContent() << endl;


	CXMLDocument request;
	request.LoadFromString(pUPnPAction->GetContent());


	CXMLNode* tmp = request.RootNode()->FindNodeByName("Body");
	if(tmp == NULL)
		return;

	tmp = tmp->ChildNode(0);
	
	stringstream content;

	switch(action->type()) {


		case FUPPES_CTRL_GET_DIR:
			getDir(tmp, content);
			break;
			
		case FUPPES_CTRL_GET_SHARED_OBJECTS:
			getSharedObjects(content);
			break;

		case FUPPES_CTRL_ADD_SHARED_OBJECT:
			addSharedObject(tmp, content);
			break;

		case FUPPES_CTRL_DEL_SHARED_OBJECT:
			delSharedObject(tmp, content);
			break;
			
		
		case FUPPES_CTRL_DATABASE_REBUILD:
			break;
	}
	

	if(action->type() == FUPPES_CTRL_TEST) {
			content << 
				"<c:TestResponse xmlns:c=\"urn:fuppesControl\">"
				"<Result>test</Result>"
				"</c:TestResponse>";


		//CContentDatabase::exportData("/home/ulrich/Desktop/export.db", "/home/ulrich/Desktop/fuppes-test/Oregon/", true);		
	}


	string data;
  if(!content.str().empty()) { 
		data = "<c:" + tmp->name() + "Response xmlns:c=\"urn:fuppesControl\">";
		data += "<Result>" + content.str() + "</Result>";
		data += "</c:" + tmp->name() + "Response>";
	}
	else {	
		data = "<c:Error xmlns:c=\"urn:fuppesControl\">"
				"<Code>123</Code>"
				"<Message>fuppes soap control error</Message>"
				"</c:Error>";
	}
  
	string result = 
		"<?xml version=\"1.0\" encoding=\"utf-8\"?>"
		"<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
		"  <s:Body>"
		+ data +
		"  </s:Body>"
		"</s:Envelope>";
	
	pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_200_OK, "text/xml; charset=\"utf-8\"");	
  pMessageOut->SetContent(result);

cout << result << endl;
	
/*
  else {
    pMessageOut->SetMessage(HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR, "text/xml; charset=\"utf-8\"");            

    content = 
    "<?xml version=\"1.0\" encoding=\"utf-8\"?>"  
    "<s:Envelope xmlns:s=\"http://schemas.xmlsoap.org/soap/envelope/\" s:encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\">"
    "  <s:Body>"
    "    <s:Fault>"
    "      <faultcode>s:Client</faultcode>"
    "      <faultstring>UPnPError</faultstring>"
    "      <detail>"
    "        <UPnPError xmlns=\"urn:schemas-upnp-org:control-1-0\">"
    "          <errorCode>401</errorCode>"
    "          <errorDescription>Invalid Action</errorDescription>"
    "        </UPnPError>"
    "      </detail>"
    "    </s:Fault>"
    "  </s:Body>"
    "</s:Envelope>";
    
    pMessageOut->SetContent(content);
	}
*/
}


void SoapControl::getDir(CXMLNode* request, std::stringstream& result)
{
	CXMLNode* dirNode = request->FindNodeByName("dir");
	if(!dirNode)
		return;

	string path = dirNode->Value();
	bool files = (dirNode->Attribute("show-files") == "true");
	
	cout << "PATH: " << path << "*" << endl;

	if(path.empty()) {
#ifndef WIN32
		path = "/";		
#else
		path = "/";
#endif
		
	}


#ifdef WIN32
	if(path == "/") {
		
		DWORD drives = GetLogicalDrives();
		
		cout << "DRIVES: DWORD: " << drives << endl;
		
		DWORD bit = 0x01;
		char  letter = 'A';
		while(bit != 0x8000000) {
			if(drives & bit) {
				result << "<dir name=\"" << letter << ":/\">" << letter << ":/" << "</dir>";		
			}
			bit = bit << 1;
			letter++;
		}
		
		//result << "<dir name=\"" << iter->name() << "\">" << iter->path() << "</dir>";
		
		return;
	}	
#endif

	fuppes::Directory dir(path);
	dir.open();
	fuppes::DirEntryList info = dir.dirEntryList();
	fuppes::DirEntryListIterator iter;

	if(path != "/")
		result << "<parent>" << dir.path() << "..</parent>";
	
	for(iter = info.begin();
	    iter != info.end();
	    iter++) {

		if(!files && iter->type() != fuppes::DirEntry::Directory)
			continue;
				
		cout << iter->path() << "* " << iter->name()<< "*" << endl;
		result << "<dir name=\"" << iter->name() << "\">" << iter->path() << "</dir>";
	}

	dir.close();
	

}

void SoapControl::getSharedObjects(std::stringstream& result)
{
	SharedObject* obj;
	
	for(int i = 0;
	    i <	CSharedConfig::sharedObjects()->sharedObjectCount();
	    i++) {

		obj = CSharedConfig::sharedObjects()->sharedObject(i);

		result << "<shared-object>";
		result << "<index>" << i << "</index>";
		result << "<path>" << obj->path() << "</path>";
				
		switch(obj->type()) {
			case SharedObject::directory:
				result << "<type>directory</type>";
				break;
			case SharedObject::playlist:
				result << "<type>playlist</type>";
				break;
			case SharedObject::itunes:
				result << "<type>itunes</type>";
				break;
		}

		result << "</shared-object>";
				
	}
}




void SoapControl::addSharedObject(CXMLNode* request, std::stringstream& result)
{
	CXMLNode* typeNode = request->FindNodeByName("type", true);
	CXMLNode* pathNode = request->FindNodeByName("path", true);
	if(typeNode == NULL || pathNode == NULL) {
		return;
	}

	if(typeNode->Value().compare("directory") == 0) {
		CSharedConfig::sharedObjects()->AddSharedDirectory(pathNode->Value());

		string folder;
		ExtractFolderFromPath(pathNode->Value(), &folder);		
		CContentDatabase::insertDirectory(pathNode->Value(), folder, 0, NULL, true);
		CContentDatabase::fileAlterationMonitor()->addWatch(pathNode->Value());
		CContentDatabase::scanDirectory(pathNode->Value());
		
		result << "<Code>0</Code><Message>OK</Message>";	
	}

}

void SoapControl::delSharedObject(CXMLNode* request, std::stringstream& result)
{
	CXMLNode* indexNode = request->FindNodeByName("index", true);
	if(indexNode == NULL) {
		return;
	}

	SharedObject* obj = CSharedConfig::sharedObjects()->sharedObject(indexNode->ValueAsInt());
	

}
