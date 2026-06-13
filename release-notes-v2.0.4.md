# Arducon v2.0.4

Firmware release for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Preserves scheduled event starts after setting the current clock time.
- Preserves scheduled event starts after changing the fox role by serial command or DTMF command.
- Prevents a valid saved start/finish schedule from requiring a restart just because setup commands were sent in a different order.
- Keeps the Arducon 2.x Optiboot/STK500v1 update path, `INF` metadata, and SignalSlinger-style release package contents from v2.0.3.

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
- Firmware size check passed.
- Release package validation passed, including first-install HEX composition, checksums, and ZIP contents.
- Hardware regression was not rerun for this scheduler-state release.
- Windows/Microchip Studio Release cross-check was not rerun; the repo-owned CLI Release build remains the release gate for this package.

## Update Notes

- Updating from Arducon 1.x to 2.0.4 requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the new Optiboot update path installed.
- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Bootloader installation requires confirmed fuse programming so `BOOTRST`, boot size, and `BODLEVEL=2.7V` are set together.
- Automatic bootloader fuse programming currently requires `avrdude`, including on Windows.
- Existing SerialSlinger update code does not yet update Arducon packages; SerialSlinger still needs product-aware Arducon package support and an STK500v1/Optiboot transport.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
