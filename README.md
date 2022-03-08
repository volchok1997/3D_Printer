# 3D Printer

Introduction
------------
This project is intended for use with the Stony Brook University's 3D printer in the Light Engineering building. It is designed to work with the hardware in Dr. Shanshan Yao's research lab. The purpose of this project is to integrate a dispenser with a 3D printing stage to expand its material compatibilities.  

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
|       | 19200 | 38400 |
|-------|-------|-------|
| Start | 18ms  | 13ms  |
| Stop  | 14ms  | 8ms   |




