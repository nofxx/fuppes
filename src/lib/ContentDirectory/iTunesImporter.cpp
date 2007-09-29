/***************************************************************************
 *            iTunesImporter.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2007 Ulrich Völkel <fuppes@ulrich-voelkel.de>
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
 
#include "iTunesImporter.h"
#include "FileDetails.h"
#include <iostream>
#include <sstream>
using namespace std;
 
void CiTunesImporter::Import(std::string p_sFileName)
{
  CXMLDocument* pDoc = new CXMLDocument();
  if(!pDoc->Load(p_sFileName)) {
   cout << "error loading iTunes.xml" << endl; fflush(stdout);
   delete pDoc;
   return;  
  }

  CXMLNode* pNode = pDoc->RootNode(); // <plist>
  CXMLNode* pTmp = NULL;
  pNode = pNode->ChildNode(0); // <dict>
  int i;
  for(i = 0; i < pNode->ChildCount(); i++) {
    pTmp = pNode->ChildNode(i);
  
    if((pTmp->Name().compare("key") == 0) && (pTmp->Value().compare("Tracks") == 0)) {
      pTmp = pNode->ChildNode(i + 1);
      break;
    }
    pTmp = NULL;
  }
   
  if(!pTmp) {
    delete pDoc;
    return;
  }

  for(i = 0; i < pTmp->ChildCount(); i++) {
    pNode = pTmp->ChildNode(i);
    if(pNode->Name().compare("dict") == 0) {
      ParseDict(pNode);
    }
  }

  delete pDoc;
}

void CiTunesImporter::ParseDict(CXMLNode* pDict)
{
  int i;
  CXMLNode* pNode;
  SAudioItem track;
  string sFileName;
 
  pNode = pDict->FindNodeByValue("key", "Track Type", false);
  if(!pNode) {
    return;
  }    
  pNode = pDict->ChildNode(pNode->Index() + 1);
  
  if(pNode->Value().compare("File") != 0) {
    return;
  }
    
  for(i = 0; i < pDict->ChildCount(); i++) {
    pNode = pDict->ChildNode(i);
    
    if(pNode->Value().compare("Kind") == 0) {
      // <key>Kind</key><string>MPEG-Audiodatei</string>
    }
    else if(pNode->Value().compare("Name") == 0) {
      //cout << "Name: " << pDict->ChildNode(i + 1)->Value() << endl;
      track.sTitle = pDict->ChildNode(i + 1)->Value();
    }
    else if(pNode->Value().compare("Artist") == 0) {
      track.sArtist = pDict->ChildNode(i + 1)->Value();
    }
    else if(pNode->Value().compare("Album") == 0) {
      track.sAlbum = pDict->ChildNode(i + 1)->Value();
    }
    else if(pNode->Value().compare("Genre") == 0) {
      track.sGenre = pDict->ChildNode(i + 1)->Value();
    }
    else if(pNode->Value().compare("Total Time") == 0) {
    }
    else if(pNode->Value().compare("Track Number") == 0) {
      track.nOriginalTrackNumber = atoi(pDict->ChildNode(i + 1)->Value().c_str());
    }
    /*else if(pNode->Value().compare("Track Count") == 0) {
    } */
    else if(pNode->Value().compare("Year") == 0) {
      track.sDate = pDict->ChildNode(i + 1)->Value();
    }
    else if(pNode->Value().compare("Bit Rate") == 0) {
      track.nBitrate = atoi(pDict->ChildNode(i + 1)->Value().c_str());
    }
    else if(pNode->Value().compare("Sample Rate") == 0) {
      track.nSampleRate = atoi(pDict->ChildNode(i + 1)->Value().c_str());
    }
    else if(pNode->Value().compare("Location") == 0) {
      sFileName = pDict->ChildNode(i + 1)->Value();
      //cout << "*" << sFileName << "*" << endl;
      sFileName = sFileName.substr(string("file://localhost/").length(), sFileName.length());
      //cout << "*" << sFileName << "*" << endl;
      #ifdef WIN32
      sFileName = StringReplace(sFileName, "/", "\\");
      #endif
      sFileName = StringReplace(sFileName, "%20", " ");
      //cout << "*" << sFileName << "*" << endl;
        
    }
      
   
  }
     //cout << endl;

  stringstream sSql;
  CContentDatabase* pDb = new CContentDatabase();
  unsigned int nObjId;
    
  nObjId = pDb->GetObjId();
  
  pDb->BeginTransaction();
    
  sSql << 
	  "insert into OBJECT_DETAILS " <<
		"(A_ARTIST, A_ALBUM, A_TRACK_NO, A_GENRE, AV_DURATION, DATE, A_CHANNELS, AV_BITRATE, A_SAMPLERATE) " <<
		"values (" <<
		//"'" << SQLEscape(TrackInfo.mAudioItem.sTitle) << "', " <<
		"'" << SQLEscape(track.sArtist) << "', " <<
		"'" << SQLEscape(track.sAlbum) << "', " <<
		track.nOriginalTrackNumber << ", " <<
		"'" << SQLEscape(track.sGenre) << "', " <<
		"'" << track.sDuration << "', " <<
		"'" << track.sDate << "', " <<
		track.nNrAudioChannels << ", " <<
		track.nBitrate << ", " <<
		track.nSampleRate << ")";
    
  unsigned int nDetailId = pDb->Insert(sSql.str());
  
  sSql.str("");
  sSql << "insert into objects " <<
          "(OBJECT_ID, DETAIL_ID, TYPE, PATH, FILE_NAME, TITLE, MD5, MIME_TYPE) " <<
          "values (" <<
          nObjId << ", " <<
          nDetailId << ", " <<
          ITEM_AUDIO_ITEM_MUSIC_TRACK << ", " <<
          "'" << SQLEscape(sFileName) << "', " <<
          "'" << SQLEscape(sFileName) << "', " <<
          "'" << SQLEscape(track.sTitle) << "', " <<
          "'n/a', " <<
          "'" << "obsolete" << "') ";

  pDb->Insert(sSql.str());
  
  
  pDb->Commit();
    
  delete pDb;
    
}

/*
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
        <key>Major Version</key><integer>1</integer>
        <key>Minor Version</key><integer>1</integer>
        <key>Application Version</key><string>7.1</string>
        <key>Features</key><integer>1</integer>
        <key>Show Content Ratings</key><true/>
        <key>Music Folder</key><string>file://localhost/H:/Mp3'z/</string>
        <key>Library Persistent ID</key><string>5C56E16B4EC9254E</string>
        <key>Tracks</key>
        <dict>
                <key>707</key>
                <dict>
                        <key>Track ID</key><integer>707</integer>
                        <key>Name</key><string>Johnny Don't Do It</string>
                        <key>Artist</key><string>10CC</string>
                        <key>Album</key><string>10CC</string>
                        <key>Genre</key><string>Progressive Rock</string>
                        <key>Kind</key><string>MPEG-Audiodatei</string>
                        <key>Size</key><integer>5273855</integer>
                        <key>Total Time</key><integer>218775</integer>
                        <key>Track Number</key><integer>1</integer>
                        <key>Track Count</key><integer>15</integer>
                        <key>Year</key><integer>1973</integer>
                        <key>Date Modified</key><date>2006-10-10T14:43:06Z</date>
                        <key>Date Added</key><date>2007-01-16T14:19:33Z</date>
                        <key>Bit Rate</key><integer>192</integer>
                        <key>Sample Rate</key><integer>44100</integer>
                        <key>Play Count</key><integer>1</integer>
                        <key>Play Date</key><integer>3255421461</integer>
                        <key>Play Date UTC</key><date>2007-02-27T10:44:21Z</date>
                        <key>Artwork Count</key><integer>2</integer>
                        <key>Persistent ID</key><string>583573A8215B5BB2</string>
                        <key>Track Type</key><string>File</string>
                        <key>Location</key><string>file://localhost/H:/Mp3'z/Alben/10CC/(1973)%2010CC/01%20-%20Johnny%20Don't%20Do%20It.mp3</string>
                        <key>File Folder Count</key><integer>5</integer>
                        <key>Library Folder Count</key><integer>1</integer>
                </dict>
*/
