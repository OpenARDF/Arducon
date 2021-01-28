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
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

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
		char *tempText;

		lb_send_string((char*)"\n*** EEPROM STRING SIZES **\n", TRUE);

/* Software Version String */

		tempText = (char*)&PRODUCT_NAME_LONG[0];
		for(i = 0; i < strlen(PRODUCT_NAME_LONG); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textVersion[i], (uint8_t)tempText[i]);
		}

		eeprom_update_byte((uint8_t*)&ee_textVersion[i], 0);
		lb_send_value(i + 1, (char*)"Size of PRODUCT_NAME_LONG");

/* Help String */

		tempText = (char*)&HELP_TEXT[0];
		for(i = 0; i < strlen(HELP_TEXT); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textHelp[i], (uint8_t)tempText[i]);
		}

		eeprom_update_byte((uint8_t*)&ee_textHelp[i], 0);
		lb_send_value(i + 1, (char*)"Size of HELP_TEXT");
		
/* Set ID String */
		tempText = (char*)&TEXT_SET_ID[0];
		for(i=0; i < strlen(TEXT_SET_ID); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textSetID[i], (uint8_t)tempText[i]);
		}
		
		eeprom_update_byte((uint8_t*)&ee_textSetID[i], 0);
		lb_send_value(i+1, (char*)"Size of TEXT_SET_ID");

/* Set Time String */

		tempText = (char*)&TEXT_SET_TIME[0];
		for(i = 0; i < strlen(TEXT_SET_TIME); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textSetTime[i], (uint8_t)tempText[i]);
		}

		eeprom_update_byte((uint8_t*)&ee_textSetTime[i], 0);
		lb_send_value(i + 1, (char*)"Size of TEXT_SET_TIME");

/* Set Start String */

		tempText = (char*)&TEXT_SET_START[0];
		for(i = 0; i < strlen(TEXT_SET_START); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textSetStart[i], (uint8_t)tempText[i]);
		}

		eeprom_update_byte((uint8_t*)&ee_textSetStart[i], 0);
		lb_send_value(i + 1, (char*)"Size of TEXT_SET_START");

/* Set Finish String */

		tempText = (char*)&TEXT_SET_FINISH[0];
		for(i = 0; i < strlen(TEXT_SET_FINISH); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textSetFinish[i], (uint8_t)tempText[i]);
		}

		eeprom_update_byte((uint8_t*)&ee_textSetFinish[i], 0);

		lb_send_value(i + 1, (char*)"Size of TEXT_SET_FINISH");

		
/* Set Err Finish in Past String */
		tempText = (char*)&TEXT_ERR_FINISH_IN_PAST[0];
		for(i=0; i < strlen(TEXT_ERR_FINISH_IN_PAST); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textErrFinishInPast[i], (uint8_t)tempText[i]);
		}

		eeprom_update_byte((uint8_t*)&ee_textErrFinishInPast[i], 0);
		lb_send_value(i+1, (char*)"Size of TEXT_ERR_FINISH_IN_PAST");

		
/* Set Err Start in Past String */
		tempText = (char*)&TEXT_ERR_START_IN_PAST[0];
		for(i=0; i < strlen(TEXT_ERR_START_IN_PAST); i++)
		{
			eeprom_update_byte((uint8_t*)&ee_textErrStartInPast[i], (uint8_t)tempText[i]);
		}

		eeprom_update_byte((uint8_t*)&ee_textErrStartInPast[i], 0);
		lb_send_value(i+1, (char*)"Size of TEXT_ERR_START_IN_PAST");

/* Set Err Finish Before Start String */
tempText = (char*)&TEXT_ERR_FINISH_BEFORE_START[0];
for(i=0; i < strlen(TEXT_ERR_FINISH_BEFORE_START); i++)
{
	eeprom_update_byte((uint8_t*)&ee_textErrFinishB4Start[i], (uint8_t)tempText[i]);
}

eeprom_update_byte((uint8_t*)&ee_textErrFinishB4Start[i], 0);
lb_send_value(i+1, (char*)"Size of TEXT_ERR_FINISH_BEFORE_START");

/* Set Err Invalid Time String */
tempText = (char*)&TEXT_ERR_INVALID_TIME[0];
for(i=0; i < strlen(TEXT_ERR_INVALID_TIME); i++)
{
	eeprom_update_byte((uint8_t*)&ee_textErrInvalidTime[i], (uint8_t)tempText[i]);
}

eeprom_update_byte((uint8_t*)&ee_textErrInvalidTime[i], 0);
lb_send_value(i+1, (char*)"Size of TEXT_ERR_INVALID_TIME");

/* Done */

		lb_send_string((char*)"*** EEPROM PROGRAMMING FINISHED **\n", TRUE);
	}
#endif  /* INIT_EEPROM_ONLY */