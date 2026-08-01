/****************************** ls_rtos: LinnStrument Real Time OS ********************************
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
These functions comprise a simple Real Time OS for LinnStrument.
It consists of a delay function, delayUsec, that updates LinnStrument's LEDs and scans its foot
switches at specific time interals, all in the background. This should be used instead of
Arduino's delayMicroseconds() function.
**************************************************************************************************/

#include "ls_compiler_tweaks.h"
#include "ls_calcTimeDelta.h"

// delayUsec:
// use to insert a brief time delay.
// IMPORTANT: Use instead of Arduino's delayMicroseconds() function because this one handles background LED refresh and foot switch checking while it's waiting
inline void delayUsec(unsigned long delayTime) {    // input the delay time in microseconds
  unsigned long start = micros();                   // start is set to time that function is called
  unsigned long now = start;                        
  do {                                              // do the following while the interval between now and start less than delayTime
    performContinuousTasks(now);
    now = micros();                                 // reset now to current time and repeat...
  } while (calcTimeDelta(now, start) < delayTime);
}

// delayUsecWithScanning:
// use to insert a brief time delay but with key scanning still active.
//
// NOTE: this function will (recursively) execute the main `loop()` function!
// This is GOOD as this function is meant to be used only iff the specified delay
// is so large it will otherwise effectively 'lock up' the LinnStrument.
//
// So to make matters easier for all, this delay will take care of running the
// `loop()` anyhow, while it does protect about *recursive* invocation as we only
// want a single loop() content to execute at any time -- cooperative multitasking.
//
// This issue could potentially occur when the loop() scan detects user interaction
// (or otherwise) demanding the code to run another long-delay command, e.g. playbook
// or scrolling announcement text from inside this delay call: such would be a coding
// error (as the machine state should ideally prevent this), but we better protect 
// against this run-away layering of loop() calls by limiting the recursion to one
// layer only...
//
// Next, we should provide the caller with a signal that such recursive abuse occurred
// so they can safely abort the 'inner action'
// --> this function is used by all these:
// - playPromoAnimation()
// - font_scroll_text_flipped() / big_scroll_text_flipped() / small_scroll_text_flipped()
// - font_scroll_text() / big_scroll_text() / small_scroll_text()
// - playChristmasAnimation() / playPlayBook()
// which all have in common that they are aborted via `stopAnimation`, so we can be smart
// about it and do the same here...
//
void delayUsecWithScanning(unsigned long delayTime) {
  static byte callDepth = 0;
  callDepth++;
  bool innerUnacceptableRecursiveCallHappening = (callDepth > 1);

  if (innerUnacceptableRecursiveCallHappening) {
    if (animationActive) {
      DEBUGPRINT((-1, "\n\ndelayUsecWithScanning: inner, **unacceptable**, recursive loop() call happening: aborting the animation immediately!\n\n"));
      stopAnimation = true;
    }
    else {
      DEBUGPRINT((-1, "\n\ndelayUsecWithScanning: inner, **unacceptable**, recursive loop() call occurs outside the expected `animationActive` realm: SOFTWARE B0RK B0RK B0RK!\n\n"));
    }
  }
  else if (!setupDone) {
    // we can not have scanning unless the full setup routine is done,
    // falling back to regular delay in this case
    delayUsec(delayTime);
  }
  else {
    unsigned long start = micros();                   // start is set to time that function is called
    unsigned long now = start;                        // now is set once at function invocation...
    do {                                              // do the following while the interval between now and start less than delayTime
      loop();                                         // ( mostly eqv. to modeLoopPerformance() but this more generic.)
      now = micros();                                 // reset now to current time and repeat...
    } while (calcTimeDelta(now, start) < delayTime);
  }
  
  callDepth--;
}

void performCheckAdvanceArpeggiator() {
  static boolean continuousAdvanceArpeggiator = false;
  if (!continuousAdvanceArpeggiator) {
    continuousAdvanceArpeggiator = true;
    checkAdvanceArpeggiator();
    continuousAdvanceArpeggiator = false;
  }
}

void performCheckAdvanceSequencer() {
  static boolean continuousAdvanceSequencer = false;
  if (!continuousAdvanceSequencer) {
    continuousAdvanceSequencer = true;
    checkAdvanceSequencer();
    continuousAdvanceSequencer = false;
  }
}

inline void performContinuousTasks() {
  performContinuousTasks(micros());
}

void performContinuousTasks(unsigned long nowMicros) {
  if (!setupDone || displayMode == displaySleep) {
    return;
  }

  static boolean continuousSerialIO = false;

  boolean ledsRefreshed = false;  // TODO [GHo]
  
  static boolean continuousRefreshLeds = false;
  if (!continuousRefreshLeds && !continuousSerialIO) {
    continuousRefreshLeds = true;
    ledsRefreshed = checkRefreshLedColumn(nowMicros);
    continuousRefreshLeds = false;
  }
  
  if (ledsRefreshed) {
    unsigned long nowMillis = millis();

    static boolean continuousStopBlinkingLeds = false;
    if (!continuousStopBlinkingLeds) {
      continuousStopBlinkingLeds = true;
      checkStopBlinkingLeds(nowMillis);
      continuousStopBlinkingLeds = false;
    }
    
    static boolean continuousAdvanceTouchAnimations = false;
    if (!continuousAdvanceTouchAnimations) {
      continuousAdvanceTouchAnimations = true;
      checkTimeToRefreshTouchAnim(nowMillis);
      continuousAdvanceTouchAnimations = false;
    }

    static boolean continuousLegendDisplayTimeout = false;
    if (!continuousLegendDisplayTimeout) {
      continuousLegendDisplayTimeout = true;
      checkLegendDisplayTimeout(nowMillis);
      continuousLegendDisplayTimeout = false;
    }

    static boolean continuousFootSwitches = false;
    if (!continuousFootSwitches) {
      continuousFootSwitches = true;
      checkTimeToReadFootSwitches(nowMicros);
      continuousFootSwitches = false;
    }

    static boolean continuousSleep = false;
    if (!continuousSleep) {
      continuousSleep = true;
      checkSleep(nowMillis);
      continuousSleep = false;
    }
  }

  static boolean continuousUpdateClock = false;
  boolean clockUpdated = false;
  if (!continuousUpdateClock) {
    continuousUpdateClock = true;
    clockUpdated = checkUpdateClock(nowMicros);
    continuousUpdateClock = false;
  }

  if (clockUpdated) {
    performCheckAdvanceArpeggiator();
    performCheckAdvanceSequencer();
  }

  if (Device.serialMode) {
    if (!continuousSerialIO) {
      continuousSerialIO = true;
      handleSerialIO();
      continuousSerialIO = false;
    }
  }
  else {
    static boolean continuousMidiInput = false;
    if (!continuousMidiInput) {
      continuousMidiInput = true;
      handleMidiInput(nowMicros);
      continuousMidiInput = false;
    }

    static boolean continuousPendingMidi = false;
    if (!continuousPendingMidi) {
      continuousPendingMidi = true;
      handlePendingMidi(nowMicros);
      continuousPendingMidi = false;
    }
  }
}

// checks to see if it's time to refresh the next LED column, and if so, does it
//
// the return value indicates whether the LEDs were updated, so that we can use it
// as a coarse trigger to piggy-back other continuous tasks off of
inline boolean checkRefreshLedColumn(unsigned long now) {
  if (calcTimeDelta(now, prevLedTimerCount) > ledRefreshInterval) {        // is it time to refresh the next LED column?
    refreshLedColumn(now);                                                 // yes, refresh the next LED column...
    prevLedTimerCount = now;                                               // and reset the LED timer count to current time
    return true;
  }
  return false;
}

inline void checkTimeToRefreshTouchAnim(unsigned long now) {
  if (calcTimeDelta(now, prevTouchAnimTimerCount) > 33) {
    performAdvanceTouchAnimations(now);
    prevTouchAnimTimerCount = now;
  }
}

inline void checkTimeToReadFootSwitches(unsigned long now) {
  if (calcTimeDelta(now, prevFootSwitchTimerCount) > 20000) {              // is it time to check the foot switches?
    checkFootSwitches();                                                   // yes, check the foot switches and if state has changed, handle the event, then...
    prevFootSwitchTimerCount = now;                                        // reset the foot switch timer to current time
  }
}

void playSleepAnimation() {
  DEBUGPRINT((3,"playSleepAnimation: type="));
  DEBUGPRINT((3,Device.sleepAnimationType));
  DEBUGPRINT((3,"\n"));
  switch (Device.sleepAnimationType) {
    case animationNone:
      activateSleepMode();
      break;
    case animationStore:
      playPromoAnimation();
      break;
    case animationChristmas:
      playChristmasAnimation();
      break;
  }
}

// checks to see if it's time to sleep LinnStrument
inline void checkSleep(unsigned long now) {
  if (Device.sleepActive && Device.sleepDelay > 0 && displayMode != displayAnimation && displayMode != displaySleep &&
      calcTimeDelta(now, lastTouchMoment) > Device.sleepDelay * 60000) {
    playSleepAnimation();
  }
}

// checks whether it's time to stop blinking various LEDs
inline void checkStopBlinkingLeds(unsigned long now) {
  // should the blinking middle root note be stopped blinking
  if ((displayMode == displayNormal || displayMode == displaySplitPoint) && 
      blinkMiddleRootNote &&
      calcTimeDelta(now, displayModeStart) > (Device.operatingLowPower ? 1200 : 600)) {
    blinkMiddleRootNote = false;
    updateDisplay();
  }

  // check if there are blinking preset LEDs that need to be reset
  if (displayMode == displayPreset) {
    for (byte p = 0; p < NUMPRESETS; ++p) {
      if (presetBlinkStart[p] != 0 && calcTimeDelta(now, presetBlinkStart[p]) > 1200) {
        int color = globalColor;
        if (p == Device.lastLoadedPreset) {
          color = COLOR_CYAN;
        }
        int row = p + 2;
        if (row >= 6) row -= 6;
        setLed(getPresetDisplayColumn(), row, color, cellOn);
        presetBlinkStart[p] = 0;
      }
    }
  }

  // check if there are blinking project LEDs that need to be reset
  if (displayMode == displaySequencerProjects) {
    for (byte p = 0; p < MAX_PROJECTS; ++p) {
      if (projectBlinkStart[p] != 0 && calcTimeDelta(now, projectBlinkStart[p]) > 1200) {
        int color = globalColor;
        if (p == Device.lastLoadedProject) {
          color = COLOR_CYAN;
        }
        setLed(6 + p % 4, 2 + p / 4, color, cellOn);
        projectBlinkStart[p] = 0;
      }
    }
  }
}
