# LinnStrument Csric's Modification


Based on official firmware 2.3.4, here is the change list.

- **Played Light "Same" Mode.** When pressed, all note pads with the same pitch will light up and **follow** your finger as you slide, rather than staying in the original position. Additionally, in split mode, this lighting effect for same-pitch notes extends across the left and right zones.

- **Trigger/Feathering Sensitivity.** An undocumented feature in the stock firmware allows access to `sensorLoZ` / `sensorFeatherZ` (Default: 120/80) settings: while in Global Settings, hold Calibrate and press the note pad 1 or 2 rows above it. Values are adjustable via horizontal sliding. 
This mod expands the adjustment range by lowering the hardcoded minimum limits from 100/65 to 40/20.

    - `sensorLoZ`: the lowest acceptable raw Z value to start a touch
    - `sensorFeatherZ`: the lowest acceptable raw Z value to continue a touch

- **Auto-Switching Selected Split.** In Split mode, the split zone with the most recent touch is selected.


- **Single-Finger Adjacent Note Suppression.** In official firmware's One Channel mode, pressing between two horizontally adjacent pads with a single finger may trigger both notes simultaneously. This mod eliminates that behavior, ensuring only the pad closer to the finger is triggered. 
Multi-finger triggering remains unaffected, and this modification only applies when Pitch Mode is enabled.



## (Original Readme ↓)

LinnStrument
============

This is the firmware for LinnStrument, running on an Arduino Due processor.

These sources assume that you're using Arduino IDE v1.8.1 with SAM boards v1.6.11.
Different versions of this package might create unknown build and execution problems.

Note that customer bug reports or feature requests should not be posted here.
Instead, email Roger at support@rogerlinndesign.com. 
