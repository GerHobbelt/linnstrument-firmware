// x6 Dynamic Strum: role resolution, held voicing, and dynamic row pitch generation.
#ifndef STRUM_OFF
#define STRUM_OFF 0
#define STRUM_CLASSIC 1
#define STRUM_DYNAMIC 2
#endif

const unsigned long DYNAMIC_STRUM_HOLD_MS = 1500;

#define DYNAMIC_NOTE_INVALID (-1)
struct DynamicSoundingNote { boolean sounding; signed char channel; byte split; };
short dynamicLiveMap[MAXROWS];
short dynamicStrumSnapshot[MAXROWS];
boolean dynamicLiveMapValid = false;
boolean dynamicStrumSnapshotValid = false;
byte dynamicVoicingTouchCount = 0;
byte dynamicStrumTouchCount = 0;
DynamicSoundingNote dynamicSoundingNotes[128];

void clearDynamicPitchMaps() {
  for (byte row=0; row<MAXROWS; ++row) { dynamicLiveMap[row]=DYNAMIC_NOTE_INVALID; dynamicStrumSnapshot[row]=DYNAMIC_NOTE_INVALID; }
  dynamicLiveMapValid=false; dynamicStrumSnapshotValid=false;
}
void clearDynamicSoundingState() {
  for (byte n=0; n<128; ++n) { dynamicSoundingNotes[n].sounding=false; dynamicSoundingNotes[n].channel=-1; dynamicSoundingNotes[n].split=0; }
}
void releaseAllSoundingDynamicNotes() {
  for (byte n=0; n<128; ++n) if (dynamicSoundingNotes[n].sounding) {
    midiSendNoteOff(dynamicSoundingNotes[n].split,n,dynamicSoundingNotes[n].channel);
    releaseChannel(dynamicSoundingNotes[n].split,dynamicSoundingNotes[n].channel);
  }
  clearDynamicSoundingState();
}
byte countDynamicTouches(byte split) {
  byte count=0;
  for (byte col=1; col<NUMCOLS; ++col) if (getSplitOf(col)==split)
    for (byte row=0; row<NUMROWS; ++row) if (cell(col,row).touched==touchedCell) ++count;
  return count;
}
void rebuildDynamicLiveMap();
void refreshDynamicTouchState() {
  byte strum=getDynamicStrumSplit(); byte voicing=getDynamicVoicingSplit();
  byte oldStrum=dynamicStrumTouchCount;
  dynamicVoicingTouchCount=(voicing==255)?0:countDynamicTouches(voicing);
  dynamicStrumTouchCount=(strum==255)?0:countDynamicTouches(strum);
  if (oldStrum==0 && dynamicStrumTouchCount>0) {
    for (byte row=0; row<MAXROWS; ++row) dynamicStrumSnapshot[row]=dynamicLiveMap[row];
    dynamicStrumSnapshotValid=dynamicLiveMapValid;
  }
  else if (oldStrum>0 && dynamicStrumTouchCount==0) {
    dynamicStrumSnapshotValid=false;
    for (byte row=0; row<MAXROWS; ++row) dynamicStrumSnapshot[row]=DYNAMIC_NOTE_INVALID;
  }
  if (dynamicVoicingTouchCount==0 && dynamicStrumTouchCount==0) {
    releaseAllSoundingDynamicNotes();
    dynamicStrumSnapshotValid=false;
  }
}
void resetDynamicRuntime() { releaseAllSoundingDynamicNotes(); clearDynamicPitchMaps(); dynamicVoicingTouchCount=0; dynamicStrumTouchCount=0; }

struct DynamicStrumPressState {
  boolean active;
  boolean longPressTriggered;
  byte split;
  unsigned long startedAt;
};

DynamicStrumPressState dynamicStrumPress = { false, false, 255, 0 };

void beginDynamicStrumPress(byte split) {
  dynamicStrumPress.active = true;
  dynamicStrumPress.longPressTriggered = false;
  dynamicStrumPress.split = split;
  dynamicStrumPress.startedAt = millis();
}

boolean updateDynamicStrumHold() {
  if (!dynamicStrumPress.active || dynamicStrumPress.longPressTriggered) return false;
  if (calcTimeDelta(millis(), dynamicStrumPress.startedAt) < DYNAMIC_STRUM_HOLD_MS) return false;
  byte split = dynamicStrumPress.split;
  setDynamicStrumMode(split, dynamicLongPressMode(split));
  dynamicStrumPress.longPressTriggered = true;
  updateDisplay();
  return true;
}

void finishDynamicStrumPress() {
  if (!dynamicStrumPress.active) return;
  byte split = dynamicStrumPress.split;
  if (!dynamicStrumPress.longPressTriggered) {
    if (calcTimeDelta(millis(), dynamicStrumPress.startedAt) >= DYNAMIC_STRUM_HOLD_MS) {
      setDynamicStrumMode(split, dynamicLongPressMode(split));
    }
    else {
      setDynamicStrumMode(split, dynamicShortPressMode(split));
    }
  }
  dynamicStrumPress.active = false;
  dynamicStrumPress.longPressTriggered = false;
  dynamicStrumPress.split = 255;
  dynamicStrumPress.startedAt = 0;
}

void cancelDynamicStrumPress() {
  dynamicStrumPress.active = false;
  dynamicStrumPress.longPressTriggered = false;
  dynamicStrumPress.split = 255;
  dynamicStrumPress.startedAt = 0;
}


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

void rebuildDynamicLiveMap() {
  byte voicing=getDynamicVoicingSplit();
  if (voicing==255) { clearDynamicPitchMaps(); return; }
  buildDynamicStrumNotes(voicing,dynamicLiveMap);
  dynamicLiveMapValid=(dynamicLiveMap[0]!=DYNAMIC_NOTE_INVALID);
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
  rebuildDynamicLiveMap();
  refreshDynamicTouchState();
}

void dynamicStrumTrigger(byte split, boolean retrigger) {
  rebuildDynamicLiveMap();
  refreshDynamicTouchState();
  if (!dynamicStrumSnapshotValid || sensorRow >= MAXROWS) return;
  short pitch = dynamicStrumSnapshot[sensorRow];
  if (pitch < 0 || pitch > 127) return;
  if (dynamicSoundingNotes[pitch].sounding) {
    midiSendNoteOff(dynamicSoundingNotes[pitch].split, pitch, dynamicSoundingNotes[pitch].channel);
    releaseChannel(dynamicSoundingNotes[pitch].split, dynamicSoundingNotes[pitch].channel);
    dynamicSoundingNotes[pitch].sounding=false;
  }
  byte midiSplit=getDynamicVoicingSplit();
  if (midiSplit==255) return;
  byte channel=takeChannel(midiSplit,sensorRow);
  byte velocity=sensorCell->velocity ? sensorCell->velocity : 127;
  midiSendNoteOn(midiSplit,pitch,velocity,channel);
  dynamicSoundingNotes[pitch].sounding=true;
  dynamicSoundingNotes[pitch].channel=channel;
  dynamicSoundingNotes[pitch].split=midiSplit;
  sensorCell->note=pitch;
  sensorCell->channel=channel;
  sensorCell->velocity=velocity;
}

void dynamicStrumRelease(byte split) {
  sensorCell->note = -1;
  sensorCell->channel = -1;
  rebuildDynamicLiveMap();
  refreshDynamicTouchState();
}

void setDynamicStrumMode(byte split, byte mode) {
  if (mode > STRUM_DYNAMIC) mode = STRUM_OFF;
  // Mode changes force-release all Dynamic voices and clear runtime state.
  resetDynamicRuntime();
  for (byte col=1; col<NUMCOLS; ++col) for (byte row=0; row<NUMROWS; ++row) {
    if (getSplitOf(col)==split || getSplitOf(col)==otherSplit(split)) { cell(col,row).note=-1; cell(col,row).channel=-1; }
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
