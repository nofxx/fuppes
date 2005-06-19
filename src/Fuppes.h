/***************************************************************************
 *            Fuppes.h
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
 
#ifndef _FUPPES_H
#define _FUPPES_H

#include "SSDPCtrl.h"
#include "HTTPServer.h"
#include "HTTPMessage.h"
#include "MediaServer.h"
#include "ContentDirectory/ContentDirectory.h"


class CFuppes: public ISSDPCtrl, IHTTPServer
{
	public:
		CFuppes();
	  virtual ~CFuppes();
			
		CSSDPCtrl* GetSSDPCtrl();
	
	private:
		CSSDPCtrl*         m_pSSDPCtrl;
	  CHTTPServer*       m_pHTTPServer;
	  CMediaServer*      m_pMediaServer;
	  CContentDirectory* m_pContentDirectory;
	
		CHTTPMessage* HandleHTTPGet(CHTTPMessage*);
	  CHTTPMessage* HandleHTTPPost(CHTTPMessage*);
	
	  void OnSSDPCtrlReceiveMsg(CSSDPMessage*);
	  CHTTPMessage* OnHTTPServerReceiveMsg(CHTTPMessage*);
};

#endif /* _FUPPES_H */
