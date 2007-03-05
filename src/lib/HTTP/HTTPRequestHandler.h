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
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
 
#ifndef _HTTPREQUESTHANDLER_H
#define _HTTPREQUESTHANDLER_H

#include "HTTPMessage.h"


class CHTTPRequestHandler
{
  public:
    CHTTPRequestHandler();
  
    bool HandleRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse);

  private:
    bool HandleHTTPRequest(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
    //bool HandleHTTPResponse();
    bool HandleSOAPAction(CHTTPMessage* pRequest, CHTTPMessage* pResponse);    

    bool HandleGENAMessage(CHTTPMessage* pRequest, CHTTPMessage* pResponse);
  
    bool HandleItemRequest(std::string p_sObjectId, HTTP_MESSAGE_TYPE p_nRequestType, CHTTPMessage* pResponse);
};

#endif // _HTTPREQUESTHANDLER_H
