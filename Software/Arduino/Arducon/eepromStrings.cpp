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
#include "i2c.h"
#include "rv3028.h"

#if COMPILE_FOR_ATMELSTUDIO7
#include <avr/pgmspace.h>
#include <stdio.h>
#else
#include <EEPROM.h>
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

/* Set Firmware Version Here */
const char PRODUCT_NAME_LONG[] PROGMEM = "*** Arducon Fox Controller Ver. 0.15 ***\n";
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

/***********************************************************************
 * Global Variables & String Constants
 *
 * Identify each global with a "g_" prefix
 * Whenever possible limit globals' scope to this file using "static"
 * Use "volatile" for globals shared between ISRs and foreground loop
 ************************************************************************/
uint8_t EEMEM ee_interface_eeprom_initialization_flag = EEPROM_UNINITIALIZED;
char EEMEM ee_stationID_text[MAX_PATTERN_TEXT_LENGTH + 1];
uint16_t EEMEM ee_temperature_table[SIZE_OF_TEMPERATURE_TABLE];
uint8_t EEMEM ee_id_codespeed;
uint8_t EEMEM ee_fox_setting;
uint8_t EEMEM ee_enable_LEDs;
int16_t EEMEM ee_atmega_temp_calibration;
int16_t EEMEM ee_rv3028_offset;
uint8_t EEMEM ee_enable_start_timer;
uint8_t EEMEM ee_enable_transmitter;
time_t EEMEM ee_event_start_epoch;
time_t EEMEM ee_event_finish_epoch;
uint8_t EEMEM ee_dataModulation[SIZE_OF_DATA_MODULATION];

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

extern char g_messages_text[][MAX_PATTERN_TEXT_LENGTH + 1];
extern volatile uint8_t g_id_codespeed;
extern volatile uint8_t g_pattern_codespeed;
extern volatile uint16_t g_time_needed_for_ID;
extern volatile int16_t g_atmega_temp_calibration;
extern volatile uint8_t g_enable_LEDs;
extern volatile uint8_t g_enable_start_timer;
extern volatile uint8_t g_enable_transmitter;
extern volatile uint8_t g_temperature_check_countdown;
extern volatile int16_t g_rv3028_offset;

extern volatile Fox_t g_fox;
extern volatile time_t g_event_start_epoch;
extern volatile time_t g_event_finish_epoch;
extern BOOL g_allow_rv3028_eeprom_changes;

extern char g_tempStr[];

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
	void initEEPROMStrings(void)
	{
		uint16_t i;

	/* Software Version String */
		for (i = 0; i < strlen_P(PRODUCT_NAME_LONG); i++) {
		uint8_t byteval = pgm_read_byte(PRODUCT_NAME_LONG + i);
		eeprom_write_byte((uint8_t*)&ee_textVersion[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textVersion[i], 0);

	/* Help String */
		for (i = 0; i < strlen_P(HELP_TEXT); i++) {
		uint8_t byteval = pgm_read_byte(HELP_TEXT + i);
		eeprom_write_byte((uint8_t*)&ee_textHelp[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textHelp[i], 0);
		
	/* Set ID String */
		for (i = 0; i < strlen_P(TEXT_SET_ID); i++) {
		uint8_t byteval = pgm_read_byte(TEXT_SET_ID + i);
		eeprom_write_byte((uint8_t*)&ee_textSetID[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textSetID[i], 0);

	/* Set Time String */
		for (i = 0; i < strlen_P(TEXT_SET_TIME); i++) {
		uint8_t byteval = pgm_read_byte(TEXT_SET_TIME + i);
		eeprom_write_byte((uint8_t*)&ee_textSetTime[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textSetTime[i], 0);

	/* Set Start String */
		for (i = 0; i < strlen_P(TEXT_SET_START); i++) {
		uint8_t byteval = pgm_read_byte(TEXT_SET_START + i);
		eeprom_write_byte((uint8_t*)&ee_textSetStart[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textSetStart[i], 0);

	/* Set Finish String */
		for (i = 0; i < strlen_P(TEXT_SET_FINISH); i++) {
		uint8_t byteval = pgm_read_byte(TEXT_SET_FINISH + i);
		eeprom_write_byte((uint8_t*)&ee_textSetFinish[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textSetFinish[i], 0);
		
	/* Set Err Finish in Past String */
		for (i = 0; i < strlen_P(TEXT_ERR_FINISH_IN_PAST); i++) {
		uint8_t byteval = pgm_read_byte(TEXT_ERR_FINISH_IN_PAST + i);
		eeprom_write_byte((uint8_t*)&ee_textErrFinishInPast[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textErrFinishInPast[i], 0);
		
	/* Set Err Start in Past String */
		for (i = 0; i < strlen_P(TEXT_ERR_START_IN_PAST); i++) {
			uint8_t byteval = pgm_read_byte(TEXT_ERR_START_IN_PAST + i);
			eeprom_write_byte((uint8_t*)&ee_textErrStartInPast[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textErrStartInPast[i], 0);

	/* Set Err Finish Before Start String */
		for(i=0; i < strlen_P(TEXT_ERR_FINISH_BEFORE_START); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_FINISH_BEFORE_START + i);
			eeprom_write_byte((uint8_t*)&ee_textErrFinishB4Start[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textErrFinishB4Start[i], 0);

	/* Set Err Invalid Time String */
		for(i=0; i < strlen_P(TEXT_ERR_INVALID_TIME); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_INVALID_TIME + i);
			eeprom_write_byte((uint8_t*)&ee_textErrInvalidTime[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textErrInvalidTime[i], 0);

	/* Set Err Time In Past String */
		for(i=0; i < strlen_P(TEXT_ERR_TIME_IN_PAST); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_TIME_IN_PAST + i);
			eeprom_write_byte((uint8_t*)&ee_textErrTimeInPast[i], byteval);
		}

		eeprom_write_byte((uint8_t*)&ee_textErrTimeInPast[i], 0);

	/* Done */

		lb_send_string((char*)"EEPROM PROGRAMMING FINISHED\n", TRUE);
	}
#endif  /* INIT_EEPROM_ONLY */


BOOL readNonVolatile(void)
{
	BOOL failure = TRUE;
	uint16_t i;
	uint8_t initialization_flag = eeprom_read_byte(&ee_interface_eeprom_initialization_flag);

	if(initialization_flag == EEPROM_INITIALIZED_FLAG)   /* EEPROM is up to date */
	{
		g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM,eeprom_read_byte(&ee_id_codespeed),MAX_CODE_SPEED_WPM);
		g_fox = CLAMP(BEACON,(Fox_t)eeprom_read_byte(&ee_fox_setting),NO_CODE_START_TONES_5M);
		g_atmega_temp_calibration = (int16_t)eeprom_read_word((uint16_t*)&ee_atmega_temp_calibration);
		g_rv3028_offset = (int16_t)eeprom_read_word((uint16_t*)&ee_rv3028_offset);
		g_enable_LEDs = eeprom_read_byte(&ee_enable_LEDs);
		g_enable_start_timer = eeprom_read_byte(&ee_enable_start_timer);
		g_enable_transmitter = eeprom_read_byte(&ee_enable_transmitter);
		g_event_start_epoch = eeprom_read_dword(&ee_event_start_epoch);
		g_event_finish_epoch = eeprom_read_dword(&ee_event_finish_epoch);

		for(i = 0; i < MAX_PATTERN_TEXT_LENGTH; i++)
		{
			g_messages_text[STATION_ID][i] = (char)eeprom_read_byte((uint8_t*)(&ee_stationID_text[i]));
			if(!g_messages_text[STATION_ID][i])
			{
				break;
			}
		}
		
		/* Perform sanity checks */
		if(g_event_start_epoch && (g_event_finish_epoch <= g_event_start_epoch))
		{
			g_event_finish_epoch = g_event_start_epoch + SECONDS_24H;
		}

		failure = FALSE;
	}
	
	return failure;
}

#if INIT_EEPROM_ONLY
/*
 * Set volatile variables to their values stored in EEPROM
 */
void initializeEEPROMVars(void)
{
	uint16_t i;
				
#if !COMPILE_FOR_ATMELSTUDIO7
	/* Erase full EEPROM */
	for (i = 0 ; i < EEPROM.length() ; i++) {
		EEPROM.write(i, 0xFF);
	}
#endif // !COMPILE_FOR_ATMELSTUDIO7

	g_id_codespeed = EEPROM_ID_CODE_SPEED_DEFAULT;
	eeprom_write_byte(&ee_id_codespeed,g_id_codespeed);

	g_fox = EEPROM_FOX_SETTING_DEFAULT;
	eeprom_write_byte(&ee_fox_setting,g_fox);

	g_atmega_temp_calibration = EEPROM_TEMP_CALIBRATION_DEFAULT;
	eeprom_write_word((uint16_t*)&ee_atmega_temp_calibration,(uint16_t)g_atmega_temp_calibration);

	i2c_init();                                                         /* Needs to happen here */
	g_rv3028_offset = rv3028_get_offset_RAM();
	eeprom_write_word((uint16_t*)&ee_rv3028_offset,(uint16_t)g_rv3028_offset);
	g_allow_rv3028_eeprom_changes = TRUE;   /* Allow 1-sec interrupt to get permanently saved within the RTC EEPROM */

	g_enable_LEDs = EEPROM_ENABLE_LEDS_DEFAULT;
	eeprom_write_byte(&ee_enable_LEDs,g_enable_LEDs);  /* Only gets set by a serial command */

	g_enable_start_timer = EEPROM_ENABLE_STARTTIMER_DEFAULT;
	eeprom_write_byte(&ee_enable_start_timer,g_enable_start_timer);    /* Only gets set by a serial command */

	g_enable_transmitter = EEPROM_ENABLE_TRANSMITTER_DEFAULT;
	eeprom_write_byte(&ee_enable_transmitter,g_enable_transmitter);    /* Only gets set by a serial command */

	g_event_start_epoch = EEPROM_START_EPOCH_DEFAULT;
	eeprom_write_dword(&ee_event_start_epoch,g_event_start_epoch);

	g_event_start_epoch = EEPROM_START_EPOCH_DEFAULT;
	eeprom_write_dword(&ee_event_finish_epoch,g_event_start_epoch);

	g_messages_text[STATION_ID][0] = '\0';
	eeprom_write_byte((uint8_t*)&ee_stationID_text[0],0);

	/* Each correction pulse = 1 tick corresponds to 1 / (16384 x 64) = 0.9537 ppm.
		* ppm frequency change = -0.035 * (T-T0)^2 (+/-10%)
		* Table[0] = 25C, Table[1] = 24C or 26C, Table[2] = 23C or 27C, etc. */
	for(i = 0; i < SIZE_OF_TEMPERATURE_TABLE; i++)  /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
	{
		uint16_t val = (uint16_t)(((i * i) * 37L) / 1000L);
		eeprom_write_byte((uint8_t*)&ee_temperature_table[i],val);
	}
		
	for(i = 0; i < SIZE_OF_DATA_MODULATION; i++)  /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
	{
		float val = 16. * (1. + sinf((i + (SIZE_OF_DATA_MODULATION/4)) * 0.196)); /* Set maximum attenuation at index 0 */
		eeprom_write_byte((uint8_t*)&ee_dataModulation[i],(uint8_t)val);
	}		
	
	initEEPROMStrings();

	eeprom_write_byte(&ee_interface_eeprom_initialization_flag,EEPROM_INITIALIZED_FLAG);

	return;
}
#endif // INIT_EEPROM_ONLY


#if INIT_EEPROM_ONLY
void dumpEEPROMVars(void)
{
	uint8_t byt;
	uint16_t wrd;
	uint32_t dwrd;
			
	sendEEPROMString(&ee_textVersion[0]);
	sendEEPROMString(&ee_textHelp[0]);
	sendEEPROMString(&ee_textSetID[0]);
	sendEEPROMString(&ee_textSetTime[0]);
	sendEEPROMString(&ee_textSetStart[0]);
	sendEEPROMString(&ee_textSetFinish[0]);
	sendEEPROMString(&ee_textErrFinishInPast[0]);
	sendEEPROMString(&ee_textErrStartInPast[0]);
	sendEEPROMString(&ee_textErrFinishB4Start[0]);
	sendEEPROMString(&ee_textErrInvalidTime[0]);
	sendEEPROMString(&ee_textErrTimeInPast[0]);
	
	byt = eeprom_read_byte(&ee_id_codespeed);
	sprintf(g_tempStr, "CS=%d\n", byt);
	lb_send_string(g_tempStr, TRUE);

	byt = eeprom_read_byte(&ee_fox_setting);
	sprintf(g_tempStr, "FX=%d\n", byt);
	lb_send_string(g_tempStr, TRUE);

	wrd = (uint16_t)eeprom_read_word((uint16_t*)&ee_atmega_temp_calibration);
	sprintf(g_tempStr, "TC=%u\n", wrd);
	lb_send_string(g_tempStr, TRUE);
	
	wrd = (uint16_t)eeprom_read_word((uint16_t*)&ee_rv3028_offset);
	sprintf(g_tempStr, "RVO=%u\n", wrd);
	lb_send_string(g_tempStr, TRUE);

	byt = eeprom_read_byte(&ee_enable_LEDs);
	sprintf(g_tempStr, "LED=%d\n", byt);
	lb_send_string(g_tempStr, TRUE);
	
	byt = eeprom_read_byte(&ee_enable_start_timer);
	sprintf(g_tempStr, "STA=%d\n", byt);
	lb_send_string(g_tempStr, TRUE);

	byt = eeprom_read_byte(&ee_enable_transmitter);
	sprintf(g_tempStr, "ETX=%d\n", byt);
	lb_send_string(g_tempStr, TRUE);
	
	dwrd = eeprom_read_dword(&ee_event_start_epoch);
	sprintf(g_tempStr, "SE=%lu\n", dwrd);
	lb_send_string(g_tempStr, TRUE);
	
	dwrd = eeprom_read_dword(&ee_event_finish_epoch);
	sprintf(g_tempStr, "FE=%lu\n", dwrd);
	lb_send_string(g_tempStr, TRUE);

	for(int i = 0; i < MAX_PATTERN_TEXT_LENGTH; i++)
	{
		g_messages_text[STATION_ID][i] = (char)eeprom_read_byte((uint8_t*)(&ee_stationID_text[i]));
		if(!g_messages_text[STATION_ID][i])
		{
			break;
		}
	}
	
	sprintf(g_tempStr, "ID=\"%s\"\n", g_messages_text[STATION_ID]);
	lb_send_string(g_tempStr, TRUE);

	/* Each correction pulse = 1 tick corresponds to 1 / (16384 x 64) = 0.9537 ppm.
		* ppm frequency change = -0.035 * (T-T0)^2 (+/-10%)
		* Table[0] = 25C, Table[1] = 24C or 26C, Table[2] = 23C or 27C, etc. */
	for(int i = 0; i < SIZE_OF_TEMPERATURE_TABLE; i++)  /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
	{
		lb_send_value((char)eeprom_read_word(&ee_temperature_table[i]), (char*)"T");
	}
	
	lb_send_NewLine();
		
	for(int i = 0; i < SIZE_OF_DATA_MODULATION; i++)  /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
	{
		lb_send_value((char)eeprom_read_byte(&ee_dataModulation[i]), (char*)"M");
	}		
	
	lb_send_NewLine();
}
#endif // INIT_EEPROM_ONLY
