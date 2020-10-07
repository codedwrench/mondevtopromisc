# mondevtopromisc
Emulates a promiscuous mode wifi card using monitor mode and packet injection and sends the data over XLink Kai.

As of right now it is a working proof of concept where you can receive and send PSP traffic succesfully through XLink Kai with a few restrictions. Namely it only works on channel 1, and you will have to find the BSSID the PSP is sending on yourself and use it in the command line when starting this program.

## Wifi cards this has been tested and found working on (Linux, kernel 5.x):
- Azurewave AR5BHB92 (Atheros 9280 (ath9k))
- Intel Pro Wireless 5100 AGN (iwlwifi)
- Intel Centrino Wireless-N 1000 (iwlwifi)
- WTXUP Ralink RT3070 (rt2800usb)
- TP-Link TL-WN823N V2 (Realtek rtl8192eu (kimocoder/rtl8192eu))
- Dell Wireless 1390 (Broadcom BCM4311 (b43)) with the following patch: https://gist.github.com/codedwrench/8d7916d63993574e1dd089a62dd523a9
- Hercules Guillemot HWGUm-54 (rtl8192su (codedwrench/rtl8192su))

## How to compile

## Debian Testing and above
This program has only been tested on Debian Testing and above.
It requires the following packages to be installed:
- cmake
- gcc-10
- libboost-dev (version 1.71 or above)
- libboost-thread-dev
- libboost-program-options-dev
- libpcap-dev
- libpthread-stubs0-dev

If those packages are installed, you can compile the program using the following command (from the project's root):
```bash
mkdir build && cd build && cmake .. && cmake --build . -- -j`nproc`
``` 

## Windows
This program occasionally gets compiled for Windows 10 using Visual Studio 2019. 

**Note:** Even though this program occasionally does compile for Windows, the program will be useless until npcap supports packet injection, see: https://github.com/nmap/npcap/issues/85 . Or this program supports a second adapter to do the injection. Also for the Monitor mode part to work at all you need the 0.9982 version of npcap due to the following bug: https://github.com/nmap/npcap/issues/159 .

To compile this on Windows (using MSVC v14), the following libraries are needed:
- Boost 0.7.1 or higher https://www.boost.org/users/download/ (threads and program_options required).
- PDCurses or other ncurses compatible library (only pdcurses tested) https://github.com/Bill-Gray/PDCursesMod \
  (Make sure you use the x64 toolchain to compile this, otherwise you'll get very undescriptive errors when trying to compile the program.)
- NPcap 0.9982 and NPcap SDK. \
  https://nmap.org/npcap/dist/npcap-0.9982.exe \
  https://nmap.org/npcap/dist/npcap-sdk-1.06.zip
  
After installing these, open the CMakeLists.txt and set the paths to the libraries to the paths you installed these libraries to. \
By default it looks in the following paths:
- c:\Program Files\boost\boost_1_71_0
- c:\pdcurses\
- c:\npcapsdk

If that is all in order you should be able to compile the program using Visual Studio 2019 or higher, by opening the project using it and then pressing the compile button. 

After compiling, the program needs the following DLLs to be copied over to the binary directory:
- boost_program_options-(compiler)-(architecture)-(version).dll
- boost_thread-(compiler)-(architecture)-(version).dll
- Packet.dll (found in: c:\windows\system32\npcap\)
- wpcap.dll (found in: c:\windows\system32\npcap\)

After that the program should be able to run.


## How to run (Linux, probably also MacOS?)
To put a card in monitor mode, which is needed for this program to work, I'd recommend installing aircrack-ng and setting your capture device to monitor mode as follows (frequency 2412 is the same as channel 1):
```bash
sudo airmon-ng start wificard 2412
``` 

After the program has been built succesfully it can be ran as follows:
```bash
sudo ./mondevtopromisc --bssid "xx:xx:xx:xx:xx:xx" --xlink_kai --capture_interface wificardmon
``` 

The BSSID could be found by looking at traffic from the device you're trying to forward in Wireshark.

## Known issues
- Packet injection on Windows does not work
- Resizing the window in Windows causes the window to corrupt due to Windows not providing the right size hints.
