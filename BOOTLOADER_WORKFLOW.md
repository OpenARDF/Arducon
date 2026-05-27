# Arducon Bootloader Workflow

This project is moving toward an ATmega328P-native bootloader flow while retaining the existing Atmel Studio 7 firmware path.

## Direction

- Use an Optiboot/STK500v1-compatible ATmega328P bootloader first.
- Keep the Arducon application linked at `0x0000`.
- Reserve the top 512 bytes of flash for the bootloader.
- Treat `0x0000..0x7DFF` as the bootloader-safe application range.
- Keep serial app communications at `57600` baud.
- Use `115200` baud for the bootloader/update side unless the selected bootloader requires otherwise.

The reviewed bootloader artifact is checked in at:

```text
Bootloaders/optiboot-atmega328p-arduino-1.8.6/optiboot_atmega328.hex
```

It is copied from Arduino AVR Boards `arduino:avr@1.8.6` with its source subset and occupies `0x7E00..0x7FFF`.

## Application Commands

Arducon exposes two serial-only LinkBus commands for updater tools:

```text
INF
UPD
```

`INF` reports machine-readable identity in short lines that fit the current LinkBus transmit buffer:

```text
* INF product=Arducon
* INF update=UPD
* INF sw=1.0.1
* INF hw=ATmega328P-16
* INF app=0x0000
* INF baud=57600
* INF bl=unknown
* INF proto=stk500v1
```

`UPD` stops active transmissions, prints `* Bootloader update mode`, then forces a watchdog reset so the bootloader can accept the update.

DTMF bootloader entry is intentionally out of scope for the first bootloader cleanup.

## Local Build And Size Check

Use the repo-owned CLI Release build from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-cli-release.ps1 -Clean
powershell -ExecutionPolicy Bypass -File .\compare-cli-release.ps1
```

The CLI build invokes `avr-g++` directly, emits `tmp\cli-release\Arducon.elf`, `.hex`, `.map`, `.eep`, `.lss`, and `.srec`, then runs the size check. The comparison script checks the CLI output against the current Microchip Studio Release baseline from `CODEX_MAILBOX.md`.

Microchip/Atmel Studio Release remains the short-term authoritative build check. Use it after material firmware changes until the CLI build has stayed aligned across several changes:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-firmware.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File .\check-firmware-size.ps1 -Configuration Release
```

The size check parses the generated HEX file, reports `avr-size` SRAM numbers and EEPROM image bytes when artifacts are present, and fails if the application overlaps the reserved bootloader area.

## Release Package

Build a machine-readable package with:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-release-package.ps1
powershell -ExecutionPolicy Bypass -File .\validate-release-package.ps1
```

The package includes:

- `Arducon-Update-...hex`
- `Arducon-Bootloader-Optiboot-ATmega328P.hex`
- `Arducon-Release-Info-...json`
- `Arducon-Checksums-...txt`
- `README-Arducon-...txt`
- a zipped copy of the release files

The manifest records the product, version, board, app baud, update baud, app start, app limit, flash size, page size, update command, and hashes.
It also records the bootloader file, source package, protocol, baud rate, high-fuse target, address range, and image byte count.

## Provisioning

Provisioning remains guarded: first read and review fuses, then explicitly opt in to flash and fuse writes.

```powershell
powershell -ExecutionPolicy Bypass -File .\provision-bootloader.ps1 -Backend Avrdude -CheckPrereqs -SkipFlash
powershell -ExecutionPolicy Bypass -File .\provision-bootloader.ps1 -Backend Avrdude -SkipFlash -ReadFusesOnly
```

The first-install path should merge a reviewed ATmega328P bootloader HEX and Arducon application HEX, program the combined image with a programmer, and verify flash.
By default, `provision-bootloader.ps1` uses the repo-owned Optiboot HEX above.

For a 512-byte bootloader with `BOOTRST`, the high-fuse boot-bit transform is:

```text
newHigh = (oldHigh & 0xF8) | 0x06
```

The script applies that transform when `-ProgramFuses -ConfirmFuseWrite` are both supplied. Do not change unrelated fuse bits.

Example first-install review flow with an Atmel-ICE using `avrdude`:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-cli-release.ps1 -Clean
powershell -ExecutionPolicy Bypass -File .\provision-bootloader.ps1 -Backend Avrdude -SkipFlash -ReadFusesOnly
powershell -ExecutionPolicy Bypass -File .\provision-bootloader.ps1 -Backend Avrdude -DryRun -HighFuseValue 0xDA -ProgramFuses -ConfirmFuseWrite
```

Only after reviewing the exact combined image path and fuse value should the real flash/fuse command be run. `-ChipErase` is available when a full chip erase is intentionally required, but it can erase EEPROM unless the EESAVE fuse is programmed.

## Recovery

If a bootloader update fails:

1. Connect the serial adapter at the bootloader baud rate.
2. Try the selected bootloader's normal sync/info command.
3. If serial recovery fails, use an ISP programmer to restore the bootloader and application.
4. Re-read fuses before changing them.
