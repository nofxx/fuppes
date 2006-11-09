/***************************************************************************
 *            PlaylistFactory.cpp
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
 
#include "PlaylistFactory.h"
#include <sstream>

#include "ContentDatabase.h"
#include "../Common.h"
#include "../SharedConfig.h"
#include "../Fuppes.h"

#include <iostream>

using namespace std;
 
std::string CPlaylistFactory::BuildPlaylist(std::string p_sObjectId)
{
  string sExt = ExtractFileExt(p_sObjectId);
  p_sObjectId = TruncateFileExt(p_sObjectId);
  if(sExt.compare("pls") == 0)
    return BuildPLS(p_sObjectId);
  else if(sExt.compare("m3u") == 0)
    return BuildM3U(p_sObjectId);
  
  return "";
}


std::string CPlaylistFactory::BuildPLS(std::string p_sObjectId)
{
  std::stringstream sResult;  
  std::stringstream sSql;    
  CSelectResult*    pRes      = NULL;
  unsigned int      nObjectId = HexToInt(p_sObjectId);
  CContentDatabase* pDb       = new CContentDatabase();
  OBJECT_TYPE       nObjectType = OBJECT_TYPE_UNKNOWN;
  
  sSql << "select o.* from OBJECTS o, PLAYLIST_ITEMS p where o.ID = p.OBJECT_ID and p.PLAYLIST_ID = " << nObjectId << ";";
  pDb->Select(sSql.str());
  
  cout << sSql.str() << endl;
  
  int nNumber = 0;
  sResult << "[playlist]\r\n";
  
  while(!pDb->Eof())
  {    
    pRes = pDb->GetResult();
    
    char szItemId[11];         
    unsigned int nItemId = atoi(pRes->GetValue("ID").c_str());
    sprintf(szItemId, "%010X", nItemId);
    
    nObjectType = (OBJECT_TYPE)atoi(pRes->GetValue("TYPE").c_str());
    switch(nObjectType)
    {
      case ITEM_AUDIO_ITEM:
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:
        sResult << "File" << nNumber + 1 << "=";
        sResult << "http://" << CSharedConfig::Shared()->GetFuppesInstance(0)->GetHTTPServerURL() << "/MediaServer/AudioItems/" <<
                   szItemId << "." << ExtractFileExt(pRes->GetValue("FILE_NAME")) << "\r\n";      
        nNumber++;
        break;
      
      case ITEM_VIDEO_ITEM_VIDEO_BROADCAST:
        sResult << pRes->GetValue("FILE_NAME") << "\r\n";
        nNumber++;
        break;
    }
    
    
    pDb->Next();
  }  
  
  sResult << "NumberOfEntries=" << nNumber << "\r\n" <<
             "Version=2\r\n";
  
  pDb->ClearResult();
  delete pDb;
  cout << sResult.str() << endl;
  return sResult.str();  
}

std::string CPlaylistFactory::BuildM3U(std::string p_sObjectId)
{
  std::stringstream sResult;  
  std::stringstream sSql;    
  CSelectResult*    pRes      = NULL;
  unsigned int      nObjectId = HexToInt(p_sObjectId);
  CContentDatabase* pDb       = new CContentDatabase();
  OBJECT_TYPE       nObjectType = OBJECT_TYPE_UNKNOWN;
  
  sSql << "select o.* from OBJECTS o, PLAYLIST_ITEMS p where o.ID = p.OBJECT_ID and p.PLAYLIST_ID = " << nObjectId << ";";
  pDb->Select(sSql.str());
  
    
  //sResult << "#EXTM3U\r\n"; 
/*  
#EXTM3U
#EXTINF:221,Queen - Bohemian Rhapsody
Titel 1.mp3
#EXTINF:473,Dire Straits - Walk Of Life
Pop\Meine Auswahl\Titel 2.mp3
#EXTINF:264,Keep The Faith
C:\Dokumente und Einstellungen\All Users\Dokumente\Eigene Musik\Titel 3.mp3
#EXTINF:504,Bob Marley - Buffalo Soldier
http://www.seite.de/musik/titel4.mp3 */
  
  
  while(!pDb->Eof())
  {
    pRes = pDb->GetResult();
    
    char szItemId[11];         
    unsigned int nItemId = atoi(pRes->GetValue("ID").c_str());
    sprintf(szItemId, "%010X", nItemId);
    
    nObjectType = (OBJECT_TYPE)atoi(pRes->GetValue("TYPE").c_str());
    switch(nObjectType)
    {
      case ITEM_AUDIO_ITEM:
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:        
        sResult << "http://" << CSharedConfig::Shared()->GetFuppesInstance(0)->GetHTTPServerURL() << "/MediaServer/AudioItems/" <<
                   szItemId << "." << ExtractFileExt(pRes->GetValue("FILE_NAME")) << "\r\n";      
        break;
    }
    
    
    pDb->Next();
  }
  
  pDb->ClearResult();
  delete pDb;
  return sResult.str();  
}
