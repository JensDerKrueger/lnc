function pageLoad() {
  window.addEventListener('resize', recomputeGraphs);
  recomputeGraphs();
}

function recomputeGraphs(){
  var tempData = [ /*append_tempData_here*/ ];
  var humidData = [ /*append_humidData_here*/ ];
  var queueData = [ /*append_queueData_here*/ ];

  createGraph("tempGraph", tempData, "Temperaturhistorie", unescape("%B0C"), 20, 40, 1, 60, 0 );
  createGraph("humidGraph", humidData, "Feuchtehistorie", "%", 30, 70, 1, 60, 0 );
  createGraph("queueGraph", queueData, "Script Queue Size", "", 0, 10, 1, 60 );
}
