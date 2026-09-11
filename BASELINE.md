# LinnStrument OS 2.3.4 Build Baseline

- Board: Arduino Due (`arduino:sam:arduino_due_x`)
- Arduino CLI: 1.5.1
- Arduino SAM Boards: 1.6.11
- ARM compiler: `arm-none-eabi-gcc` 4.8.3-2014q1
- Bundled library: `libraries/DueFlashStorage`
- Program size: 163,828 bytes of 524,288 bytes (31%)
- Baseline binary SHA-256:
  `2f1b60023bf8bf675813df62d6cdf9254af0874def9712195f8355a12a11478e`
- Baseline Git commit: `3660dae`

Run `scripts/compile-firmware.sh <build-name>` from any working directory to
create an isolated build under `build/<build-name>`.
