/***************************************************************************
 *            fuppes-control.js
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

function fuppesCtrl(action) 
{

  var request = new Request({
    url: '/', 
    onSuccess: function(responseText, responseXML) {

			result = "";

			body = responseXML.documentElement.getFirst();
			body.getChildren().each(function(item) {

				// action response
				if(item.get('tag') == ('c:' + action.toLowerCase() + 'response')) {
					result = "SOAP RESPONSE";
				}

				// error
				else if(item.get('tag') == 'c:error') {
					code = item.getFirst();
					msg = code.getNext();
					result = "SOAP ERROR: " + msg.get('text') + " :: code: " + code.get('text');
				}

				// something else
				else {
					result = "unknown result";
				}

			});

			$('ctrl-result').innerHTML = result;

 	}});

  request.setHeader('SOAPAction','"fuppesctrl#' + action + '"');
  request.setHeader('Content-Type','text/xml; charset=utf-8');
  request.setHeader('User-Agent','fuppes webinterface');

  var body = '<?xml version="1.0"?>' +
    '<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">' +
    '<s:Body>' + 
			'<c:' + action + ' xmlns:c="urn:fuppesControl">' +
  		'</c:' + action + '>' +
    '</s:Body>' +
    '</s:Envelope>';

  request.send({
    method: 'post',
    data: body
  });

}

