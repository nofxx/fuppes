/* -*- Mode: C++; indent-tabs-mode: nil; c-basic-offset: 2; tab-width: 2 -*- */
/***************************************************************************
 *            PageConfig.cpp
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

#include "PageConfig.h"

#include "../Common/Common.h"
#include "../SharedConfig.h"

std::string PageConfig::content()
{
  std::stringstream result;
  
  result << "<h1>shared objects</h1>";
  result << "<div id=\"shared-objects-result\"></div>";

  result << "<a id=\"shared-object-add\" href=\"javascript:dlgAddSharedObject();\">add shared object</a>";
    

  result << "<div id=\"dlg-shared-object\">";

  result << "<div>";
  result << "<input type=\"radio\" name=\"object-type\" id=\"directory\" value=\"directory\" checked=\"checked\" /><label for=\"directory\">directory</label>";
  result << "<input type=\"radio\" name=\"object-type\" id=\"playlist\" value=\"playlist\" disabled=\"disabled\" /><label for=\"playlist\">playlist</label>";
  result << "<input type=\"radio\" name=\"object-type\" id=\"itunes\" value=\"itunes\" disabled=\"disabled\" /><label for=\"itunes\">itunes</label>";
  result << "<input type=\"radio\" name=\"object-type\" id=\"stream-url\" value=\"stream-url\" disabled=\"disabled\" /><label for=\"stream-url\">stream-url</label>";
  result << "</div>";
  
  result << "<div id=\"object-list\"></div>";
  result << "<div id=\"selected-object-type\"></div>";
  result << "<div id=\"selected-object-path\"></div>";

  result << "<span><a id=\"submit\" href=\"javascript:submit();\">submit</a></span> ";
  result << "<span><a id=\"submit\" href=\"javascript:cancel();\">cancel</a></span>";
  
  result << "</div>";
  
  return result.str();
}


