'use strict';

function SensorMonitor( canvas )
{
    this._canvas = canvas;

    var now = new Date().getTime();
    var initialWidth = 10 * 1000;
    var initialX = now - (initialWidth/2);
   
    this._graphDataWindow = {
        x: initialX,
        y: -5,
        width: initialWidth,
        height: 40
    };

    // The backed-up graph data windows for each type of data
    this._graphDataWindows = {
        'accelerationX' : {
            x: initialX,    
            y: -4,
            width: initialWidth,
            height: 8
        },
        'accelerationY' : {
            x: initialX,    
            y: -4,
            width: initialWidth,
            height: 8
        },
        'accelerationZ' : {
            x: initialX,    
            y: -4,
            width: initialWidth,
            height: 8
        },

        'angularSpeedX' : {
            x: initialX,    
            y: -1000,
            width: initialWidth,
            height: 2000
        },
        'angularSpeedY' : {
            x: initialX,    
            y: -1000,
            width: initialWidth,
            height: 2000
        },
        'angularSpeedZ' : {
            x: initialX,    
            y: -1000,
            width: initialWidth,
            height: 2000
        },
        'temperature' : {
            x: initialX,
            y: -5,
            width: initialWidth,
            height: 40
        },
    };

    this._graphData = [];

    this._graphOptions = {
        yPropertyName: "angularSpeedX",
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

    // The type of graph data currently being displayed
    this._graphDataType = 'temperature';                            // JBM: this could go away, and the source of truth could be the graphOptions.yPropertyName!
    this._graphOptions.yPropertyName = this._graphDataType;
  
    this._onGraphDataTypeChanged = null;
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

SensorMonitor.prototype.getGraphDataType = function()
{
    return this._graphDataType;
};

SensorMonitor.prototype.setGraphDataType = function( graphDataType )
{
    if ( graphDataType===this._graphDataType )
        return;

    // Remember current graph data y/height for current data type 
    this._graphDataWindows[this._graphDataType].y = this._graphDataWindow.y;
    this._graphDataWindows[this._graphDataType].height = this._graphDataWindow.height;

    // Change data type
    var prevGraphDataType = this._graphDataType;
    this._graphDataType = graphDataType;
    this._graphOptions.yPropertyName = this._graphDataType;

    // Restore graph data y/height for new current type of data to display
    this._graphDataWindow.y = this._graphDataWindows[this._graphDataType].y;
    this._graphDataWindow.height = this._graphDataWindows[this._graphDataType].height;

    // Render graph
    this._graphController.render();

    // Notify
    if ( this._onGraphDataTypeChanged )
        this._onGraphDataTypeChanged( prevGraphDataType, this._graphDataType );
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

    var dataPoints = JSON.parse( message.data );
    for ( var i=dataPoints.length-1; i>=0; i-- )
    {
        var dataPoint = dataPoints[i];
        this._graphData.splice(0, 0, {                      // use directly the dataPoint... need xPropertyName though for rendering...
            x: dataPoint.timestamp, 
            accelerationX: dataPoint.accelerationX,
            accelerationY: dataPoint.accelerationY,
            accelerationZ: dataPoint.accelerationZ,
            angularSpeedX: dataPoint.angularSpeedX,
            angularSpeedY: dataPoint.angularSpeedY,
            angularSpeedZ: dataPoint.angularSpeedZ,
            temperature: dataPoint.temperature,
            
        });
    }
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

