# LinnStrument 2.3.4 Scalar Layout x5

An experimental LinnStrument 200 firmware fork by **GZ_Beatz**.

This project adds a global **3×4 Scalar Layout** to LinnStrument OS 2.3.4.
The layout uses a fixed +3-semitone horizontal interval and +4-semitone vertical
interval, with scale-aware swiping, per-note MPE behavior, and scale-degree LED
feedback.

中文说明：这是由 **GZ_Beatz** 维护的 LinnStrument 200 固件实验性分叉。
它在官方 LinnStrument OS 2.3.4 的基础上加入全局 3×4 音阶布局，支持调内
滑动触发、MPE 复音表达和音阶级数灯光反馈。

## Current release

- Firmware: [`release/LinnStrument-2.3.4-Scalar-x5.bin`](release/LinnStrument-2.3.4-Scalar-x5.bin)
- Device version display: `234-x5`
- SHA-256: `d3966e6dca01188c74841c46058efc01d5ce2e53e0085f1f640ec0b3721eebe1`
- Status: hardware-test build; compile-verified, but not a production release.

## Features

- Global 3×4 layout for LinnStrument 200 performance mode.
- Fixed geometry: right/left = ±3 semitones; up/down = ±4 semitones.
- Layout-only default register one octave lower; turning the mode off restores
  the normal register without changing saved Octave settings.
- Scale-aware Scalar Swipe: crossing a pad retriggers the next active-scale
  note in the travel direction.
- Independent multi-touch/MPE state for simultaneous fingers.
- Seven-degree LED colors; out-of-scale pads are unlit.
- Red played-note LEDs match the exact MIDI note number, not other octaves.
- Atomic played-note LED refresh to avoid transient column flashes.

## Installation

1. Download `LinnStrument-2.3.4-Scalar-x5.bin` from the `release` folder.
2. Use the official LinnStrument Updater to flash the firmware.
3. Confirm that the device reports version `234-x5`.
4. Review [`SCALAR_LAYOUT_TESTING.md`](SCALAR_LAYOUT_TESTING.md) before the
   first performance test.

Do not use direct Arduino upload as the normal installation method.

## Enable Scalar Layout

1. Enter Global Settings on a LinnStrument 200.
2. Tap playable column 19 on the bottom row.
3. A blue indicator flashing in Tap Tempo sync means the layout is OFF.
4. A stable white indicator means the layout is ON.
5. Exit Global Settings normally. The selection is stored by the device.

## Build from source

The target is Arduino Due with Arduino SAM Boards 1.6.11.

```sh
scripts/compile-firmware.sh scalar-x5
```

The exported binary is placed under `build/scalar-x5/`.

## Project files

- `ls_scalarLayout.ino` — Scalar Layout pitch, scale, swipe, and LED helpers.
- `SCALAR_LAYOUT_TESTING.md` — hardware validation checklist.
- `DEVELOPMENT_PLAN.md` — implementation and compatibility notes.
- `release/` — current flashable firmware image.

## Credits and license

Original LinnStrument firmware: Roger Linn Design.

Scalar Layout fork and x5 release: **GZ_Beatz**.

This project retains the upstream Apache License 2.0. See [`LICENSE.txt`](LICENSE.txt).
