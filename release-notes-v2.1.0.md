# Arducon v2.1.0

Firmware release for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Adds multi-day scheduled event support with a persisted days-to-run setting.
- Extends scheduler logic so daily event windows advance across the configured run length.
- Adds `CLK D` support to the compact command help for configuring event duration.
- Keeps the Arducon 2.x Optiboot/STK500v1 update path and SignalSlinger-style single ATmega328P release package contents from v2.0.7.

## Package Contents

- Arducon application update HEX.
- Combined first-install HEX for programming a new board with an ISP programmer.
- ATmega328P Arducon-patched Optiboot-compatible bootloader HEX.
- Optiboot source archive and notices.
- Friendly setup launcher plus advanced provisioning and bootloader serial-test scripts.
- Machine-readable release manifest.
- Checksums and package README.

## Validation

- Host scheduler unit tests passed.
- Firmware interrupt regression passed.
- Hardware tests passed.
- CLI Release build passed.
- Firmware size check passed.
- Release package validation passed, including first-install HEX composition, checksums, and ZIP contents.
- Windows/Microchip Studio Release cross-check was not rerun; the repo-owned CLI Release build remains the release gate for this firmware release.

## Update Notes

- Updating from Arducon 1.x to 2.1.0 requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the new Optiboot update path installed.
- Current SerialSlinger builds support Arducon updates with the `arducon-update` path.
- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Bootloader installation requires confirmed fuse programming so `BOOTRST`, boot size, and `BODLEVEL=2.7V` are set together.
- Automatic bootloader fuse programming currently requires `avrdude`, including on Windows.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
