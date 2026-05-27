# SerialSlinger Compatibility Audit

SerialSlinger is intended to configure both SignalSlinger and Arducon, so Arducon bootloader work must stay compatible with SerialSlinger's real update paths.

## Current Finding

The current Arducon Optiboot artifact is not yet compatible with SerialSlinger's existing firmware-update implementation.

The checked-in Arducon bootloader is:

- product package: `Arducon`
- bootloader protocol: Optiboot/STK500v1
- update baud: `115200`
- app baud: `57600`
- app start: `0x0000`
- bootloader range: `0x7E00..0x7FFF`

The current SerialSlinger update implementation expects:

- release manifest format `signalslinger-release-info-v1`
- package product `SignalSlinger`
- a custom SignalSlinger bootloader identity line beginning with `SignalSlinger`
- numeric protocol version metadata
- custom framed bootloader commands `E`, `W`, `C`, and `R`
- 512-byte update pages
- a nonzero app start address

Optiboot/STK500v1 does not expose the SignalSlinger identity line and does not implement the custom `E/W/C/R` framed protocol. SerialSlinger also currently parses `* INF baud=...` as the update baud, while Arducon currently reports its app serial baud there.

## Consequence

The Arducon bootloader can be provisioned and updated by tools that speak Optiboot/STK500v1, such as `avrdude`, but the existing SerialSlinger firmware-update code will not update Arducon through this bootloader without SerialSlinger changes.

## Compatibility Options

### Selected Path

Add product-aware firmware-update support to SerialSlinger:

- keep the existing SignalSlinger custom updater unchanged
- add an Arducon product/package model
- add an STK500v1/Optiboot update transport
- accept Arducon's `appStartAddress = 0x0000`
- use ATmega328P page behavior instead of SignalSlinger AVR128DA assumptions
- treat `bootloader.protocol = "stk500v1"` as the updater selector
- keep package validation strict by product, protocol, flash size, app limit, and HEX range

This preserves a standard ATmega328P bootloader and keeps chip-specific updater logic out of Arducon firmware.

Licensing note: the vendored Optiboot source states GPL version 2 or later. Arducon keeps the copied source beside the HEX artifact, and release packages include a bootloader source archive beside the bootloader HEX.

### Rejected Alternative

Replace Optiboot with a small ATmega328P bootloader that speaks the SignalSlinger custom protocol.

This would reduce SerialSlinger changes, but it is riskier because it requires new bootloader work on ATmega328P and gives up the main advantage of using a proven Optiboot-compatible bootloader.

## Arducon Firmware Follow-Up

Before SerialSlinger compatibility testing, decide the final app `INF` schema for Arducon. If SerialSlinger keeps using `baud` as update baud, Arducon should report:

```text
* INF baud=115200
```

If we want to expose both bauds cleanly, add explicit fields such as:

```text
* INF appbaud=57600
* INF baud=115200
```

SerialSlinger should parse those fields product-aware rather than assuming SignalSlinger semantics for all products.
