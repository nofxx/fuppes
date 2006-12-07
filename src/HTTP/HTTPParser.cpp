/***************************************************************************
 *            HTTPParser.cpp
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
 
#include "HTTPParser.h"
#include "../Common/RegEx.h"

bool CHTTPParser::Parse(CHTTPMessage* pMessage)
{
  /* find out the message type and HTTP version */
  std::string sType;
  int nVersion;
  
  RegEx rxType("([GET|HEAD|POST|SUBSCRIBE|UNSUBSCRIBE|NOTIFY]+) +(.+) +HTTP/1\\.([1|0])", PCRE_CASELESS);
  // it's a request
  if(rxType.Search(pMessage->GetHeader().c_str()))
  { 
    sType = rxType.Match(1);
    nVersion = atoi(rxType.Match(3));
  }
  else
  {
    // check if it's a response
    RegEx rxReq("HTTP/1\\.([1|0]) +(\\d+) +(.+)", PCRE_CASELESS);
    if(rxReq.Search(pMessage->GetHeader().c_str()))
    {
      sType = rxReq.Match(2);
      nVersion = atoi(rxReq.Match(1));
    }
    else
      return false;
  }
    

  sType = ToUpper(sType);
  /*cout << "TYPE: " << sType << endl;
  cout << "VERSION: " << nVersion << endl;*/
  
  /* GET|HEAD */
  if((sType.compare("GET") == 0) | (sType.compare("HEAD") == 0))
  {
  }
  
  /* POST */
  
  /* SUBSCRIBE|UNSUBSCRIBE */
  
  /* NOTIFY */
  
  /* 200 OK */
  
  return true;
}
