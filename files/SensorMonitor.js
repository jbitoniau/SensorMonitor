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
   this._graphData.splice(0, 0, dataPoint);

    GraphDataPresenter.render( this._canvas, this._graphData, this._graphDataWindow, this._graphOptions );
};

SensorMonitor.prototype._onSocketError = function(error)
{
    alert('WebSocket error: ' + error);
};
