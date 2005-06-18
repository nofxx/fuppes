/***************************************************************************
 *            Message.cpp
 *
 *  Sun Jun 12 18:29:01 2005
 *  Copyright  2005  Ulrich Völkel
 *  mail@ulrich-voelkel.de
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
 
#include "Message.h"

CMessage::CMessage(std::string p_sMessage)
{
	m_sContent = p_sMessage;
}

CMessage::~CMessage()
{
}

sockaddr_in CMessage::get_local_ep()
{
	return local_ep;
}

void CMessage::set_local_ep(sockaddr_in a_ep)
{
	local_ep = a_ep;
}

sockaddr_in CMessage::get_remote_ep()
{
	return remote_ep;
}

void CMessage::set_remote_ep(sockaddr_in a_ep)
{
	remote_ep = a_ep;
}

std::string CMessage::GetContent()
{
	return m_sContent;
}
