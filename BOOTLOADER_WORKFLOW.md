# Arducon Bootloader Workflow

This project is moving toward an ATmega328P-native bootloader flow while retaining the existing Atmel Studio 7 firmware path.

## Direction

- Use an Optiboot/STK500v1-compatible ATmega328P bootloader first.
- Keep the Arducon application linked at `0x0000`.
- Reserve the top 512 bytes of flash for the bootloader.
- Treat `0x0000..0x7DFF` as the bootloader-safe application range.
- Keep serial app communications at `57600` baud.
- Use `115200` baud for the bootloader/update side unless the selected bootloader requires otherwise.

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

Use the wrapper scripts from the repository root:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-firmware.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File .\check-firmware-size.ps1
```

The size check parses the generated HEX file and fails if the application overlaps the reserved bootloader area.

## Release Package

Build a machine-readable package with:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-release-package.ps1
powershell -ExecutionPolicy Bypass -File .\validate-release-package.ps1
```

The package includes:

- `Arducon-Update-...hex`
- `Arducon-Release-Info-...json`
- `Arducon-Checksums-...txt`
- `README-Arducon-...txt`
- a zipped copy of the release files

The manifest records the product, version, board, app baud, update baud, app start, app limit, flash size, page size, update command, and hashes.

## Provisioning

Provisioning must remain guarded while the bootloader choice is being finalized.

```powershell
powershell -ExecutionPolicy Bypass -File .\provision-bootloader.ps1 -CheckPrereqs
```

The first-install path should merge a reviewed ATmega328P bootloader HEX and Arducon application HEX, program the combined image with a programmer, and verify flash.

Fuse writes are deliberately not automatic yet. Before production provisioning, read the ATmega328P high fuse and review the exact value. For a 512-byte bootloader with `BOOTRST`, the intended boot-bit transform is:

```text
newHigh = (oldHigh & 0xF8) | 0x06
```

Do not change unrelated fuse bits.

## Recovery

If a bootloader update fails:

1. Connect the serial adapter at the bootloader baud rate.
2. Try the selected bootloader's normal sync/info command.
3. If serial recovery fails, use an ISP programmer to restore the bootloader and application.
4. Re-read fuses before changing them.
