window.addEvent('domready', function() {

	$('directory').addEvent('click', function() {
		alert('directory');
	});


	getSharedObjects();

});


function getSharedObjects() {

	var request = new FuppesControl({
		action: 'GetSharedObjects',
		onComplete: function(result) {

			type = undefined;
			path = undefined;

			tmp = "<table>";

			result.getChildren().each(function(item) {

				index = item.getElement('index').get('text');
				type = item.getElement('type').get('text');
				path = item.getElement('path').get('text');

				tmp += "<tr>";
				tmp += "<td>" + type + " :: " + path + "</td>";
				tmp += "<td><a href=\"javascript:delSharedObject(" + index + ");\">DEL</a></td>";
				tmp += "</tr>";
			});


			tmp += "</table>";

			$('shared-objects-result').innerHTML = tmp;
		}
	});

	request.send();
}

function getDir(dir) {

	body = "<dir show-files=\"false\">" + dir + "</dir>";

	var request = new FuppesControl({
		action: 'GetDir',
		body: body,
		onComplete: function(result) {

			type = undefined;
			path = undefined;

			tmp = "<table width=\"100%\">";
			result.getChildren().each(function(item) {

				tmp += "<tr>";

				if(item.get('tag') == 'parent') {
					tmp += '<td colspan="2"><a href="javascript:getDir(\'' + item.get('text') + '\');">' + item.get('text') + '</a></td>';
				}
				else {
					tmp += '<td>';
					tmp += '<a href="javascript:getDir(\'' + item.get('text') + '\');">' + item.get('name') + '</a>';
					tmp += '</td>';
					tmp += '<td>';
					tmp += '<a href="javascript:setDir(\'' + item.get('text') + '\');">SET</a>';
					tmp += '</td>';
				}

				tmp += "</tr>";

			});
			tmp += "</table>";

			$('object-list').innerHTML = tmp;
		}
	});

	request.send();
}

function setDir(dir) {
	$('selected-object-type').innerHTML = 'directory';
	$('selected-object-path').innerHTML = dir;
}

function dlgAddSharedObject() {

	var dlg = $('dlg-shared-object');
  if(dlg == undefined)
    return;

	visible = (dlg.getStyle('display') != "none");
  if(visible) {
		dlg.setStyle('display', 'none');
    return;
  }

	width = $('mainframe').getStyle('margin-left').toInt() + $('mainframe').getStyle('width').toInt();
	width = (width / 2) - (dlg.getStyle('width').toInt() / 2);
	left = ($('mainframe').getStyle('margin-left').toInt() * -1) + width;
	dlg.setStyle('left', left);

  dlg.fade('hide');
  dlg.setStyle('display', 'block');
  dlg.get('tween', {property: 'opacity', duration: 'short'}).start(1);

	getDir("");
}

function submit() {

	body = "<object>";
	body += "<type>" + $('selected-object-type').get('text') + "</type>";
	body += "<path>" + $('selected-object-path').get('text') + "</path>";
	body += "</object>";

	var request = new FuppesControl({
		action: 'AddSharedObject',
		body: body,
		onComplete: function(result) {
			getSharedObjects();
			dlgAddSharedObject();
		}
	});

	request.send();
}

function cancel() {
	dlgAddSharedObject();
}

function delSharedObject(index) {

	body = "<index>" + index + "</index>";

	var request = new FuppesControl({
		action: 'DelSharedObject',
		body: body,
		onComplete: function(result) {
			getSharedObjects();
		}
	});

	request.send();
}

