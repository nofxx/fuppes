/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            ControlInterface.h
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

#ifndef _CONTROLTERFACE_H
#define _CONTROLTERFACE_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <string>
#include <map>

typedef std::multimap<std::string, std::string>	argList;
typedef std::multimap<std::string, std::string>::iterator	argListIter;

typedef std::map<std::string, std::string>	stringList;
typedef std::map<std::string, std::string>::iterator	stringListIter;

class CControlInterface
{
	public:
		int	action(std::string action, stringList* args, stringList* result);
		//int	action(std::string action, stringList* values);
};

#endif // _CONTROLTERFACE_H
