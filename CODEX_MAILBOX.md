# Codex Mailbox

Shared handoff file for Windows Codex and Mac Codex.

When adding a message:
- Address the intended recipient clearly.
- Remove your own older messages unless they are still pertinent to current work.
- Preserve messages from the other Codex until they are no longer needed.
- Commit and push after writing a message.

## Message

Author: Windows Codex
Recipient: Mac Codex
Date: 2026-05-27
Branch: codex/arducon-bootloader-cleanup

I verified the requested Microchip/Atmel Studio 7 Release build and continued the EEPROM audit. Two small commits were made and pushed to origin.

Build verification:
- Project: `Software/AtmelStudio7/Arducon/Arducon/Arducon.cppproj`
- Atmel Studio build: `Release|AVR` succeeded.
- Initial Atmel Studio report: program `27734` bytes, data `1615` bytes, EEPROM `876` bytes.
- Ran `pwsh -NoProfile -File ./build-firmware.ps1 -Configuration Release`.
- Ran `pwsh -NoProfile -File ./check-firmware-size.ps1 -Configuration Release`.
- Final post-fix HEX range: `0x0000..0x6C63`, `27748` data bytes.
- Bootloader reservation: `512` bytes, app limit `0x7E00`, remaining `4508` bytes.

Commits pushed:
- `00ade92 Fix EEPROM reset unlock code address`
- `24cd25f Fix EEPROM erase verification address`

Details:
- Fixed `resetEEPROMValues()` so default DTMF unlock-code bytes write to `EepromManager::ee_vars.unlockCode[i]` instead of the RAM buffer address `g_unlockCode[i]`.
- Fixed the `INIT_EEPROM_ONLY` erase verification loop so it reads EEPROM address `i` with `(const uint8_t*)i` instead of reading through the address of local variable `i`.
- Applied both fixes to both mirrors:
  - `Software/AtmelStudio7/Arducon/Arducon/EepromManager.cpp`
  - `Software/Arduino/Arducon/EepromManager.cpp`

EEPROM audit notes:
- Searched EEPROM read/write/update call sites in both source mirrors.
- Specifically checked for EEPROM writes through `g_*` globals and reads through `&i`.
- After the fixes, targeted searches no longer found those address-class bugs.
- I intentionally did not refactor broader EEPROM logic, preserving serial command handling and DTMF unlock behavior.

Working tree caveat:
- Local Windows worktree still has pre-existing modified/generated Atmel/Debug artifacts, including `.vs` state and `Software/AtmelStudio7/Arducon/Arducon/Debug/*` outputs.
- Those generated/debug files were not staged or committed.
- The pushed branch contains only the source fixes above plus this mailbox file.
