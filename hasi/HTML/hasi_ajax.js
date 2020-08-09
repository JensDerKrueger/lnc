var root;
var url = "134.91.11.186";
var port = 11111;

function setValue(name, value) {
  var xmlhttp;
  if (window.XMLHttpRequest) {
    xmlhttp=new XMLHttpRequest(); // code for practically any reasonable browser
  } else {
    xmlhttp=new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
  }
  xmlhttp.open("GET", "http://"+url+":"+port+"/set?variable="+name+"&value="+value, true);
  xmlhttp.send(null);
}

function getValue(name) {
  if (root == null)
    return 0;
  else
    return 1*root.getElementsByTagName(name)[0].firstChild.nodeValue;
}

function refreshHASValues() {
  var xmlhttp;
  if (window.XMLHttpRequest) {
    xmlhttp=new XMLHttpRequest(); // code for practically any reasonable browser
  } else {
    xmlhttp=new ActiveXObject("Microsoft.XMLHTTP"); // code for IE6, IE5
  }
  xmlhttp.onreadystatechange=function()
  {
    if (xmlhttp.readyState==4 && xmlhttp.status==200 && xmlhttp.responseXML)
    {
      root = xmlhttp.responseXML.documentElement;
      updatePage();
    }
  }
  xmlhttp.open("GET","volatile/hasStates.xml",true);
  xmlhttp.send();
}

var interval = setInterval(refreshHASValues, 1000);

