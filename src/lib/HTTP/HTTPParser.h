/***************************************************************************
 *            HTTPParser.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006, 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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


#ifndef _HTTPPARSER_H
#define _HTTPPARSER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "HTTPMessage.h"

class CHTTPParser
{ 
  public:
    bool Parse(CHTTPMessage* pMessage);
		
		void ConvertURLEncodeContentToPlain(CHTTPMessage* pMessage);
	
  private:
	  CHTTPMessage* m_pMessage;
	
		std::string URLEncodeValueToPlain(std::string p_sValue);
		void ParseCommonValues();
		
};

#endif // _HTTPPARSER_H
