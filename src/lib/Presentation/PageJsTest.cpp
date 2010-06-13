/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            PageJsTest.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2010 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "PageJsTest.h"

#include "../Common/Common.h"
#include "../SharedConfig.h"

std::string PageJsTest::content()
{
  std::stringstream result;
  
  result << "<h1>JS test</h1>";


  // soap controll
  result << "<h2>fuppes soap controll</h2>";
  result << "<div>";
  result << "<a href=\"javascript:fuppesCtrl();\">ctrl test</a>";
  result << "<div id=\"ctrl-result\"></div>";
  result << "</div>";
  
  return result.str();
}


