LinnStrument
============

This is the firmware for LinnStrument, running on an Arduino Due processor.

These sources assume that you're using Arduino IDE v1.8.1 with SAM boards v1.6.11.
Different versions of this package might create unknown build and execution problems.

> This repo fork however has been used to rebuild the firmware using a fresh Arduino IDE + CLI version (see below) and nothing extremely nasty jumped at us yet. Fingers crosssed...


## Developer Notes

To rebuild the LinnStrument / MicroLinn firmware binary, set up your development environment as follows: 

- download and install the Arduino IDE v2.x (tested with v2.3.8)
- download and install the Arduino CLI (tested with v1.4.1)
- start the Arduino IDE
- wait a little bit for the board package specs to download (particularly when you have added additional boards' package JSON URLs to the Arduino IDE preferences)
- click on the Board Manager icon: a sidebar with a list of available board packages shows up
- search for `due`: find `Arduino SAM boards (32 bits ARM Cortex M3`
- install this board package (currently at v1.6.12)
- open the `linnstrument-firmware`/`MicroLINN` repo base directory
- pick any of the `*.ino` sketch files to open in the Arduino IDE: it should subsequently open *all of them*. (I prefer to use `ls_main.ino` for this pick & open action.)
- hit the build/*Verify* button in the IDE to compile the whole kaboodle...
- *profit!*

### Pro Tip

In the Arduino IDE preferences, turn on "*All*" Warnings for the compiler: compiler warnings may seem useless and verbose sometimes, but they *are* important!

This repo fork has a special `ls_compiler_tweaks.h` header file which deals with the obnoxious warnings, while letting through all the (useful) others...

Further info:

- https://docs.arduino.cc/arduino-cli/sketch-build-process/
- https://docs.arduino.cc/arduino-cli/sketch-specification/
- 

## Support

Note that customer bug reports or feature requests should not be posted here.
Instead, email Roger at support@rogerlinndesign.com. 
