/******************************* ls_demo: LinnStrument LED Colour Demo *******************************
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
This is a looping colour show, launched from global settings by pressing the pad that normally
scrolls the "LINNSTRUMENT" audience message.  It uses colour-theory pairings (complementary hues and spectrum
sweeps) and spatial effects (an orbiting glow casting expanding rings).  Any touch cancels it.
**************************************************************************************************/

// Each scene plays this long (ms) before the next scene takes over.
#define DEMO_SCENE_DURATION_MS 8000

// Map a hue (0-359 degrees) with saturation and value (each 0-5, 5 = full) to RGB.
static void demoHueToRgb(short hue, byte sat, byte val, byte& r, byte& g, byte& b) {
  byte sector = (hue / 60) % 6;
  byte frac = (hue % 60) * 5 / 60;
  // standard HSV on the 0-5 scale: p = v(1-s), q = v(1-f*s), t = v(1-(1-f)*s)
  byte p = val * (5 - sat) / 5;
  byte q = val * (25 - sat * frac) / 25;
  byte t = val * (25 - sat * (5 - frac)) / 25;
  switch (sector) {
    case 0: r = val; g = t;   b = p;   break;
    case 1: r = q;   g = val; b = p;   break;
    case 2: r = p;   g = val; b = t;   break;
    case 3: r = p;   g = q;   b = val; break;
    case 4: r = t;   g = p;   b = val; break;
    default: r = val; g = p;  b = q;   break;
  }
}

// Quantize the demo's 0-5 HSV components to the LED driver's native 0-4 RGB range.
static inline byte demoRgbToColor(byte r, byte g, byte b) {
  r = (r * 4u + 2u) / 5u;
  g = (g * 4u + 2u) / 5u;
  b = (b * 4u + 2u) / 5u;
  return r * 25u + g * 5u + b;
}

// The demo generates raw RGB indices, which must not be interpreted as named palette colours.
static inline void demoSetLed(byte col, byte row, byte color) {
  setLedColorIndex(col, row, color, cellOn, LED_LAYER_MAIN);
}

// Scene 1: the full colour space.  Hue sweeps across the columns (each column is a
// slice of the wheel, and hueOffset glides the whole wheel past) while each row shows
// the same hue at a different tone: a clean tint-to-shade ladder.  As the hue
// scrolls, every colour family passes through every tone.
static void demoSceneSpectrum(unsigned long startTime) {
  // (saturation, value) ladder per row, 5 = full: pale tint on top, darkest
  // shade on the bottom, so each column is one hue in a smooth tonal ramp.
  static const byte ladder[8][2] = {
    {1, 5},  // row 0: pale tint
    {2, 5},  // row 1: light pastel
    {3, 5},  // row 2: pastel
    {4, 5},  // row 3: rich
    {5, 5},  // row 4: vivid
    {5, 4},  // row 5: deep
    {5, 3},  // row 6: deeper
    {5, 2},  // row 7: darkest shade
  };
  const short hueStep = 360 / (NUMCOLS - 1);
  short hueOffset = 0;
  while (!stopAnimation && calcTimeDelta(millis(), startTime) < DEMO_SCENE_DURATION_MS) {
    hueOffset = (hueOffset + 2) % 360;
    for (byte col = 1; col < NUMCOLS; ++col) {
      short hue = (short)(((col - 1) * hueStep + hueOffset) % 360);
      for (byte row = 0; row < NUMROWS; ++row) {
        byte r, g, b;
        demoHueToRgb(hue, ladder[row][0], ladder[row][1], r, g, b);
        demoSetLed(col, row, demoRgbToColor(r, g, b));
      }
    }
    delayUsecWithScanning(16667);
  }
}

// Scene 2: complementary colour pairs (hue and hue + 180 degrees) split by a slowly rotating divider.
static void demoSceneComplementary(unsigned long startTime) {
  short hueOffset = 0;
  short frame = 0;
  while (!stopAnimation && calcTimeDelta(millis(), startTime) < DEMO_SCENE_DURATION_MS) {
    hueOffset = (hueOffset + 2) % 360;
    float angle = frame++ * 0.012f;
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    float centreX = (NUMCOLS - 1) / 2.0f;
    float centreY = (NUMROWS - 1) / 2.0f;
    for (byte col = 1; col < NUMCOLS; ++col) {
      for (byte row = 0; row < NUMROWS; ++row) {
        // which side of the rotating divider is this cell on?
        float dx = col - centreX;
        float dy = row - centreY;
        bool side = (dx * cosA + dy * sinA) >= 0.0f;
        short hue = (hueOffset + (side ? 0 : 180)) % 360;
        byte r, g, b;
        demoHueToRgb(hue, 5, 5, r, g, b);
        demoSetLed(col, row, demoRgbToColor(r, g, b));
      }
    }
    delayUsecWithScanning(16667);
  }
}

// Scene 3: an orbiting glow casts expanding colour rings; brightness fades with distance from it.
static void demoSceneRipple(unsigned long startTime) {
  short hueOffset = 0;
  short frame = 0;
  while (!stopAnimation && calcTimeDelta(millis(), startTime) < DEMO_SCENE_DURATION_MS) {
    hueOffset = (hueOffset + 2) % 360;
    float cx = (NUMCOLS - 1) / 2.0f + cosf(frame * 0.015f) * ((NUMCOLS - 1) / 3.0f);
    float cy = (NUMROWS - 1) / 2.0f + sinf(frame * 0.020f) * ((NUMROWS - 1) / 3.0f);
    ++frame;
    for (byte col = 1; col < NUMCOLS; ++col) {
      for (byte row = 0; row < NUMROWS; ++row) {
        float dx = col - cx;
        float dy = row - cy;
        float dsq = dx * dx + dy * dy;
        short hue = (hueOffset + (short)(dsq * 3.0f)) % 360;
        int bright = 255 - (int)(dsq * 1.5f);
        if (bright < 70) bright = 70;
        byte r, g, b;
        demoHueToRgb(hue, 5, 5, r, g, b);
        byte color = demoRgbToColor((byte)((r * bright) / 255),
                                    (byte)((g * bright) / 255),
                                    (byte)((b * bright) / 255));
        demoSetLed(col, row, color);
      }
    }
    delayUsecWithScanning(16667);
  }
}

// Scene 4: the whole surface breathes through the six fully-saturated primaries plus white,
// crossfading smoothly between them at full brightness.
static void demoScenePrimaryCycle(unsigned long startTime) {
  static const byte primaries[7][3] = {
    {5, 0, 0},  // red
    {5, 5, 0},  // yellow
    {0, 5, 0},  // green
    {0, 5, 5},  // cyan
    {0, 0, 5},  // blue
    {5, 0, 5},  // magenta
    {5, 5, 5},  // white
  };
  for (byte c = 0; c < 7 && !stopAnimation && calcTimeDelta(millis(), startTime) < DEMO_SCENE_DURATION_MS; ++c) {
    const byte* from = primaries[c];
    const byte* to = primaries[(c + 1) % 7];
    for (byte step = 0; step <= 20 && !stopAnimation && calcTimeDelta(millis(), startTime) < DEMO_SCENE_DURATION_MS; ++step) {
      byte r = (byte)((from[0] * 20 + ((int)to[0] - (int)from[0]) * step + 10) / 20);
      byte g = (byte)((from[1] * 20 + ((int)to[1] - (int)from[1]) * step + 10) / 20);
      byte b = (byte)((from[2] * 20 + ((int)to[2] - (int)from[2]) * step + 10) / 20);
      byte color = demoRgbToColor(r, g, b);
      for (byte col = 1; col < NUMCOLS; ++col) {
        for (byte row = 0; row < NUMROWS; ++row) {
          demoSetLed(col, row, color);
        }
      }
      delayUsecWithScanning(16667);
    }
  }
}

// Runs the colour demo.  Launched from global settings while the global-settings
// button is still held, so it deliberately does NOT switch display modes: entering
// displayAnimation clears controlButton (enterDisplayMode), which would make the
// still-held button's release get swallowed (handleControlButtonRelease bails when
// controlButton != sensorRow), leaving the instrument stuck in global settings.
// Instead it follows the stock text-scroll pattern -- animationActive only -- which
// updateDisplay() and the settings-screen repaint both defer to, and any touch
// still cancels.
void playColorShowDemo() {
  colorShowDemoActive = true;
  clearFullDisplay();

  // each scene plays for DEMO_SCENE_DURATION_MS, then the next takes over;
  // any touch cancels the whole show.  No clearing between scenes: every scene
  // repaints every cell every frame, so the hand-off is seamless.
  while (!stopAnimation) {
    animationActive = true;
    lastTouchMoment = millis();   // mode stays displayGlobal, so keep sleep at bay

    demoSceneSpectrum(millis());
    if (stopAnimation) break;

    demoSceneComplementary(millis());
    if (stopAnimation) break;

    demoSceneRipple(millis());
    if (stopAnimation) break;

    demoScenePrimaryCycle(millis());
  }

  stopAnimation = false;
  animationActive = false;
  colorShowDemoActive = false;
  clearFullDisplay();

  lastTouchMoment = millis();
  updateDisplay();
}
