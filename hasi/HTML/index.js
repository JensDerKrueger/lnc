function pageLoad() {
  refreshHASValues();
  window.addEventListener('resize', recomputeGraphs);
  recomputeGraphs();
}

function recomputeGraphs(){
  createGraph("tempGraph", tempData, "LNC Temperaturhistorie", unescape("%B0C"), 1, 50);
}

function honk(melody) {
  setValue("CH.1",melody)
}

function updatePage() {
  document.getElementById("temp").innerHTML="Temperatur: " + getValue("CH.3").toFixed(1) + "°C";
  document.getElementById("humid").innerHTML="Luftfeuchtigkeit: " + getValue("CH.4").toFixed(1) + " %";
}