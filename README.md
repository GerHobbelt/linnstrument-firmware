LinnStrument — chord-grid fork
==============================

This is the firmware for LinnStrument, the expressive MIDI controller made by Roger Linn Design,
together with the tools around it. The firmware runs on the Arduino Due inside the instrument.
The repository is set up so that anyone can work on it with an AI coding assistant, musicians
included, not only programmers.

Three ways in
-------------

1. **Use the browser pages, nothing to install.** The [browser pages](examples/README.md) talk
   to a LinnStrument over MIDI from Chrome, Edge or Firefox. There's a surface designer that
   turns the instrument into a custom controller of pads, keys, faders, XY pads and buttons for
   any DAW, an editor for every setting, and a live view of User Firmware Mode. Just open
   https://uwyn.com/linnstrument/ in Chrome, Edge or Firefox (Safari has no MIDI for web pages),
   allow MIDI when the browser asks, open one of the pages and pick the instrument at the top
   right. [examples/README.md](examples/README.md) describes them, and how to run them from a
   copy of this repository instead. They work with the current release,
   2.3.4. The next firmware, 2.4.0, isn't released yet and adds model detection, faster lighting
   and text. Until then, the connection panel tells you what's waiting for it.

2. **Change the firmware without installing anything.** Claude Code on the web edits the
   firmware on a branch in the cloud. Every push is compiled by GitHub Actions, which attaches
   the firmware file to the run, and the LinnStrument Updater flashes it while keeping your
   settings. "On the web, start to finish" in [CONTRIBUTING.md](CONTRIBUTING.md) lists the
   steps, and the rest of that file has the prompts to start with and tells you what's safe.

3. **Develop on your own computer, with or without an assistant.** Plain development works as it
   always has. You can open `linnstrument-firmware.ino` in Arduino IDE 2.x and press Verify,
   after copying its one library (see `CONTRIBUTING.md`), or build from the command line with Go
   and arduino-cli. A desktop test suite checks the firmware's logic in seconds without an
   instrument, the Virtual LinnStrument runs the firmware behind a window with a virtual MIDI
   port, and a pull request brings the change in. An AI assistant in the terminal can drive all
   of that for you and, when you say so, a connected instrument too. `CONTRIBUTING.md` lists
   what to install and the workflow, and its "On a Mac, start to finish" lists the steps with an
   assistant. `CLAUDE.md` is what the assistant reads (other assistants find it through
   `AGENTS.md`), and it's useful reading for people too.

![The surface designer page with a LinnStrument 128 connected](docs/images/surface-designer.png)

![The Virtual LinnStrument running the firmware on a Mac](docs/images/virtual-linnstrument.png)

For the browser pages, the first way in and `examples/README.md` are all you need. To change the
firmware, start with this file and then read `CONTRIBUTING.md`, which takes half an hour and
covers the workflow, safety and working with an assistant. After that, read `CLAUDE.md` when you
want to know how the firmware is built, how it runs and what it must not do.

Building
--------

The sources are an Arduino sketch. They build with Arduino IDE 2.x or with arduino-cli, using
the Arduino SAM Boards core version 1.6.12 (select the board "Arduino Due (Native USB Port)").
Other versions of the SAM core are untested and may create unknown build or execution problems.

To build from the command line, you need Go and arduino-cli installed. After running
`arduino-cli core install arduino:sam@1.6.12` once, `go run ./tools/linnstrument build` compiles
the firmware into `build/` and `go run ./tools/linnstrument test` runs the desktop tests in
`tests/`. The same tool flashes a unit and backs up and restores its settings. `tools/virtual/`
builds the Virtual LinnStrument. Every push to GitHub is also compiled by GitHub Actions, which
attaches the resulting `.bin` file, the tool and the Virtual LinnStrument for Apple silicon Macs
to the workflow run, so you don't have to install anything to get a build.

On Apple silicon Macs, the Arduino SAM toolchain is an Intel binary and needs Rosetta installed
(`softwareupdate --install-rosetta`).

Flash a `.bin` with the LinnStrument Updater, which keeps your settings. It flashes the `.bin`
in the folder it sits in, so on a Mac, drag the unzipped app into a folder holding your file.
Left where it was unzipped, it doesn't see the file. `CONTRIBUTING.md` has the steps and the way
back. Note that uploading with bossac or the Arduino IDE directly erases all settings and
calibration.

Contributing
------------

See `CONTRIBUTING.md` for the branch and pull request workflow, what to install, what's safe and
what isn't, and how to work on this code with an AI coding assistant.

Note that customer bug reports or feature requests shouldn't be posted here. Instead, email
Roger at support@rogerlinndesign.com.


---


LinnStrument — chord-grid fork
==============================

This is a fork of [`rogerlinndesign/linnstrument-firmware`](https://github.com/rogerlinndesign/linnstrument-firmware)
that adds a **chord-grid mode** to the LinnStrument 128: a self-contained
chord-progression instrument with role × tension chord cells, a tonic
selector, and parsimonious voice-leading — no host computer needed.

The stock firmware sources are kept verbatim; the chord-grid logic lives
in six new `ls_chord_*.{ino,h}` files plus a handful of small targeted
patches to existing stock files.

These sources assume that you're using Arduino IDE v1.8.1 with SAM boards
v1.6.11. Different versions of this package might create unknown build and
execution problems.

Bug reports or feature requests for the **stock firmware** belong with
Roger Linn: support@rogerlinndesign.com. Issues with the **chord-grid
additions** belong on this fork's GitHub Issues.


## What the fork adds

A per-split toggle (configured in **Per-Split Settings**) turns each of
the LinnStrument's two splits into one of these layouts:

- **Chord grid** (8 cols × 8 rows). Each row is one scale-degree role —
  I, ii, iii, IV, V, vi, vii°, bVII. Columns follow a role-locked scheme:
  col 1 = triad, col 2 = sus, col 3 = 7th chord, cols 4-7 add function-
  aware extensions, col 8 = altered wildcard. Press a cell to sound that
  chord. 64 hand-authored chord templates per palette.

- **Tonic strip** (8 cols × 2 rows). The 12 chromatic pitch classes are
  laid out as a tonic selector; pressing one re-keys the chord grid live.
  Four reserved cells bind to: **voicing mode** (close-position ↔
  parsimonious), **voice spread** (tight / drop-2+4 / wide), **chord
  palette** (Pop ↔ Jazz), and **latch** (on/off — when on, a chord cell
  release no longer sends note-off; the chord keeps ringing until the
  next chord-cell press or until latch is toggled off).

Two chord palettes ship in firmware: **Pop** (default, friendly diatonic
voicings) and **Jazz** (sus4 over sus2, 7-stacked extensions, maj13 /
m11 / m(maj7) / lydian-dominant wildcards). The column scheme is the same
in both — switching palette swaps the specific voicings without changing
which column holds the sus / 7 / extension.

A split with neither toggle enabled behaves exactly like stock — same
note resolution, same note-lights, same per-split MIDI channel and
expression settings.

**Default layout on a fresh flash:** split active with split point at
col 9. Left split = chord grid. Right split bottom 2 rows = tonic strip.
Right split top 6 rows = stock note-lights (your normal LinnStrument).
Chord notes go out on MIDI channel 3; tonic strip presses don't send MIDI
themselves (they change which key the chord grid plays in); melody / lead
zones use whatever channel and layout your per-split settings specify.


## Voicing

Two voicing modes (toggled at the voicing reserved cell):

- **Close position** — the chord template's intervals stacked above
  `CHORD_BASE_OCTAVE` + tonic. Each chord press starts fresh; common tones
  between chords are not deliberately preserved.
- **Parsimonious** — `voice_lead` matches the previous chord's voices to
  the new chord's pitch classes within ±6 semitones; common tones (same
  MIDI value) are held without retrigger. Voices move minimally between
  chord changes. The previous voicing **survives release**, so playing
  chords one at a time (release before next press) still voice-leads —
  reset only on tonic change, palette change, or preset load. When the
  new chord has fewer voices than the previous one (e.g. Imaj9 → V
  triad), upper common tones are preserved instead of dropped.

A separate **voice spread** parameter (3 levels at the spread reserved
cell) applies a drop-2+4 jazz-comping post-process to the *initial*
voicing of a chord. The spread shape is then inherited via parsimonious
voice-leading on subsequent chord changes, so the chord stays in roughly
the same pitch range without compounding octave drops.


## Where to read more

The chord-grid design docs live in [`docs/`](docs/):

| Doc                                       | What's in it                                                  |
|-------------------------------------------|---------------------------------------------------------------|
| [`docs/LAYOUT.md`](docs/LAYOUT.md)        | Physical layout: zones, cells, LED scheme, palette            |
| [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md) | Firmware structure: state, zone dispatch, touch lifecycles |
| [`docs/VOCABULARY.md`](docs/VOCABULARY.md)| The 64-cell chord table (hand-authored C++ data)              |
| [`docs/BUILD.md`](docs/BUILD.md)          | Toolchain, build steps, flashing, reverting to stock          |
| [`docs/ROADMAP.md`](docs/ROADMAP.md)      | Phase ladder + implementation history                          |


## Where the chord-grid lives in this tree

| File                       | Role                                                |
|----------------------------|-----------------------------------------------------|
| `ls_chord_config.h`        | Constants + `ChordEngineState` struct + tonic-strip PC map |
| `ls_chord_vocab.h`         | 64-entry chord vocabulary + `chord_template_for_cell` |
| `ls_chord_engine.ino`      | Runtime state + zone dispatch + chord/tonic touch handlers |
| `ls_chord_voicing.ino`     | `compute_chord_notes`, `voice_lead`, `apply_spread` |
| `ls_chord_leds.ino`        | LED painters for the chord grid and tonic strip     |
| `ls_chord_melody.ino`      | Empty stub (melody zone is stock-delegated)         |

Patched stock files (small, additive hooks — original behavior preserved
outside the chord-engine zones):

| File                        | Patch                                                       |
|-----------------------------|-------------------------------------------------------------|
| `linnstrument-firmware.ino` | `setup()` calls `chordEngineInit()`; `GlobalSettings` and `SplitSettings` gain a few chord-engine fields |
| `ls_handleTouches.ino`      | Zone dispatch in `handleNewTouch` / `handleTouchRelease`; chord-engine cells suppress stock note resolution in `handleXYZupdate` |
| `ls_displayModes.ino`       | `paintNormalDisplay` overpaints chord-engine zones; `paintPerSplitDisplay` shows the chord-mode and tonic-strip toggle indicators |
| `ls_settings.ino`           | Default values for new fields; preset sync in `applyPresetSettings`; press handling for the per-split toggle cells |

Hardware drivers — touch detection, LED driver, MIDI send, EEPROM
framework, calibration, switches, sensors, sequencer — are **untouched**.
