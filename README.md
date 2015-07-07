# M3D_Linux
© 2015 donovan6000

A Linux program that can communicate with the Micro M3D printer

The parameters that can be provided when running this program are as follows
m3d-linux -v -p -w -t -b -l -f -r firmware.rom -g file.gcode
-v | --validation: Use validation pre-processor
-p | --preparation: Use preparation pre-processor
-w | --wavebonding: Use wave bonding pre-processor
-t | --thermalbonding: Use thermal bonding pre-processor
-b | --bedcompensation: Use bed compensation pre-processor
-l | --backlashcompensation: Use backlash compensation pre-processor
-f | --feedrateconversion: Use feed rate conversion pre-processor
-r | --firmwarerom: Use the following parameter as the firmware ROM in case firmware is corrupt
-g | --gcodefile: Use the following parameter as the G-code file to processes and send to the printer. G-code commands can be manually entered if not G-code file is not provided. If not file is provided, then pre-processor stages aren't used.

If the printer's firmware is corrcupt, it will update the firmware if a rom is provided. Firmware roms must be named after their version number, ex: 2015062401.rom. If the Z calibration on the printer is invalid, it will calibrate it. If a G-code file is provided and it contains a line that indicated the ideal temperature, ex: ;ideal temp:195, then that temperature will be used. Otherwise the temperature stored in the printer's EEPROM will be used.


The 90-m3d-local.rules needs to applied in order to avoid issues. You can apply it like this:
cp ./90-m3d-local.rules /etc/udev/rules.d/
sudo /etc/init.d/udev restart


TODO:
* Maybe create a GUI
