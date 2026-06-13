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

#ifndef __EEPROMMANAGER_H__
#define __EEPROMMANAGER_H__

#include "defs.h"

#ifdef ATMEL_STUDIO_7
#include <avr/eeprom.h>
#endif  /* ATMEL_STUDIO_7 */

#include <time.h>

// Set version information here
#define ARDUCON_FIRMWARE_VERSION "2.0.5"

#if INCLUDE_RV3028_SUPPORT
	#if SUPPORT_ONLY_80M
		/* Set Firmware Version Here */
		#define PRODUCT_NAME_LONG_TXT "*** Arducon Fox Controller Ver. 2.0.5(80m) ***\n"
		#define HELP_TEXT_TXT "\nCommands:\n  CLK [T|S|F|O [\"YYMMDDhhmmss\"]] - Read/set time/start/finish/offset\n  FOX [fox]- Set fox role\n  ID [callsign] -  Set callsign\n  SYN 0-3 - Synchronize\n  PWD [pwd] - Set DTMF password\n  UTI [H tempC|X] - Read volts/temp, set thermal shutdown, or reset max-ever temp\n  SET S|P [setting] - Set ID code speed or PTT reset\n  INF - Firmware information\n  UPD - Enter bootloader update mode"
	#else
		/* Set Firmware Version Here */
		#define PRODUCT_NAME_LONG_TXT "*** Arducon Fox Controller Ver. 2.0.5B ***\n"
		#define HELP_TEXT_TXT "\nCommands:\n  CLK [T|S|F|O [\"YYMMDDhhmmss\"]] - Read/set time/start/finish/offset\n  FOX [fox]- Set fox role\n  ID [callsign] -  Set callsign\n  SYN 0-3 - Synchronize\n  PWD [pwd] - Set DTMF password\n  AM [0-6] - Set AM tone frequency\n  UTI [H tempC|X] - Read volts/temp, set thermal shutdown, or reset max-ever temp\n  SET S|P [setting] - Set ID code speed or PTT reset\n  INF - Firmware information\n  UPD - Enter bootloader update mode"
	#endif  /* SUPPORT_ONLY_80M */
#elif INCLUDE_DS3231_SUPPORT
	#if SUPPORT_ONLY_80M
		/* Set Firmware Version Here */
		#define PRODUCT_NAME_LONG_TXT "*** Arducon Fox Controller Ver. 2.0.5(80m) ***\n"
		#define HELP_TEXT_TXT "\nCommands:\n  CLK [T|S|F [\"YYMMDDhhmmss\"]] - Read/set time/start/finish/offset\n  FOX [fox]- Set fox role\n  ID [callsign] -  Set callsign\n  SYN 0-3 - Synchronize\n  PWD [pwd] - Set DTMF password\n  UTI [H tempC|X] - Read volts/temp, set thermal shutdown, or reset max-ever temp\n  SET S|P [setting] - Set ID code speed or PTT reset\n  INF - Firmware information\n  UPD - Enter bootloader update mode"
	#else
		/* Set Firmware Version Here */
		#define PRODUCT_NAME_LONG_TXT "*** Arducon Fox Controller Ver. 2.0.5 ***\n"
		#define HELP_TEXT_TXT "\nCommands:\n  CLK [T|S|F [\"YYMMDDhhmmss\"]] - Read/set time/start/finish/offset\n  FOX [fox]- Set fox role\n  ID [callsign] -  Set callsign\n  SYN 0-3 - Synchronize\n  PWD [pwd] - Set DTMF password\n  AM [0-6] - Set AM tone frequency\n  UTI [H tempC|X] - Read volts/temp, set thermal shutdown, or reset max-ever temp\n  SET S|P [setting] - Set ID code speed or PTT reset\n  INF - Firmware information\n  UPD - Enter bootloader update mode"
	#endif  /* SUPPORT_ONLY_80M */
#endif          /* INCLUDE_RV3028_SUPPORT */

#define TEXT_SET_TIME_TXT "CLK T YYMMDDhhmmss <- Set current time\n"
#define TEXT_SET_START_TXT "CLK S YYMMDDhhmmss <- Set start time\n"
#define TEXT_SET_FINISH_TXT "CLK F YYMMDDhhmmss <- Set finish time\n"
#define TEXT_SET_ID_TXT "ID [\"callsign\"] <- Set callsign\n"
#define TEXT_ERR_FINISH_BEFORE_START_TXT "Err: Finish before start!\n"
#define TEXT_ERR_FINISH_IN_PAST_TXT "Err: Finish in past!\n"
#define TEXT_ERR_START_IN_PAST_TXT "Err: Start in past!\n"
#define TEXT_ERR_INVALID_TIME_TXT "Err: Invalid time!\n"
#define TEXT_ERR_TIME_IN_PAST_TXT "Err: Time in past!\n"

struct EE_prom
{
	uint16_t eeprom_initialization_flag;
	uint16_t eeprom_layout_version;
	uint16_t temperature_table[SIZE_OF_TEMPERATURE_TABLE];
	int16_t atmega_temp_calibration;
	int8_t thermal_shutdown_temperature_c;
	int16_t max_ever_temperature_tenths;
	int16_t rv3028_offset;
	time_t event_start_epoch;
	time_t event_finish_epoch;
	char stationID_text[MAX_PATTERN_TEXT_LENGTH + 1];

	uint8_t dataModulation[SIZE_OF_DATA_MODULATION];
	uint8_t unlockCode[MAX_UNLOCK_CODE_LENGTH + 1];

	uint8_t id_codespeed;
	uint8_t fox_setting;
	uint8_t am_audio_frequency;
	uint8_t utc_offset;
	uint8_t ptt_periodic_reset;
};

typedef enum
{
	TextVersion,
	TextHelp,
	TextSetTime,
	TextSetStart,
	TextSetFinish,
	TextSetID,
	TextErrFinishB4Start,
	TextErrFinishInPast,
	TextErrStartInPast,
	TextErrInvalidTime,
	TextErrTimeInPast,

	StationID_text,

	Temperature_table,
	DataModulation,
	UnlockCode,

	Id_codespeed,
	Fox_setting,
	Am_audio_frequency,
	Atmega_temp_calibration,
	Thermal_shutdown_temperature_c,
	Max_ever_temperature_tenths,
	Rv3028_offset,
	Event_start_epoch,
	Event_finish_epoch,
	Utc_offset,
	Ptt_periodic_reset,
	Eeprom_initialization_flag
} EE_var_t;

class EepromManager
{
/*variables */
public:
protected:
private:

/*functions */
public:
EepromManager();
~EepromManager();

static const struct EE_prom ee_vars;

//void initEEPROMStrings(void);
BOOL initializeEEPROMVars(void);
BOOL readNonVols(void);
void send_Help(void);
// Historical API name retained for low-risk call-site stability. Fixed text now
// lives in PROGMEM; this helper no longer reads command response text from EEPROM.
void sendEEPROMString(EE_var_t v);
void updateEEPROMVar(EE_var_t v, void* val);
uint16_t readTemperatureTable(int i);
void resetEEPROMValues(void);


protected:
private:
EepromManager( const EepromManager &c );
EepromManager& operator=( const EepromManager &c );
BOOL eepromLayoutIsCurrent(void);

};  /*EepromManager */

#endif  /*__EEPROMMANAGER_H__ */
