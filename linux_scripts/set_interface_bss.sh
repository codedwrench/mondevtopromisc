#!/bin/bash
sudo systemctl stop NetworkManager
sudo ip link set "$1" down
sudo ip link set "$1" mtu 2272
sudo iw "$1" set type ibss 
sudo ip link set "$1" up
