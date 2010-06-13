
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


function browseDirectChildren(objectId, startIdx, requestCnt, vfolder) 
{

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
        title = '<a href="javascript:browseDirectChildren(\'' + item.get('id') + '\', 0, 0, \'' + vfolder + '\');">' + dc_title + '</a>';
        type = '<a href="javascript:showObjectDetails(\'' + item.get('id') + '\', \'' + vfolder + '\');">' + "container" + '</a>';
      }
      else if(upnp_class.indexOf("object.item.") != -1) {
        title = dc_title;
        type = '<a href="javascript:showObjectDetails(\'' + item.get('id') + '\', \'' + vfolder + '\');">' + "item" + '</a>';
      }

      table += '<tr>';

      table += '<td>' + item.get('id')  + '</td>';
      table += '<td>' + type  + '</td>';
      table += '<td>' + title  + '</td>';
      table += '<td>' + item.get('childCount')  + '</td>';

      table += '</tr>';


      table += '<tr>';
      table += '<td colspan="4" class="detail-td" style="display: none;" id="detail-td-' + item.get('id') + '">load details</td>';
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

		browseMetadata(objectId, vfolder);
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



function browseMetadata(objectId, vfolder, details) 
{
	if(details == undefined)
		details = false;

  var request = new Request({
    url: '/UPnPServices/ContentDirectory/control/', 
    onSuccess: function(responseText, responseXML) {

			var result = loadResult(responseXML.getElement('Result').get('text'));
		  var items = result.documentElement.getChildren();
		  items.each(function(item, index) {

				if(!details) {
					$('parent-id').innerHTML = item.get('parentID');
					if(item.get('parentID') != -1)
						$('parent-browse').innerHTML = '<a href="javascript:browseDirectChildren(\'' + item.get('parentID') + '\', 0, 0, \''  + vfolder + '\');">up</a>';
					else
						$('parent-browse').innerHTML = '';
				}
				else {
					setObjectDetails(item);
				}
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



function showObjectDetails(objectId, vfolder)
{
	browseMetadata(objectId, vfolder, true);

	var details = $('detail-td-' + objectId);
  if(details == undefined)
    return;

	//details.setStyle('background-color', "#00FF00");

	visible = (details.getStyle('display') != "none");
  if(visible) {
		closeObjectDetails(objectId);
    return;
  }

  details.fade('hide');
  details.setStyle('display', '');
  details.get('tween', {property: 'opacity', duration: 'short'}).start(1);
}

function setObjectDetails(object)
{
	result = '<div><a href="javascript:closeObjectDetails(\'' + object.get('id') + '\');">close</a></div>';

	result += '<div>';

		result += 'object id: ' +  object.get('id') + '<br />';		
		result += 'parent id: ' +  object.get('parentID') + '<br />';

    var nodes = object.getChildren();
    nodes.each(function(node, node_index) {
			result += node.get('tag') + ": " + node.get('text') + "<br />";
    });


	result += '</div>';

	var details = $('detail-td-' + object.get('id'));
	details.innerHTML = result;
}

function closeObjectDetails(objectId)
{
	var details = $('detail-td-' + objectId);
  if(details == undefined)
    return;

  visible = (details.getStyle('display') != "none");
  if(!visible) {
    return;
  }

	details.setStyle('display', 'none');
  /*details.get('tween', {property: 'opacity', duration: 'short'}).start(0).chain (
    function() {
      this.element.setStyle('display', 'none')
    }
  );*/
}

