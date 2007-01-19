/***************************************************************************
 *            HTTPClient.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 - 2007 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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
 
#ifndef _HTTPCLIENT_H
#define _HTTPCLIENT_H

#include "HTTPMessage.h"
#include "../Common/Common.h"

class CHTTPClient
{
  public:
    CHTTPClient();
    ~CHTTPClient();


  bool Send(
    CHTTPMessage* pMessage,
    std::string   p_sTargetIPAddress,
    unsigned int  p_nTargetPort = 80
    );
  
  bool Get(
    std::string   p_sGetURL,
    CHTTPMessage* pResult
    );

  bool Get(
    std::string   p_sGet,
    CHTTPMessage* pResult,
    std::string   p_sTargetIPAddress,
    unsigned int  p_nTargetPort = 80
    );

    void AsyncNotify(std::string p_sCallback);

 
    fuppesThread m_AsyncThread;
    std::string  m_sAsyncResult;
    std::string  m_sNotifyCallback;
    bool         m_bAsyncDone;
    bool         m_bIsAsync;

  private:
    std::string BuildGetHeader(std::string  p_sGet,
                               std::string  p_sTargetIPAddress,
                               unsigned int p_nTargetPort);
};

#endif /* _HTTPCLIENT_H */
