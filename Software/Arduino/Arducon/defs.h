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

#ifndef DEFS_H
#define DEFS_H

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

/***********************************************************
 * IMPORTANT:                                               *
 * Compile for initializing EEPROM values                   *
 * Note: Build and run with the following compilation flag  *
 *       set to TRUE. Then build and run with the flag      *
 *       set to FALSE.                                      */
/***********************************************************/
#define INIT_EEPROM_ONLY FALSE
/***********************************************************/


/***********************************************************
 * Uncomment the correct frequency for the Arduino Pro Mini *
 * you will be using                                        */
/***********************************************************/
#define PROCESSOR_FREQ_HERTZ 16000000UL
// #define PROCESSOR_FREQ_HERTZ 8000000UL
/***********************************************************/


/*************************************************************************
 * IMPORTANT: CHANGING ANY COMPILE SETTINGS BELOW THIS LINE WILL REQUIRE  *
 * BUILDING AND RUNNING THE PROGRAM WITH INIT_EEPROM_ONLY TRUE,           *
 * AND THEN BUILDING AND RUNNING THE PROGRAM WITH INIT_EEPROM_ONLY FALSE. */
/*************************************************************************/


/***********************************************************
 * OPTIONAL COMPILE:                                        *
 * Compile for AM Attenuator Support                        *
 * Note: Setting the following compile flag to TRUE will    *
 *  disable software support for the AM attenuator, and     *
 *  will cause the Arduino Pro Mini's green LED to be used  *
 *  for visible enunciations.                               */
/***********************************************************/
#define SUPPORT_ONLY_80M FALSE
/***********************************************************/


#ifdef F_CPU
 #undef F_CPU
#endif

#define F_CPU PROCESSOR_FREQ_HERTZ

#ifdef ATMEL_STUDIO_7
	#include <avr/io.h>
	#include <avr/interrupt.h>
	#define USE_WDT_RESET TRUE
#else
	#include "Arduino.h"
	#define USE_WDT_RESET FALSE
#endif  /* ATMEL_STUDIO_7 */

#ifndef HIGH
#define HIGH 0x1
#endif

#ifndef LOW
#define LOW  0x0
#endif

#ifndef UP
#define UP 1
#endif

#ifndef DOWN
#define DOWN 0
#endif

#ifndef INPUT
#define INPUT 0x0
#endif

#ifndef OUTPUT
#define OUTPUT 0x1
#endif

#ifndef INPUT_PULLUP
#define INPUT_PULLUP 0x3
#endif

#define MAX_PATTERN_TEXT_LENGTH 20
#define TEMP_STRING_LENGTH (MAX_PATTERN_TEXT_LENGTH + 20)
#define MAX_DTMF_ARG_LENGTH TEMP_STRING_LENGTH
#define SIZE_OF_TEMPERATURE_TABLE 60
#define SIZE_OF_DATA_MODULATION 32
#define MAX_UNLOCK_CODE_LENGTH 8
#define MIN_UNLOCK_CODE_LENGTH 4


/******************************************************
 * Set Hardware Settings */

/*#define TRANQUILIZE_WATCHDOG
 *#define DEBUG_DTMF */

#define HARDWARE_EXTERNAL_DIP_PULLUPS_INSTALLED FALSE
#define INCLUDE_RV3028_SUPPORT FALSE
#define INCLUDE_DS3231_SUPPORT TRUE

#if !INCLUDE_DS3231_SUPPORT && !INCLUDE_RV3028_SUPPORT
#error One real-time clock device must be supported
#endif

#if INCLUDE_DS3231_SUPPORT && INCLUDE_RV3028_SUPPORT
#error Only one real-time clock device may be enabled
#endif

/*******************************************************/

#ifndef uint16_t_defined
#define uint16_t_defined
	typedef unsigned int uint16_t;
#endif

#ifndef uint32_t_defined
#define uint32_t_defined
	typedef unsigned long uint32_t;
#endif

#ifndef unit8_t_defined
#define unit8_t_defined
	typedef unsigned char uint8_t;
#endif

#ifndef null
#define null 0
#endif

#ifndef PI
#define PI 3.141592653589793
#endif

#ifndef MINUTE
#define MINUTE 60UL
#endif

#ifndef HOUR
#define HOUR 3600UL
#endif

#ifndef DAY
#define DAY 86400UL
#endif

#ifndef YEAR
#define YEAR 31536000UL
#endif

/*
 *  Arduino Pro Mini pin definitions per Arduino IDE
 */
#define D2 2
#define D3 3
#define D4 4
#define D5 5
#define D6 6
#define D7 7
#define D8 8
#define D9 9
#define D10 10
#define D11 11
#define D12 12
#define D13 13
#ifdef ATMEL_STUDIO_7
#define A0 14
#define A1 15
#define A2 16
#define A3 17
#define A4 18
#define A5 19
#define SDA A4
#define SCL A5
#define A6 20
#define A7 21
#endif  /* ATMEL_STUDIO_7 */

/*
 *  Arducon Pin Definitions
 */
#if SUPPORT_ONLY_80M
	#define PIN_RXD 0               /* Arduino Pro Mini pin# 1/28 = PD0 */
	#define PIN_TXD 1               /* Arduino Pro Mini pin# 2/29 = PD1 */
	#define PIN_RESET               /* Arduino Pro Mini pin# 3/22 = PC6 */
	#define PIN_RTC_SQW D2          /* Arduino Pro Mini pin# 5 = PD2 */
	#define PIN_UNUSED_1 D3         /* Arduino Pro Mini pin# 6 = PD3 */
	#define PIN_PTT_LOGIC D4        /* Arduino Pro Mini pin# 7 = PD4 */
	#define PIN_CW_TONE_LOGIC D5    /* Arduino Pro Mini pin# 8 = PD5 */
	#define PIN_CW_KEY_LOGIC D6     /* Arduino Pro Mini pin# 9 = PD6 */
	#define PIN_PWDN D7             /* Arduino Pro Mini pin# 10 = PD7 */
	#define PIN_UNUSED_2 D8         /* Arduino Pro Mini pin# 11 = PB0 */
	#define PIN_UNUSED_3 D9         /* Arduino Pro Mini pin# 12 = PB1 */
	#define PIN_UNUSED_4 D10        /* Arduino Pro Mini pin# 13 = PB2 */
	#define PIN_MOSI D11            /* Arduino Pro Mini pin# 14 = PB3 */
	#define PIN_UNUSED_5 D11        /* Arduino Pro Mini pin# 14 = PB3 */
	#define PIN_MISO D12            /* Arduino Pro Mini pin# 15 = PB4 */
	#define PIN_UNUSED_6 D12        /* Arduino Pro Mini pin# 15 = PB4 */
	#define PIN_LED D13             /* Arduino Pro Mini pin# 16 = PB5 = SCK Use LED on Pro Mini */
	#define PIN_UNUSED_7 A0         /* Arduino Pro Mini pin# 17 = PC0 */
	#define PIN_UNUSED_8 A1         /* Arduino Pro Mini pin# 18 = PC1 */
	#define PIN_UNUSED_9 A2         /* Arduino Pro Mini pin# 19 = PC2 */
	#define PIN_SYNC A3             /* Arduino Pro Mini pin# 20 = PC3 */
	#define PIN_SDA SDA             /* Arduino Pro Mini pin# 33 = SDA */
	#define PIN_SCL SCL             /* Arduino Pro Mini pin# 34 = SCL */
	#define PIN_AUDIO_INPUT A6      /* Arduino Pro Mini pin# 31 = ADC6 */
	#define PIN_BATTERY_LEVEL A7    /* Arduino Pro Mini pin# 32 = ADC7 */
#else
	#define PIN_RXD 0               /* Arduino Pro Mini pin# 1/28 = PD0 */
	#define PIN_TXD 1               /* Arduino Pro Mini pin# 2/29 = PD1 */
	#define PIN_RESET               /* Arduino Pro Mini pin# 3/22 = PC6 */
	#define PIN_RTC_SQW D2          /* Arduino Pro Mini pin# 5 = PD2 */
	#define PIN_UNUSED_1 D3         /* Arduino Pro Mini pin# 6 = PD3 */
	#define PIN_PTT_LOGIC D4        /* Arduino Pro Mini pin# 7 = PD4 */
	#define PIN_CW_TONE_LOGIC D5    /* Arduino Pro Mini pin# 8 = PD5 */
	#define PIN_CW_KEY_LOGIC D6     /* Arduino Pro Mini pin# 9 = PD6 */
	#define PIN_PWDN D7             /* Arduino Pro Mini pin# 10 = PD7 */
	#define PIN_D0 D8               /* Arduino Pro Mini pin# 11 = PB0 */
	#define PIN_D1 D9               /* Arduino Pro Mini pin# 12 = PB1 */
	#define PIN_D2 D10              /* Arduino Pro Mini pin# 13 = PB2 */
	#define PIN_MOSI D11            /* Arduino Pro Mini pin# 14 = PB3 */
	#define PIN_D3 D11              /* Arduino Pro Mini pin# 14 = PB3 */
	#define PIN_MISO D12            /* Arduino Pro Mini pin# 15 = PB4 */
	#define PIN_D4 D12              /* Arduino Pro Mini pin# 15 = PB4 */
	#define PIN_D5 D13              /* Arduino Pro Mini pin# 16 = PB5 = SCK Use LED on Arducon PCB */
	#define PIN_UNUSED_2 A0         /* Arduino Pro Mini pin# 17 = PC0 */
	#define PIN_UNUSED_3 A1         /* Arduino Pro Mini pin# 18 = PC1 */
	#define PIN_LED A2              /* Arduino Pro Mini pin# 19 = PC2 */
	#define PIN_SYNC A3             /* Arduino Pro Mini pin# 20 = PC3 */
	#define PIN_SDA SDA             /* Arduino Pro Mini pin# 33 = SDA */
	#define PIN_SCL SCL             /* Arduino Pro Mini pin# 34 = SCL */
	#define PIN_AUDIO_INPUT A6      /* Arduino Pro Mini pin# 31 = ADC6 */
	#define PIN_BATTERY_LEVEL A7    /* Arduino Pro Mini pin# 32 = ADC7 */
#endif /* SUPPORT_ONLY_80M */

typedef enum
{
	UNSTABLE,
	STABLE_HIGH,
	STABLE_LOW
} ButtonStability_t;

typedef enum
{
	BEACON = 0,
	FOX_1,
	FOX_2,
	FOX_3,
	FOX_4,
	FOX_5,
	FOXORING,
	SPECTATOR,
	SPRINT_S1,
	SPRINT_S2,
	SPRINT_S3,
	SPRINT_S4,
	SPRINT_S5,
	SPRINT_F1,
	SPRINT_F2,
	SPRINT_F3,
	SPRINT_F4,
	SPRINT_F5,
	INVALID_FOX,
	REPORT_BATTERY
} Fox_t;

typedef enum
{
	NO_KEY = 0,
	ONE_KEY = '1',
	TWO_KEY = '2',
	THREE_KEY = '3',
	A_KEY = 'A',
	FOUR_KEY = '4',
	FIVE_KEY = '5',
	SIX_KEY = '6',
	B_KEY = 'B',
	SEVEN_KEY = '7',
	EIGHT_KEY = '8',
	NINE_KEY = '9',
	C_KEY = 'C',
	STAR_KEY = '*',
	ZERO_KEY = '0',
	POUND_KEY = '#',
	D_KEY = 'D'
} DTMF_key_t;

typedef enum
{
	AM_DISABLED,
	AM_500Hz,
	AM_600Hz,
	AM_700Hz,
	AM_800Hz,
	AM_900Hz,
	AM_1000Hz
} AM_Tone_Freq_t;

#define Goertzel_N 201

#define MAX_CODE_SPEED_WPM 20
#define MIN_CODE_SPEED_WPM 5
#define SPRINT_FAST_CODE_SPEED 15
#define SPRINT_SLOW_CODE_SPEED 8

#define MAX_AM_TONE_FREQUENCY 6
#define MIN_AM_TONE_FREQUENCY 0

#define TEMPERATURE_POLL_INTERVAL_SECONDS 59
#define VOLTAGE_POLL_INTERVAL_SECONDS 11

#define LED_TIMEOUT_SECONDS 300

typedef enum
{
	WD_SW_RESETS,
	WD_HW_RESETS,
	WD_FORCE_RESET,
	WD_DISABLE
} WDReset;


typedef enum
{
	STATE_SHUTDOWN,
	STATE_SENTENCE_START,
	STATE_A,
	STATE_PAUSE_TRANSMISSIONS,
	STATE_START_TRANSMISSIONS,
	STATE_START_TRANSMISSIONS_WITH_RTC,
	STATE_START_TRANSMITTING_NOW,
	STATE_C,
	STATE_RECEIVING_CALLSIGN,
	STATE_RECEIVING_FOXFORMATANDID,
	STATE_RECEIVING_START_TIME,
	STATE_RECEIVING_FINISH_TIME,
	STATE_RECEIVING_UTC_OFFSET,
	STATE_RECEIVING_SET_CLOCK,
#if !SUPPORT_ONLY_80M
		STATE_SET_AM_TONE_FREQUENCY,
		STATE_TEST_ATTENUATOR,  /* Temporary test definition */
#endif /* !SUPPORT_ONLY_80M */
	STATE_SET_PTT_PERIODIC_RESET,
	STATE_GET_BATTERY_VOLTAGE,
	STATE_SET_PASSWORD,
	STATE_CHECK_PASSWORD,
	STATE_RECEIVING_FOXES_TO_ADDRESS
} KeyprocessState_t;


/*******************************************************/

#ifndef SELECTIVELY_DISABLE_OPTIMIZATION
	#define SELECTIVELY_DISABLE_OPTIMIZATION
#endif

/******************************************************
 * EEPROM definitions */
#define EEPROM_INITIALIZED_FLAG 0x00BB  /* Never set to 0xFFFF */
#define EEPROM_UNINITIALIZED 0x00

#define EEPROM_STATION_ID_DEFAULT "FOXBOX"
#define EEPROM_PATTERN_TEXT_DEFAULT "PARIS|"

#define EEPROM_START_TIME_DEFAULT 0
#define EEPROM_FINISH_TIME_DEFAULT 0
#define EEPROM_EVENT_ENABLED_DEFAULT FALSE
#define EEPROM_ID_CODE_SPEED_DEFAULT 20
#define EEPROM_PATTERN_CODE_SPEED_DEFAULT 8
#define EEPROM_ON_AIR_TIME_DEFAULT 60
#define EEPROM_OFF_AIR_TIME_DEFAULT 240
#define EEPROM_INTRA_CYCLE_DELAY_TIME_DEFAULT 0
#define EEPROM_TEMP_CALIBRATION_DEFAULT -110
#define EEPROM_RV3028_OFFSET_DEFAULT 0
#define EEPROM_FOX_SETTING_DEFAULT (Fox_t)1
#define EEPROM_AM_AUDIO_FREQ_DEFAULT (AM_Tone_Freq_t)0
#define EEPROM_ENABLE_LEDS_DEFAULT 1
#define EEPROM_ENABLE_STARTTIMER_DEFAULT 0
#define EEPROM_ENABLE_TRANSMITTER_DEFAULT 1
#define EEPROM_START_EPOCH_DEFAULT 0
#define EEPROM_FINISH_EPOCH_DEFAULT 0
#define EEPROM_UTC_OFFSET_DEFAULT 0
#define EEPROM_PTT_PERIODIC_RESET_DEFAULT 0
#define EEPROM_DTMF_UNLOCK_CODE_DEFAULT ("1357")

#define MINIMUM_EPOCH ((time_t)1609459200)  /* 1 Jan 2021 00:00:00 */
#define SECONDS_24H 86400

typedef enum
{
	NULL_CONFIG,
	WAITING_FOR_START,
	CONFIGURATION_ERROR,
	SCHEDULED_EVENT_DID_NOT_START,
	SCHEDULED_EVENT_WILL_NEVER_RUN,
	EVENT_IN_PROGRESS
} ConfigurationState_t;

#define ERROR_BLINK_PATTERN ((char*)"E ")
#define WAITING_BLINK_PATTERN ((char*)"EE          ")
#define DTMF_ERROR_BLINK_PATTERN ((char*)"EEEEEEEE")
#define DTMF_DETECTED_BLINK_PATTERN ((char*)"T")

#ifndef BOOL
	typedef uint8_t BOOL;
#endif

#ifndef Frequency_Hz
	typedef unsigned long Frequency_Hz;
#endif

#ifndef UINT16_MAX
#define UINT16_MAX __INT16_MAX__
#endif

#define OFF             0
#define ON              1
#define TOGGLE          2
#define UNDETERMINED    3

#define MIN(A, B)    ({ __typeof__(A)__a = (A); __typeof__(B)__b = (B); __a < __b ? __a : __b; })
#define MAX(A, B)    ({ __typeof__(A)__a = (A); __typeof__(B)__b = (B); __a < __b ? __b : __a; })

#define CLAMP(low, x, high) ({ \
		__typeof__(x)__x = (x); \
		__typeof__(low)__low = (low); \
		__typeof__(high)__high = (high); \
		__x > __high ? __high : (__x < __low ? __low : __x); \
	})

#define MAX_TIME 4294967295L
#define MAX_UINT16 65535U
#define MAX_INT16 32767
#define MIN_INT16 -32768

/* Periodic TIMER2 interrupt timing definitions */
#define TIMER2_57HZ 10
#define TIMER2_20HZ 49
#define TIMER2_5_8HZ 100
#define TIMER2_0_5HZ 1000
#define TIMER2_SECONDS_30 42840
#define TIMER2_SECONDS_10 14280
#define TIMER2_SECONDS_6 8566
#define TIMER2_SECONDS_5 7138
#define TIMER2_SECONDS_3 4283
#define TIMER2_SECONDS_2 2855
#define TIMER2_SECONDS_1 1428
#define TIMER2_MSECONDS_100 143
#define TIMER2_MSECONDS_200 286

#define BLINK_FAST 30
#define BLINK_SHORT 100
#define BLINK_LONG 500

/* TIMER0 tone frequencies */
#if F_CPU == 16000000UL
	#define DEFAULT_TONE_FREQUENCY 0x2F
	#define TONE_600Hz 0x1F
	#define TONE_500Hz 0x3F
	#define TONE_400Hz 0x4F
#else
	#define DEFAULT_TONE_FREQUENCY 0x18
	#define TONE_600Hz 0x0F
	#define TONE_500Hz 0x1F
	#define TONE_400Hz 0x27
#endif

#define ADC_REF_VOLTAGE_mV 1100L
#define VOLTAGE_MAX_MV 15000L
#define VOLTS(x) ((x * VOLTAGE_MAX_MV) / 1023L)

/* Input voltage thresholds */
#define VOLTAGE_THRESHOLD_LOW   1160
#define VOLTAGE_THRESHOLD_HIGH  1250

/******************************************************
 * UI Hardware-related definitions                     */

typedef enum
{
	FrequencyFormat,
	HourMinuteSecondFormat,
	HourMinuteSecondDateFormat
} TextFormat;

#define DISPLAY_WIDTH_STRING_SIZE (NUMBER_OF_LCD_COLS + 1)

typedef enum
{
	Minutes_Seconds,                        /* minutes up to 59 */
	Hours_Minutes_Seconds,                  /* hours up to 23 */
	Day_Month_Year_Hours_Minutes_Seconds,   /* Year up to 99 */
	Minutes_Seconds_Elapsed,                /* minutes up to 99 */
	Time_Format_Not_Specified
} TimeFormat;

#define NO_TIME_SPECIFIED (-1)

#define SecondsFromHours(hours) ((hours) * 3600)
#define SecondsFromMinutes(min) ((min) * 60)

typedef enum
{
	PATTERN_TEXT,
	STATION_ID
} TextIndex;

typedef enum
{
	POWER_UP,
	PUSHBUTTON,
	PROGRAMMATIC,
	NO_ACTION
} EventActionSource_t;

typedef enum
{
	START_NOTHING,
	START_EVENT_NOW,
	START_TRANSMISSIONS_NOW,
	START_EVENT_WITH_STARTFINISH_TIMES
} EventAction_t;

typedef enum
{
	INIT_NOT_SPECIFIED,
	INIT_EVENT_STARTING_NOW,
	INIT_TRANSMISSIONS_STARTING_NOW,
	INIT_EVENT_IN_PROGRESS_WITH_STARTFINISH_TIMES
} InitializeAction_t;

typedef enum
{
	AUDIO_SAMPLING,
	TEMPERATURE_SAMPLING,
	VOLTAGE_SAMPLING
} ADCChannel_t;

#endif  /* DEFS_H */
