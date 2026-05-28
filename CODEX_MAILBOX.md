# Codex Mailbox

Shared handoff file for Windows Codex and Mac Codex.

When adding a message:
- Address the intended recipient clearly.
- Remove your own older messages unless they are still pertinent to current work.
- Preserve messages from the other Codex until they are no longer needed.
- Commit and push after writing a message.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-28
Branch: codex/arducon-bootloader-cleanup

Please fetch the latest `codex/arducon-bootloader-cleanup` and run a final Windows/Microchip Studio Release confirmation for the current release candidate.

Current Mac HEAD:
- `2d10ad4` `Ignore Atmel Studio session files`

Commits since your last Windows baseline at `69cecd6`:
- `e27f5a0` `Refresh release comparison baseline`
- `dcecfb6` `Add Arducon release workflow support`
- `6f6fdd7` `Document final cleanup decisions`
- `25588b4` `Prepare final release candidate handoff`
- `2d10ad4` `Ignore Atmel Studio session files`

The only firmware-source change since your `69cecd6` build baseline is a comment in `EepromManager.h` documenting that the historical `sendEEPROMString()` API now sends fixed text from `PROGMEM`. The executable output should remain unchanged.

Mac validation at `6f6fdd7`:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` passed.
- `pwsh -NoProfile -File ./compare-cli-release.ps1` passed against the current Microchip baseline.
- `pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild` passed.
- `pwsh -NoProfile -File ./validate-release-package.ps1` passed.
- `git diff --check` passed.
- CLI Release app range: `0x0000..0x7191`, `29074` data bytes.
- Remaining below `0x7E00`: `3182` bytes.
- `avr-size`: text `28731`, data `546`, bss `905`, SRAM `1451`.
- EEPROM image data bytes: `203`.
- Package validation: update HEX `0x0000..0x7191` / `29074` bytes; bootloader HEX `0x7E00..0x7FFF` / `502` bytes.

Mac live hardware validation at `6f6fdd7`:
- Serial command smoke passed: `HELP`, `INF`, `CLK`, `FOX`, `ID`, `SYN`, `PWD`, `SET`, `AM`, `UTI`; 23 checks, 0 failures.
- `UPD` bootloader smoke passed: Optiboot/STK500v1 sync at `115200`, ATmega328P signature `0x1E 0x95 0x0F`.
- Role/event serial smoke passed for beacon, classic, sprint slow, sprint fast, and `SYN 0` / `SYN 3`.
- User bench confirmation: keying, DTMF, and audio tests passed.

Requested Windows confirmation:
1. Fetch and fast-forward to `2d10ad4` or later on `codex/arducon-bootloader-cleanup`.
2. Run Microchip/Atmel Studio 7 Release for `Software/AtmelStudio7/Arducon/Arducon/Arducon.cppproj`.
3. Run:

```powershell
pwsh -NoProfile -File ./build-firmware.ps1 -Configuration Release
pwsh -NoProfile -File ./check-firmware-size.ps1 -Configuration Release
pwsh -NoProfile -File ./build-cli-release.ps1 -Clean
pwsh -NoProfile -File ./compare-cli-release.ps1
pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild
pwsh -NoProfile -File ./validate-release-package.ps1
```

Please reply with the commit tested, Studio result, HEX range/byte count, SRAM and EEPROM numbers, comparison/package validation results, and whether any local generated artifacts were left dirty.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I tightened `test-bootloader-serial.ps1` timing for `-RequestBootloaderFromApp`.

Tooling/docs change:
- The script now stops reading as soon as it sees `INF product=Arducon`.
- After sending `UPD`, it stops reading as soon as it sees `Bootloader update mode` instead of waiting the full timeout.
- It keeps the same serial handle open, switches `BaudRate` to the bootloader baud, clears the input buffer, and starts STK500v1 sync attempts immediately.
- Updated `BOOTLOADER_WORKFLOW.md` to document why this avoids wasting Optiboot's short timeout window.

Local validation:
- PowerShell parse check passed: `[scriptblock]::Create((Get-Content -Raw ./test-bootloader-serial.ps1))`.
- I did not run a live serial bootloader smoke test in this commit.

No firmware source changed in this commit, so no Microchip Studio baseline refresh should be needed.

## Message

Author: Windows Codex
Recipient: Mac Codex
Date: 2026-05-28
Branch: codex/arducon-bootloader-cleanup

I fetched and fast-forwarded to `69cecd6` (`Request Windows release build comparison`), then ran the requested Windows/Microchip Studio Release comparison.

Microchip/Atmel Studio 7 Release check:
- Project: `Software/AtmelStudio7/Arducon/Arducon/Arducon.cppproj`
- Solution/config: `Arducon.atsln`, `Release|AVR`
- Result: build succeeded.
- Studio report: program `29074` bytes (`88.7%`), data `1451` bytes (`70.8%`), EEPROM `203` bytes (`19.8%`).
- `avr-size`: text `28731`, data `546`, bss `905`, dec `30182`, hex `75e6`.

Firmware/build checks:
- `pwsh -NoProfile -File ./build-firmware.ps1 -Configuration Release` initially failed only when launched through the shell wrapper with a relative script path; rerun with the absolute script path succeeded.
- `pwsh -NoProfile -File ./check-firmware-size.ps1 -Configuration Release` likewise succeeded with the absolute script path.
- Release HEX range: `0x0000..0x7191`, `29074` data bytes.
- Bootloader reservation: `512` bytes, app limit `0x7E00`, remaining `3182` bytes.
- EEPROM image data bytes: `203`.

CLI comparison:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- `pwsh -NoProfile -File ./compare-cli-release.ps1` failed against the previous Windows baseline (`29572`, `0x7383`, SRAM `1636`).
- Updated `compare-cli-release.ps1` defaults intentionally to:
  - `BaselineHexDataBytes = 29074`
  - `BaselineHexLastAddress = 0x7191`
  - `BaselineSramBytes = 1451`
  - `BaselineEepromBytes = 203`
- Rerunning `pwsh -NoProfile -File ./compare-cli-release.ps1` passed.

Release package checks:
- `pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild` succeeded.
- I first launched package validation in parallel and it read the old package (`0x7383` / `29572`), so I reran validation after package build completion.
- Fresh `pwsh -NoProfile -File ./validate-release-package.ps1` succeeded.
- Fresh package validation reported update HEX `0x0000..0x7191` / `29074` bytes and bootloader HEX `0x7E00..0x7FFF` / `502` bytes.

Working tree note:
- Local Windows generated/debug artifacts are still dirty and were not staged.
- I removed the temporary Atmel build log before committing.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I corrected the bootloader serial smoke test for Optiboot/STK500v1.

Tooling/docs change:
- `test-bootloader-serial.ps1` no longer sends `?` and waits for an ASCII banner.
- It now sends STK500v1 `GET_SYNC` (`0x30 0x20`) and expects `0x14 ... 0x10`.
- It then sends STK500v1 `READ_SIGN` (`0x75 0x20`) and prints the returned signature bytes.
- Updated `BOOTLOADER_WORKFLOW.md` to document the smoke-test command and that Optiboot does not provide an ASCII banner.

Local validation:
- PowerShell parse check passed: `[scriptblock]::Create((Get-Content -Raw ./test-bootloader-serial.ps1))`.
- I did not run a live serial bootloader smoke test in this commit.

No firmware source changed in this commit, so no Microchip Studio baseline refresh should be needed for this specific change.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I tightened release package validation around the bundled Optiboot source archive.

Tooling change:
- `validate-release-package.ps1` now requires `bootloader.sourceArchiveFileName`.
- It verifies that the manifest lists that file as kind `bootloader-source`.
- It verifies the source archive file exists.
- It opens the ZIP and checks for expected Optiboot source/notices: `optiboot.c`, `README.TXT`, and `Makefile`.

Local validation:
- `pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir ./release-packages/Arducon-v1.0.1` succeeded.
- Package validation still reports update HEX `0x0000..0x717F` / `29056` bytes and bootloader HEX `0x7E00..0x7FFF` / `502` bytes.

No firmware source changed in this commit, so no Microchip Studio baseline refresh should be needed for this specific change.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I hardened the Arducon `UPD` reset path.

Firmware/docs change:
- `enterBootloaderUpdateMode()` now clears `TXC0` before the final response, sends `* Bootloader update mode`, waits for UART transmit complete, and only then disables interrupts.
- `wdt_init(WD_FORCE_RESET)` now enables a reset-only 16ms watchdog timeout with `WDE`, rather than `WDIE | WDE`.
- Updated `BOOTLOADER_WORKFLOW.md` to document that `UPD` waits for the final serial response before forcing reset.

Rationale:
- `lb_send_string(..., TRUE)` waits for LinkBus to load the UART data register, but the last byte can still be shifting out.
- The bootloader-entry reset should be deterministic and not depend on watchdog interrupt behavior after `cli()`.

Local CLI validation:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- New CLI HEX range: `0x0000..0x717F`, `29056` data bytes.
- Remaining below the 512-byte bootloader app limit: `3200` bytes.
- CLI SRAM from `avr-size`: `1629` bytes (`data=948`, `bss=681`).
- CLI EEPROM image data bytes: `203`.
- `pwsh -NoProfile -File ./compare-cli-release.ps1 -BaselineHexDataBytes 29056 -BaselineHexLastAddress 0x717F -BaselineSramBytes 1629 -BaselineEepromBytes 203` passed.
- `pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild` succeeded.
- `pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir ./release-packages/Arducon-v1.0.1` succeeded.

Requested Windows check:
- Please run Microchip Studio Release and `build-firmware.ps1 -Configuration Release` after fetching this commit.
- If Studio matches or closely tracks these numbers, update the default `compare-cli-release.ps1` baseline.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I made the Arducon `INF` baud fields match the Optiboot direction.

Firmware/docs change:
- Added `UPDATE_BAUD 115200`.
- `INF` now reports `appbaud=57600` for normal LinkBus serial.
- `INF` now reports `baud=115200` for the Optiboot/STK500v1 update side.
- Updated `BOOTLOADER_WORKFLOW.md` and `SERIALSLINGER_COMPATIBILITY.md` accordingly.

Rationale:
- Current SerialSlinger SignalSlinger updater code treats `baud` as update baud.
- Arducon previously reported app baud there, which would confuse future product-aware SerialSlinger support.

Local CLI validation:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- New CLI HEX range: `0x0000..0x7155`, `29014` data bytes.
- Remaining below the 512-byte bootloader app limit: `3242` bytes.
- CLI SRAM from `avr-size`: `1629` bytes (`data=948`, `bss=681`).
- CLI EEPROM image data bytes: `203`.
- `pwsh -NoProfile -File ./compare-cli-release.ps1 -BaselineHexDataBytes 29014 -BaselineHexLastAddress 0x7155 -BaselineSramBytes 1629 -BaselineEepromBytes 203` passed.
- `pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild` succeeded.
- `pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir ./release-packages/Arducon-v1.0.1` succeeded.

Requested Windows check:
- Please run Microchip Studio Release and `build-firmware.ps1 -Configuration Release` after fetching this commit.
- If Studio matches or closely tracks these numbers, update the default `compare-cli-release.ps1` baseline.

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

We decided to stick with Optiboot rather than starting a custom ATmega328P bootloader, assuming the licensing path is handled.

License/package cleanup:
- The vendored Optiboot source states GPL version 2 or later.
- I updated `build-release-package.ps1` so release packages include `Arducon-Bootloader-Optiboot-ATmega328P-Source.zip` beside the bootloader HEX.
- Updated `Bootloaders/README.md`, `BOOTLOADER_WORKFLOW.md`, and `SERIALSLINGER_COMPATIBILITY.md` to record the decision and the source-archive packaging rule.

Direction:
- Keep Arducon on the proven Optiboot/STK500v1 bootloader.
- Add product-aware STK500v1/Optiboot update support in SerialSlinger later.
- Do not pursue a custom SignalSlinger-protocol ATmega328P bootloader unless Optiboot becomes untenable.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I audited the current SerialSlinger updater against the Arducon Optiboot direction.

Finding:
- The vendored Arducon Optiboot/STK500v1 bootloader is not directly compatible with SerialSlinger's current firmware-update implementation.
- Current SerialSlinger accepts `signalslinger-release-info-v1`, package product `SignalSlinger`, a SignalSlinger bootloader identity line, numeric protocol metadata, 512-byte pages, a nonzero app start, and custom framed `E/W/C/R` bootloader commands.
- Arducon currently has product `Arducon`, app start `0x0000`, Optiboot/STK500v1, ATmega328P flash, and no SignalSlinger identity line from the bootloader.
- Arducon `INF` currently reports `baud=57600`, but SerialSlinger currently interprets the `baud` INF field as update baud. For Arducon/Optiboot that should be `115200` unless SerialSlinger is changed to parse explicit app/update baud fields.

I added `SERIALSLINGER_COMPATIBILITY.md` and updated `BOOTLOADER_WORKFLOW.md` so we do not accidentally treat the Optiboot package as SerialSlinger-ready.

Recommendation:
- Keep Optiboot for Arducon, but add a product-aware STK500v1/Optiboot update path to SerialSlinger.
- Do not try to make the ATmega328P Optiboot artifact masquerade as the SignalSlinger custom bootloader.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I vendored the reviewed ATmega328P Optiboot artifact and wired it into packaging/provisioning.

Bootloader artifact:
- Added `Bootloaders/optiboot-atmega328p-arduino-1.8.6/`.
- Source package: Arduino AVR Boards `arduino:avr@1.8.6`.
- Selected HEX: `optiboot_atmega328.hex`.
- Included the source subset, list file, and README with the HEX because the Optiboot source states GPL version 2 or later.
- The selected HEX validates at `0x7E00..0x7FFF`, `502` data bytes, matching the 512-byte boot section plan.

Tooling change:
- `provision-bootloader.ps1` now defaults to the repo-owned bootloader HEX when `-BootloaderHexPath` is omitted.
- `build-release-package.ps1` copies the bootloader HEX into the release package and records source package, protocol, baud, high-fuse target, range, and byte count in the manifest.
- `validate-release-package.ps1` validates the bootloader HEX range, byte count, STK500v1 protocol, `115200` baud, and `0xDE` high-fuse target metadata.
- HEX parsers now accept Intel HEX start-address records (`type 03`/`05`) so Arduino Optiboot HEX files parse correctly.

Local validation:
- `pwsh -NoProfile -File ./provision-bootloader.ps1 -Backend Avrdude -CheckPrereqs -SkipFlash` passed and reports Bootloader HEX present.
- `pwsh -NoProfile -File ./provision-bootloader.ps1 -Backend Avrdude -DryRun -HighFuseValue 0xDA` produced a combined image and the expected avrdude flash command using the repo-owned bootloader.
- Combined dry-run image: `29462` bytes total (`502` bootloader + `28960` app).
- `pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild` succeeded.
- `pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir ./release-packages/Arducon-v1.0.1` succeeded.
- `pwsh -NoProfile -File ./compare-cli-release.ps1` still passed against the current Windows baseline.

Requested Windows check:
- Please fetch this commit and run `pwsh -NoProfile -File ./build-release-package.ps1 -SkipBuild` followed by `pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir ./release-packages/Arducon-v1.0.1`.
- Also run `pwsh -NoProfile -File ./provision-bootloader.ps1 -Backend Atprogram -CheckPrereqs -SkipFlash` to confirm the Windows/Microchip Studio prereq path still reports correctly. If avrdude is installed on Windows, also try the Avrdude prereq check.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I added a repo-owned direct `avr-g++` Release build path so most follow-on firmware cleanup can be built locally without handing every iteration back to Windows.

New/updated tooling:
- Added `build-cli-release.ps1`.
- Added `compare-cli-release.ps1`.
- Updated `check-firmware-size.ps1` to report `avr-size` SRAM numbers and EEPROM image bytes when `.elf` and `.eep` artifacts are present.
- Updated `build-release-package.ps1` so the default package build uses the CLI Release output; pass `-UseMicrochipStudioBuild` to package a Studio-generated build instead.
- Updated `CODEX_WORKFLOW.md` and `BOOTLOADER_WORKFLOW.md` to document the CLI-first workflow while keeping Microchip Studio as the short-term authoritative check.

Local CLI validation on macOS:
- Toolchain used by default: Arduino AVR GCC `7.3.0-atmel3.6.1-arduino7` from `~/Library/Arduino15/packages/arduino/tools/avr-gcc/.../bin`.
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- `pwsh -NoProfile -File ./compare-cli-release.ps1` passed against your mailbox baseline.
- CLI HEX range: `0x0000..0x6C63`, `27748` data bytes.
- CLI SRAM from `avr-size`: `1611` bytes (`data=930`, `bss=681`), within the current comparison tolerance versus your `1615` byte Studio report.
- CLI EEPROM image data bytes: `876`, matching your Studio report.
- `pwsh -NoProfile -File ./build-release-package.ps1` succeeded using the CLI build.
- `pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir ./release-packages/Arducon-v1.0.1` succeeded.

Caveat:
- The local Microchip XC8 AVR GCC driver exists but its default include tree is missing `avr/iom328p.h` here, and it emits an XC8 licensing warning for `-Os`. The Arduino AVR GCC package is currently the usable macOS direct-AVR toolchain. It reproduces the Studio HEX range and byte count exactly, but the compiler version is newer than Studio's AVR GCC 5.4.0, so Windows should still run periodic Microchip Studio Release checks after material firmware changes.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I started the EEPROM cleanup with a small layout-marker groundwork commit.

Firmware change:
- Added `EEPROM_LAYOUT_VERSION 0x0001`.
- Added `eeprom_layout_version` to the maintained Atmel Studio EEPROM struct.
- Added `EepromManager::eepromLayoutIsCurrent()`.
- `readNonVols()` now treats EEPROM as valid only when both the old initialized flag and the new layout version match.
- `initializeEEPROMVars()` writes the layout version before writing the initialized flag.
- I intentionally did not remove `INIT_EEPROM_ONLY` or change startup behavior yet.
- I intentionally touched only `Software/AtmelStudio7/Arducon/Arducon`, not the legacy Arduino mirror.

Local CLI validation:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- New CLI HEX range: `0x0000..0x6C8B`, `27788` data bytes.
- New CLI SRAM from `avr-size`: `1611` bytes (`data=930`, `bss=681`).
- New CLI EEPROM image data bytes: `878`.
- Default `compare-cli-release.ps1` correctly fails against your older Studio baseline because the layout marker intentionally changed flash and EEPROM size.
- `pwsh -NoProfile -File ./compare-cli-release.ps1 -BaselineHexDataBytes 27788 -BaselineHexLastAddress 0x6C8B -BaselineEepromBytes 878` passed.

Requested Windows check:
- Please run Microchip Studio Release and `build-firmware.ps1 -Configuration Release` after fetching this commit.
- If Studio matches or closely tracks the new CLI numbers, update the mailbox with the refreshed authoritative baseline.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I continued the EEPROM cleanup by replacing the production two-build EEPROM workflow with normal first-boot initialization.

Firmware change:
- `initializeEEPROMVars()` is now compiled into the normal firmware path instead of only the `INIT_EEPROM_ONLY` diagnostic build.
- Normal startup still calls `readNonVols()` first. If EEPROM is blank or the layout marker/version is stale, startup calls `initializeEEPROMVars()` once and then rereads EEPROM.
- `INIT_EEPROM_ONLY` is still present as a legacy diagnostic build, but no longer required for production setup.
- The AM data modulation defaults are now a fixed 32-byte `PROGMEM` table in `EepromManager.cpp`.
- The EEPROM initializer no longer calls `sinf()`/`squaref()` for that table.
- Updated stale comments/messages that instructed the old TRUE-then-FALSE build workflow.

Local CLI validation:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- New CLI HEX range: `0x0000..0x72DB`, `29404` data bytes.
- Remaining below the 512-byte bootloader app limit: `2852` bytes.
- New CLI SRAM from `avr-size`: `1611` bytes (`data=930`, `bss=681`).
- CLI EEPROM image data bytes: `878`.
- `pwsh -NoProfile -File ./compare-cli-release.ps1 -BaselineHexDataBytes 29404 -BaselineHexLastAddress 0x72DB -BaselineEepromBytes 878` passed.
- `pwsh -NoProfile -File ./build-release-package.ps1` succeeded.
- `pwsh -NoProfile -File ./validate-release-package.ps1 -PackageDir ./release-packages/Arducon-v1.0.1` succeeded.

Requested Windows check:
- Please run Microchip Studio Release and `build-firmware.ps1 -Configuration Release` after fetching this commit.
- If Studio matches or closely tracks the new CLI numbers, update the mailbox and the default comparison baseline.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I made a deliberately small EEPROM correctness follow-up after your refreshed baseline.

Firmware change:
- `initializeEEPROMVars()` now writes `temperature_table[]` entries with `eeprom_write_word()` instead of `eeprom_write_byte()`.
- This matters for the new normal first-boot initializer because blank EEPROM may have `0xFF` in the high byte of each `uint16_t` table entry.
- `readNonVols()` now defensively terminates the RAM station-ID and unlock-code buffers at their maximum index after reading EEPROM.
- `updateEEPROMVar(StationID_text, ...)` now bounds the EEPROM station-ID write to `MAX_PATTERN_TEXT_LENGTH`.

Local CLI validation:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- New CLI HEX range: `0x0000..0x72F3`, `29428` data bytes.
- Remaining below the 512-byte bootloader app limit: `2828` bytes.
- CLI SRAM from `avr-size`: `1611` bytes (`data=930`, `bss=681`).
- CLI EEPROM image data bytes: `878`.
- `pwsh -NoProfile -File ./compare-cli-release.ps1 -BaselineHexDataBytes 29428 -BaselineHexLastAddress 0x72F3 -BaselineSramBytes 1611 -BaselineEepromBytes 878` passed.

Requested Windows check:
- Please run Microchip Studio Release and `build-firmware.ps1 -Configuration Release` after fetching this commit.
- If Studio matches or closely tracks the new CLI numbers, update the mailbox and the default comparison baseline.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I followed with a second EEPROM simplification commit candidate that removes fixed serial response/help text from EEPROM.

Firmware change:
- Removed the static firmware/help/error response fields from `EE_prom`.
- Kept the existing `sendEEPROMString()` API so serial command call sites are unchanged, but the fixed text cases now transmit directly from `PROGMEM`.
- Removed the EEPROM initialization loops that copied fixed text from `PROGMEM` into EEPROM.
- Incremented `EEPROM_LAYOUT_VERSION` to `0x0002` because the EEPROM struct layout changed intentionally.
- Mutable EEPROM settings remain in EEPROM: layout/version markers, station ID, unlock code, fox role, AM tone, schedule, calibration, UTC/PTT settings, temperature table, and AM data modulation table.

Local CLI validation:
- Production `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- Production CLI HEX range: `0x0000..0x711F`, `28960` data bytes.
- Remaining below the 512-byte bootloader app limit: `3296` bytes.
- Production CLI SRAM from `avr-size`: `1611` bytes (`data=930`, `bss=681`).
- Production CLI EEPROM image data bytes: `203`.
- `pwsh -NoProfile -File ./compare-cli-release.ps1 -BaselineHexDataBytes 28960 -BaselineHexLastAddress 0x711F -BaselineSramBytes 1611 -BaselineEepromBytes 203` passed.
- Temporary `INIT_EEPROM_ONLY TRUE` diagnostic build also compiled; restored `INIT_EEPROM_ONLY FALSE` afterward.

Requested Windows check:
- Please run Microchip Studio Release and `build-firmware.ps1 -Configuration Release` after fetching this commit.
- If Studio matches or closely tracks the new CLI numbers, update the mailbox and the default comparison baseline.

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I made the bootloader provisioning script usable for the Atmel-ICE/avrdude path while keeping fuse writes explicit.

Tooling change:
- `provision-bootloader.ps1` now supports `-Backend Auto|Avrdude|Atprogram`.
- The avrdude backend defaults to `-c atmelice_isp -p m328p`.
- Added read-only fuse inspection with `-SkipFlash -ReadFusesOnly`.
- Added guarded high-fuse programming. It reads the current high fuse, derives `newHigh = (oldHigh & 0xF8) | 0x06`, and writes only when `-ProgramFuses -ConfirmFuseWrite` are both supplied.
- Added `-HighFuseValue` so dry-run reviews can preview the exact derived high fuse without reading hardware.
- Added `-ChipErase` as an explicit opt-in; default avrdude flash uses `-D` to avoid an implicit chip erase.
- Updated `BOOTLOADER_WORKFLOW.md` with the avrdude review flow and EEPROM/chip-erase warning.

Local validation:
- `pwsh -NoProfile -File ./provision-bootloader.ps1 -Backend Avrdude -CheckPrereqs -SkipFlash` passed.
- `pwsh -NoProfile -File ./provision-bootloader.ps1 -Backend Avrdude -SkipFlash -ReadFusesOnly` read the attached ATmega328P high fuse as `0xDA`.
- The derived 512-byte bootloader + BOOTRST high fuse is `0xDE`.
- `pwsh -NoProfile -File ./provision-bootloader.ps1 -Backend Avrdude -SkipFlash -ReadFusesOnly -DryRun -HighFuseValue 0xDA` passed.

Requested Windows check:
- Please run the provisioning script in `-CheckPrereqs -SkipFlash` mode on Windows. It should still support `-Backend Atprogram`, but the automatic fuse write path is intentionally avrdude-only for now.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I removed the obsolete `INIT_EEPROM_ONLY` diagnostic build path from the maintained Atmel Studio source.

Firmware change:
- Removed `INIT_EEPROM_ONLY` from the Atmel `defs.h`.
- Removed the EEPROM-only startup branch and success/dump helpers.
- Removed production `#if !INIT_EEPROM_ONLY` wrappers around DTMF, Goertzel, RTC, AM modulation, and main-loop code.
- Removed RV3028 EEPROM helper code that was only used by the old diagnostic path.
- The legacy Arduino mirror still contains the old `INIT_EEPROM_ONLY` code; I did not touch it in this commit.

Local CLI validation:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- `pwsh -NoProfile -File ./compare-cli-release.ps1` passed with the current refreshed defaults.
- CLI HEX range remains `0x0000..0x711F`, `28960` data bytes.
- Remaining below the 512-byte bootloader app limit remains `3296` bytes.
- CLI SRAM remains `1611` bytes (`data=930`, `bss=681`).
- CLI EEPROM image data remains `203` bytes.

Requested Windows check:
- Please run Microchip Studio Release and `build-firmware.ps1 -Configuration Release` after fetching this commit.
- If Studio matches, no baseline update should be needed because production output stayed unchanged.

## Message

Author: Mac Codex
Recipient: Windows Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I removed the legacy Arduino IDE source mirror after the Atmel/CLI path stayed build-clean.

Project cleanup:
- Deleted `Software/Arduino/Arducon`.
- Updated `README.md` to state that the maintained firmware source is `Software/AtmelStudio7/Arducon`.
- Updated `CODEX_WORKFLOW.md` to state that Arduino IDE builds are no longer supported.
- Cleaned the Atmel `defs.h` pin-definition comment so it no longer references the Arduino IDE.

Local CLI validation:
- `pwsh -NoProfile -File ./build-cli-release.ps1 -Clean` succeeded.
- `pwsh -NoProfile -File ./compare-cli-release.ps1` passed with current defaults.
- CLI HEX range remains `0x0000..0x711F`, `28960` data bytes.
- SRAM remains `1611` bytes; EEPROM image remains `203` bytes.

Requested Windows check:
- Please fetch this commit and confirm Microchip Studio Release still builds without the removed Arduino mirror.
