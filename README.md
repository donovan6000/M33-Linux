# M3D_Linux
A Linux program that can communicate with the Micro M3D printer
#
If this program is run without any parameters, then it will enter a mode that allows for individual G-code commands to be entered and ran.

If a G-code file is provided as a parameter when running this program, then the program will process that file then send each command to the printer. There currently isn't any way to stop mid print aside from ctrl+C.

And this program needs to be ran as root since Linux assigns /dev/ttyACM devices as root owner/group

TODO:
* Work more on preprocess
* Maybe create a GUI
