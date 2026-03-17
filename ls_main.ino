/************************** ls_main: base file for Arduino IDE 'Sketch' load **************************
Copyright 2023 Roger Linn Design (https://www.rogerlinndesign.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
***************************************************************************************************

**************************************************************************************************/

/*
- Open this file in your Arduino IDE (open sketch)
- make sure the [Arduino Due] has been installed as a target via your "Boards Manager" in the IDE.
- select the [Arduino Due] as your target board for this sketch.

All *.ino files SHOULD have been loaded into the IDE by now. (plus the README.md, etc.)

- click the [Verify] icon to compile the sketch = LinnStrument firmware.

Notes:

when you have turned on all warnings in your Arduino IDE preferences (a very sane thing to do, as warnings are often hints for errors),
then these warnings should now show up -- all of them harmless, though I intend to get rid of most of these as they
obscure any *future* warnings in new code that may turn out to be important, so it's best to strive for a 0-warnings
result:


FQBN: arduino:sam:arduino_due_x_dbg
Using board 'arduino_due_x_dbg' from platform in folder: C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12
Using core 'arduino' from platform in folder: C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12

Detecting libraries used...
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\tools\arm-none-eabi-gcc\4.8.3-2014q1/bin/arm-none-eabi-g++ -c -g -Os -w -std=gnu++11 -ffunction-sections -fdata-sections -nostdlib -fno-threadsafe-statics --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -w -x c++ -E -CC -mcpu=cortex-m3 -mthumb -DF_CPU=84000000L -DARDUINO=10607 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM -D__SAM3X8E__ -mthumb -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON -DUSB_MANUFACTURER="Arduino LLC" -DUSB_PRODUCT="Arduino Due" -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/libsam -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/CMSIS/CMSIS/Include/ -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/CMSIS/Device/ATMEL/ -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\variants\arduino_due_x C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\sketch\linnstrument-firmware.ino.cpp -o nul
Alternatives for SPI.h: [SPI@1.0]
ResolveLibrary(SPI.h)
  -> candidates: [SPI@1.0]
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\tools\arm-none-eabi-gcc\4.8.3-2014q1/bin/arm-none-eabi-g++ -c -g -Os -w -std=gnu++11 -ffunction-sections -fdata-sections -nostdlib -fno-threadsafe-statics --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -w -x c++ -E -CC -mcpu=cortex-m3 -mthumb -DF_CPU=84000000L -DARDUINO=10607 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM -D__SAM3X8E__ -mthumb -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON -DUSB_MANUFACTURER="Arduino LLC" -DUSB_PRODUCT="Arduino Due" -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/libsam -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/CMSIS/CMSIS/Include/ -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/CMSIS/Device/ATMEL/ -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\variants\arduino_due_x -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\libraries\SPI\src C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\sketch\linnstrument-firmware.ino.cpp -o nul
Using cached library dependencies for file: C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\libraries\SPI\src\SPI.cpp
Generating function prototypes...
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\tools\arm-none-eabi-gcc\4.8.3-2014q1/bin/arm-none-eabi-g++ -c -g -Os -w -std=gnu++11 -ffunction-sections -fdata-sections -nostdlib -fno-threadsafe-statics --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -w -x c++ -E -CC -mcpu=cortex-m3 -mthumb -DF_CPU=84000000L -DARDUINO=10607 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM -D__SAM3X8E__ -mthumb -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON -DUSB_MANUFACTURER="Arduino LLC" -DUSB_PRODUCT="Arduino Due" -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/libsam -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/CMSIS/CMSIS/Include/ -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\system/CMSIS/Device/ATMEL/ -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\variants\arduino_due_x -IC:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\libraries\SPI\src C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\sketch\linnstrument-firmware.ino.cpp -o C:\Users\Ger\AppData\Local\Temp\2554885057\sketch_merged.cpp
C:\Users\Ger\AppData\Local\Arduino15\packages\builtin\tools\ctags\5.8-arduino11/ctags -u --language-force=c++ -f - --c++-kinds=svpf --fields=KSTtzns --line-directives C:\Users\Ger\AppData\Local\Temp\2554885057\sketch_merged.cpp

Compiling sketch...
"C:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\arm-none-eabi-gcc\\4.8.3-2014q1/bin/arm-none-eabi-g++" -c -g -Os -Wall -Wextra -std=gnu++11 -ffunction-sections -fdata-sections -nostdlib -fno-threadsafe-statics --param max-inline-insns-single=500 -fno-rtti -fno-exceptions -MMD -mcpu=cortex-m3 -mthumb -DF_CPU=84000000L -DARDUINO=10607 -DARDUINO_SAM_DUE -DARDUINO_ARCH_SAM -D__SAM3X8E__ -mthumb -DUSB_VID=0x2341 -DUSB_PID=0x003e -DUSBCON "-DUSB_MANUFACTURER=\"Arduino LLC\"" "-DUSB_PRODUCT=\"Arduino Due\"" "-IC:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\system/libsam" "-IC:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\system/CMSIS/CMSIS/Include/" "-IC:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\system/CMSIS/Device/ATMEL/" "-IC:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\cores\\arduino" "-IC:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\variants\\arduino_due_x" "-IC:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\libraries\\SPI\\src" "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646\\sketch\\linnstrument-firmware.ino.cpp" -o "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646\\sketch\\linnstrument-firmware.ino.cpp.o"
In file included from C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/Arduino.h:44:0,
                 from C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\sketch\linnstrument-firmware.ino.cpp:1:
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void resetLastMidiPolyPressure(byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2098:10: note: in expansion of macro 'constrain'
   note = constrain(note, 0, 127);
          ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void resetLastMidiCC(byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2180:16: note: in expansion of macro 'constrain'
   controlnum = constrain(controlnum, 0, 127);
                ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: At global scope:
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2367:6: warning: unused parameter 'split' [-Wunused-parameter]
 void preSendFader(byte split, byte v) {
      ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2367:6: warning: unused parameter 'v' [-Wunused-parameter]
In file included from C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/Arduino.h:44:0,
                 from C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\sketch\linnstrument-firmware.ino.cpp:1:
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendControlChange(byte, byte, byte, boolean)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2538:16: note: in expansion of macro 'constrain'
   controlnum = constrain(controlnum, 0, 127);
                ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2539:16: note: in expansion of macro 'constrain'
   controlval = constrain(controlval, 0, 127);
                ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendControlChange14BitUserFirmware(byte, byte, short int, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2570:16: note: in expansion of macro 'constrain'
   controlMsb = constrain(controlMsb, 0, 127);
                ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2571:16: note: in expansion of macro 'constrain'
   controlLsb = constrain(controlLsb, 0, 127);
                ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendControlChange14BitMIDISpec(byte, byte, short int, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2612:16: note: in expansion of macro 'constrain'
   controlMsb = constrain(controlMsb, 0, 127);
                ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2613:16: note: in expansion of macro 'constrain'
   controlLsb = constrain(controlLsb, 0, 127);
                ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendNoteOn(byte, byte, byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2655:11: note: in expansion of macro 'constrain'
   split = constrain(split, 0, 1);
           ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2656:13: note: in expansion of macro 'constrain'
   notenum = constrain(notenum, 0, 127);
             ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2657:14: note: in expansion of macro 'constrain'
   velocity = constrain(velocity, 0, 127);
              ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'boolean hasActiveMidiNote(byte, byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2681:11: note: in expansion of macro 'constrain'
   split = constrain(split, 0, 1);
           ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2682:13: note: in expansion of macro 'constrain'
   notenum = constrain(notenum, 0, 127);
             ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendNoteOff(byte, byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2688:11: note: in expansion of macro 'constrain'
   split = constrain(split, 0, 1);
           ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2689:13: note: in expansion of macro 'constrain'
   notenum = constrain(notenum, 0, 127);
             ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendNoteOffWithVelocity(byte, byte, byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2699:11: note: in expansion of macro 'constrain'
   split = constrain(split, 0, 1);
           ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2700:13: note: in expansion of macro 'constrain'
   notenum = constrain(notenum, 0, 127);
             ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendNoteOffForAllTouches(byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2727:11: note: in expansion of macro 'constrain'
   split = constrain(split, 0, 1);
           ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendProgramChange(byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2778:12: note: in expansion of macro 'constrain'
   preset = constrain(preset, 0, 127);
            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendAfterTouch(byte, byte, boolean)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2800:11: note: in expansion of macro 'constrain'
   value = constrain(value, 0, 127);
           ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendPolyPressure(byte, byte, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2828:11: note: in expansion of macro 'constrain'
   value = constrain(value, 0, 127);
           ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendNRPN(short unsigned int, short unsigned int, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2854:12: note: in expansion of macro 'constrain'
   number = constrain(number, 0, 0x3fff);
            ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2855:11: note: in expansion of macro 'constrain'
   value = constrain(value, 0, 0x3fff);
           ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino: In function 'void midiSendRPN(short unsigned int, short unsigned int, byte)':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2887:12: note: in expansion of macro 'constrain'
   number = constrain(number, 0, 0x3fff);
            ^
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_midi.ino:2888:11: note: in expansion of macro 'constrain'
   value = constrain(value, 0, 0x3fff);
           ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_noteTouchMapping.ino: In function 'boolean validNoteNumAndChannel(signed char, signed char)':
C:\Users\Ger\Downloads\linnstrument-firmware\ls_noteTouchMapping.ino:31:32: warning: comparison is always false due to limited range of data type [-Wtype-limits]
   if (noteNum < 0 || noteNum > 127 || noteChannel < 1 || noteChannel > 16) {
                                ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_sequencer.ino: In member function 'void StepSequencerState::handleStepEditingTouch(boolean, int, byte)':
C:\Users\Ger\Downloads\linnstrument-firmware\ls_sequencer.ino:1996:17: warning: comparison is always false due to limited range of data type [-Wtype-limits]
   if (stepNum < 0 || stepNum >= MAX_SEQUENCER_STEPS) return;
                 ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_sequencer.ino: At global scope:
C:\Users\Ger\Downloads\linnstrument-firmware\ls_sequencer.ino:2072:6: warning: unused parameter 'stepState' [-Wunused-parameter]
 void StepSequencerState::createNewEvent(int noteNum, byte stepNum, byte eventNum, StepEvent& event, StepDataState& stepState) {
      ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_sequencer.ino: In member function 'void StepSequencerState::handleStepEditingRelease(int, byte)':
C:\Users\Ger\Downloads\linnstrument-firmware\ls_sequencer.ino:2082:17: warning: comparison is always false due to limited range of data type [-Wtype-limits]
   if (stepNum < 0 || stepNum >= MAX_SEQUENCER_STEPS) return;
                 ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_settings.ino: In function 'void handleGlobalSettingNewTouch()':
C:\Users\Ger\Downloads\linnstrument-firmware\ls_settings.ino:2606:26: warning: comparison is always true due to limited range of data type [-Wtype-limits]
         if (sensorRow >= 0 && sensorRow <= 3) {
                          ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_settings.ino: In function 'void trimEditedAudienceMessage()':
C:\Users\Ger\Downloads\linnstrument-firmware\ls_settings.ino:3262:33: warning: comparison is always true due to limited range of data type [-Wtype-limits]
   if (audienceMessageToEdit != -1) {
                                 ^
In file included from C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/Arduino.h:44:0,
                 from C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\sketch\linnstrument-firmware.ino.cpp:1:
C:\Users\Ger\Downloads\linnstrument-firmware\ls_touchInfo.ino: In member function 'void TouchInfo::refreshZ()':
C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\cores\arduino/wiring_constants.h:74:44: warning: comparison is always false due to limited range of data type [-Wtype-limits]
 #define constrain(amt,low,high) ((amt)<(low)?(low):((amt)>(high)?(high):(amt)))
                                            ^
C:\Users\Ger\Downloads\linnstrument-firmware\ls_touchInfo.ino:559:17: note: in expansion of macro 'constrain'
     pressureZ = constrain(usablePressureZ, 0, 1016);
                 ^
Compiling libraries...
Compiling library "SPI"
Using previously compiled file: C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\libraries\SPI\SPI.cpp.o
Compiling core...
Using previously compiled file: C:\Users\Ger\AppData\Local\arduino\sketches\3A25C2FDE09FC88530F07BF006A93646\core\variant.cpp.o
Using precompiled core: C:\Users\Ger\AppData\Local\arduino\cores\arduino_sam_arduino_due_x_dbg_7870f019bef9a8f5f8f2c62678d97c7e\core.a
Linking everything together...
"C:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\arm-none-eabi-gcc\\4.8.3-2014q1/bin/arm-none-eabi-gcc" -mcpu=cortex-m3 -mthumb -Os -Wl,--gc-sections "-TC:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\variants\\arduino_due_x/linker_scripts/gcc/flash.ld" "-Wl,-Map,C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646/linnstrument-firmware.ino.map" -o "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646/linnstrument-firmware.ino.elf" "-LC:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646" -Wl,--cref -Wl,--check-sections -Wl,--gc-sections -Wl,--entry=Reset_Handler -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-section-align -Wl,--start-group -u _sbrk -u link -u _close -u _fstat -u _isatty -u _lseek -u _read -u _write -u _exit -u kill -u _getpid "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646\\sketch\\linnstrument-firmware.ino.cpp.o" "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646\\libraries\\SPI\\SPI.cpp.o" "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646\\core\\variant.cpp.o" "C:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\hardware\\sam\\1.6.12\\variants\\arduino_due_x/libsam_sam3x8e_gcc_rel.a" "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646/..\\..\\cores\\arduino_sam_arduino_due_x_dbg_7870f019bef9a8f5f8f2c62678d97c7e\\core.a" -Wl,--end-group -lm -lgcc
"C:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\arm-none-eabi-gcc\\4.8.3-2014q1/bin/arm-none-eabi-objcopy" -O binary "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646/linnstrument-firmware.ino.elf" "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646/linnstrument-firmware.ino.bin"
Using library SPI at version 1.0 in folder: C:\Users\Ger\AppData\Local\Arduino15\packages\arduino\hardware\sam\1.6.12\libraries\SPI 
"C:\\Users\\Ger\\AppData\\Local\\Arduino15\\packages\\arduino\\tools\\arm-none-eabi-gcc\\4.8.3-2014q1/bin/arm-none-eabi-size" -A "C:\\Users\\Ger\\AppData\\Local\\arduino\\sketches\\3A25C2FDE09FC88530F07BF006A93646/linnstrument-firmware.ino.elf"
Sketch uses 157252 bytes (29%) of program storage space. Maximum is 524288 bytes.


*/
