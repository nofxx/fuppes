/***************************************************************************
 *            HTTPMessage.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "MessageBase.h"
#include <string>

using namespace std;

/*===============================================================================
 FORWARD DECLARATIONS
===============================================================================*/

class CUPnPBrowse;

/*===============================================================================
 DEFINITIONS
===============================================================================*/

typedef enum tagHTTP_VERSION
{
	HTTP_VERSION_INVALID            = -1,
  HTTP_VERSION_UNKNOWN            =  0,
  HTTP_VERSION_1_0                =  1,
	HTTP_VERSION_1_1                =  2,
  HTTP_VERSION_MAX                =  3
}HTTP_VERSION;

typedef enum tagHTTP_MESSAGE_TYPE
{
  HTTP_MESSAGE_TYPE_INVALID       = -1,
  HTTP_MESSAGE_TYPE_UNKNOWN       =  0,
  HTTP_MESSAGE_TYPE_GET           =  1,
	HTTP_MESSAGE_TYPE_POST          =  2,
	HTTP_MESSAGE_TYPE_200_OK        =  3,
	HTTP_MESSAGE_TYPE_404_NOT_FOUND =  4,
  HTTP_MESSAGE_TYPE_MAX           =  5
}HTTP_MESSAGE_TYPE;

typedef enum tagHTTP_CONTENT_TYPE
{
	HTTP_CONTENT_TYPE_INVALID       = -1,
  HTTP_CONTENT_TYPE_UNKNOWN       =  0,
  HTTP_CONTENT_TYPE_TEXT_HTML     =  1,
	HTTP_CONTENT_TYPE_TEXT_XML      =  2,
	HTTP_CONTENT_TYPE_AUDIO_MPEG    =  3,
  HTTP_CONTENT_TYPE_IMAGE_PNG     = 4
}HTTP_CONTENT_TYPE;

/*===============================================================================
 CLASS CHTTPMessage
===============================================================================*/

class CHTTPMessage: public CMessageBase
{

/* <PUBLIC> */
  
public:

/*===============================================================================
 CONSTRUCTOR / DESTRUCTOR
===============================================================================*/

  CHTTPMessage();
  virtual ~CHTTPMessage();

/*===============================================================================
 INIT
===============================================================================*/

  void         SetMessage(HTTP_MESSAGE_TYPE nMsgType, HTTP_VERSION nVersion);
  void         SetMessage(HTTP_MESSAGE_TYPE nMsgType, HTTP_CONTENT_TYPE nCtntType);
  virtual bool SetMessage(std::string p_sMessage);

/*===============================================================================
 GET MESSAGE DATA
===============================================================================*/

  std::string			  GetRequest()          { return m_sRequest;          }  
  HTTP_CONTENT_TYPE GetContentType()      { return m_HTTPContentType;   }
  HTTP_MESSAGE_TYPE GetMessageType()      { return m_HTTPMessageType;   }
  std::string       GetContent()          { return m_sContent;          }
  unsigned int      GetBinContentLength() { return m_nBinContentLength; }
  char*             GetBinContent()       { return m_pszBinContent;     }
	bool              IsChunked()           { return m_bIsChunked;        }

  bool              GetAction(CUPnPBrowse* pBrowse);
  std::string 		  GetHeaderAsString();		  
	std::string			  GetMessageAsString();

/*===============================================================================
 SET MESSAGE DATA
===============================================================================*/

  void             SetMessageType(HTTP_MESSAGE_TYPE p_HTTPMessageType) { m_HTTPMessageType = p_HTTPMessageType; }
  void             SetContentType(HTTP_CONTENT_TYPE p_HTTPContentType) { m_HTTPContentType = p_HTTPContentType; }
	void						 SetContent(std::string p_sContent)                  { m_sContent        = p_sContent;        }
  void             SetBinContent(char* p_szBinContent, unsigned int p_nBinContenLength);
  
/*===============================================================================
 OTHER
===============================================================================*/

  bool             BuildFromString(std::string p_sMessage);
  bool             LoadContentFromFile(std::string);	
  

/* <\PUBLIC> */

/* <PRIVATE> */

private:
    
/*===============================================================================
 MEMBERS
===============================================================================*/
  
  HTTP_VERSION       m_HTTPVersion;
  HTTP_MESSAGE_TYPE  m_HTTPMessageType;
  HTTP_CONTENT_TYPE  m_HTTPContentType;
  std::string	       m_sRequest;
  int                m_nContentLength;  
  char*              m_pszBinContent;
  unsigned int       m_nBinContentLength;  
  bool               m_bIsChunked;

/*===============================================================================
 HELPER
===============================================================================*/    

  void ParsePOSTMessage(std::string);

/* <\PRIVATE> */

};

#endif /* _HTTPMESSAGE_H */
