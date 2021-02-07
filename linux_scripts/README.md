# Helper Scripts
The scripts in this folder help you set up your WiFi card for use with XLink Handheld Assistant.
These scripts assume you are normally connected by wire! If this is not the case you may need to follow different steps.
This is because these scripts turn NetworkManager off!

The following scripts are available:
- set_interface_bss.sh
- set_interface_monitor.sh
- restore_managed.sh

You call the scripts using script.sh interface.
set_interface_bss.sh is used for PSP plugin mode.
set_interface_monitor.sh is used for monitor mode.
restore_managed.sh is used to restore the default mode and start networkmanager back up.
