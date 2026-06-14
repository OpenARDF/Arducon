# Arducon v2.0.6

Firmware release for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Tightens scheduled-event behavior across fox role changes, including start/stop action decisions and RTC schedule gates.
- Fixes beacon scheduler timing around station IDs and AM modulation/PTT state when roles move on or off air.
- Adds host-side scheduler coverage for fox-role timing plans, RTC gate decisions, and scheduler start/stop actions.
- Keeps the Arducon 2.x Optiboot/STK500v1 update path, `INF` metadata, and SignalSlinger-style release package contents from v2.0.5.

## Package Contents

- Arducon application update HEX.
- Combined first-install HEX for programming a new board with an ISP programmer.
- ATmega328P Optiboot-compatible bootloader HEX.
- Optiboot source archive and notices.
- Friendly setup launcher plus advanced provisioning and bootloader serial-test scripts.
- Machine-readable release manifest.
- Checksums and package README.

## Validation

- Host scheduler unit tests passed.
- Firmware interrupt regression passed.
- CLI Release build passed.
- Firmware size check passed.
- Release package validation passed, including first-install HEX composition, checksums, and ZIP contents.
- SerialSlinger Arducon update path was used on connected ATmega328P hardware and verified the updated app.
- Windows/Microchip Studio Release cross-check was not rerun; the repo-owned CLI Release build remains the release gate for this firmware-only scheduler release.

## Update Notes

- Updating from Arducon 1.x to 2.0.6 requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the new Optiboot update path installed.
- Current SerialSlinger builds support Arducon updates with the `arducon-update` path.
- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Bootloader installation requires confirmed fuse programming so `BOOTRST`, boot size, and `BODLEVEL=2.7V` are set together.
- Automatic bootloader fuse programming currently requires `avrdude`, including on Windows.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
