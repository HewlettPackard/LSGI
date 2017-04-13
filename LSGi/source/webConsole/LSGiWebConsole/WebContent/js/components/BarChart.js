/**
 * @class BarChart
 * @param selector to find DOM element
 * @param height integer
 *
 * @reference http://bl.ocks.org/mbostock/3885304
 * @reference https://bl.ocks.org/RandomEtc/cff3610e7dd47bef2d01
 * @author Romina Espinosa
 * @date june 9, 2016
 */
 function BarChart(selector, height ) {

   // PROPERTIES
   // private properties
   var chart= this;
   var hashMap = {};
   var margin = {top: 20, bottom: 30}; // px
   var axisWidth = 70; // width for x-axis
   var spaceProportion = 5; // let x% for bar's spacing

   // public properties
   chart.svg = d3.select(selector).append("div")
       .attr("width", "100%")
       .attr("height", height)
       .style("display", "flex");

    chart.lSvg = chart.svg.append("svg")
      .attr("width", axisWidth+"px")
      .attr("height", height)
      .append("g")
      .attr("transform", "translate(0,"+margin.top+")");

    chart.rSvg = chart.svg.append("svg")
      .attr("width", "100%")
      .attr("height", height)
      .append("g")
      .attr("transform", "translate(0,"+margin.top+")");

     height-= (margin.top + margin.bottom);

     // x axis
     chart.x = d3.scale.linear()
            .range([0, 100]);

     chart.xAxis = d3.svg.axis()
         .scale(chart.x)
         .orient("bottom")
         .ticks(10,"d") // max elements in axis

     chart.rSvg.append("g")
         .attr("class", "x axis")
         .attr("transform", "translate(0," + height + ")");

     // y axis
    chart.y = d3.scale.linear()
        .range([height, 0]);

    chart.yAxis = d3.svg.axis()
        .scale(chart.y)
        .orient("left")
        .ticks(10);

    chart.lSvg.append("g")
     .attr("class", "y axis")

     // METHODS
     // private methods
     var put = function ( x, y ) {
       // x
       var xkey = Object.keys(x)[0];
       var xval = x[xkey];

       var ykey = Object.keys(y)[0];
       var yval = y[ykey];

       hashMap["i-"+xval] = {
         x:{ key:xkey , val:xval } ,
         y:{ key:ykey , val:yval }
       };

       return dataArray();
     }

     var dataArray = function () {
       var data = Object.keys(hashMap).map(function(key){ return hashMap[key] });
       //data.shift();
       return data;
     }

     // public methods
    chart.clear= function(){
    	hashMap = {};// empty map
        chart.svg.selectAll(".bar").remove();
    }

    /**
     * @method update
     * @param x { key: number }
     * @param y { key: number }
     * {x,y} are pushed to local data array
     */
    chart.update= function( x , y ) {
      var data = put(x,y);

      var len = data.length;
      var a = data[0].x.val;
      var b = data[len-1].x.val;
      var a_b = (b-a)+1; // real length (from a to b)

      var xShift= 100/a_b;
      var xWidth= (100-spaceProportion)/a_b;

      // x axis
      //chart.x.domain([a,b]);      
      if(len <= 1)
    	  chart.x.domain([0,b]);//show tick when only one bar
      else
          chart.x.domain([a,b]);

      var xAxisCall= chart.rSvg.select('.x.axis').transition().duration(300).call(chart.xAxis);
      xAxisCall.selectAll(".tick")
       .attr("transform", "translate(0,0)")
       .style("display", function(d) { return d%1 ? "none" : "inherit" });

       xAxisCall.selectAll("text")
           .attr("x",function(val){ return (xShift* (val - a) )+(xShift/2)+"%"});
       xAxisCall.selectAll("line")
            .attr("x1",function(val){return (xShift* (val-a) )+(xShift/2)+"%"})
            .attr("x2",function(val){return (xShift* (val-a) )+(xShift/2)+"%"});

       // y axis
       chart.y.domain([0,Math.ceil(d3.max(data,function(d){ return d.y.val }))]);
       var yAxisCall= chart.lSvg.select(".y.axis").transition().duration(300).call(chart.yAxis);

       yAxisCall.selectAll("text")
            .attr("x","85%");
       yAxisCall.selectAll("line")
            .attr("x1","90%")
            .attr("x2","100%");
       yAxisCall.selectAll("path")
            .attr("transform", "translate("+ (axisWidth-5) + ",0)");

      // bars
      var bars = chart.rSvg.selectAll(".bar")
          .data(data);

       bars.enter().append("rect")
        .attr("class", "bar")
        .attr("y", chart.y(0))
        .attr("height", height-chart.y(0) )
        .append("title")
          .text(function(d){
            return d.x.key + ": " + d.x.val + "\n" +
                    d.y.key + ": " + d.y.val.toFixed(1);
          });

      bars.transition().duration(300)
        .attr("x", function(d) { return ((xShift||1)*(d.x.val - a )) + "%"})
        .attr("width", (xWidth||1) + "%")
        .attr("y", function(d) { return chart.y( d.y.val ); })
        .attr("height", function(d) { return height-chart.y( d.y.val ); });
    }
}
