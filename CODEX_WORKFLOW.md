# Codex Workflow

## Everyday Expectations

- Before making firmware or workflow changes, state the current git branch to the user.
- At the start of cross-environment work, fetch the current branch and read `CODEX_MAILBOX.md` before making changes.
- Prefer `rg` for text search and `rg --files` or `fd` for file discovery.
- Keep commits scoped. Do not include KiCad cache/session files, backup archives, or generated firmware outputs unless explicitly requested.
- Treat the Atmel Studio 7 project under `Software/AtmelStudio7/Arducon` as the maintained firmware path.
- Arduino IDE builds are no longer supported.

## Codex Mailbox

- Use `CODEX_MAILBOX.md` for handoffs between Mac Codex and Windows Codex.
- Address each message to the intended recipient and include author, date, branch, commits, validation results, and any local worktree caveats.
- Preserve active messages from the other Codex until they are no longer needed.
- Remove or replace your own older mailbox messages when they have been superseded.
- Commit and push after writing or materially updating a mailbox message so the other environment can fetch it.

## Build And Verification

- Use `.\build-cli-release.ps1 -Clean` as the normal reproducible local Release build entry point.
- Use `.\build-firmware.ps1 -Configuration Release` when verifying against Microchip/Atmel Studio generated makefiles.
- After executable firmware source or configuration changes, run the CLI build and comparison:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-cli-release.ps1 -Clean
powershell -ExecutionPolicy Bypass -File .\compare-cli-release.ps1
```

- Keep Windows Codex/Microchip Studio as the short-term authoritative Release build check until CLI and Microchip Studio artifacts have been compared across several firmware changes.
- For documentation-only or ignore-only changes, a firmware build is optional.
- For bootloader/release-package changes, run:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-release-package.ps1 -SkipBuild
powershell -ExecutionPolicy Bypass -File .\validate-release-package.ps1
```

## Generated And Hand-Maintained Files

- Hand-maintained firmware files include the source files under `Software/AtmelStudio7/Arducon/Arducon` and the root PowerShell workflow scripts.
- Treat `Debug\*`, `Release\*`, `.elf`, `.hex`, `.eep`, `.lss`, `.srec`, `.o`, `.d`, and `.map` as generated build output.
- Treat `tmp\cli-release\*` as generated output from the repo-owned CLI Release build.
- Existing checked-in generated artifacts are historical; do not remove them until a separate cleanup verifies reproducible builds.

## Bootloader Direction

- Use an ATmega328P-native bootloader path. Do not copy the AVR128DA28 SignalSlinger bootloader implementation into Arducon.
- Prefer Optiboot/STK500v1 compatibility unless a measured requirement forces a custom ATmega328P bootloader.
- `INF` reports firmware/update identity over LinkBus.
- `UPD` is serial-only and reboots through the watchdog so the bootloader can catch the reset.
- DTMF update entry is intentionally out of scope for the first bootloader cleanup.
