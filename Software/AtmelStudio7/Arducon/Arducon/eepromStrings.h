/*
 *  MIT License
 *
 *  Copyright (c) 2021 DigitalConfections
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */


#ifndef EEPROMSTRINGS_H_
#define EEPROMSTRINGS_H_

#include "defs.h"

#if COMPILE_FOR_ATMELSTUDIO7
#include <avr/eeprom.h>
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

#define HELP_TEXT ("\nCommands:\n  CLK [T|S|F [\"YYMMDDhhmmss\"]] - Read/set time/start/finish\n  FOX [n]- Set fox role\n  ID [c...c] -  Set callsign\n  STA [0|1] - Start tones on/off\n  SYN 0|1|2 - Synchronize\n  TEM - Read temp\n  SPD [s] - Set ID code speed\n  VER - S/W version" )
#define TEXT_SET_TIME ("CLK T YYMMDDhhmmss <- Set current time\n")
#define TEXT_SET_START ("CLK S YYMMDDhhmmss <- Set start time\n")
#define TEXT_SET_FINISH ("CLK F YYMMDDhhmmss <- Set finish time\n")
#define TEXT_SET_ID ("ID [\"callsign\"] <- Set callsign\n")
#define TEXT_ERR_FINISH_BEFORE_START ("Err: Finish before start!\n")
#define TEXT_ERR_FINISH_IN_PAST ("Err: Finish in past!\n")
#define TEXT_ERR_START_IN_PAST ("Err: Start in past!\n")
#define TEXT_ERR_INVALID_TIME ("Err: Invalid time!")

void sendEEPROMString(char ee_addr[]);
void saveEEPROM(void);


#endif // EEPROMSTRINGS_H_
