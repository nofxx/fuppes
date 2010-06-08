window.addEvent('domready', function() {

 //alert("domready");

});


function loadResult(result)
{
  var doc;

  // code for IE
  if (window.ActiveXObject) {
    doc=new ActiveXObject("Microsoft.XMLDOM");
    doc.async="false";
    doc.loadXML(result);
  }
  // code for Mozilla, Firefox, Opera, etc.
  else {
    var parser=new DOMParser();
    doc=parser.parseFromString(result,"text/xml");
  }

  return doc;  
}


function browseDirectChildren(objectId, startIdx, requestCnt) 
{
	var vfolder = $('virtual-layout').value;

  var request = new Request({
    url: '/UPnPServices/ContentDirectory/control/', 
    onSuccess: function(responseText, responseXML) {

    var table = '<table id="table-browse" width="100%">' +
      '<tr>' +
      '<th width="100">id</th>' +
      '<th width="100">class</th>' +
      '<th>title</th>' +
      '<th width="60">count</th>' +
      '</tr>';

		table += '<tr>' +
         '<td id="parent-id"></td>' +
	       '<td colspan="3" id="parent-browse">&nbsp;</td>' +
			'</tr>';


    var msg = '';
    msg += 'NumberReturned: ' + responseXML.documentElement.getElement('NumberReturned').get('text') + '<br />';
    msg += 'TotalMatches: ' + responseXML.documentElement.getElement('TotalMatches').get('text') + '<br />';


    var result = loadResult(responseXML.getElement('Result').get('text'));
    var items = result.documentElement.getChildren();
    items.each(function(item, index) {

      var nodes = item.getChildren();

      var upnp_class = null;
      var dc_title = null;
      var res = null;
      nodes.each(function(node, node_index) {

        if(node.get('tag') == 'upnp:class') {
          upnp_class = node.get('text');
        }
        else if(node.get('tag') == 'dc:title') {
          dc_title = node.get('text');
        }
        else if(node.get('tag') == 'res') {
          res = node.get('text');
        }

      });


      
      var title = null;
      var type = null;

      if(upnp_class.indexOf("object.container.") != -1) {
        title = '<a href="javascript:browseDirectChildren(\'' + item.get('id') + '\', 0, 0);">' + dc_title + '</a>';
        type = "container";
      }
      else if(upnp_class.indexOf("object.item.") != -1) {
        title = dc_title;
        type = "item";
      }

      table += '<tr>';

      table += '<td>' + item.get('id')  + '</td>';
      table += '<td>' + type  + '</td>';
      table += '<td>' + title  + '</td>';
      table += '<td>' + item.get('childCount')  + '</td>';

      table += '</tr>';
      

/*
<container id="0000000001" searchable="0" parentID="0" restricted="0" childCount="48">
<dc:title>title</dc:title>
<upnp:class>object.container.storageFolder</upnp:class>
</container>

<item id="000000963D" parentID="6FF6" restricted="0">
<dc:title>title</dc:title>
<upnp:class>object.item.videoItem</upnp:class>
<upnp:albumArtURI xmlns:dlna="urn:schemas-dlna-org:metadata-1-0/" dlna:profileID="JPEG_TN">http://192.168.0.8:5080/MediaServer/ImageItems/000000963D.jpg</upnp:albumArtURI>
<res protocolInfo="http-get:*:video/x-msvideo:DLNA.ORG_PS=1;DLNA.ORG_CI=0;DLNA.ORG_OP=01;DLNA.ORG_FLAGS=21700000000000000000000000000000" duration="00:14:30.05" resolution="320x240" bitrate="13563" size="11806720">http://192.168.0.8:5080/MediaServer/VideoItems/000000963D.avi</res>
</item>
*/
    });


    table += '</table>';
    $('browse-result').innerHTML = table;

		browseMetadata(objectId);
  }});

  request.setHeader('SOAPAction','"urn:schemas-upnp-org:service:ContentDirectory:1#Browse"');
  request.setHeader('Content-Type','text/xml; charset=utf-8');
  request.setHeader('User-Agent','fuppes webinterface');
  request.setHeader('Virtual-Layout', vfolder);

  var body = '<?xml version="1.0"?>' +
    '<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">' +
    '<s:Body>' + 
    '<u:Browse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1"> ' +
    '<ObjectID>' + objectId + '</ObjectID>' + 
    '<Filter>*</Filter>' + 
    '<StartingIndex>' + startIdx + '</StartingIndex>' + 
    '<RequestedCount>' + requestCnt + '</RequestedCount>' + 
    '<SortCriteria></SortCriteria>' + 
    '<BrowseFlag>BrowseDirectChildren</BrowseFlag>' + 
    '</u:Browse>' +
    '</s:Body>' +
    '</s:Envelope>';

  request.send({
    method: 'post',
    data: body
  });

}



function browseMetadata(objectId) 
{
	var vfolder = $('virtual-layout').value;

  var request = new Request({
    url: '/UPnPServices/ContentDirectory/control/', 
    onSuccess: function(responseText, responseXML) {

			var result = loadResult(responseXML.getElement('Result').get('text'));
		  var items = result.documentElement.getChildren();
		  items.each(function(item, index) {

				$('parent-id').innerHTML = item.get('parentID');
				if(item.get('parentID') != -1)
					$('parent-browse').innerHTML = '<a href="javascript:browseDirectChildren(\'' + item.get('parentID') + '\', 0, 0);">up</a>';
				else
					$('parent-browse').innerHTML = '';
			});

 	}});

  request.setHeader('SOAPAction','"urn:schemas-upnp-org:service:ContentDirectory:1#Browse"');
  request.setHeader('Content-Type','text/xml; charset=utf-8');
  request.setHeader('User-Agent','fuppes webinterface');
  request.setHeader('Virtual-Layout', vfolder);

  var body = '<?xml version="1.0"?>' +
    '<s:Envelope xmlns:s="http://schemas.xmlsoap.org/soap/envelope/" s:encodingStyle="http://schemas.xmlsoap.org/soap/encoding/">' +
    '<s:Body>' + 
    '<u:Browse xmlns:u="urn:schemas-upnp-org:service:ContentDirectory:1"> ' +
    '<ObjectID>' + objectId + '</ObjectID>' + 
    '<Filter>*</Filter>' + 
    '<StartingIndex>0</StartingIndex>' + 
    '<RequestedCount>0</RequestedCount>' + 
    '<SortCriteria></SortCriteria>' + 
    '<BrowseFlag>BrowseMetadata</BrowseFlag>' + 
    '</u:Browse>' +
    '</s:Body>' +
    '</s:Envelope>';

  request.send({
    method: 'post',
    data: body
  });

}



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

