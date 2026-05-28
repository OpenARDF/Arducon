# Arducon Release Workflow

This document captures the standard release process for Arducon firmware packages published on GitHub Releases at `OpenARDF/Arducon`.

Unless stated otherwise, commands below assume the repository root is the current directory.

## Version Policy

- The firmware version is defined in `Software/AtmelStudio7/Arducon/Arducon/EepromManager.h` as `ARDUCON_FIRMWARE_VERSION`.
- Release versions use plain `x.y.z`, for example `1.0.2`.
- Git tags use `vX.Y.Z`, matching the firmware version, for example `v1.0.2`.
- Do not add a suffix such as `-test`, `-beta`, or build metadata to release versions unless that is explicitly requested for a prerelease.
- `build-release-package.ps1` and `validate-release-package.ps1` enforce the plain `x.y.z` format for normal release packages.

## Branch Roles

- `main` is the stable release branch.
- `codex/*` branches are development and validation branches.
- Before release work, confirm the current branch and working-tree state.
- Release from `main` unless a development-branch prerelease is explicitly requested.

## Standard Release Checklist

1. Confirm the branch, commit, and working tree:

```powershell
git status --short --branch
git log -1 --oneline
```

2. Confirm the intended firmware version in `EepromManager.h`.

3. Run the reproducible CLI Release build and comparison:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-cli-release.ps1 -Clean
powershell -ExecutionPolicy Bypass -File .\compare-cli-release.ps1
```

4. Get the Windows/Microchip Studio Release cross-check before publishing:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-firmware.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File .\check-firmware-size.ps1 -Configuration Release
```

Record the program bytes, SRAM, EEPROM image bytes, HEX range, and remaining bytes below `0x7E00`.

5. Run or confirm hardware regression for the release candidate:

- serial command smoke: `HELP`, `INF`, `FOX`, `ID`, `PWD`, `UTI`, `UPD`
- DTMF command smoke for lock/unlock, callsign, fox role, time/start/finish, AM tone, and PTT reset
- RF behavior smoke for keying, station ID, tone path, and event start/stop
- bootloader smoke with `test-bootloader-serial.ps1 -RequestBootloaderFromApp`

If hardware testing has already been completed, state exactly which hardware-test layer is being treated as complete.

6. Build and validate the release package:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-release-package.ps1
powershell -ExecutionPolicy Bypass -File .\validate-release-package.ps1
```

7. Verify expected package outputs under `release-packages\Arducon-vX.Y.Z`:

- `Arducon-Update-vX.Y.Z-ATmega328P.hex`
- `Arducon-Bootloader-Optiboot-ATmega328P.hex`
- `Arducon-Bootloader-Optiboot-ATmega328P-Source.zip`
- `Arducon-Release-Info-vX.Y.Z-ATmega328P.json`
- `Arducon-Checksums-vX.Y.Z-ATmega328P.txt`
- `README-Arducon-vX.Y.Z-ATmega328P.txt`
- `Arducon-vX.Y.Z-ATmega328P-Release-Files.zip`

8. Draft concise GitHub release notes before tagging.
   Include the firmware version, hardware target, user-visible changes, bootloader/update notes if relevant, and any EEPROM/fuse caveats.

9. Create the Git tag and GitHub release. For a stable release:

```powershell
git tag vX.Y.Z
git push origin vX.Y.Z
gh release create vX.Y.Z `
  .\release-packages\Arducon-vX.Y.Z\Arducon-Update-vX.Y.Z-ATmega328P.hex `
  .\release-packages\Arducon-vX.Y.Z\Arducon-vX.Y.Z-ATmega328P-Release-Files.zip `
  --repo OpenARDF/Arducon `
  --title "Arducon vX.Y.Z" `
  --notes-file .\release-notes-vX.Y.Z.md
```

Upload the release ZIP and the standalone update HEX. The ZIP contains the bootloader HEX, Optiboot source archive, manifest, checksums, and package README.

10. Verify the published release:

```powershell
gh release view vX.Y.Z --repo OpenARDF/Arducon
```

Confirm the release page shows the expected title, notes, tag, standalone update HEX, and release ZIP.

11. Re-check this checklist against the work just performed before declaring the deployment complete.

## Notes

- Release package directories and ZIPs are generated artifacts and are ignored by Git. Upload them to GitHub Releases; do not commit them unless explicitly requested.
- Normal Arducon release packages are not yet directly updateable by SerialSlinger. SerialSlinger needs product-aware Arducon/STK500v1 support before automatic Arducon updates can consume these packages.
- `BOOTLOADER_WORKFLOW.md` remains the source of truth for Optiboot provisioning, EESAVE, and serial bootloader smoke testing.
