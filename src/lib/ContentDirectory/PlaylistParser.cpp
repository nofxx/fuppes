/***************************************************************************
 *            PlaylistParser.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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
#include "../Common/Common.h"

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
