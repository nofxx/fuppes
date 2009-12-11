/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            PlaylistFactory.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006-2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#include "PlaylistFactory.h"
#include <sstream>

#include "DatabaseConnection.h"
#include "UPnPObjectTypes.h"

#include "../Common/Common.h"
#include "../Common/XMLParser.h"
#include "../SharedConfig.h"
#include "../Fuppes.h"

#include <iostream>

using namespace std;
 
static string SqlGetObjectsString(unsigned int nObjectId) {
  stringstream Sql;
  Sql << 
    "select " <<
    "  * " <<
    "from " <<
    "  OBJECTS o, " <<
    "  MAP_OBJECTS m " <<
    "  left join OBJECT_DETAILS d on (d.ID = o.DETAIL_ID) " <<
    "where " <<
    "  m.PARENT_ID = " << nObjectId << " and " <<
    "  o.OBJECT_ID = m.OBJECT_ID";
  return Sql.str();
}

std::string CPlaylistFactory::BuildPlaylist(std::string p_sObjectId)
{
  string sExt = ExtractFileExt(p_sObjectId);
  p_sObjectId = TruncateFileExt(p_sObjectId);
  if(sExt.compare("pls") == 0)
    return BuildPLS(p_sObjectId);
  else if(sExt.compare("m3u") == 0)
    return BuildM3U(p_sObjectId);
  else if(sExt.compare("wpl") == 0)
    return BuildWPL(p_sObjectId);
  
  return "";
}

#warning todo: audio details
std::string CPlaylistFactory::BuildPLS(std::string p_sObjectId)
{
  std::stringstream sResult;  
  std::stringstream sSql;    
  CSQLResult*    pRes      = NULL;
  unsigned int      nObjectId = HexToInt(p_sObjectId);
	CSQLQuery* qry = CDatabase::query();
  OBJECT_TYPE       nObjectType = OBJECT_TYPE_UNKNOWN;
  
	int nNumber = 0;
  sResult << "[playlist]\r\n";

	qry->select(SqlGetObjectsString(nObjectId));
	while(!qry->eof()) {    
    pRes = qry->result();
		
    char szItemId[11];         
    unsigned int nItemId = pRes->asInt("OBJECT_ID");
    sprintf(szItemId, "%010X", nItemId);
    
    nObjectType = (OBJECT_TYPE)pRes->asInt("TYPE");
    switch(nObjectType)
    {
      case ITEM_AUDIO_ITEM:
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:        
        sResult << "File" << nNumber + 1 << "=";
        sResult << "http://" << CSharedConfig::Shared()->GetFuppesInstance(0)->GetHTTPServerURL() << "/MediaServer/AudioItems/" <<
                   szItemId << "." << ExtractFileExt(pRes->asString("FILE_NAME")) << "\r\n";      
        sResult << "Title" << nNumber + 1 << "=" << TruncateFileExt(pRes->asString("FILE_NAME")) << "\r\n";
        nNumber++;
        break;
      
      case ITEM_AUDIO_ITEM_AUDIO_BROADCAST:
      case ITEM_VIDEO_ITEM_VIDEO_BROADCAST:
        sResult << "File" << nNumber + 1 << "=";
        sResult << pRes->asString("FILE_NAME") << "\r\n";
        nNumber++;
        break;
				
			default:
			  break;
    }
    
		qry->next();
  }  
  
  sResult << "NumberOfEntries=" << nNumber << "\r\n" <<
             "Version=2\r\n";
  
  //pDb->ClearResult();

	delete qry;
  //cout << sResult.str() << endl;
  return sResult.str();  
}

std::string CPlaylistFactory::BuildM3U(std::string p_sObjectId)
{
  std::stringstream sResult;  
  CSQLResult*    pRes      = NULL;
  unsigned int      nObjectId = HexToInt(p_sObjectId);
	CSQLQuery* qry = CDatabase::query();
	OBJECT_TYPE       nObjectType = OBJECT_TYPE_UNKNOWN;
  
  // get the information out of the database and place every item in the file
	qry->select(SqlGetObjectsString(nObjectId));  
  while(!qry->eof()) {
    pRes = qry->result();
		
    char szItemId[11];         
    unsigned int nItemId = pRes->asInt("OBJECT_ID");
    sprintf(szItemId, "%010X", nItemId);
    
    nObjectType = (OBJECT_TYPE)pRes->asInt("TYPE");
    switch(nObjectType)
    {
      case ITEM_AUDIO_ITEM:
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:        
        sResult << "http://" << CSharedConfig::Shared()->GetFuppesInstance(0)->GetHTTPServerURL() << "/MediaServer/AudioItems/" <<
                   szItemId << "." << ExtractFileExt(pRes->asString("FILE_NAME")) << "\r\n";      
        break;
				
			default:
			  break;
    }

    qry->next();
  }
  delete qry;
		
  return sResult.str();  
}

/*
<?wpl version="1.0"?>
<smil>
    <head>
        <meta name="Generator" content="Microsoft Windows Media Player -- 11.0.5721.5145"/>
        <meta name="AverageRating" content="33"/>
        <meta name="TotalDuration" content="1102"/>
        <meta name="ItemCount" content="3"/>
        <author/>
        <title>Bach Organ Works</title>
    </head>
    <body>
        <seq>
            <media src="\\server\vol\music\Classical\Bach\OrganWorks\cd03\track01.mp3"/>
            <media src="\\server\vol\music\Classical\Bach\OrganWorks\cd03\track02.mp3"/>
            <media src="\\server\vol\music\Classical\Bach\OrganWorks\cd03\track03.mp3"/>
        </seq>
    </body>
</smil>
*/

// TODO use the XML API to do this instead
std::string CPlaylistFactory::BuildWPL(std::string p_sObjectId)
{
  stringstream ssResult, ssMedia;
  CSQLResult* pRes = NULL;
  unsigned int      nObjectId = HexToInt(p_sObjectId);
	CSQLQuery* qry = CDatabase::query();
	OBJECT_TYPE       nObjectType = OBJECT_TYPE_UNKNOWN;

  // generate header
  ssResult << "<?wpl version=\"1.0\"?><smil><head>"
            << "<meta name=\"Generator\" content=\"Microsoft Windows Media Player -- 11.0.5721.5145\"/>";

  // for every object in the database, add it to the XML
	qry->select(SqlGetObjectsString(nObjectId));  
  int itemCount = 0;
  while(!qry->eof()) {
    pRes = qry->result();
		
    char szItemId[11];         
    unsigned int nItemId = pRes->asInt("OBJECT_ID");
    // 010X says => print uppercase hex number with a min of ten characters and left padded with zeroes
    sprintf(szItemId, "%010X", nItemId);
    
    nObjectType = (OBJECT_TYPE)pRes->asInt("TYPE");
    switch(nObjectType)
    {
      case ITEM_AUDIO_ITEM:
      case ITEM_AUDIO_ITEM_MUSIC_TRACK:        
        ssMedia << "<media src=\"" 
                << "http://" << CSharedConfig::Shared()->GetFuppesInstance(0)->GetHTTPServerURL() 
                << "/MediaServer/AudioItems/" << szItemId 
                << "." << ExtractFileExt(pRes->asString("FILE_NAME")) 
                << "\"/>\r\n";      
        break;
				
			default:
			  break;
    }
    ++itemCount;
  }

  // finish the XML file
  ssResult  << "<meta name=\"AverageRating\" content=\"0\"/>"
            << "<meta name=\"ItemCount\" content=\"" << itemCount << "\"/>"
            << "</head><body><seq>"
            << ssMedia
            << "</seq></body></smil>";

  return ssResult.str();
}
