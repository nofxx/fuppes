/***************************************************************************
 *            fuppes.js
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
 */

window.addEvent('domready', function() {

 //alert("domready");

});


function deviceDetails(idx)
{
  div = $('remote-device-details-' + idx);
  if(div == null) {
    alert("no div");
    return;
  }

  visible = div.getStyle('display') == "block"; 
  if(!visible) {
    div.setStyle('display', 'block');
  }
  else {
    div.setStyle('display', 'none');
  }
  
}

