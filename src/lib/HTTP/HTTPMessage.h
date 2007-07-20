/***************************************************************************
 *            HTTPMessage.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _HTTPMESSAGE_H
#define _HTTPMESSAGE_H

#include "../DeviceSettings/DeviceSettings.h"
#include "../MessageBase.h"
#include "../UPnPActions/UPnPAction.h"
#include "../Transcoding/TranscodingCache.h"
#include "../ContentDirectory/FileDetails.h"
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

class CUPnPBrowse;

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

typedef enum HTTP_TRANSFER_ENCODING {
  HTTP_TRANSFER_ENCODING_NONE,
  HTTP_TRANSFER_ENCODING_CHUNKED
} HTTP_TRANSFER_ENCODING;

class CHTTPMessage;

class CHTTPMessage: public CMessageBase
{
  public:
    CHTTPMessage();
    virtual ~CHTTPMessage();

    void         SetMessage(HTTP_MESSAGE_TYPE nMsgType, std::string p_sContentType);
    virtual bool SetMessage(std::string p_sMessage);
    virtual bool SetHeader(std::string p_sHeader);

    std::string			  GetRequest()          { return m_sRequest;          }  
    std::string       GetContentType()      { return m_sHTTPContentType;  }
    HTTP_MESSAGE_TYPE GetMessageType()      { return m_nHTTPMessageType;  }
    HTTP_VERSION      GetVersion()          { return m_nHTTPVersion;      }
    //std::string       GetContent()          { return m_sContent;          }
    unsigned int      GetBinContentLength();
    char*             GetBinContent()       { return m_pszBinContent;     }
	
    bool              IsBinary()            { return m_bIsBinary;         }  
    void              SetIsBinary(bool p_bIsBinary) { m_bIsBinary = p_bIsBinary; }
  

    HTTP_TRANSFER_ENCODING GetTransferEncoding() { return m_nTransferEncoding; }
    void SetTransferEncoding(HTTP_TRANSFER_ENCODING p_nTransferEncoding) { m_nTransferEncoding = p_nTransferEncoding; }
  
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
  

    void             SetMessageType(HTTP_MESSAGE_TYPE p_nHTTPMessageType) { m_nHTTPMessageType = p_nHTTPMessageType; }
    void             SetVersion(HTTP_VERSION p_nHTTPVersion)              { m_nHTTPVersion     = p_nHTTPVersion;     }
    void             SetContentType(std::string p_sContentType)           { m_sHTTPContentType = p_sContentType;     }
  	void						 SetContent(std::string p_sContent)                   { m_sContent         = p_sContent;         }
    void             SetBinContent(char* p_szBinContent, unsigned int p_nBinContenLength);  
    
    std::string  GetGENASubscriptionID() { return m_sGENASubscriptionID; }
    void         SetGENASubscriptionID(std::string p_sSubscriptionID) { m_sGENASubscriptionID = p_sSubscriptionID; }
  
 
    bool             LoadContentFromFile(std::string);
    bool             TranscodeContentFromFile(std::string p_sFileName, SMusicTrack p_sTrackDetails);
    void             BreakTranscoding();  
    bool             IsTranscoding();
  
	  CDeviceSettings* GetDeviceSettings() { return m_pDeviceSettings; }
	  void SetDeviceSettings(CDeviceSettings* pSettings) { m_pDeviceSettings = pSettings; }
  
  public:
    char*         m_pszBinContent;
    unsigned int  m_nBinContentLength; 
    bool          m_bIsBinary;
  
	  std::string   m_sUserAgent;
	
    CTranscodeSessionInfo* m_pTranscodingSessionInfo;  
    #ifndef DISABLE_TRANSCODING
    CTranscodingCacheObject* m_pTranscodingCacheObj;
    #endif
    //fuppesThreadMutex TranscodeMutex;
    
private:
    bool             BuildFromString(std::string p_sMessage);

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
  
	
    HTTP_TRANSFER_ENCODING m_nTransferEncoding;
  
	  CUPnPAction*       m_pUPnPAction;
    std::fstream       m_fsFile;
    unsigned int       m_nRangeStart;
    unsigned int       m_nRangeEnd;
	
	  CDeviceSettings*   m_pDeviceSettings;

    unsigned int       m_nBinContentPosition;


    bool ParsePOSTMessage(std::string p_sMessage);
    bool ParseSUBSCRIBEMessage(std::string p_sMessage);
};

#endif // _HTTPMESSAGE_H
