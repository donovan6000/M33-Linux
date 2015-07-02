# M3D_Linux
© 2015 donovan6000

A Linux program that can communicate with the Micro M3D printer

If this program is run without any parameters, then it will enter a mode that allows for individual G-code commands to be entered and ran.

If a G-code file is provided as a parameter when running this program, then the program will process that file then send each command to the printer. There currently isn't any way to stop mid print aside from ctrl+C. So you run it like this:

./M3D_Linux file.gcode

The 90-m3d-local.rules needs to applied in order to avoid issues. You can apply it like this:

cp ./90-m3d-local.rules /etc/udev/rules.d/
sudo /etc/init.d/udev restart


TODO:
* Work more on the preprocessor
* Maybe create a GUI
