# 3×4 Scalar Layout x5 Hardware Test Guide

This is an experimental LinnStrument 200 build based on OS 2.3.4. It is not a
production release.

## Confirm the installed build

The device must display `234-x5` as its firmware version.

## Core checks

1. Enable Scalar Layout in Global Settings using playable column 19, bottom row.
2. Confirm that a horizontal neighbor is ±3 semitones and a vertical neighbor
   is ±4 semitones.
3. Confirm that enabling the layout lowers its effective register by one octave.
   Disable the layout and confirm the normal register returns immediately.
4. In C Major, swipe in each direction across one pad. Right/up must select the
   next scale note upward; left/down must select the next scale note downward.
5. Swipe quickly across two to four pads and verify that intermediate scale
   notes are retriggered.
6. Play two or more fingers in MPE/Channel Per Note mode and confirm they act
   independently.
7. Confirm that red LEDs match only pads with the exact sounding MIDI note,
   not the same pitch class in other octaves.
8. Repeatedly tap and release notes while watching the panel. No unrelated
   column should briefly flash white or another scale color.
9. Disable Scalar Layout and spot-check normal layout, split, low row,
   arpeggiator, sequencer, and settings behavior.

Record any missed trigger, double trigger, unexpected MIDI event, LED residue,
or gesture that feels too sensitive.
