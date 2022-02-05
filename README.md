# 3D_Printer
Integrate Sonoplot Microplotter II stage with Musashi dispenser

## Environment setup
``` 
git clone https://github.com/renaissanxe/3D_Printer.git
git submodule init
git submodule update
```

### Work in WSL2
USB device has to be mapped from windows to WSL.  
Get windows package usbipd at Powershell (https://github.com/dorssel/usbipd-win)  
```
winget install --interactive --exact dorssel.usbipd-win
```
Get hwdata at WSL2 
```
sudo apt install linux-tools-5.4.0-77-generic hwdata
```
Get usbid on host machine:
```
usbipd wsl list
```
Attach usb device to wsl
```
usbipd wsl attach --busid <busid>
```
Opne WSL 2 instance and run lsusb:
```
lsusb
```
After finished, detach on wsl
```
usbipd wsl detach --busid <busid>
```

## Compile
under ./src, run 
```
make
```

