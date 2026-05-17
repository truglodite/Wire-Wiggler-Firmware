# Wire Wiggler Firmware: Yet another satellite antenna rotator project
Wire Wiggler is a satellite antenna rotator control firmware intended for use with wide range of commonly available nc controller hardware. The code is based off of satnogs original stepper rotator code. So Wire Wiggler can be used with many popular rotator apps just like a "satnogs" rotator. It includes an open source hardware design using 3d printed parts, and commonly available parts.

Read about the original satnogs stepper rotator project here: [SatNOGS Rotator Controller Based on Arduino UNO and CNC V3 Shield](https://gitlab.com/Quartapound/satnogs-rotator-firmware/).

## Features
* Works with satellite tracking apps with a rotator feature, like Gpredict
* Compatible with common stepper motors & NC controllers (aka 3d printer electronics)
* Optional 3rd axis for antenna polarization control
* Auto homes all axis at boot up
* Watchdog timer shuts down all drivers when triggered (not supported by all hardware)
* ...currently under development
    * Automatic polarization for ISS
    * IMU feedback for moving vehicles
    * Parking feature
    * Softserial debugging

![refDesBuild3.png](/images/refDesBuild3.png)

## Currently Compatible Hardware
* Arduino UNO w/ UNO CNC Shield v3
* Arduino Nano w/ Nano CNC V4 Shield
* Arduino Mega2560 w/ Ramps v1.4
* Teensy 3.2 and 4.0 w/ UNO CNC Shield v3
* Anet A8 3d Printer Motherboard (1280p w/ built-in a4988's)
* N.O. endstop switches
* Linear potentiometers (use 10nF between wiper and gnd)
* Standard stepper drivers with STEP/DIR/EN pins (eg a4988, drv8825, TMCXXXX in standalone mode)

## Instructions
### Build the mechanics
Wire Wiggler firmware could be used with a broad assortment of mechanical designs. To test this project, an open source prototype "reference design" was created. Info on this design can be found here: [Link to Wire Wiggler Reference Design](/models/referenceDesign.md)

### Configure, compile, and upload firmware:
*A basic understanding of how to install and use VSCode with PlatformIO is assumed in this text.*

Download and decompress the Wire Wiggler repository in your project folder, and open the project in PlatformIO. Edit the `config.h` file to suit your hardware (comment/uncomment and edit any relevant lines). Uncomment the section that matches your microcontroller in `platformio.ini`. Finally, compile and upload Wire Wiggler firmware to your board by selecting the upload option that matches your board in the PIO "Project Tasks" tree. If completed succesfully, your board is now ready for use.

### Install Hamlib and Gpredict Software:
Hamlib contains the `rotctld` driver that is used for communication between the rotator control software of your choice (ie Gpredict) and your board's usb or serial port. Download and install the latest hamlib installer that matches your OS from here:
[https://github.com/Hamlib/Hamlib/releases](https://github.com/Hamlib/Hamlib/releases)

Details on downloading, installing, and generally operating GPredict or other control software are beyond the scope of this project. However the process is typically fairly easy to follow.

### Create a launch script for rotctld:
Plug your hardware into your computer, and find the port used by your board using device manager in Windows, or `dmesg | grep tty` in a Linux terminal. This port may be for example `COM3` in windows, or `/dev/ttyUSB3` in Linux. Use your favorite text editor to make a shell script with the following line to launch rotctld, replacing the port name with the one your found earlier:

```
rotctld -m 202 -r /dev/ttyUSB0 -s 57600 -T 127.0.0.1 -t 4533 -C timeout=500 -C retry=0 -vvvvvvvv
```
Make the script executable so you can use convenient shortcuts to start rotctld before using your rotator software. You can also just use that line in a terminal to start rotctld manually.

### Configure GPredict
In Gpredict, go to Edit > Preferences > Interfaces > Rotators > Add New

Give it a name, for example "Wire Wiggler". Leave the defaults alone (localhost, port 4533). 

### Operating
First use the terminal or startup script to launch rotctld. Now open Gpredict, go to the Rotator Controller, click the enable rotator button, select a satellite to track in the drop down menu, and hit track. That's it... the rotator should start following the sat from AOS to LOS. After reaching LOS, the rotator will move to the next predicted AOS location for the satellite.

## Easycomm Commands
The easycomm lib is used by the rotator control to interface with computer software, via a UART connection (com port). The default baud rate for Wire Wiggler is 9600. Here is the list of valid terminal commands that easycomm responds to (* currently not all commands are implemented):

* AZ, Move azimuth to (degrees 1 decimal place)
* EL, Move elevation to (degrees 1 decimal place)
* SA, Stop azimuth movement
* SE, Stop elevation movement
* RESET, Initiate homing subroutine
* PARK, Move to park position
* IP, Read an input
    * Temperature = 0
    * SW1 = 1
    * SW2 = 2
    * Encoder1 = 3
    * Encoder2 = 4
    * Load of M1/AZ = 5
    * Load of M2/EL = 6
    * Speed of M1/AZ (DPS) = 7
    * Speed of M2/EL (DPS) = 8
* VE, Request Version
* GS, Get status register
    * idle = 1
    * moving = 2
    * pointing = 4
    * error = 8
* GE, Get error register
    * no_error = 1
    * sensor_error = 2
    * homing_error = 4
    * motor_error = 8
    * over_temperature = 12
    * wdt_error = 16
* VL, Velocity Left ,number [mdeg/s]
* VR, Velocity Right, number [mdeg/s]
* VU, Velocity Up, number [mdeg/s]
* VD, Velocity Down, number [mdeg/s]
* CR, Read config... `CR 2 1.0` sets azimuth I gain to 1.0
    * Gain P for AZ = 1
    * Gain I for AZ = 2
    * Gain D for AZ = 3
    * Gain P for EL = 4
    * Gain I for EL = 5
    * Gain D for EL = 6
    * Azimuth park position = 7
    * Elevation park position = 8
    * Control mode (position = 0, speed = 1) = 9
* CW, Write config... `CW 7 0.0` sets azimuth park position to 0.0degrees
    * Gain P for AZ = 1
    * Gain I for AZ = 2
    * Gain D for AZ = 3
    * Gain P for EL = 4
    * Gain I for EL = 5
    * Gain D for EL = 6
    * Azimuth park position = 7
    * Elevation park position = 8

* RB, custom command to reboot controller

## Wire Wiggler Gallery
See some existing builds and design renderings using this project in the gallery: [Link to Wire Wiggler Gallery](/models/gallery.md)

## Troubleshooting
Understanding how to stepper motors work in general can help users fix many problems than can arise while operating Wire Wiggler. While it is out of scope to discuss stepper concepts here, there are some very common issues that may result from inadequate motor torque,aka "skipped steps". First and foremost the mechanics should be as well balanced and friction free as possible in order to minimize required torque to move and hold the antenna position. Increasing motor voltage, motor current, and/or gear ratio, together with reducing acceleration, microstepping, and max speed can all help to reduce skipped steps.

Wire Wiggler by default uses the onboard LED to indicate status (note some compatible boards do not have an LED). A solid on LED indicates an error has occured (typically from mechanism not closing endstops within specified travel limits). A slow 2hz flashing LED means the controller is in position mode (ie ready to be controlled by Gpredict). A faster 12hz flashing LED indicates the controller is homing. Thus LED status can help troubleshoot where problems may occur. In addition the code includes a DEBUG definition that can be used to disable normal LED behavior, which allows for LED code snippets to be added where troubleshooting is needed.

## Contribute
The main repository lives on [Github](https://github.com/truglodite/Wire-Wiggler-Firmware).

Anyone is welcome to use/edit/share this code as they please as long as it is within the terms of the GPLv3 license. We welcome everyone to join in on the fun with pull requests, bug reports, suggestions, etc. Stay on the air and have fun!

## License
Licensed under the [GPLv3](LICENSE)
