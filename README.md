# mondevtopromisc
Hopefully will in the future add a way to create a virtual network card that emulates a promiscuous mode wifi card using monitor mode and packet injection.

As of right now it is a working proof of concept where you can receive and send PSP traffic succesfully through XLink Kai with a few restrictions. Namely it only works on channel 1, and you will have to find the BSSID the PSP is sending on yourself and use it in the command line when starting this program.

## Wifi cards this has been tested and found working on:
- Azurewave AR5BHB92 (Atheros 9280 (ath9k))
- Intel Pro Wireless 5100 AGN (iwlwifi)
- Intel Centrino Wireless-N 1000 (iwlwifi)
- WTXUP Ralink RT3070 (rt2800usb)
- TP-Link TL-WN823N V2 (Realtek rtl8192eu (kimocoder/rtl8192eu))
- Dell Wireless 1390 (Broadcom BCM4311 (b43)) with the following patch: https://gist.github.com/codedwrench/8d7916d63993574e1dd089a62dd523a9

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

To put a card in monitor mode, which is needed for this program to work, I'd recommend installing aircrack-ng and setting your capture device to monitor mode as follows (frequency 2412 is the same as channel 1):
```bash
sudo airmon-ng start wificard 2412
``` 

After the program has been built succesfully it can be ran as follows:
```bash
sudo ./mondevtopromisc --bssid "xx:xx:xx:xx:xx:xx" --xlink_kai --capture_interface wificardmon
``` 

The BSSID could be found by looking at traffic from the device you're trying to forward in Wireshark.
