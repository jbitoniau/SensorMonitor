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
/*
function forwardApiCall( path, query, res )
{
	var options = {
		hostname: 'backend.sigfox.com',
		path: path + '?' + query,
		auth: sigfoxBackendAuth
	};

	console.log("Forwarding Sigfox REST api call: " + options.hostname + options.path );
	
	var callback = function(response) 
		{
			//console.log('statusCode:', response.statusCode);
			//console.log('headers:', response.headers);
			var str = '';
			response.on('data', function (chunk) 
				{
					str += chunk;
				});	

			response.on('end', 
				function() 
				{
					res.writeHead(200, {"Content-Type:": "application/json"});
					res.write(str);
					res.end();
				});
		};

	var req = https.request(options, callback);

	req.on('error', 
		function(err) 
		{
			console.log("ERROR: " + err);
			createHTMLErrorResponse( res, 500, err );
		});

	req.end();
}*/

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

function SensorReader()
{
	this._max = 5;
	this._data = [];

	this._intervalHandler = this._fetchSensorData.bind(this);
	this._interval = setInterval( this._intervalHandler, 1000 );
}

SensorReader.prototype.dispose = function()
{
	clearInterval( this._interval );
}

SensorReader.prototype._fetchSensorData = function()
{
	var data = {
		value: Math.random() * 100,
		timestamp: new Date().getTime()
	};
	this._data.push( data );

	if ( this._data.length>this._max )
	{
		this._data.shift();
	}

/*	console.log("----");
	for ( var i=0; i<this._data.length; i++ )
	{
		console.log( this._data[i].timestamp );
	}
*/
};

function SensorDataSender( connection )
{
	this._connection = connection;
	this._intervalHandler = this._send.bind(this);
	this._interval = setInterval( this._intervalHandler, 20 );
}

//var last = 0;

SensorDataSender.prototype._send = function()
{
	var now = new Date().getTime();
	var t0 = (now % 4000) / 4000;
	var t1 = (now % 1000) / 1000;
	var t2 = (now % 333) / 333;
	
	var value = 
		Math.sin( t0 * Math.PI * 2) * 10 + 
		Math.sin( t1 * Math.PI * 2) * 10 + 
		Math.sin( t2 * Math.PI * 2) * 10 + 
		50;

	var data = {
		value: value,
		timestamp: new Date().getTime()
	};

	var jsonData = JSON.stringify(data);
	this._connection.sendUTF(jsonData);
/*
var delta = data.timestamp - last;
console.log("delta:" + delta );
last = data.timestamp;
*/
};

function Main()
{
//	var sensorReader = new SensorReader();


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

	// Inspired from http://hawkee.com/snippet/16051/
	// See ref https://github.com/theturtle32/WebSocket-Node/blob/master/docs/index.md
	var websocketServer = new websocket.server({
		httpServer:httpServer
	});


	var sensorDataSenders = [];

	websocketServer.on('request', 
		function(request) 
		{
    		var connection = request.accept(null, request.origin);

    		console.log("adding sender..." );
    		var sensorDataSender = new SensorDataSender( connection );
    		sensorDataSenders.push( sensorDataSender );

    		console.log("that's now " + sensorDataSenders.length + " senders...");

			/*connection.on('message', 
				function(message) 
				{
				    console.log('Websocket server received from ' + connection.remoteAddress + ': ' + message.utf8Data);
				    
				    connection.sendUTF('hello from server');
				    
				    setTimeout(
				    	function() 
				    	{
				        	connection.sendUTF('this is a websocket example sent from server');
				    	}, 
				    	1000);
			});*/

    		connection.on('close', 
    			function(connection) 
    			{
					console.log('Websocket server connection closed' + connection);

					var index = sensorDataSenders.indexOf(sensorDataSender);
					if (index!==-1) 
					{
					    sensorDataSenders.splice(index, 1);
					}
					else
					{
						console.warn("couldn't find sender for connection...");
					}

   				});
		}); 

	console.log("SensorMonitor server started");
}

Main();

