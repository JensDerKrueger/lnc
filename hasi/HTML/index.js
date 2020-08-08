function honk(melody) {
  setValue("CH.1",melody)

  /*
  var xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function() {
      if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
          x(xmlHttp.responseText);
  }
  xmlHttp.open("GET", "http://134.91.11.186:11111/set?variable=CH.1&value="+melody, true);
  xmlHttp.send(null);


  function honkCallback(info) {
    // nothing yet
  }

  */
}

function updatePage() {
  document.getElementById("temp").innerHTML="Temperatur: " + getValue("CH.3").toFixed(1) + "°C";
  document.getElementById("humid").innerHTML="Luftfeuchtigkeit: " + getValue("CH.4").toFixed(1) + " %";
}
