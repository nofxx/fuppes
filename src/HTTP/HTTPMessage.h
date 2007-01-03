/***************************************************************************
 *            HTTPMessage.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005, 2006 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 *  Copyright (C) 2005 Thomas Schnitzler <tschnitzler@users.sourceforge.net>
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
 
#ifndef _HTTPMESSAGE_H
#define _HTTPMESSAGE_H

/*===============================================================================
 INCLUDES
===============================================================================*/

#include "../MessageBase.h"
#include "../UPnPActions/UPnPAction.h"
#include "../Transcoding/TranscodingCache.h"
#include <string>
#include <iostream>
#include <fstream>

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
  HTTP_VERSION_UNKNOWN            =  0,
  HTTP_VERSION_1_0                =  1,
	HTTP_VERSION_1_1                =  2
}HTTP_VERSION;

typedef enum tagHTTP_MESSAGE_TYPE
{
  HTTP_MESSAGE_TYPE_UNKNOWN                   = 0,
  
  /* HTTP 1.0 and 1.1 message types */  
  HTTP_MESSAGE_TYPE_GET                       = 1,
  HTTP_MESSAGE_TYPE_HEAD                      = 2,
	HTTP_MESSAGE_TYPE_POST                      = 3,  
	HTTP_MESSAGE_TYPE_200_OK                    = 4,
  HTTP_MESSAGE_TYPE_206_PARTIAL_CONTENT       = 5,
  
  HTTP_MESSAGE_TYPE_400_BAD_REQUEST           = 6,  
  HTTP_MESSAGE_TYPE_403_FORBIDDEN             = 7,
	HTTP_MESSAGE_TYPE_404_NOT_FOUND             = 8,
	HTTP_MESSAGE_TYPE_500_INTERNAL_SERVER_ERROR = 9,
  
  /* SOAP */
  HTTP_MESSAGE_TYPE_POST_SOAP_ACTION = 10,
	
  /* GENA message types */
  HTTP_MESSAGE_TYPE_SUBSCRIBE        = 11,
  HTTP_MESSAGE_TYPE_UNSUBSCRIBE      = 12,
  HTTP_MESSAGE_TYPE_GENA_OK          = 13,
  HTTP_MESSAGE_TYPE_NOTIFY           = 14
  
}HTTP_MESSAGE_TYPE;

typedef enum tagHTTP_CONNECTION
{
  HTTP_CONNECTION_UNKNOWN,
  HTTP_CONNECTION_CLOSE
}HTTP_CONNECTION;

  /*
  SUBSCRIBE publisher path HTTP/1.1
HOST: publisher host:publisher port
CALLBACK: <delivery URL>
NT: upnp:event
TIMEOUT: Second-requested subscription duration


HTTP/1.1 200 OK
DATE: when response was generated
SERVER: OS/version UPnP/1.0 product/version
SID: uuid:subscription-UUID
TIMEOUT: Second-actual subscription duration
 */

/*typedef enum tagHTTP_CONTENT_TYPE
{
  HTTP_CONTENT_TYPE_UNKNOWN       =  0,
  HTTP_CONTENT_TYPE_TEXT_HTML     =  1,
	HTTP_CONTENT_TYPE_TEXT_XML      =  2,
	HTTP_CONTENT_TYPE_AUDIO_MPEG    =  3,
  HTTP_CONTENT_TYPE_IMAGE_PNG     =  4,
  HTTP_CONTENT_TYPE_IMAGE_JPEG    =  5
}HTTP_CONTENT_TYPE;*/

class CHTTPMessage;

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

  void         SetMessage(HTTP_MESSAGE_TYPE nMsgType, std::string p_sContentType);
  virtual bool SetMessage(std::string p_sMessage);

/*===============================================================================
 GET MESSAGE DATA
===============================================================================*/

  std::string			  GetRequest()          { return m_sRequest;          }  
  std::string       GetContentType()      { return m_sHTTPContentType;  }
  HTTP_MESSAGE_TYPE GetMessageType()      { return m_nHTTPMessageType;  }
  HTTP_VERSION      GetVersion()          { return m_nHTTPVersion;      }
  std::string       GetContent()          { return m_sContent;          }
  unsigned int      GetBinContentLength() { return m_nBinContentLength; }
  char*             GetBinContent()       { return m_pszBinContent;     }
	
  bool              IsChunked()           { return m_bIsChunked;        }
  void              SetIsChunked(bool p_bIsChunked) { m_bIsChunked = p_bIsChunked;
                                                      if(m_bIsChunked)
                                                        m_bIsBinary = m_bIsChunked;
                                                    }  
  

  CUPnPAction*      GetAction();
  std::string 		  GetHeaderAsString();		  
	std::string			  GetMessageAsString();

  unsigned int      GetBinContentChunk(char* p_sContentChunk, unsigned int p_nSize, unsigned int p_nOffset = 200);

  unsigned int      GetRangeStart() { return m_nRangeStart; }
  unsigned int      GetRangeEnd() { return m_nRangeEnd; }
  void              SetRangeStart(unsigned int p_nRangeStart) { m_nRangeStart = p_nRangeStart; }
  void              SetRangeEnd(unsigned int p_nRangeEnd) { m_nRangeEnd = p_nRangeEnd; }
  HTTP_CONNECTION   GetHTTPConnection() { return m_nHTTPConnection; }
  
  bool              PostVarExists(std::string p_sPostVarName);
  std::string       GetPostVar(std::string p_sPostVarName);
  
/*===============================================================================
 SET MESSAGE DATA
===============================================================================*/

  void             SetMessageType(HTTP_MESSAGE_TYPE p_nHTTPMessageType) { m_nHTTPMessageType = p_nHTTPMessageType; }
  void             SetVersion(HTTP_VERSION p_nHTTPVersion)              { m_nHTTPVersion     = p_nHTTPVersion;     }
  void             SetContentType(std::string p_sContentType)           { m_sHTTPContentType = p_sContentType;     }
	void						 SetContent(std::string p_sContent)                   { m_sContent         = p_sContent;         }
  void             SetBinContent(char* p_szBinContent, unsigned int p_nBinContenLength);
  //void             SetUPnPItem(CUPnPItem* pUPnPItem);
  
  
  std::string  GetGENASubscriptionID() { return m_sGENASubscriptionID; }
  void         SetGENASubscriptionID(std::string p_sSubscriptionID) { m_sGENASubscriptionID = p_sSubscriptionID; }
  
  
/*===============================================================================
 OTHER
===============================================================================*/

  bool             BuildFromString(std::string p_sMessage);
  bool             LoadContentFromFile(std::string);
  bool             TranscodeContentFromFile(std::string p_sFileName);
  void             BreakTranscoding();  
  bool             IsTranscoding();
  
  
/* <\PUBLIC> */

public:
  char*         m_pszBinContent;
  unsigned int  m_nBinContentLength; 
  bool          m_bIsBinary;
  
  CTranscodeSessionInfo* m_pTranscodingSessionInfo;  

/* <PRIVATE> */

private:
    
/*===============================================================================
 MEMBERS
===============================================================================*/
  
  // Header information: [HTTP 1.0|HTTP 1.1]
  HTTP_VERSION       m_nHTTPVersion;
  // Message type
  HTTP_MESSAGE_TYPE  m_nHTTPMessageType;
  // Header information: Content type
  std::string        m_sHTTPContentType;
  // Header information: HTTP request line
  std::string	       m_sRequest;
  // Header information: content length
  int                m_nContentLength;
  // Header information: Connection [close|keep alive]
  HTTP_CONNECTION    m_nHTTPConnection;
   
  // Header information: Call-Back (GENA - Request)
  std::string        m_sGENACallBack;
  // Header information: NT (notification type) (GENA - Request)
  std::string        m_sGENANT;
  // Header information: Timeout (GENA - Request)
  std::string        m_sGENATimeout;  
  // Header information: Subscription-ID (GENA - Request & Response)
  std::string        m_sGENASubscriptionID;
  
   
  bool               m_bIsChunked;  
  CUPnPAction*       m_pUPnPAction;
  std::fstream       m_fsFile;
  unsigned int       m_nRangeStart;
  unsigned int       m_nRangeEnd;

  unsigned int       m_nBinContentPosition;
  fuppesThread       m_TranscodeThread;

/*===============================================================================
 HELPER
===============================================================================*/    

  bool ParsePOSTMessage(std::string p_sMessage);
  bool ParseSUBSCRIBEMessage(std::string p_sMessage);

/* <\PRIVATE> */

};

#endif /* _HTTPMESSAGE_H */
