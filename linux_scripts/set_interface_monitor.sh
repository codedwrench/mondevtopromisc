#!/bin/bash
sudo systemctl stop NetworkManager
sudo ip link set "$1" down
sudo ip link set "$1" mtu 2272
sudo iw "$1" set monitor otherbss fcsfail control
sudo ip link set "$1" up
sudo iw "$1" set channel 1
