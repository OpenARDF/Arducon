# Arducon v2.1.1

Firmware release for Arduino Pro Mini / ATmega328P Arducon controllers.

## Highlights

- Fixes scheduled-event synchronization after power-up when an active event has been running for more than 32767 seconds.
- Preserves the full RTC-derived elapsed event time when selecting the active fox slot.
- Prevents long-running active events from wrapping the elapsed-time calculation back to an incorrect Fox 1 slot.

## Package Contents

- Arducon application update HEX.
- Combined first-install HEX for programming a new board with an ISP programmer.
- ATmega328P Arducon-patched Optiboot-compatible bootloader HEX.
- Optiboot source archive and notices.
- Friendly setup launcher plus advanced provisioning and bootloader serial-test scripts.
- Machine-readable release manifest.
- Checksums and package README.

## Validation

- Host scheduler unit tests passed, including long-running event slot regression cases.
- Firmware interrupt regression passed.
- CLI Release build passed.
- Firmware size check passed.
- Release package validation passed, including first-install HEX composition, checksums, and ZIP contents.
- The updated firmware was installed on connected Arducon hardware and confirmed to power up with an active configured event synchronized to the RTC schedule.
- Additional hardware regression was not required for this low-risk scheduler width fix by release-operator decision.
- Windows/Microchip Studio Release cross-check was not rerun; the repo-owned CLI Release build remains the release gate for this firmware release.

## Update Notes

- Current SerialSlinger builds support Arducon updates with the `arducon-update` path.
- Arducon uses Optiboot/STK500v1 at `115200` baud for serial firmware updates.
- Normal app serial remains `57600` baud.
- Updating from Arducon 1.x requires a programming device, such as an Atmel-ICE or compatible ISP programmer, because 1.x units do not already have the Optiboot update path installed.
- ISP recovery remains available with an Atmel-ICE or compatible programmer.
