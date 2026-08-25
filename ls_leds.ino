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
uint16_t leds[2][LED_ARRAY_SIZE];
byte visibleLeds = 0;
byte bufferedLeds = 0;
#define ledBuffered(layer, col, row)  leds[bufferedLeds][layer * LED_LAYER_SIZE + row * MAXCOLS + col]
#define ledVisible(layer, col, row)  leds[visibleLeds][layer * LED_LAYER_SIZE + row * MAXCOLS + col]

#define LED_CELL_DISPLAY_MASK 0x0007u
#define LED_CELL_COLOR_SHIFT  3u
#define LED_CELL_COLOR_MASK   0x03f8u

static const byte paletteColorToRgb[COLOR_PALETTE_LAST + 1] = {
  0, 100, 90, 20, 18, 4, 78, 0, 62, 105, 70, 82,
  110, 22, 14, 54, 102, 42,
};

static uint16_t packLedCell(byte color, CellDisplay disp) {
  return ((uint16_t)color << LED_CELL_COLOR_SHIFT) | (disp & LED_CELL_DISPLAY_MASK);
}

static uint16_t unpackStoredLedCell(byte data) {
  byte color = data >> LED_CELL_COLOR_SHIFT;
  return packLedCell(color <= COLOR_PALETTE_LAST ? paletteColorToRgb[color] : 0,
                     (CellDisplay)(data & LED_CELL_DISPLAY_MASK));
}

static byte packPersistentLedCell(uint16_t data) {
  byte color = (data & LED_CELL_COLOR_MASK) >> LED_CELL_COLOR_SHIFT;
  byte display = data & LED_CELL_DISPLAY_MASK;
  if (color == 0 && display != cellOff) {
    return (COLOR_BLACK << LED_CELL_COLOR_SHIFT) | display;
  }

  byte nearest = COLOR_OFF;
  unsigned int nearestDistance = UINT_MAX;
  for (byte candidate = COLOR_OFF; candidate <= COLOR_PALETTE_LAST; ++candidate) {
    int dr = (int)(color / 25) - (int)(paletteColorToRgb[candidate] / 25);
    int dg = (int)((color / 5) % 5) - (int)((paletteColorToRgb[candidate] / 5) % 5);
    int db = (int)(color % 5) - (int)(paletteColorToRgb[candidate] % 5);
    unsigned int distance = (unsigned int)(dr * dr + dg * dg + db * db);
    if (distance < nearestDistance) {
      nearest = candidate;
      nearestDistance = distance;
    }
  }
  return (nearest << LED_CELL_COLOR_SHIFT) | display;
}

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
  memset(leds[bufferedLeds], 0, sizeof(leds[bufferedLeds]));
}

void initializeLedsLayer(byte layer) {
  memset(&leds[bufferedLeds][layer * LED_LAYER_SIZE], 0,
         LED_LAYER_SIZE * sizeof(leds[bufferedLeds][0]));
}

int getActiveCustomLedPattern() {
  return Global.activeNotes - 9;
}

void loadCustomLedLayer(int pattern)
{
  if (pattern < 0 || pattern >= LED_PATTERNS) {
    if (customLedPatternActive) {
      memset(&leds[0][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE * sizeof(leds[0][0]));
      memset(&leds[1][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE * sizeof(leds[1][0]));
    }
    customLedPatternActive = false;
    return;
  }

  for (uint16_t i = 0; i < LED_LAYER_SIZE; ++i) {
    uint16_t data = unpackStoredLedCell(Device.customLeds[pattern][i]);
    leds[0][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE + i] = data;
    leds[1][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE + i] = data;
  }
  customLedPatternActive = true;
  lightSettings = 2;
  completelyRefreshLeds();
}

void storeCustomLedLayer(int pattern)
{
  if (pattern < 0 || pattern >= LED_PATTERNS) {
    if (customLedPatternActive) {
      memset(&leds[0][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE * sizeof(leds[0][0]));
      memset(&leds[1][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE], 0, LED_LAYER_SIZE * sizeof(leds[1][0]));
    }
    customLedPatternActive = false;
    return;
  }

  for (uint16_t i = 0; i < LED_LAYER_SIZE; ++i) {
    Device.customLeds[pattern][i] =
        packPersistentLedCell(leds[visibleLeds][LED_LAYER_CUSTOM1 * LED_LAYER_SIZE + i]);
  }
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
  memcpy(leds[bufferedLeds], leds[visibleLeds], sizeof(leds[bufferedLeds]));
}

void finishBufferedLeds() {
  memcpy(leds[visibleLeds], leds[bufferedLeds], sizeof(leds[visibleLeds]));
  bufferedLeds = 0;
}

inline uint16_t getCombinedLedData(byte col, byte row) {
  uint16_t data = 0;
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
  while (layer > 0 && (data & LED_CELL_DISPLAY_MASK) == cellOff);

  return data;
}

void setLed(byte col, byte row, byte color, CellDisplay disp) {
  setLed(col, row, color, disp, LED_LAYER_MAIN);
}

void setLed(byte col, byte row, byte color, CellDisplay disp, byte layer) {
  if (color == COLOR_OFF) {
    disp = cellOff;
  }
  else if (color <= COLOR_PALETTE_LAST) {
    color = paletteColorToRgb[color];
  }
  setLedColorIndex(col, row, color, disp, layer);
}

void setLedColorIndex(byte col, byte row, byte color, CellDisplay disp, byte layer) {
  if (col >= NUMCOLS || row >= NUMROWS || layer > MAX_LED_LAYERS) return;
  if (disp == cellOff) color = 0;
  uint16_t data = packLedCell(color, disp);
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
  return (ledVisible(layer, col, row) & LED_CELL_COLOR_MASK) >> LED_CELL_COLOR_SHIFT;
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
  tta_led_set_enabled(false);
}

void disableLedDisplay() {
  ledDisplayEnabled = false;
  clearDisplayImmediately();
}

void enableLedDisplay() {
  ledDisplayEnabled = true;
  tta_led_set_enabled(true);
}

// refreshLedColumn:
// Called when it's time to refresh the next column of LEDs. Internally increments the column number every time it's called.
void refreshLedColumn(unsigned long now) {
  if (!ledDisplayEnabled) return;

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
  byte actualCol = COL_INDEX[ledCol];

  for (byte rowCount = 0; rowCount < NUMROWS; ++rowCount) {       // step through the 8 rows
    uint16_t ledData = ledVisible(LED_LAYER_COMBINED, actualCol, rowCount);
    byte color = (ledData & LED_CELL_COLOR_MASK) >> LED_CELL_COLOR_SHIFT;
    byte cellDisplay = ledData & LED_CELL_DISPLAY_MASK;

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

    byte r = 0, g = 0, b = 0;
    if (cellDisplay) {
      r = color / 25u;
      g = (color / 5u) % 5u;
      b = color % 5u;
    }

    tta_led[(uint16_t)actualCol * MAXROWS + rowCount] =
        (byte)(r * 25u + g * 5u + b);
  }

  if (++ledCol >= NUMCOLS) ledCol = 0;
  tta_led_commit(1u << actualCol);
  tta_led_set_enabled(true);
}
