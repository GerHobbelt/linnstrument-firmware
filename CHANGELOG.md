# Changelog

## x5

- Added atomic played-note LED refreshes to eliminate transient panel-column
  flashes during note-on, note-off, and Scalar Swipe events.
- Changed the Scalar Layout default register to one octave below the normal
  layout without modifying the user's saved Octave setting.
- Device version display is `234-x5`.

## Earlier development builds

Earlier S01, S02, x3, and x4 builds were internal hardware-test iterations and
are intentionally not included in this clean release branch.

## 2.3.4-x6

- Added per-split Dynamic Strum mode with symmetric output/voicing roles.
- Added 1.5-second long press for Dynamic Strum and retained short-press Classic Strum.
- Added live voicing-derived eight-row pitch mapping and Dynamic LED presentation.

## Dynamic Strum sustain update

- Added per-gesture voicing snapshots so Live Voicing changes do not retune an active sweep.
- Added shared sustain lifetime based on both Voicing and Strum touch counts.
- Added one-entry-per-MIDI-pitch sounding tracking with Note Off + Note On retrigger.
