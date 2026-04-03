/*=====================================================================================================================
======================================== LinnStrument Operating System v2.3.4 =========================================
=======================================================================================================================

Operating System for the LinnStrument (c) music controller by
Roger Linn Design (https://www.rogerlinndesign.com).

Written by Roger Linn and Geert Bevin (https://www.uwyn.com) with significant
help by Tim Thompson (https://timthompson.com).

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

For any questions about this, contact Roger Linn Design at support@rogerlinndesign.com.

=======================================================================================================================
=======================================================================================================================
=====================================================================================================================*/

/*
 * DUE Board pin   |  PORT  | Label
 * ----------------+--------+-------
 *   0             |  PA8   | "RX0"
 *   1             |  PA9   | "TX0"
 *   2       TIOA0 |  PB25  |
 *   3       TIOA7 |  PC28  |
 *   4       NPCS1 |  PA29  | SPI_SENSOR : touch sensor control over SPI
 *           TIOB6 |  PC26  |
 *   5       TIOA6 |  PC25  |
 *   6       PWML7 |  PC24  |
 *   7       PWML6 |  PC23  |
 *   8       PWML5 |  PC22  |
 *   9       PWML4 |  PC21  |
 *  10       NPCS0 |  PA28  | SPI_LEDS : LED control over SPI
 *           TIOB7 |  PC29  |
 *  11       TIOA8 |  PD7   |
 *  12       TIOB8 |  PD8   |
 *  13       TIOB0 |  PB27  | ~~LED AMBER "L"~~
 *  14       TXD3  |  PD4   | "TX3"
 *  15       RXD3  |  PD5   | "RX3"
 *  16       TXD1  |  PA13  | "TX2"
 *  17       RXD1  |  PA12  | "RX2"
 *  18       TXD0  |  PA11  | "TX1"
 *  19       RXD0  |  PA10  | "RX1"
 *  20             |  PB12  | "SDA"
 *  21             |  PB13  | "SCL"
 *  22             |  PB26  |
 *  23             |  PA14  |
 *  24             |  PA15  |
 *  25             |  PD0   |
 *  26             |  PD1   |
 *  27             |  PD2   |
 *  28             |  PD3   |
 *  29             |  PD6   |
 *  30             |  PD9   |
 *  31             |  PA7   |
 *  32             |  PD10  |
 *  33             |  PC1   | FOOT_SW_LEFT
 *  34             |  PC2   | FOOT_SW_RIGHT
 *  35             |  PC3   | MIDI or SERIAL operation (output) : set it HIGH for serial operation i.e. debug output & firmware uploads
 *  36             |  PC4   | DIN or USB connector (output) : set it HIGH for USB operation
 *  37             |  PC5   | the output enable line for the 2 LED display chips (output) : set HIGH to enable the LEDs.
 *  38             |  PC6   | LINNMODEL (input) : high = Model 200, low = Model 128
 *  39             |  PC7   |
 *  40             |  PC8   |
 *  41             |  PC9   |
 *  42             |  PA19  |
 *  43             |  PA20  |
 *  44             |  PC19  |
 *  45             |  PC18  |
 *  46             |  PC17  |
 *  47             |  PC16  |
 *  48             |  PC15  |
 *  49             |  PC14  |
 *  50             |  PC13  |
 *  51             |  PC12  |
 *  52       NPCS2 |  PB21  | SPI_ADC : input from TI ADS7883 12-bit A/D converter
 *  53             |  PB14  |
 *  54             |  PA16  | "A0"
 *  55             |  PA24  | "A1"
 *  56             |  PA23  | "A2"
 *  57             |  PA22  | "A3"
 *  58       TIOB2 |  PA6   | "A4"
 *  69             |  PA4   | "A5"
 *  60       TIOB1 |  PA3   | "A6"
 *  61       TIOA1 |  PA2   | "A7"
 *  62             |  PB17  | "A8"
 *  63             |  PB18  | "A9"
 *  64             |  PB19  | "A10"
 *  65             |  PB20  | "A11"
 *  66             |  PB15  | "DAC0"
 *  67             |  PB16  | "DAC1"
 *  68             |  PA1   | "CANRX"
 *  69             |  PA0   | "CANTX"
 *  70             |  PA17  | "SDA1"
 *  71             |  PA18  | "SCL1"
 *  72             |  PC30  | ~~LED AMBER "RX"~~
 *  73             |  PA21  | ~~LED AMBER "TX"~~
 *  74       MISO  |  PA25  |
 *  75       MOSI  |  PA26  |
 *  76       SCLK  |  PA27  |
 *  77       NPCS0 |  PA28  |
 *  78       NPCS3 |  PB23  | ~~unconnected!~~
 *
 * USB pin         |  PORT
 * ----------------+--------
 *  ID             |  PB11
 *  VBOF           |  PB10
 *
 */

#include "ls_compiler_tweaks.h"

/*************************************** INCLUDED LIBRARIES **************************************/
#include <SPI.h>
#include <limits.h>

#include "ls_FlashStorage.h"
#include "ls_debug.h"
#include "ls_channelbucket.h"
#include "ls_midi.h"


/******************************************** CONSTANTS ******************************************/

static const struct OSinfo {
  const char* OSVersion;
  const char* OSVersionBuild;
} OSinfo = {
  .OSVersion = "234.",
  .OSVersionBuild = ".074",
};

// SPI addresses
#define SPI_LEDS    10               // Arduino pin for LED control over SPI
#define SPI_SENSOR  4                // Arduino pin for touch sensor control over SPI
#define SPI_ADC     52               // Arduino pin for input from TI ADS7883 12-bit A/D converter

// Uncomment to immediately start X, Y, or Z frame debugging when the LinnStrument launches
// This is useful when having to inspect the sensor data without being able to
// use the switches to change the active settings
//
//#define DISPLAY_XFRAME_AT_LAUNCH
//#define DISPLAY_YFRAME_AT_LAUNCH
//#define DISPLAY_ZFRAME_AT_LAUNCH
#define DISPLAY_SURFACESCAN_AT_LAUNCH
#define DISPLAY_FREERAM_AT_LAUNCH
#define DISPLAY_DEBUGMIDI_AT_LAUNCH
//#define TESTING_SENSOR_DISABLE

// Touch surface constants
byte LINNMODEL = 200;

#define MAXCOLS 26
#define MAXROWS 8

byte NUMCOLS = 26;                   // number of touch sensor columns currently used for device
constexpr const byte NUMROWS = 8;    // number of touch sensor rows

#define NUMSPLITS  2                 // number of splits supported
#define LEFT       0
#define RIGHT      1

// Foot switch Arduino pins
#define FOOT_SW_LEFT   33
#define FOOT_SW_RIGHT  34

// Input options for setSwitches
#define READ_X  0
#define READ_Y  1
#define READ_Z  2

// Supported colors:
//
// RGB led ==>
// - Red
// - Green
// - Blue
// plus all its permutations:
// - Yellow         : Red + Green
// - Purple/Magenta : Red + Blue
// - Cyan           : Green + Blue
// - White          : Red + Green + Blue
// - Black          : all OFF
// Then there's also the 50% duty cycle remixes: half the time color A, the other half it's color B:
// a.k.a. 'composite colors':
// - Orange         : Yellow + Red
// - Pink           : Purple/Magenta + Yellow
// - Lime           : Yellow + Green
// - (Cold) White   : White + Cyan
//
#define COLOR_OFF      0
#define COLOR_RED      1
#define COLOR_YELLOW   2
#define COLOR_GREEN    3
#define COLOR_CYAN     4
#define COLOR_BLUE     5
#define COLOR_MAGENTA  6
#define COLOR_BLACK    7
#define COLOR_WHITE    8
#define COLOR_ORANGE   9
#define COLOR_LIME     10
#define COLOR_PINK     11
#define COLOR_LAST     11

// Special row offset values, for legacy reasons
#define ROWOFFSET_NOOVERLAP        0x00
#define ROWOFFSET_OCTAVECUSTOM     0x0c
#define ROWOFFSET_GUITAR           0x0d
#define ROWOFFSET_ZERO             0x7f

#define DEFAULT_MAINLOOP_DIVIDER      2
#define DEFAULT_LED_REFRESH           333
#define DEFAULT_MIDI_DECIMATION       8000
#define DEFAULT_MIDI_INTERVAL         235

// Differences for low power mode
// increase the number of call to continuous tasks in low power mode since the leds are refreshed more often
// accelerate led refresh so that they can be lit only a fraction of the time
#define LOWPOWER_MAINLOOP_DIVIDER     2
#define LOWPOWER_LED_REFRESH          250
#define LOWPOWER_MIDI_DECIMATION      12000    // use a decimation rate of 12 ms in low power mode
#define LOWPOWER_MIDI_INTERVAL        350      // use a minimum interval of 350 microseconds between MIDI messages in low power mode

// Values related to the Z sensor, continuous pressure
#define DEFAULT_SENSOR_SENSITIVITY_Z  75       // by default the sensor Z sensitivity is unchanged, ie. 75%
#define DEFAULT_SENSOR_LO_Z           120      // lowest acceptable raw Z value to start a touch
#define DEFAULT_SENSOR_FEATHER_Z      80       // lowest acceptable raw Z value to continue a touch
#define DEFAULT_SENSOR_RANGE_Z        648      // default range of the pressure
#define MAX_SENSOR_RANGE_Z            1016     // upper value of the pressure

#define MAX_TOUCHES_IN_COLUMN  MAXROWS

// Sequencer constants
#define MAX_PROJECTS              16
#define MAX_SEQUENCERS            2
#define MAX_SEQUENCER_PATTERNS    4
#define MAX_SEQUENCER_STEPS       32
#define MAX_SEQUENCER_STEP_EVENTS 4
#define SEQ_DRUM_NOTES            14

// Pitch correction behavior
#define PITCH_CORRECT_HOLD_SAMPLES_FAST    8
#define PITCH_CORRECT_HOLD_SAMPLES_MEDIUM  48
#define PITCH_CORRECT_HOLD_SAMPLES_SLOW    350
#define PITCH_CORRECT_HOLD_SAMPLES_DEFAULT PITCH_CORRECT_HOLD_SAMPLES_MEDIUM

// Threshold below which the average rate of change of X is considered 'stationary'
#define RATEX_THRESHOLD_FAST    2.2
#define RATEX_THRESHOLD_MEDIUM  2.0
#define RATEX_THRESHOLD_SLOW    1.6
#define RATEX_THRESHOLD_DEFAULT RATEX_THRESHOLD_MEDIUM

#define SENSOR_PITCH_Z           173               // lowest acceptable raw Z value for which pitchbend is sent
#define ROGUE_SWEEP_X_THRESHOLD  48                // the maximum threshold of instant X changes since the previous sample, anything higher will be considered a rogue pitch sweep

// The values here MUST be the same as the row numbers of the cells in per-split settings
#define MIDICHANNEL_MAIN     7
#define MIDICHANNEL_PERNOTE  6
#define MIDICHANNEL_PERROW   5

// The values for the different LED layers
#define LED_LAYER_MAIN      0
#define LED_LAYER_CUSTOM1   1
#define LED_LAYER_CUSTOM2   2
#define LED_LAYER_LOWROW    3
#define LED_LAYER_PLAYED    4
#define LED_LAYER_SEQUENCER 5
#define LED_LAYER_COMBINED  6
#define MAX_LED_LAYERS      6

// The values here MUST be the same as the row numbers of the cells in GlobalSettings
#define LIGHTS_MAIN    0
#define LIGHTS_ACCENT  1
#define LIGHTS_ACTIVE  2

// The values of SWITCH_ here MUST be the same as the row numbers of the cells used to set them.
#define SWITCH_FOOT_L    0
#define SWITCH_FOOT_R    1
#define SWITCH_SWITCH_2  2
#define SWITCH_SWITCH_1  3
#define SWITCH_FOOT_B    4

#define ASSIGNED_OCTAVE_DOWN            0
#define ASSIGNED_OCTAVE_UP              1
#define ASSIGNED_SUSTAIN                2
#define ASSIGNED_CC_65                  3
#define ASSIGNED_ARPEGGIATOR            4
#define ASSIGNED_ALTSPLIT               5
#define ASSIGNED_AUTO_OCTAVE            6
#define ASSIGNED_TAP_TEMPO              7
#define ASSIGNED_LEGATO                 8
#define ASSIGNED_LATCH                  9
#define ASSIGNED_PRESET_UP              10
#define ASSIGNED_PRESET_DOWN            11
#define ASSIGNED_REVERSE_PITCH_X        12
#define ASSIGNED_SEQUENCER_PLAY         13
#define ASSIGNED_SEQUENCER_PREV         14
#define ASSIGNED_SEQUENCER_NEXT         15
#define ASSIGNED_STANDALONE_MIDI_CLOCK  16
#define ASSIGNED_SEQUENCER_MUTE         17
#define ASSIGNED_TRANSPOSE_DOWN         18
#define ASSIGNED_TRANSPOSE_UP           19
#define MAX_ASSIGNED                    ASSIGNED_TRANSPOSE_UP
#define ASSIGNED_DISABLED               255

#define GLOBAL_SETTINGS_ROW  0
#define SPLIT_ROW            1
#define SWITCH_2_ROW         2
#define SWITCH_1_ROW         3
#define OCTAVE_ROW           4
#define VOLUME_ROW           5
#define PRESET_ROW           6
#define PER_SPLIT_ROW        7

#define SWITCH_HOLD_DELAY  500
#define SENSOR_HOLD_DELAY  300

#define EDIT_MODE_HOLD_DELAY  1000
#define CONFIRM_HOLD_DELAY    800

#define DEFAULT_MIN_USB_MIDI_INTERVAL  DEFAULT_MIDI_INTERVAL

#define TEMPO_ARP_SIXTEENTH_SWING 0xff

static const unsigned short ccFaderDefaults[8] = {1, 2, 3, 4, 5, 6, 7, 8};

constexpr const int LED_PATTERNS = 3;

// Two buffers of ...
// A 26 by 8 byte array containing one byte for each LED:
// bits 3-6: 4 bits to select the color: 0:off, 1:red, 2:yellow, 3:green, 4:cyan, 5:blue, 6:magenta, etc.
// bits 0-2: 0:off, 1: on, 2: pulse
const unsigned long LED_LAYER_SIZE = MAXCOLS * MAXROWS;
const unsigned long LED_ARRAY_SIZE = (MAX_LED_LAYERS+1) * LED_LAYER_SIZE;

/******************************************** VELOCITY *******************************************/

#define VELOCITY_SAMPLES       4
#define VELOCITY_TOTAL_SAMPLES (VELOCITY_SAMPLES * 2)
#define VELOCITY_ZERO_POINTS   1
#define VELOCITY_N             (VELOCITY_SAMPLES + VELOCITY_ZERO_POINTS)
#define VELOCITY_SUMX          10   // x1 + x2 + x3 + ... + xn
#define VELOCITY_SUMXSQ        30   // x1^2 + x2^2 + x3^2 + ... + xn^2
#define VELOCITY_SCALE_LOW     43
#define VELOCITY_SCALE_MEDIUM  41
#define VELOCITY_SCALE_HIGH    40

#define DEFAULT_MIN_VELOCITY   1    // default minimum velocity value
#define DEFAULT_MAX_VELOCITY   127  // default maximum velocity value
#define DEFAULT_FIXED_VELOCITY 96   // default fixed velocity value


/*************************************** CONVENIENCE MACROS **************************************/

#define INVALID_DATA SHRT_MAX

// convenience macros to easily access the cells with touch information
#define cell(col, row)             touchInfo[col][row]
#define virtualCell()              virtualTouchInfo[sensorRow]

// calculate the difference between now and a previous timestamp, taking a possible single overflow into account
#define calcTimeDelta(now, last)   (now < last ? now + ~last : now - last)

// obtain the focused cell for a channel in a asplit
#define focus(split, channel)      focusCell[split][channel - 1]


/****************************************** TOUCH TRACKING ***************************************/

// Current cell in the scan routine
byte cellCount = 0;                         // the index of the cell that's currently being processed
byte sensorCol = 0;                         // currently read column in touch sensor
byte sensorRow = 0;                         // currently read row in touch sensor
byte sensorSplit = 0;                       // the split of the currently read touch sensor

// The most-recently touched cell within each channel of each split is said to have "focus",
// saved as the specific column and row for the focus cell.
// If in 1Ch/Poly mode, continuous X and Y messages are sent only from movements within the focused cell.
// If in 1Ch/Chan mode, continuous X, Y and Z messages are sent only from movements within the focused cell.
struct __attribute__ ((packed)) FocusCell {
  byte col:5;
  byte row:3;
};
FocusCell focusCell[NUMSPLITS][16];             // 2 splits and 16 MIDI channels for each split

enum VelocityState {
  velocityCalculating = 0,
  velocityCalculated = 1,
  velocityNew = 2
};

enum TouchState {
  untouchedCell = 0,
  ignoredCell = 1,
  transferCell = 2,
  touchedCell = 3
};

struct __attribute__ ((packed)) TouchInfo {
  void shouldRefreshData();                  // indicate that the X, Y and Z data should be refreshed
  unsigned short rawX();                     // ensure that X is updated to the latest scan and return its raw value
  short calibratedX();                       // ensure that X is updated to the latest scan and return its calibrated value
  inline void refreshX();                    // ensure that X is updated to the latest scan
  unsigned short rawY();                     // ensure that Y is updated to the latest scan and return its raw value
  byte calibratedY();                        // ensure that Y is updated to the latest scan and return its calibrated value
  inline void refreshY();                    // ensure that Y is updated to the latest scan
  unsigned short rawZ();                     // ensure that Z is updated to the latest scan and return its raw value
  inline boolean isMeaningfulTouch();        // ensure that Z is updated to the latest scan and check if it was a meaningful touch
  inline boolean isActiveTouch();            // ensure that Z is updated to the latest scan and check if it was an active touch
  inline boolean isStableYTouch();           // ensure that Z is updated to the latest scan and check if the touch is capable of providing stable Y reading
  inline void refreshZ();                    // ensure that Z is updated to the latest scan
  inline boolean isPastDebounceDelay();      // indicates whether the touch is past the debounce delay
  boolean hasNote();                         // check if a MIDI note is active for this touch
  void clearPhantoms();                      // clear the phantom coordinates
  void clearAllPhantoms();                   // clear the phantom coordinates of all the cells that are involved
  boolean hasPhantoms();                     // indicates whether there are phantom coordinates
  void setPhantoms(byte, byte, byte, byte);  // set the phantoom coordinates
  boolean isHigherPhantomPressure(short);    // checks whether this is a possible phantom candidate and has higher pressure than the argument
  boolean hasRogueSweepX();                  // indicates whether the current X information is a rogue sweep
  boolean hasUsableX();                      // indicates whether the X data is usable
  void clearMusicalData();                   // clear the musical data
  void clearSensorData();                    // clears the measured sensor data
  boolean isCalculatingVelocity();           // indicates whether the initial velocity is being calculated
  int32_t fxdInitialReferenceX();            // initial calibrated reference X value of each cell at the start of the touch

#ifdef TESTING_SENSOR_DISABLE
  boolean disabled;
#endif

  unsigned long lastTouch:32;                // the timestamp when this cell was last touched
  short initialX:16;                         // initial calibrated X value of each cell at the start of the touch, INVALID_DATA meaning that it's unassigned
  short initialColumn:16;                    // initial column of each cell at the start of the touch
  short quantizationOffsetX:16;              // quantization offset to be applied to the X value
  unsigned short currentRawX:16;             // last raw X value of each cell
  short currentCalibratedX:16;               // last calibrated X value of each cell
  short lastMovedX:16;                       // the last X movement, so that we can compare movement jumps
  short lastValueX:16;                       // the last calculated X value based on the current settings
  int32_t fxdRateX:32;                       // the averaged rate of change of the X values
  int32_t fxdRateCountX:32;                  // the number of times the rate of change drops below the minimal value for quantization
  int32_t fxdPrevPressure:32;                // used to average out the rate of change of the pressure when transitioning between cells
  int32_t fxdPrevTimbre:32;                  // used to average out the rate of change of the timbre
  signed char note:8;                        // note from 0 to 127, -1 meaning it's unassigned
  signed char channel:8;                     // channel from 1 to 16, -1 meaning it's unassigned
  signed char octaveOffset:8;                // the octave offset when the note started, since this can change during playing
  byte phantomCol1:5;                        // stores the col 1 of a rectangle that possibly has a phantom touch
  byte phantomRow1:3;                        // stores the row 1 of a rectangle that possibly has a phantom touch
  byte phantomCol2:5;                        // stores the col 2 of a rectangle that possibly has a phantom touch
  byte phantomRow2:3;                        // stores the row 2 of a rectangle that possibly has a phantom touch
  signed char initialY:8;                    // initial Y value of each cell, -1 meaning it's unassigned
  byte currentCalibratedY:7;                 // last calibrated Y value of each cell
  boolean shouldRefreshY:1;                  // indicate whether it's necessary to refresh Y
  unsigned short currentRawY:12;             // last raw Y value of each cell
  unsigned short currentRawZ:12;             // the raw Z value
  byte percentRawZ:7;                        // percentage of Z compared to the raw offset and range
  boolean shouldRefreshX:1;                  // indicate whether it's necessary to refresh X
  TouchState touched:2;                      // touch status of all sensor cells
  byte vcount:4;                             // the number of times the pressure was measured to obtain a velocity
  boolean slideTransfer:1;                   // indicates whether this touch is part of a slide transfer
  boolean rogueSweepX:1;                     // indicates whether the last X position is a rogue sweep
  byte pendingReleaseCount:4;                // counter before which the note release will be effective
  boolean featherTouch:1;                    // indicates whether this is a feather touch
  unsigned short pressureZ:10;               // the Z value with pressure sensitivity
  unsigned short previousRawZ:12;            // the previous raw Z value
  boolean didMove:1;                         // indicates whether the touch has ever moved
int :3;
  boolean phantomSet:1;                      // indicates whether phantom touch coordinates are set
  byte velocity:7;                           // velocity from 0 to 127
  boolean shouldRefreshZ:1;                  // indicate whether it's necessary to refresh Z
  byte velocityZ:7;                          // the Z value with velocity sensitivity
  unsigned short peakRawZ:12;                // peak raw Z seen during this touch, for ghost note release detection
};
TouchInfo touchInfo[MAXCOLS][MAXROWS];       // store as much touch information instances as there are cells

TouchInfo* sensorCell = &touchInfo[0][0];

int32_t rowsInColsTouched[MAXCOLS];          // keep track of which rows inside each column and which columns inside each row are touched, using a bitmask
int32_t colsInRowsTouched[MAXROWS];          // to makes it possible to quickly identify square formations that generate phantom presses
unsigned short cellsTouched;                 // counts the number of active touches on cells

struct VirtualTouchInfo {
  boolean hasNote();                         // check if a MIDI note is active for this touch
  void clearData();                          // clear the virtual touch data
  void releaseNote();                        // release the MIDI note that is active for the virtual touch

  byte split;                                // the split this virtual touch belongs to
  byte velocity;                             // velocity from 0 to 127
  signed char note;                          // note from 0 to 127
  signed char channel;                       // channel from 1 to 16
};
VirtualTouchInfo virtualTouchInfo[MAXROWS];  // store as much touch virtual instances as there are rows, this is used for simulating strumming open strings

// Reverse mapping to find the touch information based on the MIDI note and channel,
// this is used for the arpeggiator to know which notes are active and which cells
// to look at for continuous velocity calculation
struct NoteEntry {
  byte colRow;
  signed char nextNote;
  signed char previousNote;
  byte nextPreviousChannel;

  inline boolean hasColRow(byte, byte);
  inline void setColRow(byte, byte);
  inline byte getCol();
  inline byte getRow();
  inline boolean hasTouch();

  inline byte getNextNote();
  inline byte getNextChannel();
  inline byte getPreviousNote();
  inline byte getPreviousChannel();
  inline void setNextChannel(byte);
  inline void setPreviousChannel(byte);
};
struct NoteTouchMapping {
  void initialize(byte mappedSplit);                         // initialize the mapping data
  void releaseLatched();                                     // release all the note mappings that are latched and have no real active touch
  void noteOn(signed char, signed char, byte, byte);         // register the cell for which a note was turned on
  void noteOff(signed char, signed char);                    // turn off a note
  void changeCell(signed char, signed char, byte, byte);     // changes the cell of an active note
  boolean hasTouch(signed char, signed char);                // indicates whether there's a touch active for a particular note and channel
  inline NoteEntry* getNoteEntry(signed char, signed char);  // get the entry for a particular note and channel
  inline byte getMusicalTouchCount(signed char);             // the number of musical touches for a particular MIDI channel

  void debugNoteChain();

  unsigned char split;
  unsigned short noteCount;
  byte musicalTouchCount[16];
  signed char firstNote;
  signed char firstChannel;
  signed char lastNote;
  signed char lastChannel;
  NoteEntry mapping[128][16];
};
NoteTouchMapping noteTouchMapping[NUMSPLITS];


/**************************************** DISPLAY STATE ******************************************/

enum CellDisplay {
  cellOff = 0,
  cellOn = 1,
  cellFastPulse = 2,
  cellSlowPulse = 3,
  cellFocusPulse = 4,
  cellTempoPulse = 5
};

enum DisplayMode {
  displayNormal,
  displayPerSplit,
  displayPreset,
  displayVolume,
  displayOctaveTranspose,
  displaySplitPoint,
  displayGlobal,
  displayGlobalWithTempo,
  displayOsVersion,
  displayOsVersionBuild,
  displayCalibration,
  displayReset,
  displayBendRange,
  displayLimitsForY,
  displayCCForY,
  displayInitialForRelativeY,
  displayLimitsForZ,
  displayCCForZ,
  displayPlayedTouchModeConfig,
  displayCCForFader,
  displayLowRowBendConfig,
  displayLowRowCCXConfig,
  displayLowRowCCXYZConfig,
  displayCCForSwitchCC65,
  displayCCForSwitchSustain,
  displayCustomSwitchAssignment,
  displayLimitsForVelocity,
  displayValueForFixedVelocity,
  displayMinUSBMIDIInterval,
  displaySensorSensitivityZ,
  displaySensorLoZ,
  displaySensorFeatherZ,
  displaySensorRangeZ,
  displayAnimation,
  displayEditAudienceMessage,
  displaySleep,
  displaySleepConfig,
  displaySplitHandedness,
  displayRowOffset,
  displayGuitarTuning,
  displayMIDIThrough,
  displaySequencerProjects,
  displaySequencerDrum0107,
  displaySequencerDrum0814,
  displaySequencerColors,
  displayCustomLedsEditor
};
DisplayMode displayMode = displayNormal;


/***************************************** CALIBRATION *******************************************/

enum CalibrationPhase {
  calibrationInactive,
  calibrationRows,
  calibrationCols
};
byte calibrationPhase = calibrationInactive;

struct __attribute__ ((packed)) CalibrationSample {
  unsigned short minValue:12;
  unsigned short maxValue:12;
  byte pass:4;
};
CalibrationSample calSampleRows[MAXCOLS][4]; // store four rows of calibration measurements
CalibrationSample calSampleCols[9][MAXROWS]; // store nine columns of calibration measurements

struct CalibrationX {
  int32_t fxdMeasuredX;
  int32_t fxdReferenceX;
  int32_t fxdRatio;
};

struct __attribute__ ((packed)) CalibrationY {
  unsigned short minY:12;
  unsigned short maxY:12;
  int32_t fxdRatio;
};


/***************************************** PANEL SETTINGS ****************************************/

enum PlayedTouchMode {
  playedCell,
  playedSame,
  playedCrosses,
  playedCircles,
  playedSquares,
  playedDiamonds,
  playedStars,
  playedSparkles,
  playedCurtains,
  playedBlinds,
  playedTargets,
  playedUp,
  playedDown,
  playedLeft,
  playedRight,
  playedOrbits
};

enum LowRowMode {
  lowRowNormal,
  lowRowSustain,
  lowRowRestrike,
  lowRowStrum,
  lowRowArpeggiator,
  lowRowBend,
  lowRowCCX,
  lowRowCCXYZ
};

enum LowRowBendBehavior {
  lowRowBendBend = 0,
  lowRowBendTranspose = 1
};

enum LowRowCCBehavior {
  lowRowCCHold = 0,
  lowRowCCFader = 1
};

enum MidiMode {
  oneChannel,
  channelPerNote,
  channelPerRow
};

enum BendRangeOption {
  bendRange2,
  bendRange3,
  bendRange12,
  bendRange24
};

enum PitchCorrectHoldSpeed {
  pitchCorrectHoldOff = 0,
  pitchCorrectHoldMedium = 1,
  pitchCorrectHoldFast = 2,
  pitchCorrectHoldSlow = 3
};

enum TimbreExpression {
  timbrePolyPressure,
  timbreChannelPressure,
  timbreCC1,
  timbreCC74,
};

enum LoudnessExpression {
  loudnessPolyPressure,
  loudnessChannelPressure,
  loudnessCC11
};

enum SequencerView {
  sequencerNotes,
  sequencerScales,
  sequencerDrums
};

enum SequencerDirection {
  sequencerForward,
  sequencerBackward,
  sequencerPingPong
};

// per-split settings
struct SplitSettings {
  byte midiMode;                          // 0 = one channel, 1 = note per channel, 2 = row per channel
  byte midiChanMain;                      // main midi channel, 1 to 16
  boolean midiChanMainEnabled;            // true when the midi main channel is enabled to send common data, false in not
  byte midiChanPerRow;                    // per-row midi channel, 1 to 16
  boolean midiChanPerRowReversed;         // indicates whether channel per row channels count upwards or downwards across the rows
  boolean midiChanSet[16];                // Indicates whether each channel is used.  If midiMode!=channelPerNote, only one channel can be set.
  BendRangeOption bendRangeOption;        // see BendRangeOption
  byte customBendRange;                   // 1 - 96
  boolean sendX;                          // true to send continuous X, false if not
  boolean sendY;                          // true to send continuous Y, false if not
  boolean sendZ;                          // true to send continuous Z, false if not
  boolean pitchCorrectQuantize;           // true to quantize pitch of initial touch, false if not
  byte pitchCorrectHold;                  // See PitchCorrectHoldSpeed values
  boolean pitchResetOnRelease;            // true to enable pitch bend being set back to 0 when releasing a touch
  TimbreExpression expressionForY;        // the expression that should be used for timbre
  unsigned short customCCForY;            // 0-129 (with 128 and 129 being placeholders for PolyPressure and ChannelPressure)
  unsigned short minForY;                 // 0-127
  unsigned short maxForY;                 // 0-127
  boolean relativeY;                      // true when Y should be sent relative to the initial touch, false when it's absolute
  unsigned short initialRelativeY;        // 0-127
  LoudnessExpression expressionForZ;      // the expression that should be used for loudness
  unsigned short customCCForZ;            // 0-127
  unsigned short minForZ;                 // 0-127
  unsigned short maxForZ;                 // 0-127
  boolean ccForZ14Bit;                    // true when 14-bit messages should be sent when Z CC is between 0-31, false when only 7-bit messages should be sent
  unsigned short ccForFader[8];           // each fader can control a CC number ranging from 0-128 (with 128 being placeholder for ChannelPressure)
  byte colorMain;                         // color for non-accented cells
  byte colorAccent;                       // color for accented cells
  byte colorPlayed;                       // color for played notes
  byte colorLowRow;                       // color for low row if on
  byte colorSequencerEmpty;               // color for sequencer low row step with no events
  byte colorSequencerEvent;               // color for sequencer low row step with events
  byte colorSequencerDisabled;            // color for sequencer low row step that's not being played
  byte playedTouchMode;                   // see PlayedTouchMode values
  byte lowRowMode;                        // see LowRowMode values
  byte lowRowBendBehavior;                // see LowRowBendBehavior values
  byte lowRowCCXBehavior;                 // see LowRowCCBehavior values
  unsigned short ccForLowRow;             // 0-128 (with 128 being placeholder for ChannelPressure)
  byte lowRowCCXYZBehavior;               // see LowRowCCBehavior values
  unsigned short ccForLowRowX;            // 0-128 (with 128 being placeholder for ChannelPressure)
  unsigned short ccForLowRowY;            // 0-128 (with 128 being placeholder for ChannelPressure)
  unsigned short ccForLowRowZ;            // 0-128 (with 128 being placeholder for ChannelPressure)
  signed char transposeOctave;            // -60, -48, -36, -24, -12, 0, +12, +24, +36, +48, +60
  signed char transposePitch;             // transpose output midi notes. Range is -12 to +12
  signed char transposeLights;            // transpose lights on display. Range is -12 to +12
  boolean ccFaders;                       // true to activated 8 CC faders for this split, false for regular music performance
  boolean arpeggiator;                    // true when the arpeggiator is on, false if notes should be played directly
  boolean strum;                          // true when this split strums the touches of the other split
  boolean mpe;                            // true when MPE is active for this split
  boolean sequencer;                      // true when the sequencer of this split is displayed
  SequencerView sequencerView;            // see SequencerView
};

#define Split config.settings.split

enum SleepAnimationType {
  animationNone,
  animationStore,
  animationChristmas
};

enum SplitHandednessType {
  reversedBoth,
  reversedLeft,
  reversedRight
};

struct DeviceSettings {
  byte version;                                   // the version of the configuration format
  boolean serialMode;                             // 0 = normal MIDI I/O, 1 = Arduino serial mode for OS update and serial monitor
  CalibrationX calRows[MAXCOLS+1][4];             // store four rows of calibration data
  CalibrationY calCols[9][MAXROWS];               // store nine columns of calibration data
  uint32_t calCrc;                                // the CRC check value of the calibration data to see if it's still valid
  boolean calCrcCalculated;                       // indicates whether the CRC of the calibration was calculated, previous firmware versions didn't
  boolean calibrated;                             // indicates whether the calibration data actually resulted from a calibration operation
  boolean calibrationHealed;                      // indicates whether the calibration data was healed
  unsigned short minUSBMIDIInterval;              // the minimum delay between MIDI bytes when sent over USB
  byte sensorSensitivityZ;                        // the scaling factor of the raw value of Z in percentage
  unsigned short sensorLoZ;                       // the lowest acceptable raw Z value to start a touch
  unsigned short sensorFeatherZ;                  // the lowest acceptable raw Z value to continue a touch
  unsigned short sensorRangeZ;                    // the maximum raw value of Z
  boolean sleepAnimationActive;                   // store whether an animation was active last
  boolean sleepActive;                            // store whether LinnStrument should go to sleep automatically
  byte sleepDelay;                                // the number of minutes it takes for sleep to kick in
  byte sleepAnimationType;                        // the animation type to use during sleep, see SleepAnimationType
  char audienceMessages[16][31];                  // the 16 audience messages that will scroll across the surface
  boolean operatingLowPower;                      // whether low power mode is active or not
  boolean otherHanded;                            // whether change the handedness of the splits
  byte splitHandedness;                           // see SplitHandednessType
  boolean midiThrough;                            // false if incoming MIDI should be isolated, true if it should be passed through to the outgoing MIDI port
  short lastLoadedPreset;                         // the last settings preset that was loaded
  short lastLoadedProject;                        // the last sequencer project that was loaded
  byte customLeds[LED_PATTERNS][LED_LAYER_SIZE];  // the custom LEDs that persist across power cycle
};
#define Device config.device

// The values here MUST match the row #'s for the leds that get lit up in GlobalSettings
enum VelocitySensitivity {
  velocityLow,
  velocityMedium,
  velocityHigh,
  velocityFixed
};

// The values here MUST match the row #'s for the leds that get lit up in GlobalSettings
enum PressureSensitivity {
  pressureLow,
  pressureMedium,
  pressureHigh
};

enum ArpeggiatorStepTempo {
  ArpFourth = 0,
  ArpEighth = 1,
  ArpEighthTriplet = 2,
  ArpSixteenth = 3,
  ArpSixteenthSwing = 4,
  ArpSixteenthTriplet = 5,
  ArpThirtysecond = 6,
  ArpThirtysecondTriplet = 7,
  ArpSixtyfourthTriplet = 8,
};

enum ArpeggiatorDirection {
  ArpUp,
  ArpDown,
  ArpUpDown,
  ArpRandom,
  ArpReplayAll
};

enum SustainBehavior {
  sustainHold,
  sustainLatch
};

struct GlobalSettings {
  void setSwitchAssignment(byte, byte, boolean);

  byte splitPoint;                           // leftmost column number of right split (0 = leftmost column of playable area)
  byte currentPerSplit;                      // controls which split's settings are being displayed
  byte activeNotes;                          // controls which collection of note lights presets is active
  int mainNotes[12];                         // bitmask array that determines which notes receive "main" lights
  int accentNotes[12];                       // bitmask array that determines which notes receive accent lights (octaves, white keys, black keys, etc.)
  byte rowOffset;                            // interval between rows. 0 = no overlap, 1-12 = interval, 13 = guitar
  signed char customRowOffset;               // the custom row offset that can be configured at the location of the octave setting
  byte guitarTuning[MAXROWS];                // the notes used for each row for the guitar tuning, 0-127
  VelocitySensitivity velocitySensitivity;   // See VelocitySensitivity values
  unsigned short minForVelocity;             // 1-127
  unsigned short maxForVelocity;             // 1-127
  unsigned short valueForFixedVelocity;      // 1-127
  PressureSensitivity pressureSensitivity;   // See PressureSensitivity values
  boolean pressureAftertouch;                // Indicates whether pressure should behave like traditional piano keyboard aftertouch or be continuous from the start
  byte switchAssignment[5];                  // The element values are ASSIGNED_*.  The index values are SWITCH_*.
  boolean switchBothSplits[5];               // Indicate whether the switches should operate on both splits or only on the focused one
  unsigned short ccForSwitchCC65[5];         // 0-127
  unsigned short ccForSwitchSustain[5];      // 0-127
  unsigned short customSwitchAssignment[5];  // ASSIGNED_TAP_TEMPO, ASSIGNED_LEGATO, ASSIGNED_LATCH, ASSIGNED_PRESET_UP, ASSIGNED_PRESET_DOWN, ASSIGNED_REVERSE_PITCH_X, ASSIGNED_SEQUENCER_PLAY, ASSIGNED_SEQUENCER_PREV, ASSIGNED_SEQUENCER_NEXT, ASSIGNED_STANDALONE_MIDI_CLOCK and ASSIGNED_SEQUENCER_MUTE
  byte midiIO;                               // 0 = MIDI jacks, 1 = USB
  ArpeggiatorDirection arpDirection;         // the arpeggiator direction that has to be used for the note sequence
  ArpeggiatorStepTempo arpTempo;             // the multiplier that needs to be applied to the current tempo to achieve the arpeggiator's step duration
  signed char arpOctave;                     // the number of octaves that the arpeggiator has to operate over: 0, +1, or +2
  SustainBehavior sustainBehavior;           // the way the sustain pedal influences the notes
  boolean splitActive;                       // false = split off, true = split on
};
#define Global config.settings.global

struct PresetSettings {
  GlobalSettings global;
  SplitSettings split[NUMSPLITS];
};

enum SequencerStepSize {
  StepSixteenthTriplet = 4,
  StepSixteenth = 6,
  StepEighthTriplet = 8,
  StepSixteenthDotted = 9,
  StepEighth = 12,
  StepFourthTriplet = 16,
  StepEighthDotted = 18,
  StepFourth = 24,
  StepFourthDotted = 36
};

struct StepEvent {
  boolean hasData();
  void clear();

  void setNewEvent(byte note, byte velocity, unsigned short duration, byte timbre, byte row);
  byte getNote();
  void setNote(byte note);
  unsigned short getDuration();
  void setDuration(unsigned short duration);
  byte getVelocity();
  void setVelocity(byte velocity);
  signed char getPitchOffset();
  void setPitchOffset(signed char pitchOffset);
  byte getTimbre();
  void setTimbre(byte timbre);
  byte getRow();
  void setRow(byte row);
  int getFaderValue(byte fader);
  void setFaderValue(byte fader, int value);
  int getFaderMin(byte fader);
  int getFaderMax(byte fader);
  int getFaderNeutral(byte fader, byte split);
  boolean calculateSequencerFaderValue(boolean newVelocity);

  void operator=(const StepEvent& e);

  // the bit-wise arrangement is like below,
  // we can't rely on structure packing since
  // it will align each element on byte boundaries
  // byte note:7;                // 0 to 127
  // byte duration:10;           // 1 to 768 in 24 PPQ ticks
  // byte velocity:7;            // 1 to 127
  // signed char pitchOffset:8;  // -96 to 96 semitones
  // byte timbre:7;              // 0 to 127
  // byte row:3;                 // 1 to 7
  byte data[6];
};
struct StepData {
  void clear();

  void operator=(const StepData& d);
  
  StepEvent events[MAX_SEQUENCER_STEP_EVENTS];  // the events for each step
};
struct SequencerPattern {
  void clear();

  void operator=(const SequencerPattern& p);

  StepData steps[MAX_SEQUENCER_STEPS];
  SequencerStepSize stepSize;             // see SequencerStepSize
  SequencerDirection sequencerDirection;  // see SequencerDirection
  boolean loopScreen;                     // on or off
  boolean swing;                          // on or off
  byte length;                            // between 1 to 32 steps
};
struct StepSequencer {
  SequencerPattern patterns[MAX_SEQUENCER_PATTERNS];  // patterns available for each sequencer
  byte seqDrumNotes[SEQ_DRUM_NOTES];                  // note numbers from 0 to 127
};
struct SequencerProject {
  StepSequencer sequencer[MAX_SEQUENCERS];            // the sequencers available in a project
  unsigned short tempo;
};
#define Project config.project

#define NUMPRESETS 6
struct Configuration {
  DeviceSettings device;
  PresetSettings settings;
  PresetSettings preset[NUMPRESETS];
  SequencerProject project;
};
struct Configuration config;

/**************************************** SECRET SWITCHES ****************************************/

#define SECRET_SWITCHES 8
#define SWITCH_DEBUGMIDI   secretSwitch[0]
#define SWITCH_XFRAME      secretSwitch[1]
#define SWITCH_YFRAME      secretSwitch[2]
#define SWITCH_ZFRAME      secretSwitch[3]
#define SWITCH_SURFACESCAN secretSwitch[4]
#define SWITCH_FREERAM     secretSwitch[5]
#define SWITCH_MCU_PINS    secretSwitch[6]
#define SWITCH_TOUCHFRAME  secretSwitch[7] 

boolean secretSwitch[SECRET_SWITCHES] = {0};  // The secretSwitch* values are controlled by cells in column 18


/***************************************** OPERATING MODE ****************************************/

enum OperatingMode {
  modePerformance,
  modeManufacturingTest,
  modeFirmware
};
OperatingMode operatingMode = modePerformance;


/************************************** FLASH STORAGE LAYOUT *************************************/

static constexpr inline int alignToWord32Boundary(int value) {
#if 0
  if (value % 4 == 0) {
    return value;
  }

  return ((value / 4) + 1) * 4;
#else
  return int((value + 3) / 4) * 4;
#endif
}

constexpr const int PROJECTS_OFFSET = 4;
constexpr const int PROJECT_VERSION_MARKER_SIZE = 4;
constexpr const int PROJECT_INDEXES_COUNT = 20;
constexpr const int PROJECTS_MARKERS_SIZE = alignToWord32Boundary(PROJECT_VERSION_MARKER_SIZE + 2 * PROJECT_INDEXES_COUNT);    // one version marker, two series on indexes for project references
constexpr const int SINGLE_PROJECT_SIZE = alignToWord32Boundary(sizeof(SequencerProject));
constexpr const int ALL_PROJECTS_SIZE = PROJECTS_MARKERS_SIZE + (MAX_PROJECTS + 1) * SINGLE_PROJECT_SIZE;
constexpr const int SETTINGS_OFFSET = PROJECTS_OFFSET + alignToWord32Boundary(ALL_PROJECTS_SIZE);

#define PROJECT_INDEX_OFFSET(marker, index)   (PROJECTS_OFFSET + PROJECT_VERSION_MARKER_SIZE + marker * PROJECT_INDEXES_COUNT + index)


/**************************************** FIXED POINT MATH ***************************************/

#define FXD_FBITS        8
#define FXD_FROM_INT(a)  (int32_t)((a) << FXD_FBITS)
#define FXD_MAKE(a)      (int32_t)((a*(1 << FXD_FBITS)))

inline int FXD_TO_INT(int32_t a) {
  a = a + ((a & (int32_t)1 << (FXD_FBITS-1)) << 1);   // rounding instead of truncation
  return ((a) >> FXD_FBITS);
}

inline int32_t FXD_MUL(int32_t a, int32_t b) {
  int32_t t = a * b;
  t = t + ((t & (int32_t)1 << (FXD_FBITS-1)) << 1);   // rounding instead of truncation
  return t >> FXD_FBITS;
}

constexpr inline int32_t FXD_DIV(int32_t a, int32_t b) {
  return ((int32_t)a << FXD_FBITS) / (int32_t)b;
}

// these macros have lower precision, but can be used for larger numbers when doing mult and div operations

#define FXD4_FBITS        4
#define FXD4_FROM_INT(a)  (int32_t)((a) << FXD4_FBITS)
#define FXD4_MAKE(a)      (int32_t)((a*(1 << FXD4_FBITS)))

inline int FXD4_TO_INT(int32_t a) {
  a = a + ((a & (int32_t)1 << (FXD4_FBITS-1)) << 1);   // rounding instead of truncation
  return ((a) >> FXD4_FBITS);
}

inline int32_t FXD4_MUL(int32_t a, int32_t b) {
  int32_t t = a * b;
  t = t + ((t & (int32_t)1 << (FXD4_FBITS-1)) << 1);   // rounding instead of truncation
  return t >> FXD4_FBITS;
}

inline int32_t FXD4_DIV(int32_t a, int32_t b) {
  return ((int32_t)a << FXD4_FBITS) / (int32_t)b;
}

const int32_t FXD_CONST_1 = FXD_FROM_INT(1);
const int32_t FXD_CONST_2 = FXD_FROM_INT(2);
const int32_t FXD_CONST_3 = FXD_FROM_INT(3);
const int32_t FXD_CONST_50 = FXD_FROM_INT(50);
const int32_t FXD_CONST_99 = FXD_FROM_INT(99);
const int32_t FXD_CONST_100 = FXD_FROM_INT(100);
const int32_t FXD_CONST_127 = FXD_FROM_INT(127);
const int32_t FXD_CONST_255 = FXD_FROM_INT(255);
const int32_t FXD_CONST_1016 = FXD_FROM_INT(1016);

const int CALX_VALUE_MARGIN = 85;                         // 4095 / 48
const int32_t FXD_CALX_HALF_UNIT = FXD_MAKE(85.3125);     // 4095 / 48
const int32_t FXD_CALX_PHANTOM_RANGE = FXD_MAKE(170);     // full cell width (~4095 / 24), accept any X within cell bounds
const int32_t FXD_CALX_FULL_UNIT = FXD_MAKE(170.625);     // 4095 / 24
const int32_t CALX_QUARTER_UNIT = FXD_TO_INT(FXD_CALX_FULL_UNIT) / 4;

const int32_t FXD_CALY_FULL_UNIT = FXD_FROM_INT(127);     // range of 7-bit CC


/*************************************** OTHER RUNTIME STATE *************************************/

DueFlashStorage dueFlashStorage;                    // access to the persistent flash storage

boolean setupDone = false;                          // indicates whether the setup routine is finished

signed char debugLevel = -1;                        // level of debug messages that should be printed
boolean firstTimeBoot = false;                      // this will be true when the LinnStrument booted up the first time after a firmware upgrade
boolean globalReset = false;                        // this will be true when the LinnStrument was just globally reset
unsigned long lastReset;                            // the last time a reset was started

short lastReadSensorRawZ = 0;                       // the last pressure value that was read straight off of the sensor without any sensor bias nor sensitivity calibration

constexpr const byte globalColor = COLOR_BLUE;                      // color for global, split point and transpose settings
constexpr const byte globalAltColor = COLOR_CYAN;                   // alternate color for global, split point and transpose settings
constexpr const byte globalLowRowColor = COLOR_GREEN;               // color for low row painting in global settings

boolean changedSplitPoint = false;                  // reflects whether the split point was changed
boolean splitButtonDown = false;                    // reflects state of Split button

signed char controlButton = -1;                     // records the row of the current controlButton being held down
unsigned long lastControlPress[MAXROWS];

byte mainLoopDivider = DEFAULT_MAINLOOP_DIVIDER;         // loop divider at which continuous tasks are ran
unsigned long ledRefreshInterval = DEFAULT_LED_REFRESH;  // LED timing
unsigned long prevLedTimerCount;                         // timer for refreshing leds
unsigned long prevTouchAnimTimerCount;                   // timer for refreshing the touch animation

boolean customLedPatternActive = false;                  // was a custom led pattern loaded from flash

ChannelBucket splitChannels[NUMSPLITS];             // the MIDI channels that are being handed out
unsigned short midiPreset[NUMSPLITS];               // preset number 0-127
byte ccFaderValues[NUMSPLITS][129];                 // the current values of the CC faders
byte currentEditedCCFader[NUMSPLITS];               // the current CC fader number that is being edited
signed char arpTempoDelta[NUMSPLITS];               // ranges from -24 to 24 to apply a speed difference to the selected arpeggiator speed

unsigned long lastSwitchPress[5];                     // the last moment a particular switch was pressed (in milliseconds)
boolean switchState[5][NUMSPLITS];                    // the current state of each switch for each split
boolean switchTargetEnabled[NUMSPLITS][MAX_ASSIGNED]; // we keep track of switch targets individually for each split and whether they're active
boolean switchCCEnabled[NUMSPLITS][128];              // we keep track of the switch targets that send out CC numbers for each split to determine whether they're active
boolean footSwitchState[5];                           // holds the last read footswitch state, so that we only react on state changes of the input signal (L/R/B -- SW1/SW2 slots are unused in this array)
boolean footSwitchOffState[2];                        // holds the OFF state of both foot switches, as read at startup, thereby permitting normally-closed or normally-open switches (L/R)
unsigned long prevFootSwitchTimerCount;               // time interval (in microseconds) between foot switch reads
boolean switchFootBothReleased = false;               // keep track of whether the last release was for both switches, in order to prevent individual releases to happen

boolean doublePerSplit = false;                     // false when only one per split is active, true if they both are

byte switchSelect = SWITCH_FOOT_L;                  // determines which switch setting is being displayed/changed
byte midiChannelSelect = MIDICHANNEL_MAIN;          // determines which midi channel setting is being displayed/changed
byte lightSettings = LIGHTS_MAIN;                   // determines which Lights array is being displayed/changed

boolean userFirmwareActive = false;                 // indicates whether user firmware mode is active or not
boolean userFirmwareSlideMode[MAXROWS];             // indicates whether slide mode is on for a particular row
boolean userFirmwareXActive[MAXROWS];               // indicates whether X data is on for a particular row
boolean userFirmwareYActive[MAXROWS];               // indicates whether Y data is on for a particular row
boolean userFirmwareZActive[MAXROWS];               // indicates whether Z data is on for a particular row

boolean animationActive = false;                    // indicates whether animation is active, preventing any other display
boolean stopAnimation = false;                      // indicates whether animation should be stopped

int32_t fxd4CurrentTempo = FXD4_FROM_INT(120);               // the current tempo
unsigned long midiDecimateRate = DEFAULT_MIDI_DECIMATION;    // default MIDI decimation rate
unsigned long midiMinimumInterval = DEFAULT_MIDI_INTERVAL;   // minimum interval between sending two MIDI bytes
byte lastValueMidiNotesOn[NUMSPLITS][128][16];               // for each split, keep track of MIDI note on to filter out note off messages that are not needed
int32_t fxdPitchHoldSamples[NUMSPLITS];                      // for each split the actual pitch hold duration in samples
int32_t fxdRateXThreshold[NUMSPLITS];                        // the threshold below which the average rate of change of X is considered 'stationary' and pitch hold quantization will start to occur
int latestNoteNumberForAutoOctave = -1;                      // keep track of the latest note number that was generated to use for auto octave switching

byte audienceMessageToEdit = 0;                     // the audience message to edit with that mode is active
short audienceMessageOffset = 0;                    // the offset in columns for printing the edited audience message
short audienceMessageLength = 0;                    // the length in pixels of the audience message to edit

int32_t fxdLimitsForYRatio[NUMSPLITS];              // the ratio to convert the full range of Y into the range applied by the limits
int32_t fxdLimitsForZRatio[NUMSPLITS];              // the ratio to convert the full range of Z into the range applied by the limits

int32_t fxdMinVelOffset;                            // the offset to apply to the velocity values
int32_t fxdVelRatio;                                // the ratio to convert the full range of velocity into the range applied by the limits

byte limitsForYConfigState = 1;                     // the last state of the Y value limit configuration, this counts down to go to further pages
byte limitsForZConfigState = 2;                     // the last state of the Z value limit configuration, this counts down to go to further pages
byte limitsForVelocityConfigState = 1;              // the last state of the velocity value limit configuration, this counts down to go to further pages
byte lowRowCCXConfigState = 1;                      // the last state of the advanced low row CCX configuration, this counts down to go to further pages
byte lowRowCCXYZConfigState = 3;                    // the last state of the advanced low row CCXYZ configuration, this counts down to go to further pages
byte sleepConfigState = 1;                          // the last state of the sleep configuration, this counts down to go to further pages

unsigned long presetBlinkStart[NUMPRESETS];         // the moments at which the preset LEDs started blinking
unsigned long projectBlinkStart[MAX_PROJECTS];      // the moments at which the project LEDs started blinking

boolean controlModeActive = false;                  // indicates whether control mode is active, detecting no expression but very sensitive cell presses intended for fast typing

unsigned long lastTouchMoment = 0;                  // last time someone touched LinnStrument in milliseconds

unsigned short clock24PPQ = 0;                      // the current clock in 24PPQ, either internal or synced to incoming MIDI clock

short restrictedRow = -1;                           // temporarily restrict touches to a particular row

byte guitarTuningRowNum = 0;                        // active row number for configuring the guitar tuning
short guitarTuningPreviewNote = -1;                 // active note that is previewing the guitar tuning pitch
short guitarTuningPreviewChannel = -1;              // active channel that is previewing the guitar tuning pitch

byte customLedColor = COLOR_GREEN;                  // color is used for drawing in the custom LED editor

/************************* FUNCTION DECLARATIONS TO WORK AROUND COMPILER *************************/

inline void selectSensorCell(byte col, byte row, byte switchCode);

inline void setLed(byte col, byte row, byte color, CellDisplay disp);
inline void setLed(byte col, byte row, byte color, CellDisplay disp, byte layer);
void initializeNoteLights(GlobalSettings& g);

boolean ensureCellBeforeHoldWait(byte resetColor, CellDisplay resetDisplay);

void setDisplayMode(DisplayMode mode);
void exitDisplayMode(DisplayMode mode);

void applyBendRange(SplitSettings& target, byte bendRange);

inline void cellTouched(TouchState state);
void cellTouched(byte col, byte row, TouchState state);

VelocityState calcVelocity(unsigned short z);

inline unsigned short readZ();
inline short applyRawZBias(short rawZ);
inline unsigned short calculateSensorRangeZ();
inline unsigned short calculatePreferredPressureRange(unsigned short sensorRangeZ);

/********************************************** SETUP ********************************************/

void reset() {
  lastReset = millis();

  Global.currentPerSplit = LEFT;
  Global.splitActive = false;

  controlButton = -1;
  for (byte i = 0; i < NUMROWS; ++i) {
    lastControlPress[i] = 0;
  }

  initializeLedLayers();

  initializeTouchAnimation();

  initializeTouchHandling();

  initializeTouchInfo();

  initializeLowRowState();

  initializeDeviceSettings();

  initializePresetSettings();

  initializeClock();

  initializeArpeggiator();

  initializeLastMidiTracking();

  initializeSwitches();
}

boolean switchPressAtStartup(byte switchRow) {
  sensorCol = 0;
  sensorRow = switchRow;
  updateSensorCell();
  // initially we need read Z a few times for the readings to stabilize
  readZ(); readZ(); unsigned short switchZ = readZ();
  if (switchZ > Device.sensorLoZ + 128) {
    return true;
  }
  return false;
}

void activateSleepMode() {
  clearSwitches();
  disableLedDisplay(); // clearDisplayImmediately();
  setDisplayMode(displaySleep);
}

void applyLedInterval() {
  // change the behavior for low power mode
  if (Device.operatingLowPower) {
    mainLoopDivider = LOWPOWER_MAINLOOP_DIVIDER;
    ledRefreshInterval = LOWPOWER_LED_REFRESH;
  }
  else {
    mainLoopDivider = DEFAULT_MAINLOOP_DIVIDER;
    ledRefreshInterval = DEFAULT_LED_REFRESH;
  }
}

void applyMidiInterval() {
  if (isMidiUsingDIN()) {
    // 256 microseconds between bytes on Serial ports
    midiMinimumInterval = 256;
  }
  else {
    midiMinimumInterval = Device.minUSBMIDIInterval;
  }

  if (Device.operatingLowPower && midiMinimumInterval < LOWPOWER_MIDI_INTERVAL) {
    midiMinimumInterval = LOWPOWER_MIDI_INTERVAL;
  }

  applyMidiDecimationRate();
}

void applyMidiDecimationRate() {
  // this is just a number made up with lots of testing in order to avoid having
  // too many MIDI messages backing up in the outgoing queue
  midiDecimateRate = midiMinimumInterval * 34;

  if (Device.operatingLowPower && midiDecimateRate < LOWPOWER_MIDI_DECIMATION) {
    midiDecimateRate = LOWPOWER_MIDI_DECIMATION;
  }
}

void applyMpeMode() {
  for (byte s = 0; s < NUMSPLITS; ++s) {
    if (Split[s].mpe) {
      if (Split[s].midiChanMainEnabled) {
        midiSendMpeState(Split[s].midiChanMain, countMpePolyphony(s));
      }
      midiSendMpePitchBendRange(s);
    }
  }
}

void setup() {
  //*************************************************************************************************************************************************
  //**************** IMPORTANT, DONT CHANGE ANYTHING REGARDING THIS CODE BLOCK AT THE RISK OF BRICKING THE LINNSTRUMENT !!!!! ***********************
  //*************************************************************************************************************************************************
  /*!!*/
  /*!!*/  pinMode(38, INPUT);
  /*!!*/  if (digitalRead(38) == HIGH) {
  /*!!*/    LINNMODEL = 200;
  /*!!*/    NUMCOLS = 26;
  /*!!*/    //NUMROWS = 8;
  /*!!*/  }
  /*!!*/  else {
  /*!!*/    LINNMODEL = 128;
  /*!!*/    NUMCOLS = 17;
  /*!!*/    //NUMROWS = 8;
  /*!!*/  }
  /*!!*/
  /*!!*/  initializeSensors();
  /*!!*/  initializeCalibration();
  /*!!*/  initializeLeds();
  /*!!*/
  /*!!*/  // Initialize output pin 35 (midi/SERIAL) and set it HIGH for serial operation
  /*!!*/  // IMPORTANT: IF YOU UPLOAD DEBUG CODE THAT DISABLES THE UI'S ABILITY TO SET THIS BACK TO SERIAL MODE, YOU WILL BRICK THE LINNSTRUMENT!!!!!
  /*!!*/  pinMode(35, OUTPUT);
  /*!!*/  digitalWrite(35, HIGH);
  /*!!*/
  /*!!*/  // Enabled DIN MIDI output first since otherwise the DIN output will have a continuous stream of junk data if USB would be active from the
  /*!!*/  // startup of the LinnStrument
  /*!!*/  pinMode(36, OUTPUT);
  /*!!*/  digitalWrite(36, LOW);                             // set to use DIN jack
  /*!!*/
  /*!!*/  // Initialize output pin 36 (din/USB) and set it HIGH for USB operation
  /*!!*/  // IMPORTANT: IF YOU UPLOAD DEBUG CODE THAT DISABLES THE UI'S ABILITY TO SET THIS BACK TO USB MODE, YOU WILL BRICK THE LINNSTRUMENT!!!!!
  /*!!*/  pinMode(36, OUTPUT);
  /*!!*/  digitalWrite(36, HIGH);                             // set to use USB jack
  /*!!*/
  /*!!*/  // initialize the SPI port for setting one column of LEDs
  /*!!*/  SPI.begin(SPI_LEDS);
  /*!!*/  SPI.setDataMode(SPI_LEDS, SPI_MODE0);
          SPI.setDataWidth(SPI_LEDS, SPI_CSR_BITS_16_BIT);
  /*!!*/  SPI.setClockDivider(SPI_LEDS, 4);                   // max clock is about 20 mHz. 4 = 21 mHz. Transferring all 4 bytes takes 1.9 uS.
  /*!!*/

  startHFLEDpaintTimer(1 /* Hz */);

  /*!!*/  // initialize the SPI port for setting analog switches in touch sensor
  /*!!*/  SPI.begin(SPI_SENSOR);
  /*!!*/  SPI.setDataMode(SPI_SENSOR, SPI_MODE0);
  /*!!*/  SPI.setClockDivider(SPI_SENSOR, 4);                 // set clock speed to 84/4 = 21 mHz. Max clock is 25mHz @ 4.5v
  /*!!*/  selectSensorCell(0, 0, READ_Z);                     // set its analog switches to read column 0, row 0 and to read pressure
            SPI.setDataWidth(SPI_SENSOR, SPI_CSR_BITS_16_BIT);

  /*!!*/
  /*!!*/  // initialize the SPI input port for reading the TI ADS7883 ADC
  /*!!*/  SPI.begin(SPI_ADC);
  /*!!*/  SPI.setDataMode(SPI_ADC, SPI_MODE0);
  /*!!*/  SPI.setClockDivider(SPI_ADC, 4);                    // set speed to 84/4 = 21 mHz. Max clock for ADC is 32 mHz @ 2.7-4.5v, 48mHz @ 4.5-5.5v
  /*!!*/
          SPI.setDataWidth(SPI_ADC, SPI_CSR_BITS_16_BIT);

  /*!!*/  // Initialize the output enable line for the 2 LED display chips
  /*!!*/  pinMode(37, OUTPUT);
  /*!!*/  digitalWrite(37, HIGH); // clearDisplayImmediately();
  /*!!*/
  /*!!*/  if (switchPressAtStartup(GLOBAL_SETTINGS_ROW)) {
  /*!!*/    // if the global settings and switch 2 buttons are pressed at startup, the LinnStrument will do a global reset
  /*!!*/    if (switchPressAtStartup(SWITCH_2_ROW)) {
  /*!!*/      globalReset = true;
  /*!!*/      dueFlashStorage.write(0, 254);
  /*!!*/    }
  /*!!*/    // if only the global settings button is pressed at startup, activate firmware upgrade mode
  /*!!*/    else {
  /*!!*/      operatingMode = modeFirmware;
  /*!!*/  
  /*!!*/      // use serial and not MIDI
  /*!!*/      pinMode(35, OUTPUT);
  /*!!*/      digitalWrite(35, HIGH);
  /*!!*/  
  /*!!*/      // use USB connection and not DIN
  /*!!*/      pinMode(36, OUTPUT);
  /*!!*/      digitalWrite(36, HIGH);
  /*!!*/  
  /*!!*/      clearDisplay();
  /*!!*/  
  /*!!*/      adaptfont_draw_string(1, 0, LINNMODEL == 200 ? "FWUP" : "FWU", COLOR_RED);
  /*!!*/      return;
  /*!!*/    }
  /*!!*/  }
  /*!!*/
  //*************************************************************************************************************************************************








#if 0

  /*
     43.5.4:: Temperature Sensor

     The temperature sensor is connected to Channel 15 of the ADC.
     The temperature sensor provides an output voltage VT that is proportional to absolute temperature (PTAT). To
     activate the temperature sensor, TSON bit (ADC_ACR) needs to be set.

     Note: ADC_ACR: This register can only be written if the WPEN bit is cleared in “ADC Write Protect Mode Register” on page 1353.

     Notes: 
     1. Use ADC_ACR.IBCTL = 00 for sampling frequency below 500 kHz.
     2. Use ADC_ACR.IBCTL = 01 for sampling frequency between 500 kHz and 1 MHz.

     45.8:: Temperature Sensor

     The temperature sensor is connected to channel 15 of the ADC.
     The temperature sensor provides an output voltage (VO_TS) that is proportional to absolute temperature (PTAT).
     VO_TS linearly varies with a temperature slope dVO_TS/dT = 2.65 mV/°C.
     VO_TS equals 0.8V at TA 27°C, with a ±15% accuracy. The VO_TS slope versus temperature dVO_TS/dT = 2.65
     mV/°C only shows a ±5% slight variation over process, mismatch and supply voltage.
     The user needs to calibrate it (offset calibration) at ambient temperature to eliminate the VO_TS spread at ambient
     temperature (±15%).

     Table 45-39 :: Temperature Sensor Characteristics
     Symbol              Parameter                 Conditions      Min      Typ      Max       Unit
     VO_TS               Output Voltage            TA = 27° C               0.800              V
     VO_TS(accuracy)     Output Voltage Accuracy   TA = 27° C      -15               +15       %
     dVO_TS/dT           Temperature Sensitivity (Slope Voltage vs Temperature)
                                                                            2.65               mV/°C
                         Slope accuracy                            -5                +5        %
                         Temperature accuracy
                         After offset calibration
                         Over temperature range -40 to 85 °C       -5                +5        °C
                         After offset calibration
                         Over temperature range 0 to 80 °C         -3                +3        °C
     tSTART              Startup Time              After ADC_ACR.TSON=1
                                                                   20                40        µs

     43.5.3:: Analog Inputs

     The analog input pins can be multiplexed with PIO lines. In this case, the assignment of the ADC input is
     automatically done as soon as the corresponding channel is enabled by writing the register ADC_CHER. By
     default, after reset, the PIO line is configured as input with its pull-up enabled and the ADC input is connected to
     the GND.                                                                   
  */
  REG_ADC_WPMR = (0x414443 << 8) | 0;       // WPEN = 0: enable write access
  REG_ADC_ACR = 0 | (0b00 << 8) | (1 << 4); // IBCTL = 00, TSON = 1 :: TSON: Temperature Sensor On
  REG_ADC_WPMR = (0x414443 << 8) | 1;       // WPEN = 1: disable write 
  
#if 0
// GerH: if *I* read the datasheet correctly, this anolog I/O activity requires REG_ADC_WPMR access to be set to write-OK iff
// we want to configure our ADC clock and/or set up a ADC channel like this.
//
// Incidentally, the adc code pasted/cludged together below does not set up the ADC module nor the ADC clock, which are mandatory
// for proper operations, so the question is: where does the default core code do that bit of chip fiddling?!

  //uint32_t analogRead(uint32_t ulPin)
{
  //uint32_t ulPin = ADC15;
  uint32_t ulValue = 0;
  const uint32_t ulChannel = 15;

#if 0
  if (ulPin < A0)
    ulPin += A0;

  ulChannel = g_APinDescription[ulPin].ulADCChannelNumber ;
#endif

//#if defined __SAM3X8E__ || defined __SAM3X8H__
	//static uint32_t latestSelectedChannel = -1;
	//switch ( g_APinDescription[ulPin].ulAnalogChannel )
	{
		// Handling ADC 12 bits channels
		//case ADC15 :

			// Enable the corresponding channel
uint32_t adc_get_channel_status(const Adc *p_adc, const enum adc_channel_num_t adc_ch)
{
	return p_adc->ADC_CHSR & (1 << adc_ch);
}
			if (adc_get_channel_status(ADC, ulChannel) != 1) {
void adc_enable_channel(Adc *p_adc, const enum adc_channel_num_t adc_ch)
{
	p_adc->ADC_CHER = 1 << adc_ch;
}
				adc_enable_channel( ADC, ulChannel );
				//if ( latestSelectedChannel != (uint32_t)-1 && ulChannel != latestSelectedChannel)
				//	adc_disable_channel( ADC, latestSelectedChannel );
				//latestSelectedChannel = ulChannel;
				g_pinStatus[ulPin] = (g_pinStatus[ulPin] & 0xF0) | PIN_STATUS_ANALOG;
			}

			// Start the ADC
void adc_start(Adc *p_adc)
{
	p_adc->ADC_CR = ADC_CR_START;
}
			adc_start( ADC );

			// Wait for end of conversion
uint32_t adc_get_status(const Adc *p_adc)
{
	return p_adc->ADC_ISR;
}
			while ((adc_get_status(ADC) & ADC_ISR_DRDY) != ADC_ISR_DRDY)
				;


uint32_t adc_get_channel_status(const Adc *p_adc, const enum adc_channel_num_t adc_ch)
{
	return p_adc->ADC_CHSR & (1 << adc_ch);
}
uint32_t adc_get_channel_value(const Adc *p_adc, const enum adc_channel_num_t adc_ch)
{
	uint32_t ul_data = 0;

	if (15 >= adc_ch) {
		ul_data = *(p_adc->ADC_CDR + adc_ch);
	}

	return ul_data;
}
uint32_t adc_get_latest_value(const Adc *p_adc)
{
	return p_adc->ADC_LCDR;
}
void adc_enable_tag(Adc *p_adc)
{
	p_adc->ADC_EMR |= ADC_EMR_TAG;
}
enum adc_channel_num_t adc_get_tag(const Adc *p_adc)
{
	return (p_adc->ADC_LCDR & ADC_LCDR_CHNB_Msk) >> ADC_LCDR_CHNB_Pos;
}


			// Read the value
uint32_t adc_get_latest_value(const Adc *p_adc)
{
	return p_adc->ADC_LCDR;
}
			ulValue = adc_get_latest_value(ADC);
			ulValue = mapResolution(ulValue, ADC_RESOLUTION, _readResolution);
	}

	return ulValue;
}
#endif // 0


#endif // 0












  // initialize input pins for 2 foot switches
  pinMode(FOOT_SW_LEFT, INPUT_PULLUP);
  pinMode(FOOT_SW_RIGHT, INPUT_PULLUP);
  
  // initialize the calibration data for it to be a no-op, unless it's loaded from a previous calibration sample result
  initializeCalibrationData();

  initializeSequencer();

  reset();

  // set display to normal performance mode & refresh it
  clearDisplay();
  setDisplayMode(displayNormal);

  // ensure that the switches that are pressed down for the global reset at boot are not taken into account any further
  if (globalReset) {
    cellTouched(0, GLOBAL_SETTINGS_ROW, touchedCell);
    cellTouched(0, SWITCH_2_ROW, touchedCell);
  }

  // setup system timers for interval between LED column refreshes and foot switch reads
  prevLedTimerCount = prevFootSwitchTimerCount = micros();

  // perform some initialization
  initializeCalibrationSamples();
  initializeStorage();
  applyConfiguration();

  for (byte ss = 0; ss < SECRET_SWITCHES; ++ss) {
    secretSwitch[ss] = false;
  }

  // detect if test mode is active by holding down the per-split button at startup
  if (switchPressAtStartup(PER_SPLIT_ROW)) {
    operatingMode = modeManufacturingTest;

    Global.velocitySensitivity = velocityLow;
    Global.minForVelocity = 0;
    Global.maxForVelocity = 127;
    applyLimitsForVelocity();
    Global.pressureSensitivity = pressureLow;

    // Disable serial mode
    Device.serialMode = false;
    applySerialMode();

    // Set the MIDI I/O to DIN
    Global.midiIO = 0;
    applyMidiIo();
    return;
  }

  // default to performance mode
  sensorCol = 0;
  sensorRow = 0;
  updateSensorCell();
  {
    operatingMode = modePerformance;

    // detect if low power mode is toggled by holding down the octave/transpose button at startup
    if (switchPressAtStartup(OCTAVE_ROW)) {
      Device.operatingLowPower = true;
      Device.serialMode = false;
      storeSettings();
      cellTouched(0, OCTAVE_ROW, touchedCell);
    }

    applyLedInterval();
    applyMidiInterval();

    applyMpeMode();

    // update the display for the last state
    updateDisplay();
  }

#ifdef DISPLAY_XFRAME_AT_LAUNCH
  #define DEBUG_ENABLED
  Device.serialMode = true;
  SWITCH_XFRAME = true;
#endif

#ifdef DISPLAY_YFRAME_AT_LAUNCH
  #define DEBUG_ENABLED
  Device.serialMode = true;
  SWITCH_YFRAME = true;
#endif

#ifdef DISPLAY_ZFRAME_AT_LAUNCH
  #define DEBUG_ENABLED
  Device.serialMode = true;
  SWITCH_ZFRAME = true;
#endif

#ifdef DISPLAY_SURFACESCAN_AT_LAUNCH
  #define DEBUG_ENABLED
  Device.serialMode = true;
  SWITCH_SURFACESCAN = true;
#endif

#ifdef DISPLAY_FREERAM_AT_LAUNCH
  #define DEBUG_ENABLED
  Device.serialMode = true;
  SWITCH_FREERAM = true;
#endif

#ifdef DISPLAY_DEBUGMIDI_AT_LAUNCH
  #define DEBUG_ENABLED
  Device.serialMode = true;
  SWITCH_DEBUGMIDI = true;
  debugLevel = 5;
#endif

  setupDone = true;

  applySerialMode();
  performContinuousTasks();

  // if the promo animation was running last time the LinnStrument was on, start it up automatically
  if (Device.sleepAnimationActive) {
    playSleepAnimation();
  }
}


/******************************* MAIN LOOP *****************************/

void loop() {
  // the default musical performance mode
  if (operatingMode == modePerformance) {
    modeLoopPerformance();
  }
  // manufactoring test mode where leds are shows for specific signals
  else if (operatingMode == modeManufacturingTest) {
    modeLoopManufacturingTest();
  }
  // firmware update mode that does nothing else but updating the leds and
  // waiting for a firmware update
  else if (operatingMode == modeFirmware) {
    checkRefreshLedColumn(micros());
  }
}

inline void modeLoopPerformance() {
  if (displayMode == displayReset) {                             // if reset is active, don't process any input data
    if (calcTimeDelta(millis(), lastReset) > 3000) {             // restore normal operations three seconds after the reset started
      applySystemState();

      storeSettings();
      setDisplayMode(displayNormal);                             // this should make the reset operation feel more predictable
      updateDisplay();
    }
  }
  else {
    TouchState previousTouch = sensorCell->touched;                              // get previous touch status of this cell

    boolean canShortCircuit = false;

    if (previousTouch != touchedCell && previousTouch != ignoredCell &&
        sensorCell->isMeaningfulTouch()) {                                       // if touched now but not before, it's a new touch
      canShortCircuit = handleNewTouch();
    }
    else if (previousTouch == touchedCell && sensorCell->isActiveTouch()) {      // if touched now and touched before
      canShortCircuit = handleXYZupdate();                                       // handle any X, Y or Z movements
    }
    else if (previousTouch != untouchedCell && !sensorCell->isActiveTouch() &&   // if not touched now but touched before, it's been released
             sensorCell->isPastDebounceDelay()) {
        handleTouchRelease();
    }

    if (canShortCircuit) {
      sensorCell->shouldRefreshData();                                           // immediately process this cell again without going through a full surface scan
      return;
    }
  }

  // When operating in low power mode, slow down the sensor scan rate in order to consume less power
  // This introduces an overall additional average latency of 2.5ms
  if (Device.operatingLowPower) {
    delayUsec(25);
  }

  // We're iterating so quickly, that it makes no sense to perform the continuous tasks
  // at each sensor cell, only call this every three cells.
  // Note that this is very much dependent on the speed of the main loop, if it slows down
  // lights will start flickering and this ratio might have to be adapted.
  if (cellCount % mainLoopDivider == 0) {
    performContinuousTasks();
  }

#ifdef DEBUG_ENABLED
  if (SWITCH_XFRAME) displayXFrame();                            // Turn on secret switch to display the X value of all cells in grid at the end of each total surface scan
  if (SWITCH_YFRAME) displayYFrame();                            // Turn on secret switch to display the Y value of all cells in grid at the end of each total surface scan
  if (SWITCH_ZFRAME) displayZFrame();                            // Turn on secret switch to display the pressure value of all cells in grid at the end of each total surface scan
  if (SWITCH_TOUCHFRAME) displayCellTouchedFrame();              // Turn on secret switch to display the 'touched' state of all cells in grid at the end of each total surface scan
  if (SWITCH_SURFACESCAN) displaySurfaceScanTime();              // Turn on secret switch to display the total time for a total surface scan
  if (SWITCH_FREERAM) debugFreeRam();                            // Turn on secret switch to display the available free RAM
  if (SWITCH_MCU_PINS) displayDigitalPins();                     // Turn on secret switch to display the SAM3X digital pins' status
#endif

  nextSensorCell();                                              // done-- move on to the next sensor cell
}
