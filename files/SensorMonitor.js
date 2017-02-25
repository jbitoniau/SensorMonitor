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

     // When the user navigates in the graph (i.e. changes the graph data window), we need to check whether more data needs to be fetched
    this._graphController._onGraphDataWindowChange = this._onGraphDataWindowChange.bind(this);
  
    this._isConnected = false;
    var host = window.location.host;
    this._socket = new WebSocket('ws://' + host);

    this._onSocketOpenHandler = this._onSocketOpen.bind(this);
    this._socket.addEventListener( "open", this._onSocketOpenHandler );
    
    this._onSocketMessageHandler = this._onSocketMessage.bind(this);
    this._socket.addEventListener( "message", this._onSocketMessageHandler );

    this._onSocketErrorHandler = this._onSocketError.bind(this);
    this._socket.addEventListener( "error", this._onSocketErrorHandler );
    
    this._onSocketCloseHandler = this._onSocketClose.bind(this);
    this._socket.addEventListener( "close", this._onSocketCloseHandler );
  
    this._onConnectionOpen = null;
    this._onConnectionError = null;
    this._onConnectionClose = null;


    this._autoscroll = true;
    this._onAutoscrollChanged = null;
}

SensorMonitor.prototype.dispose = function()
{
    this._socket.removeEventListener( "open", this._onSocketOpenHandler );
    this._socket.removeEventListener( "message", this._onSocketMessageHandler );
    this._socket.removeEventListener( "error", this._onSocketErrorHandler );
    this._socket.close();
};

SensorMonitor.prototype.getAutoscroll = function()
{
    return this._autoscroll;
};

SensorMonitor.prototype.setAutoscroll = function( autoscroll )
{
    if ( this._autoscroll===autoscroll )
        return;

    this._autoscroll = autoscroll;

    if ( this._autoscroll )
    {
        this._scrollToLatestData();
        this._render();
    }

    if ( this._onAutoscrollChanged )
        this._onAutoscrollChanged();
};

SensorMonitor.prototype._onSocketOpen = function( /*??*/ )
{
    this._isConnected = true;
    //this._socket.send('hello from the client');
    if ( this._onConnectionOpen )
        this._onConnectionOpen();
};

var i=0;
SensorMonitor.prototype._onSocketMessage = function( message )
{

if ( i%10 )
    this._socket.send('number ' + i);
i++;

    var data = JSON.parse( message.data );
    var dataPoint = {x:data.timestamp, y:data.value};
    this._graphData.splice(0, 0, dataPoint);
    
    this._render();
};

SensorMonitor.prototype._onSocketError = function(error)
{
    if ( this._onConnectionError )
        this._onConnectionError();
    alert('WebSocket error: ' + error);
};

SensorMonitor.prototype._onSocketClose = function( /*??*/ )
{
    this._isConnected = false;
    if ( this._onConnectionClose )
        this._onConnectionClose();
};

SensorMonitor.prototype._render = function()
{
    if ( this._autoscroll )
       this._scrollToLatestData();

    GraphDataPresenter.render( this._canvas, this._graphData, this._graphDataWindow, this._graphOptions );
};

SensorMonitor.prototype._onGraphDataWindowChange = function( prevGraphDataWindow )
{
    if ( this.getAutoscroll() )
    {
        if ( this._graphDataWindow.x!==prevGraphDataWindow.x )
        {
            this.setAutoscroll( false );
        }
    }
};

// Change the x position of the graph data window to show the latest data points.
// This method doesn't affect the other graph data window properties.
SensorMonitor.prototype._scrollToLatestData = function()
{
    var graphData = this._graphData;
    if ( graphData.length===0 )
        return;
    var latestDataPoint = graphData[0];
    this._graphDataWindow.x = latestDataPoint.x - this._graphDataWindow.width;
};

