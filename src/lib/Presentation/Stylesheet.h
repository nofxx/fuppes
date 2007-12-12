/***************************************************************************
 *            Stylesheet.h
 *
 *  FUPPES - Free UPnP Entertainment Service
 *
 *  Copyright (C) 2005 Ulrich Völkel <u-voelkel@users.sourceforge.net>
 ****************************************************************************/

/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as 
 *  published by the Free Software Foundation.
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
 
#ifndef _STYLESHEET_H
#define _STYLESHEET_H

#ifdef HAVE_CONFIG_H
#include "../../config.h"
#endif

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
    /*"background-image: url(\"" << p_sRelativeImagePath << "fuppes.png\");" << endl <<    
    "background-repeat: no-repeat;" << endl <<    
    "background-position: center;" << endl << */
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
    "background:    #5797BE;" << endl <<
    "vertical-align: middle;" << endl <<
    "color: #FFFFFF;" << endl << endl <<
    
    /*"background-image: url(\"" << p_sRelativeImagePath << "fuppes-small.png\");" << endl <<
    "background-repeat:no-repeat;" << endl <<
    "background-position: 5px;" << endl <<*/
    
    "background-image: url(\"" << p_sRelativeImagePath << "header-gradient.png\");" << endl <<
    "background-repeat: repeat-x;" << endl <<  
    
    "}" << endl << endl;
  
  sStylesheet << "#mainframe {" << endl <<
                 "margin-left:   145px;" << endl <<
                 "margin-top:    0pt;" << endl <<
                 "margin-right:  5px;" << endl <<
                 "margin-bottom: 10px;" << endl;
  sStylesheet << "padding-left:  0px;" << endl;
  sStylesheet << "padding-right: 0px;" << endl;  
  sStylesheet << "border-style:  solid;" << endl;
  sStylesheet << "border-color:  black;" << endl;
  sStylesheet << "border-width:  1px;" << endl;
  sStylesheet << "background:    #FFFFFF;" << endl;
  sStylesheet << "}" << endl;
  
  
  sStylesheet << "#content {"  << endl <<
                 "margin-left:   0px;" << endl <<
                 "margin-top:    0px;" << endl <<
                 "margin-right:  0px;" << endl <<
                 "margin-bottom: 0px;" << endl;
  sStylesheet << "padding-left:  10px;" << endl;
  sStylesheet << "padding-right: 10px;" << endl;  
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
  sStylesheet << "}"  << endl << endl;
    
  /* <h1> */
  sStylesheet << "h1{" << endl <<
                 "font-size: 150%;" << endl <<
                 "border-bottom: 1px solid #aaaaaa;" << endl <<
                 "}" << endl << endl;
                 
  /* <h2> */
  sStylesheet << "h2{" << endl <<
                 "font-size: 120%;" << endl <<
                 "}" << endl << endl;     


  sStylesheet << "table {" << endl <<
                 "font-size: 10pt; " << endl <<
                 "border-style: solid;" << endl << 
                 "border-width: 1px;" << endl <<
                 "border-color: #000000;" << endl <<
                 "border-spacing: 0px;" << endl <<
                 "border-collapse: collapse;" << endl <<
                 "}" << endl << endl;
                 

  sStylesheet << "th {" << endl <<
                 "background-image: url(\"" << p_sRelativeImagePath << "header-gradient-small.png\");" << endl <<
                 "background-repeat: repeat-x;" << endl << 
                 "color: #FFFFFF;" << endl <<
                 "}" << endl << endl;     
  
  sStylesheet << "td {" << endl <<                 
                 "border-style: solid;" << endl <<
                 "border-width: 1px;" << endl <<
                 "}" << endl << endl;  
  
  return sStylesheet.str();
}


/*#F9F9F9; */


  

#endif /* _STYLESHEET_H */
