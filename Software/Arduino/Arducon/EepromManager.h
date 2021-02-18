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

#if COMPILE_FOR_ATMELSTUDIO7
#include <avr/eeprom.h>
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

#include <time.h>

/* Set Firmware Version Here */
#define PRODUCT_NAME_LONG_TXT "*** Arducon Fox Controller Ver. 0.18 ***\n"
#define HELP_TEXT_TXT "\nCommands:\n  CLK [T|S|F [\"YYMMDDhhmmss\"]] - Read/set time/start/finish\n  FOX [n]- Set fox role\n  ID [c...c] -  Set callsign\n  STA [0|1] - Start tones on/off\n  SYN 0|1|2 - Synchronize\n  TEM - Read temp\n  SPD [s] - Set ID code speed\n  VER - S/W version"
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
	char textVersion[sizeof(PRODUCT_NAME_LONG_TXT)];
	char textHelp[sizeof(HELP_TEXT_TXT)];
	char textSetTime[sizeof(TEXT_SET_TIME_TXT)];
	char textSetStart[sizeof(TEXT_SET_START_TXT)];
	char textSetFinish[sizeof(TEXT_SET_FINISH_TXT)];
	char textSetID[sizeof(TEXT_SET_ID_TXT)];
	char textErrFinishB4Start[sizeof(TEXT_ERR_FINISH_BEFORE_START_TXT)];
	char textErrFinishInPast[sizeof(TEXT_ERR_FINISH_IN_PAST_TXT)];
	char textErrStartInPast[sizeof(TEXT_ERR_START_IN_PAST_TXT)];
	char textErrInvalidTime[sizeof(TEXT_ERR_INVALID_TIME_TXT)];
	char textErrTimeInPast[sizeof(TEXT_ERR_TIME_IN_PAST_TXT)];
	char stationID_text[MAX_PATTERN_TEXT_LENGTH + 1];

	uint16_t temperature_table[SIZE_OF_TEMPERATURE_TABLE];
	uint8_t dataModulation[SIZE_OF_DATA_MODULATION];

	uint8_t id_codespeed;
	uint8_t fox_setting;
	uint8_t enable_LEDs;
	int16_t atmega_temp_calibration;
	int16_t rv3028_offset;
	uint8_t enable_start_timer;
	uint8_t enable_transmitter;
	time_t event_start_epoch;
	time_t event_finish_epoch;
	uint16_t eeprom_initialization_flag;
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

	Id_codespeed,
	Fox_setting,
	Enable_LEDs,
	Atmega_temp_calibration,
	Rv3028_offset,
	Enable_start_timer,
	Enable_transmitter,
	Event_start_epoch,
	Event_finish_epoch,
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

void initEEPROMStrings(void);
void dumpEEPROMVars(void);
BOOL initializeEEPROMVars(void);
BOOL readNonVols(void);
void send_Help(void);
void sendEEPROMString(EE_var_t v);
void updateEEPROMVar(EE_var_t v, void* val);
uint16_t readTemperatureTable(int i);

protected:
private:
EepromManager( const EepromManager &c );
EepromManager& operator=( const EepromManager &c );

};  /*EepromManager */

#endif /*__EEPROMMANAGER_H__ */
