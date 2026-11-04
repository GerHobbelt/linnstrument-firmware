LinnStrument
============

This is the firmware for LinnStrument, running on an Arduino Due processor (Atmel/Microchip SAM3X8E).

Building
--------

The sources are an Arduino sketch. They build with Arduino IDE 2.x or with
arduino-cli, using the Arduino SAM Boards core version 1.6.12 (select board
"Arduino Due (Native USB Port)"). Other versions of the SAM core are untested
and may create unknown build or execution problems.

From the command line, `go run ./tools/linnstrument build` compiles the firmware into
`build/`, which needs Go and arduino-cli, plus the Xcode Command Line Tools on
macOS, and `go run ./tools/linnstrument test` runs the desktop tests in `tests/`. The
same tool flashes a unit and backs up and restores its settings. Every push to
GitHub is also compiled by GitHub Actions, which attaches the resulting `.bin`
file to the workflow run.

On Apple silicon Macs the Arduino SAM toolchain is an Intel binary and needs
Rosetta installed (`softwareupdate --install-rosetta`).

Flash the resulting `.bin` with the LinnStrument Updater, which keeps your
settings. Uploading with bossac or the Arduino IDE directly erases all settings
and calibration.

Contributing
------------

See CONTRIBUTING.md for the branch and pull request workflow, how to test on
the instrument, and how to work on this code with an AI coding assistant.
CLAUDE.md holds the architecture notes and constraints that such assistants
read automatically; they are useful reading for people too.

> This repo fork however has been used to rebuild the firmware using a fresh Arduino IDE + CLI version (see below) and nothing extremely nasty jumped at us yet. Fingers crosssed...


## Developer Notes

> This repo fork however has been used to rebuild the firmware using a fresh Arduino IDE + CLI version (see below) and nothing extremely nasty jumped at us yet. Fingers crosssed...

To rebuild the LinnStrument / MicroLinn firmware binary, set up your development environment as follows: 

- download and install the Arduino IDE v2.x (tested with v2.3.10)
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
