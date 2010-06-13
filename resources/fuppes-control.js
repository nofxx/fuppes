function fuppesCtrl() 
{

  var request = new Request({
    url: '/', 
    onSuccess: function(responseText, responseXML) {

			$('ctrl-result').innerHTML = responseXML.getElement('Result').get('text');

 	}});

  request.setHeader('SOAPAction','"fuppesctrl#Test"');
  request.setHeader('Content-Type','text/xml; charset=utf-8');
  request.setHeader('User-Agent','fuppes webinterface');

  var body = '<?xml version="1.0"?>' +
    '<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">' +
    '<s:Body>' + 
			'<c:Test xmlns:c="urn:fuppesControl">' +
  		'</c:Test>' +
    '</s:Body>' +
    '</s:Envelope>';

  request.send({
    method: 'post',
    data: body
  });

}
