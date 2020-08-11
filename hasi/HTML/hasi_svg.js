var svgNS = "http://www.w3.org/2000/svg";
var imageX, imageY, sizeX, sizeY, minValX, minValY, border=10;

function padMinutes(number) {
  return (number < 10) ? ("0" + number) : ("" + number);
}

function modToTime(mod) {
  var hour = Math.floor(mod/60);
  var min = mod % 60;
  return hour + ":" + padMinutes(min);
}

function data2posX(x) {
  return border + (imageX-2*border) * (x-minValX)/sizeX;
}

function data2posY(y) {
  return imageY - (border + ((imageY-2*border) * (y-minValY)/sizeY));
}

function indexToColor(i, count) {
  var h = Number(i)*360/Number(count);
  return "hsl("+h+", 100%, 40%)";
}

function createGraph(svgElemName, rawData, graphCaption, unit, sugMin, sugMax, scale, tickScale, ignoreValue) {
  scale = typeof scale !== 'undefined' ? scale : 1.0;
  tickScale = typeof tickScale !== 'undefined' ? tickScale : 60;

  var svg = document.getElementById(svgElemName);

  while (svg.firstChild) {
    svg.removeChild(svg.firstChild);
  }

  var graph = document.createElementNS(svgNS,"polyline");
  
  imageX = svg.getBoundingClientRect().width;
  imageY = svg.getBoundingClientRect().height;
  minValX=Number.POSITIVE_INFINITY;
  var maxValX=Number.NEGATIVE_INFINITY;
  minValY=Number.POSITIVE_INFINITY
  var maxValY=Number.NEGATIVE_INFINITY;
  var posMaxValY, posMinValY;
  
  var lastY = 0;
  var avgY = 0;


  var data = [];
  var arrayLength = rawData.length;
  if (typeof ignoreValue !== 'undefined') {
    for (var i = 0; i < arrayLength; i++) {
      var point = rawData[i].split(" ");
      point[0] = Number(point[0]);
      point[1] = Number(point[1])*scale;
      if (point[1] == ignoreValue) {
        continue;
      }
      data.push(rawData[i]);
    }
  }  else {
    data = rawData;
  }
  arrayLength = data.length;

  for (var i = 0; i < arrayLength; i++) {
    var point = data[i].split(" ");
    point[0] = Number(point[0]);
    point[1] = Number(point[1])*scale;
    
    minValX = Math.min(minValX, point[0]);
    maxValX = Math.max(maxValX, point[0]);
    
    if (parseFloat(point[1]) < parseFloat(minValY)) {
      minValY = point[1];
      posMinValY = point[0];
    }
    
    if (parseFloat(point[1]) >= parseFloat(maxValY)) {
      maxValY = point[1];
      posMaxValY = point[0];
    }
    
    lastY = point[1];
    avgY += 1*point[1];
  }
  
  if (data.length != 0) avgY /= data.length;
  
  var realMin = minValY;
  var realMax = maxValY;
  
  if (sugMax > sugMin) {
    minValY = Math.min(sugMin, minValY);
    maxValY = Math.max(sugMax, maxValY);
  } else {
    var valueBorder = (maxValY-minValY)*0.1;
    maxValY += valueBorder;
    minValY -= valueBorder;
  }
  
  sizeX = maxValX-minValX;
  sizeY = maxValY-minValY;
  
  for (var i in data) {
    var point = data[i].split(" ");
    point[0] = Number(point[0]);
    point[1] = Number(point[1])*scale;
    
    if ( point[0] % tickScale == 0) {
      var xTick = document.createElementNS(svgNS,"polyline");
      xTick.setAttribute("points","" + data2posX(point[0]) + " " + (imageY-border) + " " + data2posX(point[0]) + " " + imageY);
      xTick.setAttribute("fill","none");
      xTick.setAttribute("stroke","black");
      xTick.setAttribute("stroke-width","1");
      svg.appendChild(xTick);
      
      var xTickGrey = document.createElementNS(svgNS,"polyline");
      xTickGrey.setAttribute("points","" + data2posX(point[0]) + " " + (imageY-border) + " " + data2posX(point[0]) + " " + border);
      xTickGrey.setAttribute("fill","none");
      xTickGrey.setAttribute("stroke","lightgrey");
      xTickGrey.setAttribute("stroke-width","1");
      xTickGrey.setAttribute("stroke-dasharray","2, 5");
      svg.appendChild(xTickGrey);
      
      var tickCaption = document.createElementNS(svgNS,"text");
      tickCaption.setAttribute("x",data2posX(point[0])+2);
      tickCaption.setAttribute("y",imageY-1);
      tickCaption.setAttribute("font-size",10);
      tickCaption.setAttribute("fill","blue");
      tickCaption.textContent = tickScale == 60 ? modToTime(point[0]) : point[0];
      svg.appendChild(tickCaption);
    }
    
    data[i] = "" + data2posX(point[0]) + " " + data2posY(point[1]);
  }
  
  var yTickCount = 10;
  for ( var i = 1; i<yTickCount;++i) {
    var yTick = document.createElementNS(svgNS,"polyline");
    var yVal = minValY+i*sizeY/yTickCount;
    var y = data2posY(yVal);
    yTick.setAttribute("points","" + border + " " + y + " " + (border+10) + " " + y);
    yTick.setAttribute("fill","none");
    yTick.setAttribute("stroke","black");
    yTick.setAttribute("stroke-width","1");
    svg.appendChild(yTick);
    
    var yTickLong = document.createElementNS(svgNS,"polyline");
    var yVal = minValY+i*sizeY/yTickCount;
    var y = data2posY(yVal);
    yTickLong.setAttribute("points","" + (border+10) + " " + y + " " + (imageX-border) + " " + y);
    yTickLong.setAttribute("fill","none");
    yTickLong.setAttribute("stroke","lightgrey");
    yTickLong.setAttribute("stroke-width","1");
    yTickLong.setAttribute("stroke-dasharray","2, 5");
    svg.appendChild(yTickLong);
    
    var tickCaption = document.createElementNS(svgNS,"text");
    tickCaption.setAttribute("x",5+border);
    tickCaption.setAttribute("y",y-3);
    tickCaption.setAttribute("font-size",10);
    tickCaption.setAttribute("fill","blue");
    tickCaption.setAttribute("text-anchor", "left");
    tickCaption.textContent = yVal.toFixed(2) + " " + unit;
    svg.appendChild(tickCaption);
  }
  
  graph.setAttribute("points",data);
  graph.setAttribute("fill","none");
  graph.setAttribute("stroke","red");
  graph.setAttribute("stroke-width","2");
  
  var ruler = document.createElementNS(svgNS,"polyline");
  ruler.setAttribute("points", border + " " + border + " " + border + " " + (imageY-border) + " " + (imageX-border) + " " + (imageY-border));
  ruler.setAttribute("fill","none");
  ruler.setAttribute("stroke","black");
  ruler.setAttribute("stroke-width","2");
  
  var caption = document.createElementNS(svgNS,"text");
  caption.setAttribute("x",imageX/2);
  caption.setAttribute("y","15");
  caption.setAttribute("fill","black");
  caption.setAttribute("text-anchor", "middle");
  caption.textContent = graphCaption + " (Aktuell " + lastY.toFixed(2) + " " + unit + " Durchschnitt: " + avgY.toFixed(2) + " " + unit + ")";
  
  var captionMax = document.createElementNS(svgNS,"text");
  captionMax.setAttribute("x",5+border);
  captionMax.setAttribute("y",15);
  captionMax.setAttribute("fill","blue");
  captionMax.setAttribute("text-anchor", "left");
  captionMax.textContent = maxValY.toFixed(2) + " " + unit;
  
  var captionMin = document.createElementNS(svgNS,"text");
  captionMin.setAttribute("x",5+border);
  captionMin.setAttribute("y",imageY-(5+border));
  captionMin.setAttribute("fill","blue");
  captionMin.setAttribute("text-anchor", "left");
  captionMin.textContent = minValY.toFixed(2) + " " + unit;
  
  var avgLine = document.createElementNS(svgNS,"polyline");
  avgLine.setAttribute("points","" + border + " " + data2posY(avgY) + " " + (imageX-border) + " " + data2posY(avgY));
  avgLine.setAttribute("fill","dotted");
  avgLine.setAttribute("stroke","darkred");
  avgLine.setAttribute("stroke-width","2");
  avgLine.setAttribute("stroke-dasharray","2, 5");
  
  svg.appendChild(avgLine);
  svg.appendChild(ruler);
  svg.appendChild(graph);
  svg.appendChild(caption);
  svg.appendChild(captionMax);
  svg.appendChild(captionMin);
  
  if (posMaxValY > -1) {
    var maxCaption = document.createElementNS(svgNS,"text");
    maxCaption.setAttribute("x",imageX/2);
    maxCaption.setAttribute("y","35");
    maxCaption.setAttribute("fill","green");
    maxCaption.setAttribute("text-anchor", "middle");
    maxCaption.textContent = "Maximum: " + realMax.toFixed(2) + " " + unit + " um " + modToTime(posMaxValY) + " Uhr";
    
    var mal = document.createElementNS(svgNS,"circle");
    mal.setAttribute("cx", data2posX(posMaxValY));
    mal.setAttribute("cy", data2posY(realMax));
    mal.setAttribute("r", 5);
    mal.setAttribute("stroke","black");
    mal.setAttribute("stroke-width","1");
    mal.setAttribute("fill","green");
    
    svg.appendChild(maxCaption);
    svg.appendChild(mal);
  }
  
  if (posMinValY > -1) {
    var minCaption = document.createElementNS(svgNS,"text");
    minCaption.setAttribute("x",imageX/2);
    minCaption.setAttribute("y","55");
    minCaption.setAttribute("fill","orange");
    minCaption.setAttribute("text-anchor", "middle");
    minCaption.textContent = "Minimum: " + realMin.toFixed(2) + " " + unit + " um " + modToTime(posMinValY) + " Uhr";
    
    var mil = document.createElementNS(svgNS,"circle");
    mil.setAttribute("cx", data2posX(posMinValY));
    mil.setAttribute("cy", data2posY(realMin));
    mil.setAttribute("r", 5);
    mil.setAttribute("stroke","black");
    mil.setAttribute("stroke-width","1");
    mil.setAttribute("fill","orange");
    
    svg.appendChild(minCaption);
    svg.appendChild(mil);
  }
}


function createMultiGraph(svgElemName, rawData, graphCaptions, unit, sugMin, sugMax, scale, tickScale, ignoreValue)
{
  var pointCount = graphCaptions.length;

  scale = typeof scale !== 'undefined' ? scale : 1.0;
  tickScale = typeof tickScale !== 'undefined' ? tickScale : 60;
  
  var svg = document.getElementById(svgElemName);
  
  while (svg.firstChild) {
    svg.removeChild(svg.firstChild);
  }

  imageX = svg.getBoundingClientRect().width;
  imageY = svg.getBoundingClientRect().height;
  minValX=Number.POSITIVE_INFINITY;
  var maxValX=Number.NEGATIVE_INFINITY;
  minValY=Number.POSITIVE_INFINITY
  var maxValY=Number.NEGATIVE_INFINITY;
  var posMaxValY, posMinValY;

  var data = [];
  var arrayLength = rawData.length;
  if (typeof ignoreValue !== 'undefined') {
    for (var i = 0; i < arrayLength; i++) {
      var ok = 1;
      for (var j = 0; j < pointCount; j++) {
        var point = rawData[i].split(" ");
        if (Number(point[j+1])*scale == ignoreValue) ok = 0;
      }
      if (ok) data.push(rawData[i]);
    }
  }  else {
    data = rawData;
  }
  arrayLength = data.length;

  for (var j = 0; j < pointCount; j++) {
    for (var i = 0; i < arrayLength; i++) {
      var point = data[i].split(" ");
      point[0] = Number(point[0]);
      point[j+1] = Number(point[j+1])*scale;
      
      minValX = Math.min(minValX, point[0]);
      maxValX = Math.max(maxValX, point[0]);
      
      if (parseFloat(point[j+1]) < parseFloat(minValY)) {
        minValY = point[j+1];
        posMinValY = point[0];
      }
      
      if (parseFloat(point[j+1]) >= parseFloat(maxValY)) {
        maxValY = point[j+1];
        posMaxValY = point[0];
      }
    }
  }
  
  var realMin = minValY;
  var realMax = maxValY;
  
  if (sugMax > sugMin) {
    minValY = Math.min(sugMin, minValY);
    maxValY = Math.max(sugMax, maxValY);
  } else {
    var valueBorder = (maxValY-minValY)*0.1;
    maxValY += valueBorder;
    minValY -= valueBorder;
  }
  
  sizeX = maxValX-minValX;
  sizeY = maxValY-minValY;
  
  var pointData = [];
  pointData.length = arrayLength;
  for (var j = 0; j < pointCount; j++) {
    var avgY = 0;
    var lastY = 0;

    for (var i = 0; i < arrayLength; i++) {
      var point = data[i].split(" ");
      point[0] = Number(point[0]);
      point[j+1] = Number(point[j+1])*scale;
     
 
      lastY = point[j+1];
      avgY += point[j+1];
      
      if (j == 0 && point[0] % tickScale == 0) {
        var xTick = document.createElementNS(svgNS,"polyline");
        xTick.setAttribute("points","" + data2posX(point[0]) + " " + (imageY-border) + " " + data2posX(point[0]) + " " + imageY);
        xTick.setAttribute("fill","none");
        xTick.setAttribute("stroke","black");
        xTick.setAttribute("stroke-width","1");
        svg.appendChild(xTick);
        
        var xTickGrey = document.createElementNS(svgNS,"polyline");
        xTickGrey.setAttribute("points","" + data2posX(point[0]) + " " + (imageY-border) + " " + data2posX(point[0]) + " " + border);
        xTickGrey.setAttribute("fill","none");
        xTickGrey.setAttribute("stroke","lightgrey");
        xTickGrey.setAttribute("stroke-width","1");
        xTickGrey.setAttribute("stroke-dasharray","2, 5");
        svg.appendChild(xTickGrey);
        
        var tickCaption = document.createElementNS(svgNS,"text");
        tickCaption.setAttribute("x",data2posX(point[0])+2);
        tickCaption.setAttribute("y",imageY-1);
        tickCaption.setAttribute("font-size",10);
        tickCaption.setAttribute("fill","blue");
        tickCaption.textContent = tickScale == 60 ? modToTime(point[0]) : point[0]
        svg.appendChild(tickCaption);
      }
      pointData[i] = "" + data2posX(point[0]) + " " + data2posY(point[j+1]);
    }
    avgY /= arrayLength;

    var graph = document.createElementNS(svgNS,"polyline");
    graph.setAttribute("points",pointData);
    graph.setAttribute("fill","none");
    graph.setAttribute("stroke",indexToColor(j,pointCount));
    graph.setAttribute("stroke-width","2");
    svg.appendChild(graph);
    
    var caption = document.createElementNS(svgNS,"text");
    caption.setAttribute("x",imageX/2);
    caption.setAttribute("y",15+j*20);
    caption.setAttribute("fill",indexToColor(j,pointCount));
    caption.setAttribute("text-anchor", "middle");
    caption.textContent = graphCaptions[j] + " (Aktuell " + lastY.toFixed(2) + " " + unit + " Durchschnitt: " + avgY.toFixed(2) + " " + unit + ")";
    svg.appendChild(caption);
  }
  
  var yTickCount = 10;
  for ( var i = 1; i<yTickCount;++i) {
    var yTick = document.createElementNS(svgNS,"polyline");
    var yVal = minValY+i*sizeY/yTickCount;
    var y = data2posY(yVal);
    yTick.setAttribute("points","" + border + " " + y + " " + (border+10) + " " + y);
    yTick.setAttribute("fill","none");
    yTick.setAttribute("stroke","black");
    yTick.setAttribute("stroke-width","1");
    svg.appendChild(yTick);
    
    var yTickLong = document.createElementNS(svgNS,"polyline");
    var yVal = minValY+i*sizeY/yTickCount;
    var y = data2posY(yVal);
    yTickLong.setAttribute("points","" + (border+10) + " " + y + " " + (imageX-border) + " " + y);
    yTickLong.setAttribute("fill","none");
    yTickLong.setAttribute("stroke","lightgrey");
    yTickLong.setAttribute("stroke-width","1");
    yTickLong.setAttribute("stroke-dasharray","2, 5");
    svg.appendChild(yTickLong);
    
    var tickCaption = document.createElementNS(svgNS,"text");
    tickCaption.setAttribute("x",5+border);
    tickCaption.setAttribute("y",y-3);
    tickCaption.setAttribute("font-size",10);
    tickCaption.setAttribute("fill","blue");
    tickCaption.setAttribute("text-anchor", "left");
    tickCaption.textContent = yVal.toFixed(2) + " " + unit;
    svg.appendChild(tickCaption);
  }
  
  var ruler = document.createElementNS(svgNS,"polyline");
  ruler.setAttribute("points", border + " " + border + " " + border + " " + (imageY-border) + " " + (imageX-border) + " " + (imageY-border));
  ruler.setAttribute("fill","none");
  ruler.setAttribute("stroke","black");
  ruler.setAttribute("stroke-width","2");
  
  svg.appendChild(ruler);
  

  var captionMax = document.createElementNS(svgNS,"text");
  captionMax.setAttribute("x",5+border);
  captionMax.setAttribute("y",15);
  captionMax.setAttribute("fill","blue");
  captionMax.setAttribute("text-anchor", "left");
  captionMax.textContent = maxValY.toFixed(2) + " " + unit;
  svg.appendChild(captionMax);
  
  var captionMin = document.createElementNS(svgNS,"text");
  captionMin.setAttribute("x",5+border);
  captionMin.setAttribute("y",imageY-(5+border));
  captionMin.setAttribute("fill","blue");
  captionMin.setAttribute("text-anchor", "left");
  captionMin.textContent = minValY.toFixed(2) + " " + unit;
  svg.appendChild(captionMin);
}
