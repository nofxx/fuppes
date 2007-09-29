/***************************************************************************
 *            HTTPRequestHandler.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2006 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _HTTPREQUESTHANDLER_H
#define _HTTPREQUESTHANDLER_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "HTTPMessage.h"


class CHTTPRequestHandler
{
  public:
    CHTTPRequestHandler(std::string p_sHTTPServerURL);
  
    bool HandleRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse);

  private:
    bool HandleHTTPRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
    //bool HandleHTTPResponse();
    bool HandleSOAPAction(CHTTPMessage* pRequest, CHTTPMessage* pResponse);    

    bool HandleGENAMessage(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
  
    bool HandleItemRequest(std::string p_sObjectId, CHTTPMessage* pRequest, CHTTPMessage* pResponse);
    
    std::string m_sHTTPServerURL;
};

#endif // _HTTPREQUESTHANDLER_H
