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

#include "eepromStrings.h"
#include "linkbus.h"

#if COMPILE_FOR_ATMELSTUDIO7
#include <avr/eeprom.h>
#include <string.h>
#include <avr/pgmspace.h>
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

/* Set Firmware Version Here */
const char PRODUCT_NAME_LONG[] PROGMEM = "*** Arducon Fox Controller Ver. 0.13 ***\n";
const char HELP_TEXT[] PROGMEM = "\nCommands:\n  CLK [T|S|F [\"YYMMDDhhmmss\"]] - Read/set time/start/finish\n  FOX [n]- Set fox role\n  ID [c...c] -  Set callsign\n  STA [0|1] - Start tones on/off\n  SYN 0|1|2 - Synchronize\n  TEM - Read temp\n  SPD [s] - Set ID code speed\n  VER - S/W version";
const char TEXT_SET_TIME[] PROGMEM = "CLK T YYMMDDhhmmss <- Set current time\n";
const char TEXT_SET_START[] PROGMEM = "CLK S YYMMDDhhmmss <- Set start time\n";
const char TEXT_SET_FINISH[] PROGMEM = "CLK F YYMMDDhhmmss <- Set finish time\n";
const char TEXT_SET_ID[] PROGMEM = "ID [\"callsign\"] <- Set callsign\n";
const char TEXT_ERR_FINISH_BEFORE_START[] PROGMEM = "Err: Finish before start!\n";
const char TEXT_ERR_FINISH_IN_PAST[] PROGMEM = "Err: Finish in past!\n";
const char TEXT_ERR_START_IN_PAST[] PROGMEM = "Err: Start in past!\n";
const char TEXT_ERR_INVALID_TIME[] PROGMEM = "Err: Invalid time!\n";
const char TEXT_ERR_TIME_IN_PAST[] PROGMEM = "Err: Time in past!\n";

char EEMEM ee_textVersion[sizeof(PRODUCT_NAME_LONG)];
char EEMEM ee_textHelp[sizeof(HELP_TEXT)];
char EEMEM ee_textSetTime[sizeof(TEXT_SET_TIME)];
char EEMEM ee_textSetStart[sizeof(TEXT_SET_START)];
char EEMEM ee_textSetFinish[sizeof(TEXT_SET_FINISH)];
char EEMEM ee_textSetID[sizeof(TEXT_SET_ID)];
char EEMEM ee_textErrFinishB4Start[sizeof(TEXT_ERR_FINISH_BEFORE_START)];
char EEMEM ee_textErrFinishInPast[sizeof(TEXT_ERR_FINISH_IN_PAST)];
char EEMEM ee_textErrStartInPast[sizeof(TEXT_ERR_START_IN_PAST)];
char EEMEM ee_textErrInvalidTime[sizeof(TEXT_ERR_INVALID_TIME)];
char EEMEM ee_textErrTimeInPast[sizeof(TEXT_ERR_TIME_IN_PAST)];

void sendEEPROMString(char ee_addr[])
{
	char c = eeprom_read_byte((uint8_t*)&ee_addr[0]);
	int i = 0;

	while(c)
	{
		lb_echo_char(c);
		c = eeprom_read_byte((uint8_t*)&ee_addr[++i]);

		while(linkbusTxInProgress())
		{
			;
		}
	}
}


#if INIT_EEPROM_ONLY
	void saveEEPROM(void)
	{
		uint16_t i;

/* Software Version String */
	for (i = 0; i < strlen_P(PRODUCT_NAME_LONG); i++) {
	uint8_t byteval = pgm_read_byte(PRODUCT_NAME_LONG + i);
	eeprom_update_byte((uint8_t*)&ee_textVersion[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textVersion[i], 0);

/* Help String */
	for (i = 0; i < strlen_P(HELP_TEXT); i++) {
	uint8_t byteval = pgm_read_byte(HELP_TEXT + i);
	eeprom_update_byte((uint8_t*)&ee_textHelp[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textHelp[i], 0);
		
/* Set ID String */
	for (i = 0; i < strlen_P(TEXT_SET_ID); i++) {
	uint8_t byteval = pgm_read_byte(TEXT_SET_ID + i);
	eeprom_update_byte((uint8_t*)&ee_textSetID[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textSetID[i], 0);

/* Set Time String */
	for (i = 0; i < strlen_P(TEXT_SET_TIME); i++) {
	uint8_t byteval = pgm_read_byte(TEXT_SET_TIME + i);
	eeprom_update_byte((uint8_t*)&ee_textSetTime[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textSetTime[i], 0);

/* Set Start String */
	for (i = 0; i < strlen_P(TEXT_SET_START); i++) {
	uint8_t byteval = pgm_read_byte(TEXT_SET_START + i);
	eeprom_update_byte((uint8_t*)&ee_textSetStart[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textSetStart[i], 0);

/* Set Finish String */
	for (i = 0; i < strlen_P(TEXT_SET_FINISH); i++) {
	uint8_t byteval = pgm_read_byte(TEXT_SET_FINISH + i);
	eeprom_update_byte((uint8_t*)&ee_textSetFinish[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textSetFinish[i], 0);
		
/* Set Err Finish in Past String */
	for (i = 0; i < strlen_P(TEXT_ERR_FINISH_IN_PAST); i++) {
	uint8_t byteval = pgm_read_byte(TEXT_ERR_FINISH_IN_PAST + i);
	eeprom_update_byte((uint8_t*)&ee_textErrFinishInPast[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textErrFinishInPast[i], 0);
		
/* Set Err Start in Past String */
	for (i = 0; i < strlen_P(TEXT_ERR_START_IN_PAST); i++) {
		uint8_t byteval = pgm_read_byte(TEXT_ERR_START_IN_PAST + i);
		eeprom_update_byte((uint8_t*)&ee_textErrStartInPast[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textErrStartInPast[i], 0);

/* Set Err Finish Before Start String */
	for(i=0; i < strlen_P(TEXT_ERR_FINISH_BEFORE_START); i++)
	{
		uint8_t byteval = pgm_read_byte(TEXT_ERR_FINISH_BEFORE_START + i);
		eeprom_update_byte((uint8_t*)&ee_textErrFinishB4Start[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textErrFinishB4Start[i], 0);

/* Set Err Invalid Time String */
	for(i=0; i < strlen_P(TEXT_ERR_INVALID_TIME); i++)
	{
		uint8_t byteval = pgm_read_byte(TEXT_ERR_INVALID_TIME + i);
		eeprom_update_byte((uint8_t*)&ee_textErrInvalidTime[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textErrInvalidTime[i], 0);

/* Set Err Time In Past String */
	for(i=0; i < strlen_P(TEXT_ERR_TIME_IN_PAST); i++)
	{
		uint8_t byteval = pgm_read_byte(TEXT_ERR_TIME_IN_PAST + i);
		eeprom_update_byte((uint8_t*)&ee_textErrTimeInPast[i], byteval);
	}

	eeprom_update_byte((uint8_t*)&ee_textErrTimeInPast[i], 0);

/* Done */

		lb_send_string((char*)"EEPROM PROGRAMMING FINISHED\n", TRUE);
	}
#endif  /* INIT_EEPROM_ONLY */
