/***************************************************************************
 *            HTTPMessage.h
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
 
#ifndef _HTTPMESSAGE_H
#define _HTTPMESSAGE_H

#include "MessageBase.h"
#include "UPnPAction.h"
#include <string>

enum eHTTPVersion
{
	http_1_0,
	http_1_1	
};

enum eHTTPMessageType
{
	http_get,
	http_post,
	http_200_ok,
	http_404_not_found
};

enum eHTTPContentType
{
	text_html,
	text_xml,
	audio_mpeg
};

class CHTTPMessage: public CMessageBase
{
  public:
		CHTTPMessage(eHTTPMessageType, eHTTPVersion);
	  CHTTPMessage(eHTTPMessageType, eHTTPContentType);
    CHTTPMessage(std::string);
		
		eHTTPMessageType GetMessageType();
	  std::string			 GetRequest();
	  eHTTPContentType GetContentType();
		
		void						SetContent(std::string);
	
		bool LoadContentFromFile(std::string);	
		std::string 		 GetHeaderAsString();		  
	  std::string			 GetMessageAsString();
    CUPnPAction*     GetAction();
  
	private:
    void             ParsePOSTMessage(std::string);
  
    CUPnPAction*     m_pUPnPAction;
		eHTTPVersion     m_HTTPVersion;
		eHTTPMessageType m_HTTPMessageType;
		std::string	     m_sRequest;
		eHTTPContentType m_HTTPContentType;
    int              m_nContentLength;
};

#endif /* _HTTPMESSAGE_H */
