# SensorMonitor

- clone repo
- for the C++ bit, go into cpp directory
```Bash
  mkdir _build_
  cd _build_/
  cmake .. -DCMAKE_INSTALL_PREFIX=../_output_
  make
```
- for the Node.js bit, go to main directory
```Bash
  mkdir _build_
  npm install websocket 
```
- running type this, then go to http://192.168.1.15:8080
```Bash
  node SensorMonitorServer.js
```
