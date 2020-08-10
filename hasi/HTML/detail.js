function pageLoad() {
  window.addEventListener('resize', recomputeGraphs);
  recomputeGraphs();
}

function recomputeGraphs(){
  createGraph("tempGraph", tempData, "LNC Temperaturhistorie", unescape("%B0C"), 20, 40, 1, 60, 0 );
  createGraph("humidGraph", humidData, "LNC Feuchtehistorie", "%", 30, 70, 1, 60, 0 );
}
