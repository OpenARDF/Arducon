# Arducon
Arducon is an Arduino-based ARDF transmitter controller for conducting Amateur Radio Direction-Finding (ARDF) competitions.

When connected to the audio output of an Amateur Radio handy-talky or similar receiver Arducon will decode Dual-Tone Multi Frequency (DTMF) tones that are used to configure and control Arducon. 

When connected to the microphone input of an Amateur Radio handy-talky or similar transmitter Arducon will control the transmitter to cause it to send signals of the proper timing and format for regulation ARDF competitions. Arducon is suitable for controlling most 2m or 80m transmitters used for ARDF.

Consult the <a href="https://docs.google.com/document/d/12K_Gxl0QbTVqAUzWxVBv87l9BldALZO71cXGYb9wk7I/edit?usp=sharing">User Manual</a>  for more information on Arducon and its capabilities.

![Docs/Arducon1.png](Docs/Arducon1.png)

<b>KiCad</b>

The KiCad folder holds files for generating the schematic, BOM, and PCB files using KiCad. On an OS X system the kicad directory should be placed in ~/Documents/GitHub/Arducon.

<b>Software</b>

The maintained firmware source lives under `Software/AtmelStudio7/Arducon` and targets an Arduino Pro Mini / ATmega328P using the Atmel Studio 7 project or the repo-owned CLI Release build scripts. Arduino IDE builds are no longer supported.

Use the CLI Release build for reproducible local builds:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-cli-release.ps1 -Clean
powershell -ExecutionPolicy Bypass -File .\compare-cli-release.ps1
```

Microchip/Atmel Studio 7 Release remains the cross-check path for now:

```powershell
powershell -ExecutionPolicy Bypass -File .\build-firmware.ps1 -Configuration Release
powershell -ExecutionPolicy Bypass -File .\check-firmware-size.ps1 -Configuration Release
```

See `BOOTLOADER_WORKFLOW.md` for Optiboot provisioning, release packaging, fuse handling, and serial bootloader smoke tests.
