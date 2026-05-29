# Arducon v2.0.2

Firmware release for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Keeps the Arducon 2.x Optiboot/STK500v1 update path and `INF` metadata introduced for product-aware updater tools.
- Updates the release package contents to match the SignalSlinger update/setup package structure, except that Arducon has one ATmega328P hardware target instead of multiple hardware-option packages.
- Adds a combined first-install HEX and package-local setup/serial-validation scripts to the complete release ZIP.
- Reissued the release ZIP with SerialSlinger-compatible setup launcher options and stable `SS_SETUP_OK` / `SS_SETUP_ERROR` automation status lines.
- Reissued the release ZIP so bootloader installation also programs the ATmega328P extended-fuse `BODLEVEL` bits for 2.7 V brown-out detection.

## Package Contents

- Arducon application update HEX.
- Combined first-install HEX for programming a new board with an ISP programmer.
- ATmega328P Optiboot-compatible bootloader HEX.
- Optiboot source archive and notices.
- Friendly setup launcher plus advanced provisioning and bootloader serial-test scripts.
- Machine-readable release manifest.
- Checksums and package README.

## Validation

- CLI Release build passed.
- Release package validation passed, including first-install HEX composition, checksums, and ZIP contents.
- Hardware regression was not rerun for this packaging-only release.

## Update Notes

- Updating from Arducon 1.x to 2.0.2 requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the new Optiboot update path installed.
- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Bootloader installation now requires confirmed fuse programming so `BOOTRST`, boot size, and `BODLEVEL=2.7V` are set together.
- Automatic bootloader fuse programming currently requires `avrdude`, including on Windows. With `-Backend Auto`, the setup tool selects `avrdude` for bootloader installation and fails prereq checks if `avrdude` is unavailable; `atprogram` remains available only for non-automatic/manual workflows until equivalent fuse-write support is added and verified.
- Existing SerialSlinger update code does not yet update Arducon packages; SerialSlinger still needs product-aware Arducon package support and an STK500v1/Optiboot transport.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
