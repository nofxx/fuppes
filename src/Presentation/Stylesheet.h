/***************************************************************************
 *            Stylesheet.h
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
 
#ifndef _STYLESHEET_H
#define _STYLESHEET_H

#include <sstream>

std::string GetStylesheet(std::string p_sRelativeImagePath)
{
  std::stringstream sStylesheet;
  sStylesheet << "body {" << endl <<
    "margin:      0;" << endl <<
    "padding:     0;" << endl <<
    "font-family: Verdana, Arial, Helvetica;" << endl <<
    "font-size:   10pt;" << endl <<
    "background-color: #DACECE;" << endl <<
    "background-image: url(\"" << p_sRelativeImagePath << "fuppes.png\");" << endl <<
    "background-repeat: no-repeat;"
    "background-position: center;" << endl << 
    "}" << endl << endl;  
  
  
  sStylesheet << "#title {" << endl <<
    "margin-top:    5px;" << endl <<
    "margin-left:   0px;" << endl <<
    "margin-bottom: 5px;" << endl <<
    "margin-right:  0px;" << endl <<
    "padding-top:   0px;" << endl <<
    "height:        65px;" << endl <<
    "border-style:  solid;" << endl <<
    "border-color:  black;" << endl <<
    "border-width:  1px 0;" << endl <<
    "background:    #FF0000;" << endl <<
    "vertical-align: middle;" << endl <<
    "background-image: url(\"" << p_sRelativeImagePath << "fuppes-small.png\");" << endl <<
    "background-repeat:no-repeat;" << endl <<
    "background-position: 5px;" << endl <<
    "}" << endl << endl;
  
  
  sStylesheet << "#content {"  << endl;
  sStylesheet << "margin-left:   145px;" << endl;
  sStylesheet << "margin-top:    0pt;" << endl;
  sStylesheet << "margin-right:  5px;" << endl;
  sStylesheet << "margin-bottom: 10px;" << endl;
  sStylesheet << "padding-left:  10px;" << endl;
  sStylesheet << "padding-right: 10px;" << endl;  
  sStylesheet << "border-style:  solid;" << endl;
  sStylesheet << "border-color:  black;" << endl;
  sStylesheet << "border-width:  1px;" << endl;
  sStylesheet << "background:    #FFFFFF;" << endl;
  sStylesheet << "}" << endl;
  
  sStylesheet << "#menu{"  << endl;
  sStylesheet << "position:     absolute;"  << endl;
  sStylesheet << "top:          77px;"  << endl;
  sStylesheet << "left:         5px;"  << endl;
  sStylesheet << "width:        135px;"  << endl;
  sStylesheet << "padding:      0px;"  << endl;
  sStylesheet << "margin:       0px;"  << endl;
  sStylesheet << "border-style: solid;"  << endl;
  sStylesheet << "border-color: black;"  << endl;
  sStylesheet << "border-width: 1px;"  << endl;
  sStylesheet << "background:   #FFFFFF;"  << endl; 
  sStylesheet << "text-align:    left;"  << endl;
  sStylesheet << "}"  << endl;
  
  return sStylesheet.str();
}


/*#F9F9F9; */


  

#endif /* _STYLESHEET_H */
