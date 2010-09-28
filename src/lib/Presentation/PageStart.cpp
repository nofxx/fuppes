/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            PageStart.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "PageStart.h"

#include "../SharedConfig.h"
#include "../Fuppes.h"

#include "../Log.h"
#include "../SharedLog.h"

using namespace fuppes;

std::string PageStart::content()
{
  std::stringstream sResult;
  
  sResult << "<h1>system information</h1>" << endl;  
  
  sResult << "<p>" << endl;
  sResult << "Version: " << CSharedConfig::Shared()->GetAppVersion() << "<br />" << endl;
  sResult << "Hostname: " << CSharedConfig::Shared()->networkSettings->GetHostname() << "<br />" << endl;
  sResult << "OS: " << CSharedConfig::Shared()->GetOSName() << " " << CSharedConfig::Shared()->GetOSVersion() << "<br />" << endl;
  //sResult << "SQLite: " << CContentDatabase::Shared()->GetLibVersion() << endl;
  sResult << "</p>" << endl;
  
  sResult << "<p>" << endl;
  sResult << "build at: " << __DATE__ << " - " << __TIME__ << "<br />" << endl;
  sResult << "build with: " << __VERSION__ << endl;
  sResult << "</p>" << endl;
  
  sResult << "<p>" << endl;
  sResult << "<a href=\"http://sourceforge.net/projects/fuppes/\">http://sourceforge.net/projects/fuppes/</a><br />" << endl;
  sResult << "</p>" << endl;


  // uptime
  fuppes::DateTime now = fuppes::DateTime::now();
  int uptime = now.toInt() - CSharedConfig::Shared()->GetFuppesInstance(0)->startTime().toInt();


  int days;
  int hours;
  int minutes;
  int seconds;

  seconds = uptime % 60;
  minutes = uptime / 60;
  hours = minutes / 60;
  minutes = minutes % 60;
  days = hours / 24;
  hours = hours % 24;

  
  sResult << "<p>" << endl;
  sResult << "uptime: " << days << " days " << hours << " hours " << minutes << " minutes " << seconds << " seconds" << "<br />" << endl;
  sResult << "</p>" << endl;
  
  /*sResult << "<h1>remote devices</h1>";
  sResult << BuildFuppesDeviceList(CSharedConfig::Shared()->GetFuppesInstance(0));*/


  sResult << "<h1>database status</h1>" << endl;  
  sResult << buildObjectStatusTable() << endl;
  
  sResult << buildLogSelection() << endl;
  
  return sResult.str().c_str();
  
  
}


std::string PageStart::buildObjectStatusTable()
{
  OBJECT_TYPE nType = OBJECT_TYPE_UNKNOWN;
  std::stringstream sResult;
  
	SQLQuery qry;
  string sql = qry.build(SQL_GET_OBJECT_TYPE_COUNT, 0);
	qry.select(sql);
  
  sResult << 
    "<table rules=\"all\" style=\"font-size: 10pt; border-style: solid; border-width: 1px; border-color: #000000;\" cellspacing=\"0\" width=\"400\">" << endl <<
      "<thead>" << endl <<
        "<tr>" << endl <<        
          "<th>Type</th>" << 
          "<th>Count</th>" << endl <<
        "</tr>" << endl <<
      "</thead>" << endl << 
      "<tbody>" << endl;  


  while(!qry.eof()) {
    nType = (OBJECT_TYPE)qry.result()->asInt("TYPE");
    
    sResult << "<tr>" << endl;
    sResult << "<td>" << CFileDetails::Shared()->GetObjectTypeAsStr(nType) << "</td>" << endl;
    sResult << "<td>" << qry.result()->asString("VALUE") << "</td>" << endl;    
    sResult << "</tr>" << endl;
    
    qry.next();
  }
    
  sResult <<
      "</tbody>" << endl <<   
    "</table>" << endl;

  return sResult.str();

}


std::string PageStart::buildLogSelection()
{
  stringstream result;
  result << "<h1>logging</h1>";

  int level = CSharedLog::Shared()->GetLogLevelInt();
  
  result << "<input type=\"radio\" name=\"log-level\" id=\"log-level-0\" value=\"none\" " << 
    "onclick=\"logLevel(0)\" " << (level == 0 ? "checked=\"checked\"" : "") << " />" <<
    "<label for=\"log-level-0\">none</label>";

  result << "<input type=\"radio\" name=\"log-level\" id=\"log-level-1\" value=\"normal\" " << 
    "onclick=\"logLevel(1)\" " << (level == 1 ? "checked=\"checked\"" : "") << " />" <<
    "<label for=\"log-level-1\">normal</label>";
  
  result << "<input type=\"radio\" name=\"log-level\" id=\"log-level-2\" value=\"extended\" " << 
    "onclick=\"logLevel(2)\" " << (level == 2 ? "checked=\"checked\"" : "") << " />" <<
    "<label for=\"log-level-2\">extended</label>";
  
  result << "<input type=\"radio\" name=\"log-level\" id=\"log-level-3\" value=\"debug\" " << 
    "onclick=\"logLevel(3)\" " << (level == 3 ? "checked=\"checked\"" : "") << " />" <<
    "<label for=\"log-level-3\">debug</label>";

  result << "<br />";

/*
contentdir = 1 << 5,
contentdb  = 1 << 6,
sql        = 1 << 7,
plugin     = 1 << 8,
config     = 1 << 9,
hotplug    = 1 << 10,
*/

  result << "<input type=\"checkbox\" name=\"log-sender\" id=\"log-sender-http\" value=\"http\" " << 
    (Log::isActiveSender(Log::http) ? "checked=\"checked\"" : "") << " onclick=\"logSender('http')\" />" <<
    "<label for=\"log-sender-http\">http</label>";

  result << "<input type=\"checkbox\" name=\"log-sender\" id=\"log-sender-soap\" value=\"soap\" " << 
    (Log::isActiveSender(Log::soap) ? "checked=\"checked\"" : "") << " onclick=\"logSender('soap')\" />" <<
    "<label for=\"log-sender-soap\">soap</label>";

  result << "<input type=\"checkbox\" name=\"log-sender\" id=\"log-sender-gena\" value=\"gena\" " << 
    (Log::isActiveSender(Log::gena) ? "checked=\"checked\"" : "") << " onclick=\"logSender('gena')\" />" <<
    "<label for=\"log-sender-gena\">gena</label>"; 

  result << "<input type=\"checkbox\" name=\"log-sender\" id=\"log-sender-ssdp\" value=\"ssdp\" " << 
    (Log::isActiveSender(Log::ssdp) ? "checked=\"checked\"" : "") << " onclick=\"logSender('ssdp')\" />" <<
    "<label for=\"log-sender-ssdp\">ssdp</label>";

  result << "<input type=\"checkbox\" name=\"log-sender\" id=\"log-sender-fam\" value=\"fam\" " << 
    (Log::isActiveSender(Log::fam) ? "checked=\"checked\"" : "") << " onclick=\"logSender('fam')\" />" <<
    "<label for=\"log-sender-fam\">fam</label>";
  
  return result.str();
}

