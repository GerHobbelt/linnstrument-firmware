# LinnStrument 3x4 Scalar Layout Development Plan

## Goal

Add an opt-in global 3x4 Scalar Layout to LinnStrument OS 2.3.4 while keeping
the stock behavior unchanged whenever the feature is disabled.

## Compatibility contract

- Target hardware: LinnStrument 200.
- Base firmware: LinnStrument OS 2.3.4.
- Build target: Arduino Due (`arduino:sam:arduino_due_x`).
- Keep Global Settings, Per-Split Settings, sequencer, arpeggiator, low row,
  calibration, preset, MIDI, and user-firmware modes unchanged when the new
  mode is disabled.
- Produce a `.bin` suitable for the official LinnStrument Updater. Do not use
  direct Arduino upload as the normal installation path.

## Implementation phases

1. **Freeze and compile the stock baseline**
   - Record the unmodified 2.3.4 source state.
   - Install an isolated Arduino CLI and Arduino SAM Boards 1.6.11 toolchain.
   - Compile the unmodified firmware and record binary size and hashes.

2. **Map the existing architecture**
   - Trace physical cell to MIDI-note mapping.
   - Trace touch lifecycle, cell transitions, MPE channel ownership, and MIDI
     Note On/Off ordering.
   - Trace Global Settings input/display, flash persistence, factory reset, and
     configuration migration.
   - Trace performance-mode LED painting and active-note overlays.

3. **Add the global feature flag**
   - Add a visible, unused Global Settings cell without replacing an existing
     control.
   - Persist the flag and default it to OFF after factory reset.
   - Add backwards-compatible configuration migration.

4. **Add 3x4 tap mapping**
   - Right/left: +/-3 semitones.
   - Up/down: +/-4 semitones.
   - Continue to apply existing split transpose and octave settings.
   - Clamp final notes to MIDI 0-127.

5. **Add per-touch Scalar Swipe state**
   - New touch uses the fixed 3x4 pitch.
   - Each crossed pad step retriggers the next/previous note in the active
     Global scale.
   - Right/up ascend; left/down descend; diagonals count once.
   - Preserve pad-internal X/Y/Z and MPE behavior.
   - Add hysteresis, reversal handling, fast-swipe intermediate steps, current
     pressure to velocity mapping, and MIDI-range saturation.

6. **Add the LED system**
   - Seven scale degrees use white, cyan, green, yellow, blue, magenta, orange.
   - Out-of-scale pads are off.
   - Active physical cells and every pad whose fixed pitch matches an active
     MIDI note are red.
   - Restore the correct base color when an overlay ends.
   - Evaluate chromatic colors against the firmware's actual color set; do not
     weaken seven-note readability to force twelve colors.

7. **Verification and release artifacts**
   - Compile after each vertical slice.
   - Compare the feature-OFF paths against stock behavior at each hook point.
   - Add host-side tests for pure mapping/scale/step logic where practical.
   - Produce the test `.bin`, checksum, change log, flashing instructions, and
     an instrument test checklist.

## Current environment findings

- The source header identifies version 2.3.4.
- The bundled `DueFlashStorage` library is present.
- The repository snapshot has no Git metadata.
- No Arduino CLI, Arduino IDE, `bossac`, or Java runtime is currently present.
- The source documentation recommends Arduino IDE 1.8.1 with Arduino SAM
  Boards 1.6.11. The project will use an isolated command-line environment with
  SAM Boards 1.6.11 so the compiler/core versions remain reproducible.
- The firmware exposes six direct RGB colors plus time-multiplexed composite
  colors. Twelve reliably distinct chromatic colors are not currently
  guaranteed and require hardware evaluation.

## Hardware-gated acceptance work

The following require the physical LinnStrument and cannot be certified by a
successful compile alone:

- Swipe threshold, hysteresis, diagonal intent, and fast-swipe behavior.
- Pressure-to-velocity feel.
- Four-or-more-finger MPE behavior and MIDI event ordering.
- LED color distinguishability and refresh quality.
- Complete feature-OFF regression of the device UI and performance modes.
