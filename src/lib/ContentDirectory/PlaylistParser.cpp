/***************************************************************************
 *            PlaylistParser.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007-2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "PlaylistParser.h"
#include "../Common/RegEx.h"
#include "../Common/XMLParser.h"

#include <fstream>
#include <iostream>

using namespace std;

CPlaylistParser::CPlaylistParser()
{
  m_bEof = true;
  m_nPosition = 0;
}

CPlaylistParser::~CPlaylistParser()
{
  PlaylistEntry_t* pEntry;
  list<PlaylistEntry_t*>::iterator tmpIter;
  
  
  for(m_lEntriesIterator = m_lEntries.begin();
      m_lEntriesIterator != m_lEntries.end();) {
        
    tmpIter = m_lEntriesIterator;
    pEntry  = (PlaylistEntry_t*)*m_lEntriesIterator;
    tmpIter++;
    delete pEntry;
    m_lEntriesIterator = tmpIter;
  }
}
    
bool CPlaylistParser::LoadPlaylist(std::string p_sFileName)
{
  fstream fsPlaylist;
  
  fsPlaylist.open(p_sFileName.c_str(), ios::in);
  if(fsPlaylist.fail()) {
    return false; 
  }
  
  m_sListPath = ExtractFilePath(p_sFileName);
   
  fsPlaylist.seekg(0, ios::end); 
  int nSize = streamoff(fsPlaylist.tellg()); 
  fsPlaylist.seekg(0, ios::beg);
  char* szBuf = new char[nSize + 1];  
  fsPlaylist.read(szBuf, nSize); 
  szBuf[nSize] = '\0';
  fsPlaylist.close();
    
  string sResult = szBuf;
  delete[] szBuf;
  
  bool bResult = true;
  
  if(sResult.length() > strlen("#EXTM3U") && sResult.substr(0, strlen("#EXTM3U")).compare("#EXTM3U") == 0) {
    //cout << "M3U" << endl;
    bResult = ParseM3U(sResult);
  }
  else if(sResult.length() > strlen("[playlist]") && sResult.substr(0, strlen("[playlist]")).compare("[playlist]") == 0) {
    //cout << "PLS" << endl;
    bResult = ParsePLS(sResult);
  }
	else if(ToLower(ExtractFileExt(p_sFileName)).compare("rss") == 0) {
		bResult = ParseRSS(sResult);
	}
		
  else {
    return false;
  }
  
  m_lEntriesIterator = m_lEntries.begin();
  if(m_lEntriesIterator != m_lEntries.end()) {
    m_bEof = false;
  }
  return bResult;
}

PlaylistEntry_t* CPlaylistParser::Entry()
{
  return (PlaylistEntry_t*)*m_lEntriesIterator;
}

void CPlaylistParser::Next()
{
  if(m_lEntries.size() > 0) {    
    m_lEntriesIterator++;
  }
  
  if(m_lEntriesIterator == m_lEntries.end()) {
    m_bEof = true;
  }
}


bool CPlaylistParser::ParseM3U(std::string p_sContent)
{
  PlaylistEntry_t* pEntry;
  
  RegEx rxLines("(.*)\n", PCRE_CASELESS);
  RegEx rxFile("^[#EXTM3U]", PCRE_CASELESS);  
  RegEx rxData("^[#EXTINF]", PCRE_CASELESS);
  if(!rxLines.Search(p_sContent.c_str())) {
    return false;
  }
  
  do
  {
    if(rxFile.Search(rxLines.Match(1))) {
      continue;
    }
    
    if(rxData.Search(rxLines.Match(1))) {
      #warning todo: parse metadata
      continue;
    }
        
    pEntry = new PlaylistEntry_t();
    if(IsURL(rxLines.Match(1))) {       
      pEntry->sFileName = rxLines.Match(1);
      pEntry->bIsLocalFile = false;
    }
    else {
      pEntry->sFileName = FormatFileName(rxLines.Match(1));
      pEntry->bIsLocalFile = true;
    }
    m_lEntries.push_back(pEntry);
    
  }while(rxLines.SearchAgain());  
    
  return true;
}

bool CPlaylistParser::ParsePLS(std::string p_sContent)
{
	PlaylistEntry_t* pEntry;
	RegEx rxNumEntries("NumberOfEntries=(\\d+)", PCRE_CASELESS);
		
	int numEntries = 0;
		
	if(rxNumEntries.Search(p_sContent.c_str())) {
		numEntries = atoi(rxNumEntries.Match(1));
	}
		
	if(numEntries == 0)
		return false;
		
	for(int i = 0; i < numEntries; i++) {
		
		stringstream sExpr;
    sExpr << "File" << i + 1 << "=(.+)\\n";    
    RegEx rxFile(sExpr.str().c_str(), PCRE_CASELESS);
    if(!rxFile.Search(p_sContent.c_str()))
      continue;

    pEntry = new PlaylistEntry_t();
    if(IsURL(rxFile.Match(1))) {       
      pEntry->sFileName = rxFile.Match(1);
      pEntry->bIsLocalFile = false;
    }
    else {
      pEntry->sFileName = FormatFileName(rxFile.Match(1));
      pEntry->bIsLocalFile = true;
    }
    m_lEntries.push_back(pEntry);

	}
		
}

bool CPlaylistParser::ParseRSS(std::string p_sContent)
{
		
	CXMLDocument* pDoc = new CXMLDocument();
	if(!pDoc->LoadFromString(p_sContent)) {
		delete pDoc;
		return false;
	}
	
	CXMLNode* pTmp;
	pTmp = pDoc->RootNode()->FindNodeByName("channel");
	if(!pTmp) {
		delete pDoc;
		return false;
	}
	
	for(int i = 0; i < pTmp->ChildCount(); i++) {
		if(pTmp->ChildNode(i)->Name().compare("item") != 0) {
			continue;
		}
		
		CXMLNode* pEnc = pTmp->ChildNode(i)->FindNodeByName("enclosure");
		if(!pEnc) {
			continue;
		}
		
		PlaylistEntry_t* pEntry = new PlaylistEntry_t();	
			
		pEntry->sFileName = pEnc->Attribute("url");
		pEntry->nSize = pEnc->AttributeAsUInt("length");
		pEntry->sMimeType = pEnc->Attribute("type");
		
		pEntry->bIsLocalFile = false;
			
		if(pTmp->ChildNode(i)->FindNodeByName("title")) {
			pEntry->sTitle = pTmp->ChildNode(i)->FindNodeByName("title")->Value();
		}
			
		m_lEntries.push_back(pEntry);
		
	}
		
/*		<channel>
	<title>Marillion Online Podcast</title>
	<link>http://www.marillion.com</link>
	<language>en</language>
	<copyright>&#x2117; &amp; &#xA9; 2006 Marillion</copyright>
	<pubDate>Wed, 13 Dec 2006 15:00:00 GMT</pubDate>
	<itunes:subtitle>Find a Better Way of Life at marillion.com</itunes:subtitle>
	<itunes:author>Marillion</itunes:author>
	<itunes:summary>Official insider information for British rock band Marillion. Whether writing in the studio, recording new material, preparing for a live show, or on the road - get the scoop DIRECT from the band members themselves. Find a Better Way of Life at marillion.com</itunes:summary>
	<description>Official insider information for British rock band Marillion. Whether writing in the studio, recording new material, preparing for a live show, or on the road - get the scoop DIRECT from the band members themselves. Find a Better Way of Life at marillion.com</description>
				
		
		<item>
		<title>Album Number 14 Begins</title>
		<itunes:author>Marillion</itunes:author>
		<itunes:subtitle>Recording is set to begin on the next studio album</itunes:subtitle>
		<itunes:summary>26 May 2006. Marillion have been jamming song ideas since the beginning of 2006 and have now come up with a short-list of contenders for the next studio album. Coming to you direct from the live room at the Racket Club studio with Mark Kelly, Ian Mosley, Pete Trewavas, Mike Hunter, and the Racket Records Peanut Gallery.</itunes:summary>
		<enclosure url="http://media.marillion.com/podcast/20060526.mp3" length="3361560" type="audio/mpeg" />
		<guid>http://media.marillion.com/podcast/20060526.mp3</guid>
		<pubDate>Fri, 26 May 2006 09:00:00 GMT</pubDate>
		<itunes:duration>4:00</itunes:duration>
		<itunes:explicit>yes</itunes:explicit>
		<itunes:keywords>marillion, studio, new, album, recording, update, racket, hogarth, kelly, mosley, trewavas, rothery</itunes:keywords>
	</item>	*/
		
	delete pDoc;
		
	return true;
}

std::string CPlaylistParser::FormatFileName(std::string p_sValue)
{
  if(p_sValue.length() <= 2)
    return p_sValue;
  
  bool bRelative = false;
  
  #ifdef WIN32  
  if(p_sValue.substr(0, 2).compare(".\\") == 0) {
    p_sValue = p_sValue.substr(2, p_sValue.length());    
  }
  
  if(p_sValue.substr(1, 2).compare(":") != 0) {
    p_sValue = m_sListPath + p_sValue;
  }
  #else
  if(p_sValue.substr(0, 2).compare("./") == 0) {
    p_sValue = p_sValue.substr(2, p_sValue.length());    
  }
  
  if(p_sValue.substr(0, 1).compare("/") != 0) {
    p_sValue = m_sListPath + p_sValue;
  }
  #endif
  
  return p_sValue;
}

bool CPlaylistParser::IsURL(std::string p_sValue)
{
  RegEx rxUrl("\\w+://", PCRE_CASELESS);
  return rxUrl.Search(p_sValue.c_str());
}
