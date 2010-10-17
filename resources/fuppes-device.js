
// set the remote device 'uuid' to the device layout selected in device-'uuid'
function setDevice(uuid) {

	sel = $('device-' + uuid);
	dev = sel.options[sel.selectedIndex].text;

	body = "<uuid>" + uuid + "</uuid>" +
         "<device>" + dev + "</device>";

	var request = new FuppesControl({
		action: 'SetDevice',
		body: body,
		onComplete: function(result) {
		},
		onError: function(error) {
		}
	});
	request.send();

}

