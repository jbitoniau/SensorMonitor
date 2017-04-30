'use strict';

var http = require('http');
var https = require('https');
var url = require('url');
//var querystring = require('querystring');
var fs = require('fs');
var websocket = require('websocket');		// don't forget to run "npm install websocket"

var dgram = require('dgram');

/*
	SensorReaderSimulated

	A SensorReader is capable for reading data from a bunch on sensors all at once
	on a regular basis (basically as these data point arrive).

	Whenever this happens, the SensorReader notifies any listener of the new piece of data.
	It doesn't store/remember any data read, this is the responsibility of the code using it.
*/
function SensorReaderSimulated()
{
	this._interval = setInterval( this._onReceivedData.bind(this), 20 );
	this._onSensorDataReadyListeners = [];
}

SensorReaderSimulated.prototype.dispose = function()
{
	clearInterval( this._interval );
};

SensorReaderSimulated.prototype._onReceivedData = function()
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
		accelerationX: 10,
		accelerationY: 20,
		accelerationZ: 30,
		angularSpeedX: angularSpeedX,
		angularSpeedY: 15,
		angularSpeedZ: 25,
		temperature: temperature,
		timestamp: new Date().getTime()
	};

	//console.log("SensorReader: read point " + dataPoint.timestamp);

	for ( var i=0; i<this._onSensorDataReadyListeners.length; ++i )
	{
		var listener = this._onSensorDataReadyListeners[i];
		listener( dataPoint );
	}
};

/*
	SensorReaderUDP
*/
function SensorReaderUDP( udpSocket )
{
	this._udpSocket = udpSocket;
	this._onSensorDataReadyListeners = [];

	this._onUDPSocketMessageHandler = this._onUDPSocketMessage.bind(this);
	this._udpSocket.on('message', this._onUDPSocketMessageHandler );
}

SensorReaderUDP.prototype.dispose = function()
{
	console.log("DISPOSE SENSORREADERUDP!");
	this._udpSocket.removeListener('message', this._onUDPSocketMessageHandler );
};

SensorReaderUDP.prototype._onUDPSocketMessage = function(message, remote)
{
	var text = 'UDP socket received message on ' + remote.address + ':' + remote.port; 
	var uint8Array = new Uint8Array( message );
	var messageAsHex = "";
	for ( var i=0; i<uint8Array.length /* same as remote.size */; i++ )
		messageAsHex += uint8Array[i].toString(16) + " ";
	text += ' - ' + messageAsHex;
	//console.log(text);
	
	var dataPoint = {
		accelerationX: 0,
		accelerationY: 0,
		accelerationZ: 0,
		angularSpeedX: 0,
		angularSpeedY: 0,
		angularSpeedZ: 0,
		temperature: 0,
		timestamp: new Date().getTime()
	};

	var dataView = new DataView(uint8Array.buffer);
	var offset = 0;
	dataPoint.accelerationX = dataView.getFloat64(offset, true);  offset+=8;
	dataPoint.accelerationY = dataView.getFloat64(offset, true);  offset+=8;
	dataPoint.accelerationZ = dataView.getFloat64(offset, true);  offset+=8;
	dataPoint.angularSpeedX = dataView.getFloat64(offset, true);  offset+=8;
	dataPoint.angularSpeedY = dataView.getFloat64(offset, true);  offset+=8;
	dataPoint.angularSpeedZ = dataView.getFloat64(offset, true);  offset+=8;
	dataPoint.temperature = dataView.getFloat32(offset, true);	  offset+=4;

	for ( var i=0; i<this._onSensorDataReadyListeners.length; ++i )
	{
		var listener = this._onSensorDataReadyListeners[i];
		listener( dataPoint );
	}
};
/*
SensorReaderUDP.prototype._onReceivedData = function()
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

	for ( var i=0; i<this._onSensorDataReadyListeners.length; ++i )
	{
		var listener = this._onSensorDataReadyListeners[i];
		listener( dataPoint );
	}
};*/

/*
	SensorDataSender

	A SensorDataSender is pairs a SensorReader to a websocket connection.

	Whenever a data point is ready from the SensorReader, this object stores it. 
	Then on a regular basisc, it sends the data points accumulated on the 
	websocket. This basically data points to be send as a group and not individually
	which would be costly.
*/
function SensorDataSender( sensorReader, connection )
{
	this._sensorReader = sensorReader;
	this._connection = connection;

	this._interval = setInterval( this._sendData.bind(this), 50 );

	this._maxNumDataPointsToSend = 2000;
	this._dataPointsToSend = [];	

	this._onSensorDataReadyHandler = this._onSensorDataReadyListeners.bind(this);
	this._sensorReader._onSensorDataReadyListeners.push( this._onSensorDataReadyHandler );

	this._instanceID = SensorDataSender.numInstances;		// Just for debug
	SensorDataSender.numInstances++;

	console.log("SensorDataSender#"+ this._instanceID + ": created");
}
SensorDataSender.numInstances = 0;

SensorDataSender.prototype.dispose = function()
{
	clearInterval( this._interval );

	var index = this._sensorReader._onSensorDataReadyListeners.indexOf( this._onSensorDataReadyHandler );
	if (index!==-1)
	{
		this._sensorReader._onSensorDataReadyListeners.splice(index, 1);
	}
	else
	{
		console.error("something's wrong");
	}

	console.log("SensorDataSender#"+ this._instanceID + ": dispose");
};

SensorDataSender.prototype._onSensorDataReadyListeners = function( dataPoint )
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
	SensorMonitorServer
*/
function SensorMonitorServer()
{
	// Create UDP socket for discussing with the C++ SensorSender companion program
	var udpSocket = dgram.createSocket('udp4');
	udpSocket.on('listening', 
		function() 
		{
		    var address = udpSocket.address();
		    console.log('UDP Server listening on ' + address.address + ":" + address.port);
		});
	udpSocket.bind(8181, '127.0.0.1');

	// Create SensorReaderUDP working on the UDP socket 
	this._sensorReader = new SensorReaderUDP(udpSocket);
	
	// Prepare SensorDataSenders array for incoming websocket connections
	this._sensorDataSenders = [];

	// Create HTTP server
	this._httpServer = http.createServer( 
		function(req, res)
		{
			var path = url.parse(req.url).pathname;
			var query = url.parse(req.url).query; 

			if ( path==='/') 
			{
				SensorMonitorServer._serveFile( 'SensorMonitor.html', res );
			}
			else if ( path.indexOf('/files/')===0 )
			{
				var filename = path.substr(1);
				SensorMonitorServer._serveFile( filename, res );
			}
			else
			{
				SensorMonitorServer._createHTMLErrorResponse( res, 404, "Page not found");
			}
		});
	this._httpServer.listen(8080, 
		function() 
		{
    		console.log('HTTP server listening');
		});

	// Create Websocket server
	this._websocketServer = new websocket.server({
		httpServer: this._httpServer
	});
	this._websocketServer.on('request', 
		function(request) 
		{
    		var connection = request.accept(null, request.origin);

    		var sensorDataSender = new SensorDataSender( this._sensorReader, connection );
    		this._sensorDataSenders.push( sensorDataSender );

    		console.log("SensorMonitor: " + this._sensorDataSenders.length + " connections in progress");

    		connection.on('close', 
    			function(connection) 
    			{
					//console.log('Websocket server connection closed' + connection);
					sensorDataSender.dispose();
					var index = this._sensorDataSenders.indexOf(sensorDataSender);
					if (index!==-1) 
					{
					    this._sensorDataSenders.splice(index, 1);
					}
					else
					{
						console.warn("couldn't find sender for connection...");
					}
					
					console.log("SensorMonitor: " + this._sensorDataSenders.length + " connections in progress");
   				}.bind(this));
		}.bind(this)); 
	console.log("Websocket server created");
}

SensorMonitorServer.prototype.dispose = function()
{
	// Stop things here!
	console.log("dispose to implement here...")
};

SensorMonitorServer._serveFile = function( filename, res )
{
	console.log("Serving file: " + filename);
	fs.readFile(filename, 'utf8', 
		function(err, data) 
			{
		  		if ( err ) 
		  		{
		    		SensorMonitorServer._createHTMLErrorResponse( res, 500, err );
		  		}
		  		else
		  		{
		  			res.writeHead(200); //{"Content-Type:": "application/json"});	// The server should certainly provide content type based on file extension
					res.write(data);
					res.end();
				}
			});
};

SensorMonitorServer._createHTMLErrorResponse = function( res, code, message )
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
};

/*
	Main
*/
function Main()
{
	var sensorMonitorServer = new SensorMonitorServer();

	//http://stackoverflow.com/questions/10021373/what-is-the-windows-equivalent-of-process-onsigint-in-node-js/14861513#14861513
	//http://stackoverflow.com/questions/6958780/quitting-node-js-gracefully
	if (process.platform === "win32") 
	{
		var rl = require("readline").createInterface(
			{
				input: process.stdin,
				output: process.stdout
			});
		rl.on("SIGINT", function () 
			{
				process.emit("SIGINT");
			});
	}
	process.on("SIGINT", function () 
		{
			console.log("Stopping server...");
			sensorMonitorServer.dispose();
			process.exit();
		});
}

Main();

