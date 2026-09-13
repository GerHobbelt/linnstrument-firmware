// x6 Dynamic Strum: role resolution, held voicing, and dynamic row pitch generation.
#ifndef STRUM_OFF
#define STRUM_OFF 0
#define STRUM_CLASSIC 1
#define STRUM_DYNAMIC 2
#endif

const unsigned long DYNAMIC_STRUM_HOLD_MS = 1500;

byte getDynamicStrumSplit() {
  if (!Global.splitActive) return 255;
  if (Split[LEFT].strum == STRUM_DYNAMIC) return LEFT;
  if (Split[RIGHT].strum == STRUM_DYNAMIC) return RIGHT;
  return 255;
}

byte getDynamicVoicingSplit() {
  byte s = getDynamicStrumSplit();
  return s == 255 ? 255 : otherSplit(s);
}

boolean isDynamicStrumSplit(byte split) {
  return Global.splitActive && Split[split].strum == STRUM_DYNAMIC;
}

boolean isDynamicVoicingSplit(byte split) {
  byte s = getDynamicVoicingSplit();
  return s != 255 && s == split;
}

// Build strictly ascending notes while preserving the sorted voicing pitch-class order.
void buildDynamicStrumNotes(byte voicingSplit, short* output) {
  short held[MAXCOLS * MAXROWS];
  byte count = 0;
  for (byte col = 1; col < NUMCOLS; ++col) {
    if (getSplitOf(col) != voicingSplit) continue;
    for (byte row = 0; row < NUMROWS; ++row) {
      TouchInfo& t = cell(col, row);
      if (t.touched == touchedCell && t.note >= 0 && t.note <= 127 && count < MAXCOLS * MAXROWS)
        held[count++] = t.note;
    }
  }
  for (byte i = 1; i < count; ++i) {
    short v = held[i]; byte j = i;
    while (j > 0 && held[j - 1] > v) { held[j] = held[j - 1]; --j; }
    held[j] = v;
  }
  for (byte i = 0; i < MAXROWS; ++i) output[i] = -1;
  if (!count) return;
  output[0] = held[0];
  for (byte i = 1; i < MAXROWS; ++i) {
    byte pc = held[i % count] % 12;
    short prev = output[i - 1];
    short delta = (pc - (prev % 12) + 12) % 12;
    if (!delta) delta = 12;
    short candidate = prev + delta;
    if (candidate > 127) break;
    output[i] = candidate;
  }
}

void dynamicStrumCaptureVoicing(byte split) {
  sensorCell->note = cellTransposedNote(split);
  sensorCell->channel = -1;
}

void dynamicStrumTrigger(byte split, boolean retrigger) {
  short notes[MAXROWS];
  buildDynamicStrumNotes(getDynamicVoicingSplit(), notes);
  if (sensorRow >= MAXROWS || notes[sensorRow] < 0) return;
  if (retrigger && sensorCell->hasNote()) midiSendNoteOff(split, sensorCell->note, sensorCell->channel);
  byte channel = takeChannel(getDynamicVoicingSplit(), sensorRow);
  sensorCell->note = notes[sensorRow];
  sensorCell->channel = channel;
  sensorCell->velocity = sensorCell->velocity ? sensorCell->velocity : 127;
  midiSendNoteOn(getDynamicVoicingSplit(), sensorCell->note, sensorCell->velocity, channel);
}

void dynamicStrumRelease(byte split) {
  if (isDynamicVoicingSplit(split)) { sensorCell->note = -1; sensorCell->channel = -1; return; }
  if (isDynamicStrumSplit(split) && sensorCell->hasNote()) {
    midiSendNoteOff(getDynamicVoicingSplit(), sensorCell->note, sensorCell->channel);
    releaseChannel(getDynamicVoicingSplit(), sensorCell->channel);
    sensorCell->note = -1; sensorCell->channel = -1;
  }
}

void setDynamicStrumMode(byte split, byte mode) {
  if (mode > STRUM_DYNAMIC) mode = STRUM_OFF;
  // Release any notes owned by the current Dynamic output before changing roles.
  for (byte row = 0; row < MAXROWS; ++row) {
    if (virtualTouchInfo[row].hasNote()) virtualTouchInfo[row].releaseNote();
  }
  for (byte col = 1; col < NUMCOLS; ++col) {
    for (byte row = 0; row < NUMROWS; ++row) {
      TouchInfo& t = cell(col, row);
      if (t.hasNote() && (isDynamicStrumSplit(getSplitOf(col)) || isDynamicVoicingSplit(getSplitOf(col)))) {
        if (isDynamicStrumSplit(getSplitOf(col))) {
          midiSendNoteOff(getDynamicVoicingSplit(), t.note, t.channel);
          releaseChannel(getDynamicVoicingSplit(), t.channel);
        }
        t.note = -1; t.channel = -1;
      }
      else if (isDynamicStrumSplit(getSplitOf(col)) || isDynamicVoicingSplit(getSplitOf(col))) {
        t.note = -1; t.channel = -1;
      }
    }
  }
  Split[split].strum = mode;
  if (mode == STRUM_DYNAMIC) {
    byte other = otherSplit(split);
    if (Split[other].strum == STRUM_DYNAMIC) Split[other].strum = STRUM_OFF;
    Split[other].strum = false;
    Split[split].arpeggiator = false;
    Split[split].ccFaders = false;
    setSplitSequencerEnabled(split, false);
  }
  updateDisplay();
}

byte dynamicShortPressMode(byte split) {
  byte mode = Split[split].strum;
  if (mode == STRUM_DYNAMIC) return STRUM_CLASSIC;
  return mode == STRUM_CLASSIC ? STRUM_OFF : STRUM_CLASSIC;
}

byte dynamicLongPressMode(byte split) {
  byte mode = Split[split].strum;
  return mode == STRUM_DYNAMIC ? STRUM_OFF : STRUM_DYNAMIC;
}
