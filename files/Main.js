'use strict';

function Main()
{
	var canvas = document.getElementById('graphCanvas');
	canvas.focus();

	var sensorMonitor = new SensorMonitor(canvas);

	// Data types
	var dataTypes = Object.keys(sensorMonitor._graphDataWindows);
	var dataTypeSelect = document.getElementById('dataTypeSelect');
	for ( var i=0; i<dataTypes.length;i++ )
	{
		var dataType = dataTypes[i];
		var option = new Option( dataType, dataType );
		dataTypeSelect.options[i] = option;
	}
	dataTypeSelect.addEventListener('change', 
		function(event)
		{
			var graphDataType = event.target.value;
			sensorMonitor.setGraphDataType(graphDataType);
		});
	var currentGraphDataType = sensorMonitor.getGraphDataType();
	dataTypeSelect.value = currentGraphDataType;

	// Fullscreen 
	var fullscreenButton = document.getElementById('fullscreenButton');
	if ( screenfull && screenfull.enabled )
	{
		document.addEventListener( screenfull.raw.fullscreenchange, 
			function(event)
			{
				if ( screenfull.isFullscreen )
					fullscreenButton.className = "roundedButtonToggled";
				else
					fullscreenButton.className = "roundedButton";
			});

		fullscreenButton.onclick = function( event ) 
			{
				if ( screenfull && screenfull.enabled )
				{
					var mainDiv = document.getElementById('mainDiv');
					screenfull.toggle( mainDiv );
				}
			};
	}
	else
	{
		fullscreenButton.style.display = 'none';
	}
  
	// Autoscroll 
	var autoscrollButton = document.getElementById('autoscrollButton');
	autoscrollButton.onclick = function( event ) 
		 {   
			 var autoscroll = !sensorMonitor.getAutoscroll();
			 sensorMonitor.setAutoscroll(autoscroll);
		 };
	sensorMonitor._onAutoscrollChanged = function()
		 {
			 var autoscroll = sensorMonitor.getAutoscroll();
			 if ( autoscroll )
				 autoscrollButton.className = "roundedButtonToggled";
			 else
				 autoscrollButton.className = "roundedButton";
		 };
	sensorMonitor._onAutoscrollChanged();

	// Connection indicator
	var connectionIndicator = document.getElementById('connectionIndicator');
	sensorMonitor._onConnectionOpen = function() 
		{
			connectionIndicator.style.backgroundColor = 'green';
		};
	sensorMonitor._onConnectionError = function() 
		{
			connectionIndicator.style.backgroundColor = 'red';
		};
	sensorMonitor._onConnectionClose = function() 
		{
			connectionIndicator.style.backgroundColor = 'grey';
		};

	// Graph data type buttons
/*	var graphDataTypeButtons = {
		'accelerationX' : document.getElementById('accelerationXButton'),
		'accelerationY' : document.getElementById('accelerationYButton'),
		'accelerationZ' : document.getElementById('accelerationZButton'),
		'angularSpeedX' : document.getElementById('angularSpeedXButton'),
		'angularSpeedY' : document.getElementById('angularSpeedYButton'),
		'angularSpeedZ' : document.getElementById('angularSpeedZButton'),
		'temperature' : document.getElementById('temperatureButton'),

		'magneticHeadingX' : document.getElementById('magneticHeadingXButton'),
		'magneticHeadingY' : document.getElementById('magneticHeadingYButton'),
		'magneticHeadingZ' : document.getElementById('magneticHeadingZButton'),

		'temperature2' : document.getElementById('temperature2Button'),
		'pressure' : document.getElementById('pressureButton'),
	};
	for ( var graphDataType in graphDataTypeButtons )
	{
		var button = graphDataTypeButtons[graphDataType];
		button.graphDataType = graphDataType;
		button.onclick = function( event ) 
			{	
				var graphDataType = event.target.graphDataType;
				sensorMonitor.setGraphDataType(graphDataType);
			};
	}
	sensorMonitor._onGraphDataTypeChanged = function( prevGraphDataType, graphDataType )
		{
			var prevButton = graphDataTypeButtons[prevGraphDataType];
			prevButton.className = "roundedButton";
			var button = graphDataTypeButtons[graphDataType];
			button.className = "roundedButtonToggled";
		};
	var currentGraphDataType = sensorMonitor.getGraphDataType();
	graphDataTypeButtons[currentGraphDataType].className = "roundedButtonToggled";*/
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
