/************************** ls_test: LinnStrument debugging and testing ***************************
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
Assorted debug functions. 
**************************************************************************************************/

#include "ls_compiler_tweaks.h"


static void debugPrint1(const char* msg) {
  Serial.print(msg);
}

static void debugPrintln1(const char* msg) {
  Serial.println(msg);
}

static void debugPrint1(int val) {
  Serial.print(val);
}

static void debugPrintln1(int val) {
  Serial.println(val);
}

inline void debugPrint(int level, const char* msg) {
  if (Device.serialMode && (debugLevel >= level)) {
    debugPrint1(msg);
  }
}

inline void debugPrintln(int level, const char* msg) {
  if (Device.serialMode && (debugLevel >= level)) {
    debugPrintln1(msg);
  }
}

inline void debugPrint(int level, int val) {
  if (Device.serialMode && (debugLevel >= level)) {
    debugPrint1(val);
  }
}

inline void debugPrintln(int level, int val) {
  if (Device.serialMode && (debugLevel >= level)) {
    debugPrintln1(val);
  }
}

void debugprint_funcname(const char *fname) {
  if (SWITCH_SURFACESCAN) {
    static const char *touchInfo[] = { "untouched", "ignored", "transfer", "touched" };

    DEBUGPRINT((2,fname));
    auto l = strlen(fname);
    static const char *align_ws = "                              ";
    const auto ws_len = sizeof(align_ws) - 1;
    if (l < ws_len) {
      DEBUGPRINT((2, align_ws - l));
    }
    DEBUGPRINT((2,": col="));DEBUGPRINT((2,(int)sensorCol));
    DEBUGPRINT((2," row="));DEBUGPRINT((2,(int)sensorRow));
    DEBUGPRINT((2," veloZ="));DEBUGPRINT((2,(int)sensorCell->velocityZ));
    DEBUGPRINT((2," pressZ="));DEBUGPRINT((2,(int)sensorCell->pressureZ));
    DEBUGPRINT((2," velo="));DEBUGPRINT((2,(int)sensorCell->velocity));
    DEBUGPRINT((2," touch="));DEBUGPRINT((2,touchInfo[int(sensorCell->touched)]));
    DEBUGPRINT((2,"\n"));
  }
}

void debugprint_funcname_L5(const char *fname) {
  if (SWITCH_SURFACESCAN) {
    DEBUGPRINT((4,fname));
    auto l = strlen(fname);
    static const char *align_ws = "                              ";
    const auto ws_len = sizeof(align_ws) - 1;
    if (l < ws_len) {
      DEBUGPRINT((4, align_ws - l));
    }
    DEBUGPRINT((4,": anim="));
    DEBUGPRINT((4,int(animationActive)));
    DEBUGPRINT((4," mode="));
    DEBUGPRINT((4,int(displayMode)));
    DEBUGPRINT((4,"\n"));
  }
}

void debugprint_funcname_L5(const char *fname, const char *addenda) {
  if (SWITCH_SURFACESCAN) {
    DEBUGPRINT((4,fname));
    auto l = strlen(fname);
    static const char *align_ws = "                              ";
    const auto ws_len = sizeof(align_ws) - 1;
    if (l < ws_len) {
      DEBUGPRINT((4, align_ws - l));
    }
    DEBUGPRINT((4,": anim="));
    DEBUGPRINT((4,int(animationActive)));
    DEBUGPRINT((4," mode="));
    DEBUGPRINT((4,int(displayMode)));
    DEBUGPRINT((4,", "));
    DEBUGPRINT((4,addenda));
    DEBUGPRINT((4,"\n"));
  }
}

void debugprint_funcname_L0(const char *fname) {
  if (SWITCH_SURFACESCAN) {
    DEBUGPRINT((0,fname));
    auto l = strlen(fname);
    static const char *align_ws = "                              ";
    const auto ws_len = sizeof(align_ws) - 1;
    if (l < ws_len) {
      DEBUGPRINT((0, align_ws - l));
    }
    DEBUGPRINT((0,": anim="));
    DEBUGPRINT((0,int(animationActive)));
    DEBUGPRINT((0," mode="));
    DEBUGPRINT((0,int(displayMode)));
    DEBUGPRINT((0,"\n"));
  }
}

void displayDigitalPins() {
  static unsigned long lastFrame = 0;
  unsigned long now = micros();
  if (sensorCol == 1 && sensorRow == 0 && calcTimeDelta(now, lastFrame) >= 500000) {
    lastFrame = now;

    Serial.println();
    Serial.print("MCU/Pins info:\n");
    displayDigitalPins(0, 27);
    Serial.println();
    displayDigitalPins(27, 54);
  }
}

void displayDigitalPins(byte start, byte end) {
  for (byte p = start; p < end; ++p) {
    Serial.print(p);
    Serial.print("\t");
  }
  Serial.println();
  for (byte p = start; p < end; ++p) {
    // ripped from the Arduino RTL:
  	if (g_APinDescription[p].ulPinType == PIO_NOT_A_PIN) {
      Serial.print("---");
    }
    else {
      byte mode = g_pinStatus[p] & 0xF;
      switch (mode) {
      default:
        Serial.print("?_");
        Serial.print(mode);
        break;
      case 0:
        Serial.print("<~>");
        break;
      case PIN_STATUS_ANALOG:
        Serial.print("ANA");
        break;
      case PIN_STATUS_DIGITAL_OUTPUT:
        Serial.print("OUT");
        break;
      case PIN_STATUS_DIGITAL_INPUT:
        Serial.print("IN");
        break;
      case PIN_STATUS_DIGITAL_INPUT_PULLUP:
        Serial.print("IN+P");
        break;
      case PIN_STATUS_PWM:
        Serial.print("PWM");
        break;
      }
    }
    Serial.print("\t");
  }
  Serial.println();
  for (byte p = start; p < end; ++p) {
    Serial.print(digitalRead(p));
    Serial.print("\t");
  }
  Serial.println();
}

// displayXFrame:
// For debug, displays an entire frame of raw X values in the Arduino serial monitor. Values are collected during each full read of the touch surface.
void displayXFrame() {
  if (sensorCell->touched == touchedCell) {
    sensorCell->refreshX();
  }

  static unsigned long lastFrame = 0;
  unsigned long now = micros();
  if (sensorCol == 1 && sensorRow == 0 && calcTimeDelta(now, lastFrame) >= 500000) {
    lastFrame = now;

    Serial.println();
    Serial.print("XFrame:\n");
    for (byte x = 0; x < NUMCOLS; ++x) {
      Serial.print(x);
      Serial.print("\t");
    }
    Serial.println();
    for (byte y = NUMROWS; y > 0; --y) {
      for (byte x = 0; x < NUMCOLS; ++x) {
        Serial.print(cell(x, y-1).currentRawX);
        if (cell(x, y-1).touched == touchedCell) {
          Serial.print("_#");
        }
        Serial.print("\t");
      }
      Serial.println();
    }
  }
}

// displayYFrame:
// For debug, displays an entire frame of raw Y values in the Arduino serial monitor. Values are collected during each full read of the touch surface.
void displayYFrame() {
  if (sensorCell->touched == touchedCell) {
    sensorCell->refreshY();
  }

  static unsigned long lastFrame = 0;
  unsigned long now = micros();
  if (sensorCol == 1 && sensorRow == 0 && calcTimeDelta(now, lastFrame) >= 500000) {
    lastFrame = now;
    
    Serial.println();
    Serial.print("YFrame:\n");
    for (byte x = 0; x < NUMCOLS; ++x) {
      Serial.print(x);
      Serial.print("\t");
    }
    Serial.println();
    for (byte y = NUMROWS; y > 0; --y) {
      for (byte x = 0; x < NUMCOLS; ++x) {
        Serial.print(cell(x, y-1).currentRawY);
        if (cell(x, y-1).touched == touchedCell) {
          Serial.print("_#");
        }
        Serial.print("\t");
      }
      Serial.println();
    }
  }
}

// displayZFrame:
// For debug, displays an entire frame of raw Z values in the Arduino serial monitor. Values are collected during each full read of the touch surface.
void displayZFrame() {
  static unsigned long lastFrame = 0;
  unsigned long now = micros();
  if (sensorCol == 1 && sensorRow == 0 && calcTimeDelta(now, lastFrame) >= 500000) {
    lastFrame = now;
    
    Serial.println();
    Serial.print("ZFrame:\n");
    for (byte x = 0; x < NUMCOLS; ++x) {
      Serial.print(x);
      Serial.print("\t");
    }
    Serial.println();
    for (byte y = NUMROWS; y > 0; --y) {
      for (byte x = 0; x < NUMCOLS; ++x) {
        Serial.print(cell(x, y-1).currentRawZ);
        if (cell(x, y-1).touched == touchedCell) {
          Serial.print("_#");
        }
        Serial.print("\t");
      }
      Serial.println();
    }
  }
}

// For debug, displays an entire frame of raw Z values in the Arduino serial monitor. Values are collected during each full read of the touch surface.
void displaySurfaceScanTime() { 
  unsigned long now = micros();
  static unsigned long lastFrame = now;
  if (sensorCol == 1 && sensorRow == 0) {
    static int scanCount = 0; 
    static unsigned long scanPeriod = micros();
    ++scanCount;
    if (calcTimeDelta(now, lastFrame) >= 500000 && scanCount > 0) {
      lastFrame = now;
      Serial.print("Total surface scan time in microseconds: ");
      Serial.print((micros() - scanPeriod) / scanCount);
      Serial.print(", calculated across ");
      Serial.print(scanCount);
      Serial.println(" scans.");
      scanPeriod = micros(); 
      scanCount = 0;   
    }
  }
}

// displayCellTouchedFrame:
// For debug, displays an entire frame of 'touch' states in the Arduino serial monitor. Values are collected during each full read of the touch surface.
void displayCellTouchedFrame() {
  static unsigned long lastFrame = 0;
  unsigned long now = micros();
  if (sensorCol == 1 && sensorRow == 0 && calcTimeDelta(now, lastFrame) >= 500000) {
    lastFrame = now;

    Serial.println();
    Serial.print("CellTouchedFrame:\n");
    for (byte x = 0; x < NUMCOLS; ++x) {
      Serial.print(x);
      Serial.print("\t");
    }
    Serial.println();
    for (byte y = NUMROWS; y > 0; --y) {
      for (byte x = 0; x < NUMCOLS; ++x) {
        switch (cell(x, y-1).touched) {
        case untouchedCell:
          Serial.print("-");
          break;
        case ignoredCell:
          Serial.print("ignor");
          break;
        case transferCell:
          Serial.print("TRANS");
          break;
        case touchedCell:
          Serial.print("TOUCH");
          break;
        }
        //Serial.print(cell(x, y-1).touched);
        Serial.print("\t");
      }
      Serial.println();
    }
  }
}

void modeLoopManufacturingTest() {
  TouchState previousTouch = sensorCell->touched;
  sensorCell->refreshZ();

  // highlight the touches
  if (previousTouch != touchedCell && sensorCell->isMeaningfulTouch()) {
    cellTouched(touchedCell);

    if (sensorCol == 0) {
      byte color = COLOR_OFF;
      switch (sensorRow)
      {
        case 0:
        case 1:
          color = COLOR_YELLOW;
          break;
        case 2:
          color = COLOR_MAGENTA;
          break;
        case 3:
          color = COLOR_CYAN;
          break;
        case 4:
        case 5:
          color = COLOR_BLUE;
          break;
        case 6:
          color = COLOR_GREEN;
          break;
        case 7:
          color = COLOR_RED;
          break;
      }

      for (byte col = 0; col < NUMCOLS; ++col) {
        for (byte row = 0; row < NUMROWS; ++row) {
          setLed(col, row, color, cellOn);
        }
      }
    }
  }
  else if (previousTouch != untouchedCell && !sensorCell->isActiveTouch()) {
    cellTouched(untouchedCell);

    if (cellsTouched == 0) {
      for (byte col = 0; col < NUMCOLS; ++col) {
        for (byte row = 0; row < NUMROWS; ++row) {
          clearLed(col, row);
        }
      }
    }
  }

  if (sensorCol != 0 && sensorCell->touched != untouchedCell) {
    paintLowRowPressureBar();
  }

  if (rowsInColsTouched[0] == 0) {
    if (LINNMODEL == 200) {
      lightLed(4, 1);
      lightLed(4, 6);
      lightLed(10, 1);
      lightLed(10, 6);
      lightLed(16, 1);
      lightLed(16, 6);
      lightLed(22, 1);
      lightLed(22, 6);
    }
    else if (LINNMODEL == 128) {
      lightLed(2, 1);
      lightLed(2, 6);
      lightLed(6, 1);
      lightLed(6, 6);
      lightLed(11, 1);
      lightLed(11, 6);
      lightLed(15, 1);
      lightLed(15, 6);
    }
  }

  unsigned long now = micros();
  
  // send out MIDI activity
  midiOutQueue.push((byte)MIDIActiveSensing);
  handlePendingMidi(now);
  handleMidiInput(now);

  checkRefreshLedColumn(now);
  checkTimeToReadFootSwitches(now);
  nextSensorCell();
}

#ifdef DEBUG_ENABLED

#include <malloc.h>

extern char _end;
extern "C" char* sbrk(int i);
char* ramstart = (char*)0x20070000;
char* ramend = (char*)0x20088000;

void debugFreeRam() {
  static unsigned long lastFrame = 0;
  unsigned long now = micros();
  if (Device.serialMode && sensorCol == 1 && sensorRow == 0 && calcTimeDelta(now, lastFrame) >= 500000) {
    lastFrame = now;

    char* heapend = sbrk(0);
    register char* stack_ptr asm ("sp");
    struct mallinfo mi = mallinfo();
    Serial.print("RAM dynamic:");
    Serial.print(mi.uordblks);
    Serial.print(" static:");
    Serial.print(&_end - ramstart);
    Serial.print(" free:");
    Serial.println(stack_ptr - heapend + mi.fordblks);

    // read RTT (Real Time Timer) value (seconds elapsed):
    const RoReg& rtt_vr = REG_RTT_VR;
    Serial.print("Real Time Timer: RTT_VR:");
    Serial.print(rtt_vr);
    Serial.print("\n");

    Serial.print("Chip Identifier: 	CHIPID_CIDR:");
    const RoReg& cidr = REG_CHIPID_CIDR;
    const RoReg& cidr_ext = REG_CHIPID_EXID;
    const auto v = cidr;
    const auto ext = v >> 31;
    const auto nvptyp = (v >> 28) & 0b0111;
    const auto arch = (v >> 20) & 0b11111111;
    const auto sramsiz = (v >> 16) & 0b1111;
    const auto nvpsiz2 = (v >> 12) & 0b1111;
    const auto nvpsiz1 = (v >> 8) & 0b1111;
    const auto eproc = (v >> 5) & 0b0111;
    const auto version = (v >> 0) & 0b00011111;
    Serial.print(v, 16);
    Serial.print(" EXT:");
    Serial.print(ext, 16);
    Serial.print(" NVPTYP:");
    Serial.print(nvptyp, 16);
    Serial.print(" ARCH:");
    Serial.print(arch, 16);
    Serial.print(" SRAMSIZ:");
    Serial.print(sramsiz, 16);
    Serial.print(" NVPSIZ2:");
    Serial.print(nvpsiz2, 16);
    Serial.print(" NVPSIZ1:");
    Serial.print(nvpsiz1, 16);
    Serial.print(" EPROC:");
    Serial.print(eproc, 16);
    Serial.print(" VERSION:");
    Serial.print(version, 16);

    Serial.print("    CIDR_EXT:");
    Serial.print(cidr_ext, 16);
    Serial.print("\n");

    // TC_hitcount
    Serial.print("TC_hitcount: t=");
    Serial.print(millis() / 1000);
    auto ck_en = pmc_is_periph_clk_enabled(ID_TC1);
    Serial.print(" TC1.clock_enable=");
    Serial.print(ck_en);

    for (byte i = 0; i <= 8; i++) {
      Serial.print(" [");
      Serial.print(i);
      Serial.print("]=");
      Serial.print(TC_hitcount[i]);
      
      static Tc * const idToTC[] = {
        TC0, TC0, TC0, 
        TC1, TC1, TC1, 
        TC2, TC2, TC2 };

      auto v = TC_ReadCV(idToTC[i], i % 3);
      Serial.print(" / CV=");
      Serial.print(v);
    }
    Serial.print("\n");

    Tc *tc = TC1;
    Serial.print("TC1: CCR=");
    Serial.print(tc->TC_CHANNEL[0].TC_CCR, 16); // Write-only
    Serial.print(" CMR=");
    Serial.print(tc->TC_CHANNEL[0].TC_CMR, 16);
    Serial.print(" SMMR=");
    Serial.print(tc->TC_CHANNEL[0].TC_SMMR, 16);
    Serial.print(" CV=");
    Serial.print(tc->TC_CHANNEL[0].TC_CV, 16);
    Serial.print(" RA=");
    Serial.print(tc->TC_CHANNEL[0].TC_RA, 16);
    Serial.print(" RB=");
    Serial.print(tc->TC_CHANNEL[0].TC_RB, 16);
    Serial.print(" RC=");
    Serial.print(tc->TC_CHANNEL[0].TC_RC, 16);
    Serial.print(" SR=");
    Serial.print(tc->TC_CHANNEL[0].TC_SR, 16);
    Serial.print(" IER=");
    Serial.print(tc->TC_CHANNEL[0].TC_IER, 16); // Write-only
    Serial.print(" IDR=");
    Serial.print(tc->TC_CHANNEL[0].TC_IDR, 16); // Write-only
    Serial.print(" IMR=");
    Serial.print(tc->TC_CHANNEL[0].TC_IMR, 16);

    Serial.print("   TC_BCR=");
    Serial.print(tc->TC_BCR, 16);
    Serial.print(" TC_BMR=");
    Serial.print(tc->TC_BMR, 16);
    Serial.print(" TC_FMR=");
    Serial.print(tc->TC_FMR, 16);
    Serial.print(" TC_WPMR=");
    Serial.print(tc->TC_WPMR, 16);
    Serial.print("\n");
  }
}

#endif
