'use strict';

function SensorMonitor( canvas )
{
    this._canvas = canvas;

    var w = 10 * 1000;
    var n = new Date().getTime();
    
    this._graphDataWindow = {
        x: n - (w/2),
        y: -5,
        width: w,
        height: 105
    };

    this._graphData = [
    ];

    this._graphOptions = {
        yPropertyName: "y",
        clearCanvas: true,
        drawOriginAxes: true,
        drawDataRange: true,
        drawDataGaps: true,
        contiguityThreshold: 30,       
        textSize: 12,
        numMaxLinesX: 5,
        numMaxLinesY: 5,
        getPrimaryLinesTextX: GraphDataPresenter.getLinesTextForTime, 
        getPrimaryLinesSpacingX: GraphDataPresenter.getPrimaryLinesSpacingForTime,
        getSecondaryLinesSpacingX: GraphDataPresenter.getSecondaryLinesSpacingForTime,
        getPrimaryLinesTextY: GraphDataPresenter.getLinesText,
        getPrimaryLinesSpacingY: GraphDataPresenter.getLinesSpacing,
        getSecondaryLinesSpacingY: GraphDataPresenter.getSecondaryLinesSpacing,
        points: {
            //typicalDataPointXSpacing: 10*60*1000,     // No need if we provide a contiguityThreshold
            maxPointSize: 5,
            maxNumPoints: 500,
        }
        /*colors: {
            clear:'#FFFFFF',
            dataRange: "#EEEEEE",
            dataGaps: "#EEEEEE",
            axesLines: "#AA6666",
            primaryLinesText: '#AA6666',
            primaryLines: '#FFAAAA',
            secondaryLines: '#FFDDDD',
            dataLine: "#884444",
            dataPoint: "#884444",
        },*/
    };

    // The graph controller is responsible for rendering the graph and handling input events to navigate in it
    this._graphController = new GraphController( this._canvas, this._graphData, this._graphDataWindow, this._graphOptions );

   // GraphDataPresenter.render( canvas, graphData, this._graphDataWindow, graphOptions );

    var host = window.location.host;
    this._socket = new WebSocket('ws://' + host);

    this._onSocketOpenHandler = this._onSocketOpen.bind(this);
    this._socket.addEventListener( "open", this._onSocketOpenHandler );
    
    this._onSocketMessageHandler = this._onSocketMessage.bind(this);
    this._socket.addEventListener( "message", this._onSocketMessageHandler );

    this._onSocketErrorHandler = this._onSocketError.bind(this);
    this._socket.addEventListener( "error", this._onSocketErrorHandler );

   // this._onConnected = null;
   // this._onConnectionError = null;
}

SensorMonitor.prototype.dispose = function()
{
    this._socket.removeEventListener( "open", this._onSocketOpenHandler );
    this._socket.removeEventListener( "message", this._onSocketMessageHandler );
    this._socket.removeEventListener( "error", this._onSocketErrorHandler );
    this._socket.close();
};

SensorMonitor.prototype._onSocketOpen = function()
{
    this._socket.send('hello from the client');
};

SensorMonitor.prototype._onSocketMessage = function( message )
{
    var data = JSON.parse( message.data );
    var dataPoint = {x:data.timestamp, y:data.value};
   //this._graphData.unshift( [dataPoint]  );
   this._graphData.splice(0, 0, dataPoint);
//this._graphData.push( dataPoint );

    GraphDataPresenter.render( this._canvas, this._graphData, this._graphDataWindow, this._graphOptions );
};

SensorMonitor.prototype._onSocketError = function(error)
{
    alert('WebSocket error: ' + error);
};




function Main()
{
    var canvas = document.getElementById('graphCanvas');
    canvas.focus();

    var sensorMinitor = new SensorMonitor(canvas);


/*	var theDiv = document.getElementById('theDiv');

	var host = window.location.host;
	theDiv.innerHTML += host  + '<br/>'
*/

}

/*
	Main

	Start the grapher using the device specified in the URL and the optional starting date string.
	Here is an example URL for device id '2A88E' using 24th of December 2016 as a starting date:
	www.serverxyz.com/devices/2A88E?date=12/24/2016
*/	
// function Main()
// {
// 	var deviceID = window.location.pathname.substr( ('/devices/').length );

// 	var autoscroll = true;
// 	var date = null;
// 	var dateString = getQueryVariable('date');
// 	if ( dateString )
// 	{
// 		var epochTime = Date.parse(dateString);
// 		if ( !isNaN(epochTime) )
// 		{
// 			autoscroll = false;
// 			date = new Date(epochTime);
// 		}
// 		else
// 		{
// 			window.location.search=null;
// 		}
// 	}
// 	var canvas = document.getElementById('graphCanvas');
// 	canvas.focus();
	
// 	var weatherGrapher = new WeatherGrapher( canvas, deviceID, date, autoscroll );

// 	var buttons = {
// 		'temperature' : document.getElementById('temperatureButton'),
// 		'humidity' : document.getElementById('humidityButton'),
// 		'pressure' : document.getElementById('pressureButton')
// 	};

// 	for ( var graphDataType in buttons )
// 	{
// 		var button = buttons[graphDataType];
// 		button.graphDataType = graphDataType;
// 		button.onclick = function( event ) 
// 			{	
// 				var graphDataType = event.target.graphDataType;
// 				weatherGrapher.setGraphDataType(graphDataType);
// 			};
// 	}

// 	buttons[weatherGrapher._graphDataType].className = "roundedButtonToggled";

// 	weatherGrapher._onGraphDataTypeChanged = function( prevGraphDataType, graphDataType )
// 		{
// 			var prevButton = buttons[prevGraphDataType];
// 			prevButton.className = "roundedButton";
			
// 			var button = buttons[graphDataType];
// 			button.className = "roundedButtonToggled";
// 		};


// 	var autoscrollButton = document.getElementById('autoscrollButton');
// 	autoscrollButton.onclick = function( event ) 
// 		{	
// 			var autoscroll = !weatherGrapher.getAutoscroll();
// 			weatherGrapher.setAutoscroll(autoscroll);
// 		};
// 	weatherGrapher._onAutoscrollChanged = function()
// 		{
// 			var autoscroll = weatherGrapher.getAutoscroll();
// 			if ( autoscroll )
// 				autoscrollButton.className = "roundedButtonToggled";
// 			else
// 				autoscrollButton.className = "roundedButton";
// 		};
// 	weatherGrapher._onAutoscrollChanged();


// 	var fetchIndicator = document.getElementById('fetchIndicator');
// 	var fetchIndicatorRemove = function()
// 		{
// 			fetchIndicator.style.backgroundColor = 'transparent';
// 			fetchIndicator.className = null;
// 		};
// 	var fetchIndicatorRemoveTimeout = null;
// 	weatherGrapher._onFetch = function( state )
// 		{
// 			var color = null;
// 			switch ( state )
// 			{
// 				case 'started': 
// 					if ( fetchIndicatorRemoveTimeout ){
// 						clearTimeout(fetchIndicatorRemoveTimeout);
// 						fetchIndicatorRemoveTimeout = null;
// 					}
// 					fetchIndicator.style.backgroundColor = 'grey';
// 					fetchIndicator.className = 'blink';
// 					break;
// 				case 'ok': 
// 					fetchIndicator.style.backgroundColor = 'green';
// 					fetchIndicator.className = null;
// 					fetchIndicatorRemoveTimeout = setTimeout( fetchIndicatorRemove, 1000 );
// 					break;
// 				case 'error': 
// 					fetchIndicator.style.backgroundColor = 'red';
// 					fetchIndicator.className = null;
// 					fetchIndicatorRemoveTimeout = setTimeout( fetchIndicatorRemove, 3000 );
// 					break;
// 			}
// 		};
// }

// // https://davidwalsh.name/query-string-javascript (in comment section)
// function getQueryVariable(variable) 
// {
// 	var query = window.location.search.substring(1);
// 	var vars = query.split("&");
// 	for (var i=0;i<vars.length;i++) 
// 	{
// 		var pair = vars[i].split("=");
// 		if(pair[0] == variable)
// 		{
// 			return pair[1];
// 		}
// 	}
// 	return(false);
// }
