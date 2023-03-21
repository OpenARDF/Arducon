# Arducon
Arducon is an Arduino-based ARDF transmitter controller for conducting Radio Orienteering (aka, Amateur Radio Direction-Finding - ARDF) competitions.

When connected to the audio output of an Amateur Radio handy-talky or similar receiver Arducon will decode Dual-Tone Multi Frequency (DTMF) tones that are used to configure and control Arducon. 

When connected to the microphone input of an Amateur Radio handy-talky or similar transmitter Arducon will control the transmitter to cause it to send signals of the proper timing and format for regulation Radio Orienteering competitions. Arducon is suitable for controlling most 2m or 80m transmitters used for Radio Orienteering.

Consult the <a href="https://docs.google.com/document/d/12K_Gxl0QbTVqAUzWxVBv87l9BldALZO71cXGYb9wk7I/edit?usp=sharing">User Manual</a>  for more information on Arducon and its capabilities.

![Docs/Arducon1.png](Docs/Arducon1.png)

<b>Want to change the hardware design, or modify the software. Everything you need is provided in the following folders: </b>
<b>KiCad</b>

The KiCad folder holds files for generating the schematic, BOM, and PCB files using KiCad. On an OS X system the kicad directory should be placed in ~/Documents/GitHub/Arducon.

<b>Software</b>

The Software folder holds files for compiling executable code using either the Arduino IDE or Atmel Studio 7. The latter provides better debugging tools. Both IDEs will build the same functional executable code for flashing into an Arduino Pro Mini.
