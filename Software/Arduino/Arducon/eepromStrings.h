/*
 * eepromStrings.cpp
 *
 * Created: 1/21/2021 9:11:05 PM
 *  Author: charl
 */
/******************************************************
 * EEPROM string definitions
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

void sendEEPROMString(char ee_addr[]);
void saveEEPROM(void);


#endif // EEPROMSTRINGS_H_
