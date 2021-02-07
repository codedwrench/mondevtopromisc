#!/bin/bash
sudo ip link set "$1" down
sudo ip link set "$1" mtu 1500
sudo iw "$1" set type managed
sudo ip link set "$1" up
sudo systemctl start NetworkManager
