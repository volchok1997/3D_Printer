# 3D Printer
Integrate Sonoplot Microplotter II stage with Musashi dispenser

Introduction
------------

Environment
-----------
* Sonoplot Microplotter II
* Musashi ML808GX
* Raspberry Pi 4b

Build
-----
``` 
$ git clone https://github.com/renaissanxe/3D_Printer.git
$ cd 3D_Printer
$ git submodule init
$ git submodule update
$ cd src
$ make
```

Usage
---
Update serial port config, PWM input pin at config.ini
```
# ./dispenserController
```

Control Latency
---------------






