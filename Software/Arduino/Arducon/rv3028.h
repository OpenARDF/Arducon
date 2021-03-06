/**********************************************************************************************
 * Copyright (c) 2017 Digital Confections LLC
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in the
 * Software without restriction, including without limitation the rights to use, copy,
 * modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the
 * following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
 * PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE
 * FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR
 * OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 **********************************************************************************************
 * rv3028.h
 *
 * RV3028: Extremely Accurate I2C-Integrated RTC/TCXO/Crystal
 * The RV3028 is a low-cost, extremely accurate I2C real-time clock (RTC) with an integrated
 * crystal oscillator and crystal. The device incorporates a battery input, and maintains
 * accurate timekeeping when main power to the device is interrupted.
 *
 */

#ifndef RV3028_H_
#define RV3028_H_

#include "defs.h"
#include "i2c.h"
#include <time.h>

#define RTC_STATUS_NORMAL 0x00
#define RTC_STATUS_I2C_ERROR 7
#define RTC_STATUS_BACKUP_SWITCHOVER_OCCURRED 5
#define RTC_STATUS_EVF_OCCURRED 1
#define RTC_STATUS_CLOCK_CORRUPT 0

#if INCLUDE_RV3028_SUPPORT

#define RV3028_I2C_SLAVE_ADDR 0xA4  /* corresponds to slave address = 0b10100100x */

/**
 *  Reads hours, minutes and seconds from the RV3028 and returns them in the memory location pointed to by
 *  the first two arguments.
 *  *val - if non-NULL will receive a dword value of the time in seconds since midnight.
 *  *char - if non-NULL will receive a string representation of the time
 *  format - specifies the string format to be used for the string time representation
 */
#ifdef DATE_STRING_SUPPORT_ENABLED
			void rv3028_read_date_time(int32_t* val, char* buffer, TimeFormat format);
#endif  /* DATE_STRING_SUPPORT_ENABLED */

/**
 *  Reads time from the RV3028 and returns the epoch
 */
		time_t RTC_get_epoch(bool *result, char *datetime);


/**
 *  Set year, month, date, day, hours, minutes and seconds of the rv3028 to the time passed in the argument.
 * dateString has the format 2018-03-23T18:00:00
 * ClockSetting setting = clock or alarm to be set
 */
//		BOOL rv3028_set_date_time(char * dateString);

/**
 * Set or Get the UNIX epoch registers
 */
		BOOL RTC_set_epoch(time_t epoch);
		time_t RTC_get_epoch(void);

/**
 *  Turn on 1-second square wave on the INT/SQW pin.
 */
		BOOL RTC_1s_sqw(BOOL onOff);

/**
 *  Read current setting of temperature offset RAM
 */
		int16_t rv3028_get_offset_RAM(void);

/**
 *  Set RAM value of temperature offset
 */
		void rv3028_set_offset_RAM(uint16_t val);

/**
 *  Returns current state of 1-Hz clock pin
 */
		BOOL rv3028_1Hz_enabled(void);


#endif  /* #ifdef INCLUDE_RV3028_SUPPORT */

#endif  /* RV3028_H_ */
