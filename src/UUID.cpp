/***************************************************************************
 *            UUID.cpp
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
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

#include "UUID.h"

#include <sstream>
using namespace std;

std::string GenerateUUID()
{
  srand(time(0));
  
  int nRandom;
  stringstream sRandom;
  
  do {
    nRandom = (rand() % 10000) + 1;
    sRandom << nRandom;
  } while (sRandom.str().length() < 8);
     
  stringstream sResult;
  sResult << sRandom.str().substr(0, 8) << "-aabb-0000-ccdd-1234eeff0000";
  
  return sResult.str();
}
