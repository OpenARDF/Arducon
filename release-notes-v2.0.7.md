# Arducon v2.0.7

Firmware release for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Fixes the Optiboot leave-to-app handoff after SerialSlinger/Optiboot firmware updates.
- Adds an Arducon SRAM handoff marker so app-requested `UPD` resets enter the bootloader while the later STK500 `LEAVE_PROGMODE` reset returns to the application.
- Disables Optiboot entry LED flashes to keep the patched bootloader inside the 512-byte boot section.
- Keeps the Arducon 2.x Optiboot/STK500v1 update path, `INF` metadata, and SignalSlinger-style release package contents from v2.0.6.

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
- CLI Release build passed.
- Firmware size check passed.
- Release package validation passed, including first-install HEX composition, checksums, and ZIP contents.
- Hardware testing was completed before this deployment; changes are limited to the bootloader/update handoff path.
- Windows/Microchip Studio Release cross-check was not rerun; the repo-owned CLI Release build remains the release gate for this bootloader-focused release.

## Update Notes

- Updating from Arducon 1.x to 2.0.7 requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the new Optiboot update path installed.
- Current SerialSlinger builds support Arducon updates with the `arducon-update` path.
- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Bootloader installation requires confirmed fuse programming so `BOOTRST`, boot size, and `BODLEVEL=2.7V` are set together.
- Automatic bootloader fuse programming currently requires `avrdude`, including on Windows.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
