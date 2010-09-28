/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            SoapControl.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2008 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
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

#ifndef _SOAPCONTROL_H
#define _SOAPCONTROL_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include "../UPnPService.h"
#include "../Common/XMLParser.h"

class SoapControl: public CUPnPService
{
	public:
		SoapControl(std::string HTTPServerURL);
		
		std::string GetServiceDescription();
    void HandleUPnPAction(CUPnPAction* pUPnPAction, CHTTPMessage* pMessageOut);


	private:

		void	getDir(CXMLNode* request, std::stringstream& result);
		void	getSharedObjects(std::stringstream& result);
		void	addSharedObject(CXMLNode* request, std::stringstream& result);
		void	delSharedObject(CXMLNode* request, std::stringstream& result);
};

#endif // _SOAPCONTROL_H
