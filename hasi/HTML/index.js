function pageLoad() {
  refreshHASValues();
}

function honk(melody) {
  setValue("pulse_honk",melody)
}

function updatePage() {
  document.getElementById("statusMessage").innerHTML= "LNC Status (Chathupe ist " + ( getValue("CH.2") == 1 ? "online" : "offline" ) + ")";
  document.getElementById("temp").innerHTML="Temperatur: " + getValue("CH.3").toFixed(1) + "°C";
  document.getElementById("humid").innerHTML="Luftfeuchtigkeit: " + getValue("CH.4").toFixed(1) + " %";
}
