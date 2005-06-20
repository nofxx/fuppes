/***************************************************************************
 *            ContentDirectory.cpp
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
 
 #include "ContentDirectory.h" 
 #include "StorageFolder.h"
 #include "UPnPItem.h"
 #include "AudioItem.h"
 #include "../UPnPActions/UPnPBrowse.h"
 
 #include <iostream>
 #include <sstream>
 #include <libxml/xmlwriter.h>
 using namespace std;
 
CContentDirectory::CContentDirectory(): CUPnPService(udtContentDirectory)
{
  CStorageFolder* pBaseDir = new CStorageFolder();
  m_ObjectList["0"] = pBaseDir;
  
  CAudioItem* pAudioTest = new CAudioItem();
  pBaseDir->AddItem(pAudioTest);
}

CContentDirectory::~CContentDirectory()
{
}
 
bool CContentDirectory::HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut)
{
  cout << "[ContentDirectory] HandleUPnPAction" << endl;     
  string sContent = HandleUPnPBrowse((CUPnPBrowse*)pUPnPAction);
  
  //cout << sContent << endl;
  
  pMessageOut->SetContent(sContent);
  
  return true;
}
 
std::string CContentDirectory::HandleUPnPBrowse(CUPnPBrowse* pUPnPBrowse)
{
  xmlTextWriterPtr writer;
	xmlBufferPtr buf;
	std::stringstream sTmp;	
	
	buf    = xmlBufferCreate();   
	writer = xmlNewTextWriterMemory(buf, 0);    
	xmlTextWriterStartDocument(writer, NULL, "UTF-8", NULL);
  
  // root  
  xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Envelope", NULL);    
  xmlTextWriterWriteAttributeNS(writer, BAD_CAST "s", 
    BAD_CAST "encodingStyle", 
    BAD_CAST  "http://schemas.xmlsoap.org/soap/envelope/", 
    BAD_CAST "http://schemas.xmlsoap.org/soap/encoding/");
   
    // body
    xmlTextWriterStartElementNS(writer, BAD_CAST "s", BAD_CAST "Body", NULL);    
  
      // browse response
      xmlTextWriterStartElementNS(writer, BAD_CAST "u",        
        BAD_CAST "BrowseResponse", 
        BAD_CAST "urn:schemas-upnp-org:service:ContentDirectory:1");
  
        // result
        xmlTextWriterStartElement(writer, BAD_CAST "Result");
      
        m_ListIterator = m_ObjectList.find(pUPnPBrowse->m_sObjectID);
        if(m_ListIterator != m_ObjectList.end())
        {
          //cout << "found object: " << pUPnPBrowse->m_sObjectID << endl;
          xmlTextWriterWriteString(writer, BAD_CAST ((CStorageFolder*)m_ObjectList[pUPnPBrowse->m_sObjectID])->GetContentAsString().c_str());        
        }
      
        // end result
        xmlTextWriterEndElement(writer);
        
        // number returned
        xmlTextWriterStartElement(writer, BAD_CAST "NumberReturned");
        xmlTextWriterWriteString(writer, BAD_CAST "1");
        xmlTextWriterEndElement(writer);
        
        // total matches
        xmlTextWriterStartElement(writer, BAD_CAST "TotalMatches");
        xmlTextWriterWriteString(writer, BAD_CAST "1");
        xmlTextWriterEndElement(writer);
        
        // update id
        xmlTextWriterStartElement(writer, BAD_CAST "UpdateID");
        xmlTextWriterWriteString(writer, BAD_CAST "0");
        xmlTextWriterEndElement(writer);
  
      // end browse response
      xmlTextWriterEndElement(writer);
      
    // end body
    xmlTextWriterEndElement(writer);
   
	// end root
	xmlTextWriterEndElement(writer);
  xmlTextWriterEndDocument(writer);
	xmlFreeTextWriter(writer);
	
	std::stringstream output;
	output << (const char*)buf->content;
	
	xmlBufferFree(buf);
	return output.str();
}


/*
#include <dirent.h>
#include <cstdio>

int main()
{
DIR *d;
dirent *de;
char pfad[256];

printf("Bitte geben Sie den Pfad ein, der durchsucht werden soll: ");
scanf("%s",pfad);
if((d=opendir(pfad))!=NULL)
{
//char *name;
while(de=readdir(d))
{
//tu was
//name=de->d_name;
//if(!(name[0]!='.'&&name[1]!='\0')|| //damit verzeichnis . & .. nicht angezeigt wird
//!(name[0]!='.'&&name[1]!='.'&&name[2]!='\0'))
//{
//printf("%s\n",de->d_name);
//}
}
closedir(d);
}

return 0;
}
*/
