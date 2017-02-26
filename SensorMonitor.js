'use strict';

var http = require('http');
var https = require('https');
var url = require('url');
var querystring = require('querystring');
var fs = require('fs');
var websocket = require('websocket');		// don't forget to run "npm install websocket"

function createHTMLErrorResponse( res, code, message )
{
	res.writeHead(code, {"Content-Type:": "text/html"});
	res.write(
		'<!DOCTYPE html>'+
		'<html>'+
		'    <head>'+
		'        <meta charset="utf-8" />'+
		'        <title>Error</title>'+
		'    </head>'+ 
		'    <body>'+
		'     	<p>' + message + '</p>'+
		'    </body>'+
		'</html>');
	res.end();
}

function serveFile( filename, res )
{
	console.log("Serving file: " + filename);
	fs.readFile(filename, 'utf8', 
		function(err, data) 
			{
		  		if ( err ) 
		  		{
		    		createHTMLErrorResponse( res, 500, err );
		  		}
		  		else
		  		{
		  			res.writeHead(200); //{"Content-Type:": "application/json"});	// The server should certainly provide content type based on file extension
					res.write(data);
					res.end();
				}
			});
}

/*
	SensorReader
*/
function SensorReader()
{
	this._interval = setInterval( this._onReceivedData.bind(this), 20 );
	this._onSensorDataReady = [];
}

SensorReader.prototype.dispose = function()
{
	clearInterval( this._interval );
};

SensorReader.prototype._onReceivedData = function()
{
	var now = new Date().getTime();
	var t0 = (now % 4000) / 4000;
	var t1 = (now % 1000) / 1000;
	var t2 = (now % 333) / 333;
	var t4 = (now % 1642) / 1642;
	
	var temperature = (t0*30 - 15) + 50  + Math.sin( t2 * Math.PI * 2) * 2;

	var angularSpeedX = 
		Math.sin( t0 * Math.PI * 2) * 15 + 
		Math.sin( t1 * Math.PI * 2) * (5*(Math.sin( t4 * Math.PI * 2)+1)) +
		Math.sin( t2 * Math.PI * 2) * 2 + 
		50;
	
	var dataPoint = {
		temperature: temperature,
		angularSpeedX: angularSpeedX,
		timestamp: new Date().getTime()
	};

	//console.log("SensorReader: read point " + dataPoint.timestamp);

	for ( var i=0; i<this._onSensorDataReady.length; ++i )
	{
		var listener = this._onSensorDataReady[i];
		listener( dataPoint );
	}
};

/*
	SensorDataSender
*/
function SensorDataSender( sensorReader, connection )
{
	this._sensorReader = sensorReader;
	this._connection = connection;

	this._interval = setInterval( this._sendData.bind(this), 50 );

	this._maxNumDataPointsToSend = 2000;
	this._dataPointsToSend = [];	

	this._onSensorDataReadyHandler = this._onSensorDataReady.bind(this);
	this._sensorReader._onSensorDataReady.push( this._onSensorDataReadyHandler );

	this._instanceID = SensorDataSender.numInstances;
	SensorDataSender.numInstances++;

	console.log("SensorDataSender#"+ this._instanceID + ": created");
}
SensorDataSender.numInstances = 0;

SensorDataSender.prototype.dispose = function()
{
	clearInterval( this._interval );

	var index = this._sensorReader._onSensorDataReady.indexOf( this._onSensorDataReadyHandler );
	if (index!==-1)
	{
		this._sensorReader._onSensorDataReady.splice(index, 1);
	}
	else
	{
		console.error("something's wrong");
	}

	console.log("SensorDataSender#"+ this._instanceID + ": dispose");
};

SensorDataSender.prototype._onSensorDataReady = function( dataPoint )
{
	this._dataPointsToSend.splice(0, 0, dataPoint);
	if ( this._dataPointsToSend.length>this._maxNumDataPointsToSend )
	{
		this._dataPointsToSend.shift();
		console.warn("buffer full");
	}
};
 
SensorDataSender.prototype._sendData = function()
{
	//console.log("SensorDataSender#"+ this._instanceID + ": sending " + this._dataPointsToSend.length);
	var jsonData = JSON.stringify(this._dataPointsToSend);
	this._connection.sendUTF(jsonData);
	this._dataPointsToSend = [];
};


/*
	Main
*/
function Main()
{
	var httpServer = http.createServer( 
		function(req, res)
		{
			var path = url.parse(req.url).pathname;
			var query = url.parse(req.url).query; 

			if ( path==='/') 
			{
				serveFile( 'SensorMonitor.html', res );
			}
			else if ( path.indexOf('/files/')===0 )
			{
				var filename = path.substr(1);
				serveFile( filename, res );
			}
			else
			{
				createHTMLErrorResponse( res, 404, "Page not found");
			}
		});
	httpServer.listen(8080);

	var websocketServer = new websocket.server({
		httpServer:httpServer
	});

	var sensorReader = new SensorReader();
	var sensorDataSenders = [];

	websocketServer.on('request', 
		function(request) 
		{
    		var connection = request.accept(null, request.origin);

    		//console.log("adding sender..." );
    		var sensorDataSender = new SensorDataSender( sensorReader, connection );
    		sensorDataSenders.push( sensorDataSender );

    		console.log("SensorMonitor: " + sensorDataSenders.length + " connections in progress");

    		connection.on('close', 
    			function(connection) 
    			{
					//console.log('Websocket server connection closed' + connection);
					sensorDataSender.dispose();
					var index = sensorDataSenders.indexOf(sensorDataSender);
					if (index!==-1) 
					{
					    sensorDataSenders.splice(index, 1);
					}
					else
					{
						console.warn("couldn't find sender for connection...");
					}
					
					console.log("SensorMonitor: " + sensorDataSenders.length + " connections in progress");
   				});
		}); 

	console.log("SensorMonitor: server started");
}

Main();

