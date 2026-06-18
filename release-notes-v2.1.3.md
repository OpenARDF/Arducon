# Arducon v2.1.3

Firmware release for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Bumps the firmware version from 2.1.2 to 2.1.3.
- Sanitizes persisted EEPROM settings when existing stored values are outside valid ranges.
- Repairs invalid stored code speed, AM tone, days-to-run, UTC offset, and PTT periodic reset settings by restoring safe defaults.
- Limits `CLK D` days-to-run updates to the supported 1-100 day range.
- Keeps the Arducon 2.x Optiboot/STK500v1 update path and single ATmega328P release package contents.

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
- Live hardware regression was not rerun from this VM because no Arducon USB serial port was exposed to Windows during release packaging.
- Windows/Microchip Studio Release cross-check was not rerun; the repo-owned CLI Release build remains the release gate for this firmware release.

## Update Notes

- Current SerialSlinger builds support Arducon updates with the `arducon-update` path.
- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Updating from Arducon 1.x requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the Optiboot update path installed.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
