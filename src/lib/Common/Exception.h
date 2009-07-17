/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Exception.h
 * 
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2009 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

#include <exception>
#include <string>


namespace fuppes {

class Exception: public std::exception
{
  public:
    Exception(const std::string exception, const std::string file, int line);    
    Exception(const std::string file, int line, const char* exception, ...);  
    ~Exception() throw() {};

		std::string what() { return m_exception; }
		std::string	file() { return m_file; }
		int					line() { return m_line; }
		
  private:
		std::string m_file;
		int					m_line;
    std::string m_exception;
};

}

#endif // _EXCEPTION_H
