# SerialSlinger Compatibility Audit

SerialSlinger configures and updates both SignalSlinger and Arducon. Arducon release work must keep its Optiboot/STK500v1 package shape compatible with SerialSlinger's product-aware Arducon updater.

## Current Finding

Current SerialSlinger builds support Arducon firmware updates through a dedicated `arducon-update` path. Arducon uses one ATmega328P release package, unlike SignalSlinger releases that publish multiple hardware-option packages.

The checked-in Arducon bootloader and package metadata use:

- product package: `Arducon`
- manifest format: `arducon-release-info-v1`
- bootloader protocol: Optiboot/STK500v1
- update baud: `115200`
- app baud: `57600`
- app start: `0x0000`
- app limit: `0x7E00`
- bootloader range: `0x7E00..0x7FFF`
- update page size: `128` bytes

SerialSlinger's Arducon updater uses the app `INF` response to confirm `product=Arducon`, `update=UPD`, app/update baud rates, app start, hardware build, and `proto=stk500v1`; then it enters Optiboot and writes/verifies the application pages.

## Compatibility Requirements

- Keep the existing SignalSlinger custom updater unchanged.
- Keep Arducon release manifests product-specific as `arducon-release-info-v1`.
- Keep package validation strict by product, protocol, flash size, app limit, and HEX range.
- Treat `bootloader.protocol = "stk500v1"` as the Arducon updater selector.
- Keep Arducon app serial at `57600` baud and update serial at `115200` baud unless a future release updates both firmware and SerialSlinger together.

Licensing note: the vendored Optiboot source states GPL version 2 or later. Arducon keeps the copied source beside the HEX artifact, and release packages include a bootloader source archive beside the bootloader HEX.

## Arducon Firmware Schema

Arducon exposes both serial rates explicitly:

```text
* INF appbaud=57600
* INF baud=115200
```

For Arducon, `baud` is the Optiboot update baud and `appbaud` is the normal LinkBus app baud.
