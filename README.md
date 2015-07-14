# M3D Linux
© 2015 donovan6000

A Linux program that can communicate with the Micro M3D printer
<br>
<br>
You can download the latest executable <a href="https://www.exploitkings.com/public/m3d-linux-V0.12.zip">here</a> or the deb package <a href="https://www.exploitkings.com/public/m3d-linux-V0.12.deb">here</a>.
<br>
<br>
The parameters that can be provided when running this program are as follows:
<br>
m3d-linux -v -p -w -t -b -l -f -r firmware.rom -c -i input.gcode -s -o output.gcode

-v | --validation: Use validation pre-processor

-p | --preparation: Use preparation pre-processor

-w | --wavebonding: Use wave bonding pre-processor

-t | --thermalbonding: Use thermal bonding pre-processor

-b | --bedcompensation: Use bed compensation pre-processor

-l | --backlashcompensation: Use backlash compensation pre-processor

-f | --feedrateconversion: Use feed rate conversion pre-processor

-r | --firmwarerom: Use the following parameter as the firmware ROM in case firmware is corrupt or outdated

-c | --forceflash: Forces the firmware to update to the provided ROM

-i | --inputfile: Use the following parameter as the G-code file to processes and send to the printer. G-code commands can be manually entered if no G-code file is not provided.

-s | --translate: Uses the program as a middle man to communicate between the printer and other software

-o | --outputfile: Use the following parameter as the G-code file to output after the input file has been processed by all the desired pre-processor stages.
<br>
<br>
If the printer's firmware is corrupt or the provided firmware rom is newer, it will update the firmware if a rom is provided. Firmware roms must be named after their version number, ex: 2015062401.rom. If the Z calibration of the printer is invalid, it will calibrate it automatically. If a G-code file is provided and it contains a line that indicated the ideal temperature, ex: ;ideal temp:195, then that temperature will be used for printing. Otherwise the temperature stored in the printer's EEPROM will be used.
<br>
<br>
The 90-m3d-local.rules needs to applied in order to avoid issues. You can apply it like this:
<br>
cp ./90-m3d-local.rules /etc/udev/rules.d/
<br>
sudo /etc/init.d/udev restart
