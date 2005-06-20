/***************************************************************************
 *            MessageBase.cpp
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
 
#include "MessageBase.h"

CMessageBase::CMessageBase(std::string p_sMessage)
{
	m_sContent = p_sMessage;
}

CMessageBase::~CMessageBase()
{
}

sockaddr_in CMessageBase::GetLocalEndPoint()
{
	return m_LocalEp;
}

void CMessageBase::SetLocalEndPoint(sockaddr_in p_EndPoint)
{
	m_LocalEp = p_EndPoint;
}

sockaddr_in CMessageBase::GetRemoteEndPoint()
{
	return m_RemoteEp;
}

void CMessageBase::SetRemoteEndPoint(sockaddr_in p_EndPoint)
{
	m_RemoteEp = p_EndPoint;
}

std::string CMessageBase::GetContent()
{
	return m_sContent;
}
