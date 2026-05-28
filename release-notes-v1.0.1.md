# Arducon v1.0.1

Firmware release candidate for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Moves the maintained firmware path to Atmel Studio 7 and the repo-owned CLI Release build.
- Removes the separate EEPROM-initialization build workflow; normal first boot now initializes EEPROM when needed.
- Adds Optiboot/STK500v1 bootloader packaging and serial-only `UPD` entry from the application.
- Adds `INF` metadata for product-aware updater tools.
- Preserves serial and DTMF command behavior while tightening parser bounds, watchdog handling, and serial transmit waits.
- Reduces SRAM usage and keeps the Release application below the `0x7E00` bootloader boundary.

## Package Contents

- Arducon application update HEX.
- ATmega328P Optiboot-compatible bootloader HEX.
- Optiboot source archive and notices.
- Machine-readable release manifest.
- Checksums and package README.

## Validation

- CLI Release build passed.
- Microchip/Atmel Studio Release comparison passed against the current Windows baseline.
- Serial command smoke passed for `HELP`, `INF`, `CLK`, `FOX`, `ID`, `SYN`, `PWD`, `SET`, `AM`, `UTI`, and `UPD`.
- `UPD` bootloader smoke passed with STK500v1 sync and ATmega328P signature `0x1E 0x95 0x0F`.
- Bench validation passed for keying, DTMF, and audio behavior.

## Update Notes

- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Existing SerialSlinger update code does not yet update Arducon packages; SerialSlinger still needs product-aware Arducon package support and an STK500v1/Optiboot transport.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
