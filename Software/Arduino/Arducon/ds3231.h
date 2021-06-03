/**********************************************************************************************
 * Copyright © 2017 Digital Confections LLC
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
 * ds3231.h
 *
 * DS3231: Extremely Accurate I2C-Integrated RTC/TCXO/Crystal
 * http://datasheets.maximintegrated.com/en/ds/DS3231-DS3231S.pdf
 * The DS3231 is a low-cost, extremely accurate I2C real-time clock (RTC) with an integrated
 * temperature-compensated crystal oscillator (TCXO) and crystal. The device incorporates a
 * battery input, and maintains accurate timekeeping when main power to the device is interrupted.
 * The integration of the crystal resonator enhances the long-term accuracy of the device as well
 * as reduces the piece-part count in a manufacturing line. The DS3231 is available in commercial
 * and industrial temperature ranges, and is offered in a 16-pin, 300-mil SO package.
 *
 */

#include "defs.h"

#ifndef DS3231_H_
#define DS3231_H_

#if INCLUDE_DS3231_SUPPORT

#include "defs.h"
#include "i2c.h"
#include <time.h>

#define DS3231_BUS_BASE_ADDR 0xD0   /* corresponds to slave address = 0b1101000x */

/**
 *  Returns the UNIX epoch value for the character string passed in datetime or from the DS3231. 
 *  *datetime - If datetime is null then it returns the UNIX epoch for the time held in the DS3231 RTC. 
 *  *error - If error is not null then it holds 1 if an error occurred 
 */
		time_t RTC_get_epoch(bool *error, char *datetime);
		time_t RTC_get_epoch(void);
		
		BOOL RTC_set_datetime(char *datetime);

/**
 *  Turn on/off 1-second square wave on the INT/SQW pin.
 */
		BOOL RTC_1s_sqw(BOOL enable);
		
/** 
 *
 */
	void ds3231_set_aging(int8_t* data);
	
/** 
 *
 */
	int8_t ds3231_get_aging(void);


#endif  /* #ifdef INCLUDE_DS3231_SUPPORT */

#endif  /* DS3231_H_ */