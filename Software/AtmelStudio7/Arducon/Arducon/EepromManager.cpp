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


#include "EepromManager.h"
#include "linkbus.h"
#include "i2c.h"

#if INCLUDE_RV3028_SUPPORT
#include "rv3028.h"
#endif

#if INCLUDE_DS3231_SUPPORT
#include "ds3231.h"
#endif

#ifdef ATMEL_STUDIO_7
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#endif  /* ATMEL_STUDIO_7 */

/* Set Firmware Version Here */
const char PRODUCT_NAME_LONG[] PROGMEM = PRODUCT_NAME_LONG_TXT;
const char HELP_TEXT[] PROGMEM = HELP_TEXT_TXT;
const char TEXT_SET_TIME[] PROGMEM = TEXT_SET_TIME_TXT;
const char TEXT_SET_START[] PROGMEM = TEXT_SET_START_TXT;
const char TEXT_SET_FINISH[] PROGMEM = TEXT_SET_FINISH_TXT;
const char TEXT_SET_ID[] PROGMEM = TEXT_SET_ID_TXT;
const char TEXT_ERR_FINISH_BEFORE_START[] PROGMEM = TEXT_ERR_FINISH_BEFORE_START_TXT;
const char TEXT_ERR_FINISH_IN_PAST[] PROGMEM = TEXT_ERR_FINISH_IN_PAST_TXT;
const char TEXT_ERR_START_IN_PAST[] PROGMEM = TEXT_ERR_START_IN_PAST_TXT;
const char TEXT_ERR_INVALID_TIME[] PROGMEM = TEXT_ERR_INVALID_TIME_TXT;
const char TEXT_ERR_TIME_IN_PAST[] PROGMEM = TEXT_ERR_TIME_IN_PAST_TXT;

#if INIT_EEPROM_ONLY
const char TEXT_EEPROM_SUCCESS_MESSAGE[] PROGMEM = TEXT_EEPROM_SUCCESS_MESSAGE_TXT;
#endif // INIT_EEPROM_ONLY

/***********************************************************************
 * Global Variables & String Constants
 *
 * Identify each global with a "g_" prefix
 * Whenever possible limit globals' scope to this file using "static"
 * Use "volatile" for globals shared between ISRs and foreground loop
 ************************************************************************/

const struct EE_prom EEMEM EepromManager::ee_vars =
{
	/* .eeprom_initialization_flag = */ 0,
	/* .temperature_table = */ { 0 },
	/* .atmega_temp_calibration = */ 0,
	/* .rv3028_offset = */ 0,
	/* .event_start_epoch = */ 0,
	/* .event_finish_epoch = */ 0,
	/* .textVersion = */ "\0",
	/* .textHelp = */ "\0",
	/* .textSetTime = */ "\0",
	/* .textSetStart = */ "\0",
	/* .textSetFinish = */ "\0",
	/* .textSetID = */ "\0",
	/* .textErrFinishB4Start = */ "\0",
	/* .textErrFinishInPast = */ "\0",
	/* .textErrStartInPast = */ "\0",
	/* .textErrInvalidTime = */ "\0",
	/* .textErrTimeInPast = */ "\0",
	/* .stationID_text = */ "\0",

	/* .dataModulation = */ { 0 },
	/* .unlockCode = */ EEPROM_DTMF_UNLOCK_CODE_DEFAULT,

	/* .id_codespeed = */ 0,
	/* .fox_setting = */ 0,
	/* .am_audio_frequency = */ 0,
	/* .utc_offset = */ 0,
	/* .ptt_periodic_reset = */ 0
};

extern char g_messages_text[][MAX_PATTERN_TEXT_LENGTH + 1];
extern volatile uint8_t g_id_codespeed;
extern volatile uint8_t g_pattern_codespeed;
extern volatile uint16_t g_time_needed_for_ID;
extern volatile int16_t g_atmega_temp_calibration;
extern volatile uint8_t g_temperature_check_countdown;
extern volatile int16_t g_rv3028_offset;

extern volatile Fox_t g_fox;
extern volatile AM_Tone_Freq_t g_AM_audio_frequency;
extern volatile time_t g_event_start_epoch;
extern volatile time_t g_event_finish_epoch;
extern volatile int8_t g_utc_offset;
extern volatile uint8_t g_ptt_periodic_reset_enabled;

extern uint8_t g_dataModulation[];
extern uint8_t g_unlockCode[];

extern char g_tempStr[];

/* default constructor */
EepromManager::EepromManager()
{
}   /*EepromManager */

/* default destructor */
EepromManager::~EepromManager()
{
}   /*~EepromManager */

void EepromManager::updateEEPROMVar(EE_var_t v, void* val)
{
	uint8_t* ee_byte_addr = NULL;
	uint16_t* ee_word_addr = NULL;
	uint32_t* ee_dword_addr = NULL;

	if(!val)
	{
		return;
	}

	switch(v)
	{
		case StationID_text:
		{
			char* char_addr = (char*)val;
			char c = *char_addr++;
			int i = 0;

			while(c)
			{
				eeprom_update_byte((uint8_t*)&(EepromManager::ee_vars.stationID_text[i++]), (uint8_t)c);
				c = *char_addr++;
			}

			eeprom_update_byte((uint8_t*)&(EepromManager::ee_vars.stationID_text[i]), 0);
		}
		break;

		case UnlockCode:
		{
			uint8_t* uint8_addr = (uint8_t*)val;
			uint8_t c = *uint8_addr++;
			int i = 0;

			while(c && (i < MAX_UNLOCK_CODE_LENGTH))
			{
				eeprom_update_byte((uint8_t*)&(EepromManager::ee_vars.unlockCode[i++]), c);
				c = *uint8_addr++;
			}

			eeprom_update_byte((uint8_t*)&(EepromManager::ee_vars.unlockCode[i]), 0);
		}
		break;

		case Id_codespeed:
		{
			ee_byte_addr = (uint8_t*)&(EepromManager::ee_vars.id_codespeed);
		}
		break;

		case Fox_setting:
		{
			ee_byte_addr = (uint8_t*)&(EepromManager::ee_vars.fox_setting);
		}
		break;

		case Am_audio_frequency:
		{
			ee_byte_addr = (uint8_t*)&(EepromManager::ee_vars.am_audio_frequency);
		}
		break;

		case Atmega_temp_calibration:
		{
			ee_word_addr = (uint16_t*)&(EepromManager::ee_vars.atmega_temp_calibration);
		}
		break;

		case Rv3028_offset:
		{
			ee_word_addr = (uint16_t*)&(EepromManager::ee_vars.rv3028_offset);
		}
		break;

		case Event_start_epoch:
		{
			ee_dword_addr = (uint32_t*)&(EepromManager::ee_vars.event_start_epoch);
		}
		break;

		case Event_finish_epoch:
		{
			ee_dword_addr = (uint32_t*)&(EepromManager::ee_vars.event_finish_epoch);
		}
		break;

		case Utc_offset:
		{
			ee_byte_addr = (uint8_t*)&(EepromManager::ee_vars.utc_offset);
		}
		break;

		case Ptt_periodic_reset:
		{
			ee_byte_addr = (uint8_t*)&(EepromManager::ee_vars.ptt_periodic_reset);
		}
		break;

		case Eeprom_initialization_flag:
		{
			ee_word_addr = (uint16_t*)&(EepromManager::ee_vars.eeprom_initialization_flag);
		}
		break;

		default:
		{

		}
		break;
	}

	if(ee_byte_addr)
	{
		eeprom_update_byte(ee_byte_addr, *(uint8_t*)val);
	}
	else if(ee_word_addr)
	{
		eeprom_update_word(ee_word_addr, *(uint16_t*)val);
	}
	else if(ee_dword_addr)
	{
		eeprom_update_dword(ee_dword_addr, *(uint32_t*)val);
	}
}

void EepromManager::sendEEPROMString(EE_var_t v)
{
	char* ee_addr = NULL;

	if(!lb_enabled())
	{
		return;
	}

	switch(v)
	{
		case TextVersion:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textVersion[0]);
		}
		break;

		case TextHelp:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textHelp[0]);
		}
		break;

		case TextSetTime:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textSetTime[0]);
		}
		break;

		case TextSetStart:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textSetStart[0]);
		}
		break;

		case TextSetFinish:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textSetFinish[0]);
		}
		break;

		case TextSetID:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textSetID[0]);
		}
		break;

		case TextErrFinishB4Start:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textErrFinishB4Start[0]);

		}
		break;

		case TextErrFinishInPast:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textErrFinishInPast[0]);
		}
		break;

		case TextErrStartInPast:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textErrStartInPast[0]);
		}
		break;

		case TextErrInvalidTime:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textErrInvalidTime[0]);
		}
		break;

		case TextErrTimeInPast:
		{
			ee_addr = (char*)&(EepromManager::ee_vars.textErrTimeInPast[0]);
		}
		break;

		default:
		{

		}
		break;
	}

	if(ee_addr)
	{
		char c = eeprom_read_byte((uint8_t*)ee_addr++);

		while(c)
		{
			lb_echo_char(c);
			c = eeprom_read_byte((uint8_t*)(ee_addr++));

			while(linkbusTxInProgress())
			{
				;
			}
		}
	}
}

BOOL EepromManager::readNonVols(void)
{
	BOOL failure = TRUE;
	uint16_t i;
	uint16_t initialization_flag = eeprom_read_word(&(EepromManager::ee_vars.eeprom_initialization_flag));

	if(initialization_flag == EEPROM_INITIALIZED_FLAG)  /* EEPROM is up to date */
	{
		g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM, eeprom_read_byte(&(EepromManager::ee_vars.id_codespeed)), MAX_CODE_SPEED_WPM);
		g_fox = CLAMP(BEACON, (Fox_t)eeprom_read_byte(&(EepromManager::ee_vars.fox_setting)), SPRINT_F5);
		g_AM_audio_frequency = (AM_Tone_Freq_t)eeprom_read_byte(&(EepromManager::ee_vars.am_audio_frequency));
		g_atmega_temp_calibration = (int16_t)eeprom_read_word((uint16_t*)&(EepromManager::ee_vars.atmega_temp_calibration));
		g_rv3028_offset = (int16_t)eeprom_read_word((uint16_t*)&(EepromManager::ee_vars.rv3028_offset));
		g_event_start_epoch = eeprom_read_dword(&(EepromManager::ee_vars.event_start_epoch));
		g_event_finish_epoch = eeprom_read_dword(&(EepromManager::ee_vars.event_finish_epoch));
		g_utc_offset = (int8_t)eeprom_read_byte(&(EepromManager::ee_vars.utc_offset));
		g_ptt_periodic_reset_enabled = eeprom_read_byte(&(EepromManager::ee_vars.ptt_periodic_reset));

		for(i = 0; i < MAX_PATTERN_TEXT_LENGTH; i++)
		{
			g_messages_text[STATION_ID][i] = (char)eeprom_read_byte((uint8_t*)(&(EepromManager::ee_vars.stationID_text[i])));
			if(!g_messages_text[STATION_ID][i])
			{
				break;
			}
		}

		for(i = 0; i < MAX_UNLOCK_CODE_LENGTH; i++)
		{
			g_unlockCode[i] = (char)eeprom_read_byte((uint8_t*)(&(EepromManager::ee_vars.unlockCode[i])));
			if(!g_unlockCode[i])
			{
				break;
			}
		}

		for(i = 0; i < SIZE_OF_DATA_MODULATION; i++)    /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
		{
			g_dataModulation[i] = (uint8_t)eeprom_read_byte((uint8_t*)&(EepromManager::ee_vars.dataModulation[i]));
		}

		/* Perform sanity checks */
		if(g_event_start_epoch && (g_event_finish_epoch <= g_event_start_epoch))
		{
			g_event_finish_epoch = g_event_start_epoch + SECONDS_24H;
		}

		failure = FALSE;
	}

	return( failure);
}

#if INIT_EEPROM_ONLY
	void EepromManager::sendSuccessString(void)
	{
		sendEEPROMString(TextVersion);
		sendPROGMEMString((const char*)&TEXT_EEPROM_SUCCESS_MESSAGE);
//		dumpEEPROMVars();
	}

	void EepromManager::sendPROGMEMString(const char* fl_addr)
	{
		if(!lb_enabled())
		{
			return;
		}

		if(fl_addr)
		{
			char c = pgm_read_byte(fl_addr++);

			while(c)
			{
				lb_echo_char(c);
				c = pgm_read_byte((fl_addr++));

				while(linkbusTxInProgress())
				{
					;
				}
			}
		}
	}

/*
 * Set volatile variables to their values stored in EEPROM
 */
	BOOL EepromManager::initializeEEPROMVars(void)
	{
		BOOL err = FALSE;
		uint16_t i;

#ifndef ATMEL_STUDIO_7
			/* Erase full EEPROM */
			for(i = 0; i < 0x0400; i++)
			{
				eeprom_write_byte((uint8_t*)i, 0xFF);
			}

			for(i = 0; i < 0x0400; i++)
			{
				uint8_t x = eeprom_read_byte((const uint8_t*)&i);
				if(x != 0xFF)
				{
					err = TRUE;
				}
			}
#endif  /* !ATMEL_STUDIO_7 */

		g_id_codespeed = EEPROM_ID_CODE_SPEED_DEFAULT;
		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.id_codespeed), g_id_codespeed);

		g_fox = EEPROM_FOX_SETTING_DEFAULT;
		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.fox_setting), g_fox);

		g_AM_audio_frequency = EEPROM_AM_AUDIO_FREQ_DEFAULT;
		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.am_audio_frequency), (uint8_t)g_AM_audio_frequency);

		g_atmega_temp_calibration = EEPROM_TEMP_CALIBRATION_DEFAULT;
		eeprom_write_word((uint16_t*)&(EepromManager::ee_vars.atmega_temp_calibration), (uint16_t)g_atmega_temp_calibration);

		i2c_init(); /* Needs to happen before reading RTC */

#if INCLUDE_RV3028_SUPPORT
		g_rv3028_offset = rv3028_get_offset_RAM();
		eeprom_write_word((uint16_t*)&(EepromManager::ee_vars.rv3028_offset), (uint16_t)g_rv3028_offset);
#endif // INCLUDE_RV3028_SUPPORT

		g_event_start_epoch = EEPROM_START_EPOCH_DEFAULT;
		eeprom_write_dword((uint32_t*)&(EepromManager::ee_vars.event_start_epoch), g_event_start_epoch);

		g_event_finish_epoch = EEPROM_FINISH_EPOCH_DEFAULT;
		eeprom_write_dword((uint32_t*)&(EepromManager::ee_vars.event_finish_epoch), g_event_finish_epoch);

		g_utc_offset = EEPROM_UTC_OFFSET_DEFAULT;
		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.utc_offset), (uint8_t)g_utc_offset);

		g_ptt_periodic_reset_enabled = EEPROM_PTT_PERIODIC_RESET_DEFAULT;
		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.ptt_periodic_reset), g_ptt_periodic_reset_enabled);

		g_messages_text[STATION_ID][0] = '\0';
		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.stationID_text[0]), 0);

		uint8_t *v = (uint8_t*)EEPROM_DTMF_UNLOCK_CODE_DEFAULT;
		for(i = 0; i < strlen(EEPROM_DTMF_UNLOCK_CODE_DEFAULT); i++)
		{
			g_unlockCode[i] = *v;
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.unlockCode[i]), *v++);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.unlockCode[i]), 0);
		g_unlockCode[i] = '\0';

		/* Each correction pulse = 1 tick corresponds to 1 / (16384 x 64) = 0.9537 ppm.
		 * ppm frequency change = -0.035 * (T-T0)^2 (+/-10%)
		 * Table[0] = 25C, Table[1] = 24C or 26C, Table[2] = 23C or 27C, etc. */
		for(i = 0; i < SIZE_OF_TEMPERATURE_TABLE; i++)  /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
		{
			uint16_t val = (uint16_t)(((i * i) * 37L) / 1000L);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.temperature_table[i]), val);
		}

		for(i = 0; i < SIZE_OF_DATA_MODULATION; i++)
		{
			float val = 5.5 * squaref((1.4 + sinf((i + (SIZE_OF_DATA_MODULATION / 4)) * 0.196))); /* Set maximum attenuation to fall at index 0 */
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.dataModulation[i]), (uint8_t)val);
		}

		/* Software Version String */
		for(i = 0; i < strlen_P(PRODUCT_NAME_LONG); i++)
		{
			uint8_t byteval = pgm_read_byte(PRODUCT_NAME_LONG + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textVersion[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textVersion[i]), 0);

		/* Help String */
		for(i = 0; i < strlen_P(HELP_TEXT); i++)
		{
			uint8_t byteval = pgm_read_byte(HELP_TEXT + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textHelp[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textHelp[i]), 0);

		/* Set ID String */
		for(i = 0; i < strlen_P(TEXT_SET_ID); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_SET_ID + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetID[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetID[i]), 0);

		/* Set Time String */
		for(i = 0; i < strlen_P(TEXT_SET_TIME); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_SET_TIME + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetTime[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetTime[i]), 0);

		/* Set Start String */
		for(i = 0; i < strlen_P(TEXT_SET_START); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_SET_START + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetStart[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetStart[i]), 0);

		/* Set Finish String */
		for(i = 0; i < strlen_P(TEXT_SET_FINISH); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_SET_FINISH + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetFinish[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textSetFinish[i]), 0);

		/* Set Err Finish in Past String */
		for(i = 0; i < strlen_P(TEXT_ERR_FINISH_IN_PAST); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_FINISH_IN_PAST + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrFinishInPast[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrFinishInPast[i]), 0);

		/* Set Err Start in Past String */
		for(i = 0; i < strlen_P(TEXT_ERR_START_IN_PAST); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_START_IN_PAST + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrStartInPast[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrStartInPast[i]), 0);

		/* Set Err Finish Before Start String */
		for(i = 0; i < strlen_P(TEXT_ERR_FINISH_BEFORE_START); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_FINISH_BEFORE_START + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrFinishB4Start[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrFinishB4Start[i]), 0);

		/* Set Err Invalid Time String */
		for(i = 0; i < strlen_P(TEXT_ERR_INVALID_TIME); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_INVALID_TIME + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrInvalidTime[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrInvalidTime[i]), 0);

		/* Set Err Time In Past String */
		for(i = 0; i < strlen_P(TEXT_ERR_TIME_IN_PAST); i++)
		{
			uint8_t byteval = pgm_read_byte(TEXT_ERR_TIME_IN_PAST + i);
			eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrTimeInPast[i]), byteval);
		}

		eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.textErrTimeInPast[i]), 0);

		/* Done */

		eeprom_write_word((uint16_t*)&(EepromManager::ee_vars.eeprom_initialization_flag), EEPROM_INITIALIZED_FLAG);

		return(err);
	}


	void EepromManager::dumpEEPROMVars(void)
	{
		uint8_t byt;
		uint16_t wrd;
		uint32_t dwrd;

		sendEEPROMString(TextVersion);
		sendEEPROMString(TextHelp);
		sendEEPROMString(TextSetID);
		sendEEPROMString(TextSetTime);
		sendEEPROMString(TextSetStart);
		sendEEPROMString(TextSetFinish);
		sendEEPROMString(TextErrFinishInPast);
		sendEEPROMString(TextErrStartInPast);
		sendEEPROMString(TextErrFinishB4Start);
		sendEEPROMString(TextErrInvalidTime);
		sendEEPROMString(TextErrTimeInPast);

		byt = eeprom_read_byte(&(EepromManager::ee_vars.id_codespeed));
		sprintf(g_tempStr, "CS=%d\n", byt);
		lb_send_string(g_tempStr, TRUE);

		byt = eeprom_read_byte(&(EepromManager::ee_vars.fox_setting));
		sprintf(g_tempStr, "FX=%d\n", byt);
		lb_send_string(g_tempStr, TRUE);

		byt = eeprom_read_byte(&(EepromManager::ee_vars.am_audio_frequency));
		sprintf(g_tempStr, "AM=%d\n", byt);
		lb_send_string(g_tempStr, TRUE);

		wrd = (uint16_t)eeprom_read_word((uint16_t*)&(EepromManager::ee_vars.atmega_temp_calibration));
		sprintf(g_tempStr, "TC=%u\n", wrd);
		lb_send_string(g_tempStr, TRUE);

		wrd = (uint16_t)eeprom_read_word((uint16_t*)&(EepromManager::ee_vars.rv3028_offset));
		sprintf(g_tempStr, "RVO=%u\n", wrd);
		lb_send_string(g_tempStr, TRUE);

		dwrd = eeprom_read_dword(&(EepromManager::ee_vars.event_start_epoch));
		sprintf(g_tempStr, "SE=%lu\n", dwrd);
		lb_send_string(g_tempStr, TRUE);

		dwrd = eeprom_read_dword(&(EepromManager::ee_vars.event_finish_epoch));
		sprintf(g_tempStr, "FE=%lu\n", dwrd);
		lb_send_string(g_tempStr, TRUE);

		byt = eeprom_read_byte(&(EepromManager::ee_vars.utc_offset));
		sprintf(g_tempStr, "UO=%d\n", (int8_t)byt);
		lb_send_string(g_tempStr, TRUE);

		byt = eeprom_read_byte(&(EepromManager::ee_vars.ptt_periodic_reset));
		sprintf(g_tempStr, "PT=%d\n", byt);
		lb_send_string(g_tempStr, TRUE);

		for(int i = 0; i < MAX_PATTERN_TEXT_LENGTH; i++)
		{
			g_messages_text[STATION_ID][i] = (char)eeprom_read_byte((uint8_t*)(&(EepromManager::ee_vars.stationID_text[i])));
			if(!g_messages_text[STATION_ID][i])
			{
				break;
			}
		}

		sprintf(g_tempStr, "ID=\"%s\"\n", g_messages_text[STATION_ID]);
		lb_send_string(g_tempStr, TRUE);

		for(int i = 0; i < MAX_UNLOCK_CODE_LENGTH; i++)
		{
			g_unlockCode[i] = eeprom_read_byte((uint8_t*)(&(EepromManager::ee_vars.unlockCode[i])));
			if(!g_unlockCode[i])
			{
				break;
			}
		}

		sprintf(g_tempStr, "PW=\"%s\"\n", (char*)g_unlockCode);
		lb_send_string(g_tempStr, TRUE);


		/* Each correction pulse = 1 tick corresponds to 1 / (16384 x 64) = 0.9537 ppm.
		 * ppm frequency change = -0.035 * (T-T0)^2 (+/-10%)
		 * Table[0] = 25C, Table[1] = 24C or 26C, Table[2] = 23C or 27C, etc. */
		for(int i = 0; i < SIZE_OF_TEMPERATURE_TABLE; i++)  /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
		{
			lb_send_value((char)eeprom_read_word(&(EepromManager::ee_vars.temperature_table[i])), (char*)"T");
		}

		lb_send_NewLine();

		for(int i = 0; i < SIZE_OF_DATA_MODULATION; i++)    /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
		{
			lb_send_value((char)eeprom_read_byte(&(EepromManager::ee_vars.dataModulation[i])), (char*)"M");
		}

		lb_send_NewLine();
	}
#endif  /* INIT_EEPROM_ONLY */

void EepromManager::resetEEPROMValues(void)
{
	uint8_t i;

	uint8_t *v = (uint8_t*)EEPROM_DTMF_UNLOCK_CODE_DEFAULT;
	for(i = 0; i < strlen(EEPROM_DTMF_UNLOCK_CODE_DEFAULT); i++)
	{
		g_unlockCode[i] = *v;
		eeprom_write_byte((uint8_t*)&(g_unlockCode[i]), *v++);
	}

	eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.unlockCode[i]), 0);
	g_unlockCode[i] = '\0';

	g_messages_text[STATION_ID][0] = '\0';
	eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.stationID_text[0]), 0);

	g_AM_audio_frequency = EEPROM_AM_AUDIO_FREQ_DEFAULT;
	eeprom_write_byte((uint8_t*)&(EepromManager::ee_vars.am_audio_frequency), (uint8_t)g_AM_audio_frequency);

	g_event_start_epoch = EEPROM_START_EPOCH_DEFAULT;
	eeprom_write_dword((uint32_t*)&(EepromManager::ee_vars.event_start_epoch), g_event_start_epoch);

	g_event_finish_epoch = EEPROM_FINISH_EPOCH_DEFAULT;
	eeprom_write_dword((uint32_t*)&(EepromManager::ee_vars.event_finish_epoch), g_event_finish_epoch);
}

/***********************************************************************
 * send_Help(void)
 ************************************************************************/
void EepromManager::send_Help(void)
{

	lb_send_NewLine();
	sendEEPROMString(TextVersion);
	sendEEPROMString(TextHelp);
	lb_send_NewLine();
	lb_send_NewLine();
}

uint16_t EepromManager::readTemperatureTable(int i)
{
	return( (uint16_t)eeprom_read_word(&(EepromManager::ee_vars.temperature_table[i])));
}

