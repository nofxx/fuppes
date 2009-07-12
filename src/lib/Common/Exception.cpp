/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            Exception.cpp
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

#include "Exception.h" 
#include <sstream>
#include <stdarg.h>

using namespace fuppes;

Exception::Exception(std::string exception, char* file, int line)
: std::exception() {       
	
	m_file = file;
	m_line = line;
	m_exception = exception;
	
  /*std::stringstream sRes;
  sRes << exception << " (" << file << ", " << line << ")";
  m_exception = sRes.str();*/
};

Exception::Exception(const std::string file, int line, const char* exception, ...)
: std::exception() {       
	
	m_file = file;
	m_line = line;
	
  va_list args;
  char buffer[1024];
  va_start(args, exception);
  vsnprintf(buffer, sizeof(buffer), exception, args);
  va_end(args);
  m_exception = buffer;
};
