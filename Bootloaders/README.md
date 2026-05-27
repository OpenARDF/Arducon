# Bootloaders

This directory holds reviewed bootloader artifacts used by Arducon provisioning.

## ATmega328P Optiboot

`optiboot-atmega328p-arduino-1.8.6/` is a source-and-artifact copy of the ATmega328P Optiboot bootloader from the Arduino AVR Boards package `arduino:avr@1.8.6`.

Selected file:

- `optiboot_atmega328.hex`

Selection rationale:

- Targets ATmega328P.
- Uses the STK500v1-compatible Optiboot protocol.
- Uses `115200` baud, matching the planned updater flow.
- Occupies `0x7E00..0x7FFF`, matching a 512-byte boot section.
- Matches the ATmega328P high-fuse target `0xDE` when the existing high fuse is transformed with `(oldHigh & 0xF8) | 0x06`.

The application must remain below `0x7E00`; `check-firmware-size.ps1` enforces that limit.

License note: the copied Optiboot source states GPL version 2 or later. Keep the source files with the HEX artifact so the bootloader binary is traceable. Release packages include a source archive beside the bootloader HEX for the same reason.
