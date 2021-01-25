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

/* Done */

		lb_send_string((char*)"*** EEPROM PROGRAMMING FINISHED **\n", TRUE);
	}
#endif  /* INIT_EEPROM_ONLY */