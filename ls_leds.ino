/*********************************** ls_leds: LinnStrument LEDS ***********************************
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
These functions handle the low-level communication with LinnStrument's 208 RGB LEDs.
**************************************************************************************************/

/*
 LinnStrument contains an array of 208 RGB LEDs arranged in a 26 by 8 matrix.
 Only one column (8 LEDs) can be turned on at a time and the columns are refreshed one at a time.
 This works out to a duty cycle of 1/26, keeping the total current low enough for USB bus power. 

 These are the various functions that together perform the LED tasks:

 Data is sent to the LED board over a 32-bit SPI channel, arranged as follows:

 Byte 0 (column select):
     7:        6:        5:        4:        3:        2:        1:        0:
 colAdr4Inv colAdr4   colAdr3   colAdr2   colAdr1   colAdr0     n/a       n/a
 Note: colAdr4Inv is inverted state of colAdr4, to save an inverter

 Byte 1 (blue on/off bits for each row):
     7:        6:        5:        4:        3:        2:        1:        0:
  blueRow7  blueRow6  blueRow5  blueRow4  blueRow3  blueRow2  blueRow1  blueRow0

 Byte 2 (green on/off bits for each row):
     7:        6:        5:        4:        3:        2:        1:        0:
 greenRow7 greenRow6 greenRow5 greenRow4  greenRow3 greenRow2  greenRow1 greenRow0

 Byte 3 (red on/off bits for each row):
     7:        6:        5:        4:        3:        2:        1:        0:
  redRow7   redRow6   redRow5   redRow4   redRow3   redRow2   redRow1   redRow0
*/

byte COL_INDEX[MAXCOLS];
const byte COL_INDEX_200[MAXCOLS] = {0, 1, 6, 11, 16, 21, 2, 7, 12, 17, 22, 3, 8, 13, 18, 23, 4, 9, 14, 19, 24, 5, 10, 15, 20, 25};
const byte COL_INDEX_128[MAXCOLS] = {0, 1, 6, 11, 16, 2, 7, 12, 3, 8, 13, 4, 9, 14, 5, 10, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0};

// array holding contents of display
byte leds[2][LED_ARRAY_SIZE];
byte visibleLeds = 0;
byte bufferedLeds = 0;
#define ledBuffered(layer, col, row)  leds[bufferedLeds][layer * LED_LAYER_SIZE + row * MAXCOLS + col]
#define ledVisible(layer, col, row)  leds[visibleLeds][layer * LED_LAYER_SIZE + row * MAXCOLS + col]

bool ledDisplayEnabled = true;

void initializeLeds() {
  if (LINNMODEL == 200) {
    for (byte i = 0; i < MAXCOLS; ++i) {
      COL_INDEX[i] = COL_INDEX_200[i];
    }
  }
  else if (LINNMODEL == 128) {
    for (byte i = 0; i < MAXCOLS; ++i) {
      COL_INDEX[i] = COL_INDEX_128[i];
    }
  }
}

void initializeLedLayers() {
  memset(leds[bufferedLeds], 0, LED_ARRAY_SIZE);
}

void initializeLedsLayer(byte layer) {
  memset(&leds[bufferedLeds][layer * LED_LAYER_SIZE], 0, LED_LAYER_SIZE);
}

int getActiveCustomLedPattern() {
  return Global.activeNotes - 9;
}

void loadCustomLedLayer(int pattern)
{
  if (pattern < 0 || pattern >= LED_PATTERNS) {
    if (customLedPatternActive) {
      memset(&leds[0][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE);
      memset(&leds[1][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE);
    }
    customLedPatternActive = false;
    return;
  }

  memcpy(&leds[0][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], &Device.customLeds[pattern][0], LED_LAYER_SIZE);
  memcpy(&leds[1][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], &Device.customLeds[pattern][0], LED_LAYER_SIZE);
  customLedPatternActive = true;
  lightSettings = 2;
  completelyRefreshLeds();
}

void storeCustomLedLayer(int pattern)
{
  if (pattern < 0 || pattern >= LED_PATTERNS) {
    if (customLedPatternActive) {
      memset(&leds[0][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE);
      memset(&leds[1][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE);
    }
    customLedPatternActive = false;
    return;
  }

  memcpy(&Device.customLeds[pattern][0], &leds[visibleLeds][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], LED_LAYER_SIZE);
  customLedPatternActive = true;
  lightSettings = 2;
}

void clearStoredCustomLedLayer(int pattern)
{
  if (pattern < 0 || pattern >= LED_PATTERNS) return;

  memset(&Device.customLeds[pattern][0], 0, LED_LAYER_SIZE);
  if (getActiveCustomLedPattern() == pattern) {
    loadCustomLedLayer(pattern);
  }
}

void startBufferedLeds() {
  bufferedLeds = 1;
  memcpy(leds[bufferedLeds], leds[visibleLeds], LED_ARRAY_SIZE);
}

void finishBufferedLeds() {
  memcpy(leds[visibleLeds], leds[bufferedLeds], LED_ARRAY_SIZE);
  bufferedLeds = 0;
}

inline byte getCombinedLedData(byte col, byte row) {
  byte data = 0;
  byte layer = MAX_LED_LAYERS;
  do {
    layer -= 1;
    // when custom LED editor is active, only display those LEDs
    if (col > 0 && displayMode == displayCustomLedsEditor) {
      if (layer != LED_LAYER_CUSTOM1) continue;
      data = ledBuffered(layer, col, row);
    }
    // don't show the custom layer 1 in user firmware mode
    if (userFirmwareActive || isVisibleSequencer()) {
      if (layer == LED_LAYER_CUSTOM1) continue;
    }
    // don't show the custom layer 2 in regular firmware mode
    if (!userFirmwareActive) {
      if (layer == LED_LAYER_CUSTOM2) continue;
    }
    if (!isVisibleSequencer()) {
      if (layer == LED_LAYER_SEQUENCER) continue;
    }
    // in normal and split point display mode, show all layers and only show the main in other display modes
    if (displayMode == displayNormal || displayMode == displaySplitPoint || layer == LED_LAYER_MAIN) {
      data = ledBuffered(layer, col, row);
    }    
  }
  while (layer > 0 && (data & B00000111) == cellOff);

  return data;
}

void setLed(byte col, byte row, byte color, CellDisplay disp) {
  setLed(col, row, color, disp, LED_LAYER_MAIN);
}

void setLed(byte col, byte row, byte color, CellDisplay disp, byte layer) {
  if (col >= NUMCOLS || row >= NUMROWS || layer > MAX_LED_LAYERS) return;

  if (color == COLOR_OFF) {
    disp = cellOff;
  }
  else if (disp == cellOff) {
    color = COLOR_OFF;
  }
  // packs color and display into this cell within array
  byte data = ((color & B00011111) << 3) | (disp & B00000111);
  if (ledBuffered(layer, col, row) != data) {
    ledBuffered(layer, col, row) = data;
    ledBuffered(LED_LAYER_COMBINED, col, row) = getCombinedLedData(col, row);
  }

  if (bufferedLeds == 1) {
    performContinuousTasks();
  }
}

byte getLedColor(byte col, byte row, byte layer) {
  if (col >= NUMCOLS || row >= NUMROWS || layer > MAX_LED_LAYERS) return COLOR_OFF;
  return (ledVisible(layer, col, row) >> 3) & B00011111;
}

// light up a single LED with the default color
void lightLed(byte col, byte row) {
  setLed(col, row, globalColor, cellOn);
}

// clear a single LED
void clearLed(byte col, byte row) {
  clearLed(col, row, LED_LAYER_MAIN);
  clearLed(col, row, LED_LAYER_LOWROW);
}

void clearLed(byte col, byte row, byte layer) {
  setLed(col, row, COLOR_OFF, cellOff, layer);
}

// Turns all LEDs off
void clearFullDisplay() {
  clearSwitches();
  clearDisplay();
}

// Turns all LEDs off in columns 1 or higher
void clearDisplay() {
  for (byte col = 1; col < NUMCOLS; ++col) {
    clearColumn(col);
  }
}

// Turns all LEDs off in column 0
void clearSwitches() {
  clearColumn(0);
}

void clearColumn(byte col) {
  for (byte row = 0; row < NUMROWS; ++row) {
    clearLed(col, row);
  }
}

void clearRow(byte row) {
  for (byte col = 0; col < NUMCOLS; ++col) {
    clearLed(col, row);
  }
}

void completelyRefreshLeds() {
  for (byte row = 0; row < NUMROWS; ++row) {
    for (byte col = 0; col < NUMCOLS; ++col) {
      ledBuffered(LED_LAYER_COMBINED, col, row) = getCombinedLedData(col, row);
    }
    performContinuousTasks();
  }
}

void clearDisplayImmediately() {
  // disable the outputs of the LED driver chips
  digitalWrite(37, HIGH);
}

void disableLedDisplay() {
  ledDisplayEnabled = false;
  clearDisplayImmediately();
}

void enableLedDisplay() {
  // enable the outputs of the LED driver chips
  digitalWrite(37, LOW);
  ledDisplayEnabled = true;
}

// see LedColorMix: the seven colours the hardware can light resolve to themselves, and every colour past
// them names the two it blends and how much of the second to use. The four original composite colours are
// even blends, which is exactly what they have always been -- only the phase of the alternation changed.
static const LedColorMix LED_COLOR_MIXES[COLOR_MAX + 1] = {
  /* COLOR_OFF        */ { COLOR_OFF,     COLOR_OFF,     0 },
  /* COLOR_RED        */ { COLOR_RED,     COLOR_RED,     0 },
  /* COLOR_YELLOW     */ { COLOR_YELLOW,  COLOR_YELLOW,  0 },
  /* COLOR_GREEN      */ { COLOR_GREEN,   COLOR_GREEN,   0 },
  /* COLOR_CYAN       */ { COLOR_CYAN,    COLOR_CYAN,    0 },
  /* COLOR_BLUE       */ { COLOR_BLUE,    COLOR_BLUE,    0 },
  /* COLOR_MAGENTA    */ { COLOR_MAGENTA, COLOR_MAGENTA, 0 },
  /* COLOR_BLACK      */ { COLOR_OFF,     COLOR_OFF,     0 },
  /* COLOR_WHITE      */ { COLOR_WHITE,   COLOR_CYAN,    3 },
  /* COLOR_ORANGE     */ { COLOR_RED,     COLOR_YELLOW,  3 },
  /* COLOR_LIME       */ { COLOR_YELLOW,  COLOR_GREEN,   3 },
  /* COLOR_PINK       */ { COLOR_MAGENTA, COLOR_YELLOW,  3 },
  // hues between two neighbouring colours, leaning towards the base by taking only a third of the slots
  /* COLOR_AMBER      */ { COLOR_YELLOW,  COLOR_RED,     2 },
  /* COLOR_RUST       */ { COLOR_RED,     COLOR_YELLOW,  2 },
  /* COLOR_CHARTREUSE */ { COLOR_GREEN,   COLOR_YELLOW,  2 },
  /* COLOR_TEAL       */ { COLOR_CYAN,    COLOR_GREEN,   2 },
  /* COLOR_AZURE      */ { COLOR_BLUE,    COLOR_CYAN,    2 },
  /* COLOR_PURPLE     */ { COLOR_BLUE,    COLOR_MAGENTA, 3 },
  // washed out shades, made by blending a colour with white so that the primaries it lacks come up part of
  // the time. These read as subordinate to their saturated version while staying the same hue, which is
  // what brightness could not do. Cyan is missing because white already is cyan blended with white.
  /* COLOR_SALMON     */ { COLOR_RED,     COLOR_WHITE,   3 },
  /* COLOR_CREAM      */ { COLOR_YELLOW,  COLOR_WHITE,   3 },
  /* COLOR_MINT       */ { COLOR_GREEN,   COLOR_WHITE,   3 },
  /* COLOR_SKY        */ { COLOR_BLUE,    COLOR_WHITE,   3 },
  /* COLOR_LAVENDER   */ { COLOR_MAGENTA, COLOR_WHITE,   3 },
  // washing out orange and lime needs the third primary raised only part way rather than to match the
  // other two, so these pair the colour with a neighbour that carries the missing primary instead of with
  // white, which would raise both of the primaries it lacks equally and pull the hue towards pink
  /* COLOR_PALE_ORANGE*/ { COLOR_YELLOW,  COLOR_MAGENTA, 2 },
  /* COLOR_PALE_LIME  */ { COLOR_YELLOW,  COLOR_CYAN,    2 },
  // shades that carry less light than the colour they shadow. Blue is the dimmest thing this panel can
  // show, so giving half a colour's slots to blue roughly halves the light it puts out while shifting its
  // hue, and both of those read as receding. Note that the cell stays lit on every interval, which is what
  // separates this from blanking intervals to darken a cell: no interval goes dark, so nothing flickers.
  // Blue has nothing dimmer to blend towards, so its shadow is the one that has to blank instead, at the
  // same every-other-interval rate as low power mode, which is slow enough not to flicker.
  /* COLOR_SHADOW_RED  */ { COLOR_RED,     COLOR_BLUE,    3 },
  /* COLOR_SHADOW_YELL */ { COLOR_YELLOW,  COLOR_BLUE,    3 },
  /* COLOR_SHADOW_GREE */ { COLOR_GREEN,   COLOR_BLUE,    3 },
  /* COLOR_SHADOW_CYAN */ { COLOR_CYAN,    COLOR_BLUE,    3 },
  /* COLOR_SHADOW_BLUE */ { COLOR_BLUE,    COLOR_OFF,     3 }
};

// the colour a cell takes when it is a variant of the cell beside it, currently the quartertone cells in
// 24-EDO mode. These pairings are picked by eye rather than computed, because neither of the two things a
// formula can do turns out to read on this panel. Darkening does not work: a cell is refreshed only about
// 115 times a second, so blanking enough intervals to notice makes it flicker, and half -- all that can be
// held steady -- is not enough to see. Washing a colour out does not work either, since filling in the
// primaries it lacks adds light rather than removing it, which lands somewhere near white.
//
// What does read is a step to a neighbouring hue, and most of these are brighter than the colour they pair
// with rather than darker. Pairs sit in the same family so the two still look related: greens go to
// chartreuse, warm colours go to cream, and the palest colours pair back to a muted shade because there is
// nothing brighter left to step to.
static const byte LED_COLOR_VARIANTS[COLOR_MAX + 1] = {
  /* COLOR_OFF        */ COLOR_OFF,
  /* COLOR_RED        */ COLOR_MAGENTA,
  /* COLOR_YELLOW     */ COLOR_PALE_LIME,
  /* COLOR_GREEN      */ COLOR_CHARTREUSE,
  /* COLOR_CYAN       */ COLOR_WHITE,
  /* COLOR_BLUE       */ COLOR_CYAN,
  /* COLOR_MAGENTA    */ COLOR_LAVENDER,
  /* COLOR_BLACK      */ COLOR_BLACK,
  /* COLOR_WHITE      */ COLOR_SHADOW_YELLOW,
  /* COLOR_ORANGE     */ COLOR_CREAM,
  /* COLOR_LIME       */ COLOR_CHARTREUSE,
  /* COLOR_PINK       */ COLOR_CREAM,
  /* COLOR_AMBER      */ COLOR_PALE_LIME,
  /* COLOR_RUST       */ COLOR_AMBER,
  /* COLOR_CHARTREUSE */ COLOR_PALE_LIME,
  /* COLOR_TEAL       */ COLOR_WHITE,
  // the rest follow the same steps as the colour each one sits nearest to above
  /* COLOR_AZURE      */ COLOR_CYAN,
  /* COLOR_PURPLE     */ COLOR_SKY,
  /* COLOR_SALMON     */ COLOR_CREAM,
  /* COLOR_CREAM      */ COLOR_SHADOW_YELLOW,
  /* COLOR_MINT       */ COLOR_CHARTREUSE,
  /* COLOR_SKY        */ COLOR_WHITE,
  /* COLOR_LAVENDER   */ COLOR_CREAM,
  /* COLOR_PALE_ORANGE*/ COLOR_PALE_LIME,
  /* COLOR_PALE_LIME  */ COLOR_WHITE,
  /* COLOR_SHADOW_RED */ COLOR_MAGENTA,
  /* COLOR_SHADOW_YELL*/ COLOR_PALE_LIME,
  /* COLOR_SHADOW_GREE*/ COLOR_CHARTREUSE,
  /* COLOR_SHADOW_CYAN*/ COLOR_CYAN,
  /* COLOR_SHADOW_BLUE*/ COLOR_CYAN
};

byte variantOf(byte color) {
  if (color > COLOR_MAX) return color;
  return LED_COLOR_VARIANTS[color];
}

// refreshLedColumn:
// Called when it's time to refresh the next column of LEDs. Internally increments the column number every time it's called.
void refreshLedColumn(unsigned long now) {
  if (!ledDisplayEnabled) return;

  // disabling the power output from the LED driver pins early prevents power leaking into unwanted cells.
  digitalWrite(37, HIGH);                                         // disable the outputs of the LED driver chips

  // keep a steady pulsating going for those leds that need it
  static unsigned long lastPulse = 0;
  static boolean lastPulseOn = true;
  static unsigned long lastSlowPulse = 0;
  static boolean lastSlowPulseOn = true;
  static boolean lastFocusPulseOn = true;

  if (calcTimeDelta(now, lastPulse) > 80000) {
    lastPulse = now;
    lastPulseOn = !lastPulseOn;
  }
  if (calcTimeDelta(now, lastSlowPulse) > 120000) {
    lastSlowPulse = now;
    lastSlowPulseOn = !lastSlowPulseOn;
  }
  if (clock24PPQ < 6) {
    lastFocusPulseOn = false;
  }
  else {
    lastFocusPulseOn = true;
  }

  static byte ledCol = 0;
  static byte displayInterval[MAXCOLS][MAXROWS];

  byte red = 0;                                           // red value to be sent
  byte green = 0;                                         // green value to be sent
  byte blue = 0;                                          // blue value to be sent
  byte actualCol = 0;                                     // actual column being refreshed, permitting columns to be lit non-sequentially by using COL_INDEX[] array
  byte ledColShifted = 0;                                 // LED column address, which is shifted 2 bits to left within byte

  actualCol = COL_INDEX[ledCol];                           // using COL_INDEX[], permits non-sequential lighting of LED columns, which doesn't seem to improve appearance

   // Initialize bytes to send to LEDs over SPI. Each bit represents a single LED on or off
  for (byte rowCount = 0; rowCount < NUMROWS; ++rowCount) {       // step through the 8 rows
    // allow several levels of brightness by modulating LED's ON time
    if (++displayInterval[actualCol][rowCount] >= 12) {
      displayInterval[actualCol][rowCount] = 0;
    }

    byte color = (ledVisible(LED_LAYER_COMBINED, actualCol, rowCount) & B11111000) >> 3;    // set temp value 'color' to 4 color bits of this LED within array
    byte cellDisplay = ledVisible(LED_LAYER_COMBINED, actualCol, rowCount) & B00000111;     // get cell display value

    // low power mode blanks every other refresh interval to halve the current the panel draws. That is
    // also as dark as this hardware can go without flickering: a column is refreshed every
    // DEFAULT_LED_REFRESH microseconds and the board walks its columns one per refresh, so a single cell
    // comes round only about 115 times a second, and blanking more than every other interval stretches the
    // gap between lit ones until the cell visibly blinks -- two intervals in three flickers at 38Hz, five
    // in six at 19Hz. Marking a cell out by making it darker is therefore not available to the display,
    // which distinguishes cells by colour instead.
    byte litEvery = 1;
    if (Device.operatingLowPower) litEvery = 2;

    switch (cellDisplay) {
      case cellFastPulse:
        cellDisplay = lastPulseOn ? cellOn : cellOff;
        break;
      case cellSlowPulse:
        cellDisplay = lastSlowPulseOn ? cellOn : cellOff;
        break;
      case cellFocusPulse:
        cellDisplay = lastFocusPulseOn ? cellOn : cellOff;
        break;
    }

    if (displayInterval[actualCol][rowCount] % litEvery != 0) {
      cellDisplay = cellOff;
    }

    // if this LED is not off, process it
    // set the color bytes to the correct color
    if (cellDisplay) {
      // resolve the colour down to one of the seven the panel can actually light, blending in time when
      // the colour asks for it. The slot counts the intervals this cell is lit for rather than every
      // interval, so a dimmed or low power cell keeps the blend's ratio instead of losing one of its two
      // colours to the blanked intervals and coming out the wrong hue.
      const LedColorMix& mix = LED_COLOR_MIXES[color <= COLOR_MAX ? color : COLOR_OFF];
      byte slot = (displayInterval[actualCol][rowCount] / litEvery) % LED_MIX_SLOTS;

      // spread the mixed slots through the cycle instead of bunching them at its start, so that the two
      // colours interleave and settle into a single hue rather than alternating in visible blocks
      byte rendered = ((slot * mix.mixedSlots) % LED_MIX_SLOTS) < mix.mixedSlots ? mix.mixed : mix.base;

      switch (rendered)
      {
        case COLOR_RED:
          red = red | (B00000001 << rowCount);
          break;
        case COLOR_YELLOW:
          red = red | (B00000001 << rowCount);
          green = green | (B00000001 << rowCount);
          break;
        case COLOR_GREEN:
          green = green | (B00000001 << rowCount);
          break;
        case COLOR_CYAN:
          green = green | (B00000001 << rowCount);
          blue = blue | (B00000001 << rowCount);
          break;
        case COLOR_BLUE:
          blue = blue | (B00000001 << rowCount);
          break;
        case COLOR_MAGENTA:
          blue = blue | (B00000001 << rowCount);
          red = red | (B00000001 << rowCount);
          break;
        case COLOR_WHITE:
          blue = blue | (B00000001 << rowCount);
          red = red | (B00000001 << rowCount);
          green = green | (B00000001 << rowCount);
          break;
        default:                                            // COLOR_OFF and COLOR_BLACK light nothing
          break;
      }
    }
  }

  if (++ledCol >= NUMCOLS) ledCol = 0;

  ledColShifted = actualCol << 2;
  if ((actualCol & 16) == 0) ledColShifted |= B10000000;          // if column address 4 is 0, set bit 7

  SPI.transfer(SPI_LEDS, ~ledColShifted, SPI_CONTINUE);           // send column address
  SPI.transfer(SPI_LEDS, blue, SPI_CONTINUE);                     // send blue byte
  SPI.transfer(SPI_LEDS, green, SPI_CONTINUE);                    // send green byte
  SPI.transfer(SPI_LEDS, red);                                    // send red byte
  digitalWrite(37, LOW);                                          // enable the outputs of the LED driver chips
}
