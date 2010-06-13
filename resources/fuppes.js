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

