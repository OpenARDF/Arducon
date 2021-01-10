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
 * rv3028.c
 *
 */

#include "defs.h"

#ifdef INCLUDE_RV3028_SUPPORT

   #include "rv3028.h"
   #include <util/twi.h>
   #include <stdio.h>
   #include "i2c.h"

   #define RTC_SECONDS                     0x00
   #define RTC_MINUTES                     0x01
   #define RTC_HOURS                       0x02
   #define RTC_WEEKDAY                     0x03
   #define RTC_DATE                        0x04
   #define RTC_MONTH                                       0x05
   #define RTC_YEAR                        0x06
   #define RTC_ALARM_MINUTES               0x07
   #define RTC_ALARM_HOURS                 0x08
   #define RTC_ALARM_DAY_DATE              0x09
   #define RTC_TIMER_VAL_0                 0x0A
   #define RTC_TIMER_VAL_1                 0x0B
   #define RTC_TIMER_STATUS_0              0x0C
   #define RTC_TIMER_STATUS_1              0x0D
   #define RTC_STATUS                      0x0E
   #define RTC_CONTROL_1                   0x0F
   #define RTC_CONTROL_2                   0x10
   #define RTC_GP_BITS                     0x11
   #define RTC_CLOCK_INT_MASK              0x12
   #define RTC_EVENT_CONTROL               0x13
   #define RTC_COUNT_TS                    0x14
   #define RTC_SECONDS_TS                  0x15
   #define RTC_MINUTES_TS                  0x16
   #define RTC_HOURS_TS                    0x17
   #define RTC_DATE_TS                     0x18
   #define RTC_MONTH_TS                    0x19
   #define RTC_YEAR_TS                     0x1A
   #define RTC_UNIX_TIME_0                 0x1B
   #define RTC_UNIX_TIME_1                 0x1C
   #define RTC_UNIX_TIME_2                 0x1D
   #define RTC_UNIX_TIME_3                 0x1E
   #define RTC_USER_RAM_1                  0x1F
   #define RTC_USER_RAM_2                  0x20
   #define RTC_PASSWORD_0                  0x21
   #define RTC_PASSWORD_1                  0x22
   #define RTC_PASSWORD_2                  0x23
   #define RTC_PASSWORD_3                  0x24
   #define RTC_EE_ADDRESS                  0x25
   #define RTC_EE_DATA                     0x26
   #define RTC_EE_COMMAND                  0x27
   #define RTC_ID                          0x28
   #define RTC_EEPROM_PW_ENABLE            0x30
   #define RTC_EEPROM_PW_0                 0x31
   #define RTC_EEPROM_PW_1                 0x32
   #define RTC_EEPROM_PW_2                 0x33
   #define RTC_EEPROM_PW_3                 0x34
   #define RTC_EEPROM_CLKOUT               0x35
   #define RTC_EEPROM_OFFSET               0x36
   #define RTC_EEPROM_BACKUP               0x37

	void rv3028_read_time(int32_t* val, char* buffer, TimeFormat format)
	{
		uint8_t data[7] = { 0, 0, 0, 0, 0, 0, 0 };
		uint8_t second10;
		uint8_t second;
		uint8_t minute10;
		uint8_t minute;
		uint8_t hour10;
		uint8_t hour;
		BOOL am_pm;
		BOOL twelvehour;

		if(!i2c_device_read(RV3028_BUS_BASE_ADDR, RTC_SECONDS, data, 7))
		{
			second10 = ((data[0] & 0xf0) >> 4);
			second = (data[0] & 0x0f);

			minute10 = ((data[1] & 0xf0) >> 4);
			minute = (data[1] & 0x0f);

			am_pm = ((data[2] >> 5) & 0x01);
			hour10 = ((data[2] >> 4) & 0x01);
			hour = (data[2] & 0x0f);

			twelvehour = ((data[2] >> 6) & 0x01);

			if(!twelvehour && am_pm)
			{
				hour10 = 2;
			}

			if(buffer)
			{
				switch(format)
				{
					case Minutes_Seconds:
					{
						sprintf(buffer, "%1d%1d:%1d%1d", minute10, minute, second10, second);
					}
					break;

					case Hours_Minutes_Seconds:
					{
						if(twelvehour)  /* 12-hour */
						{
							sprintf(buffer, "%1d%1d:%1d%1d:%1d%1d%s", hour10, hour, minute10, minute, second10, second, am_pm ? "AM" : "PM");
						}
						else            /* 24 hour */
						{
							sprintf(buffer, "%1d%1d:%1d%1d:%1d%1d", hour10, hour, minute10, minute, second10, second);
						}
					}
					break;

					default:    /* Day_Month_Year_Hours_Minutes_Seconds: */
					{
						sprintf(buffer, "%1d%1d:%1d%1d", minute10, minute, second10, second);
					}
					break;
				}
			}

			if(val)
			{
				*val = second + 10 * second10 + 60 * (int32_t)minute + 600 * (int32_t)minute10 +  3600 * (int32_t)hour + 36000 * (int32_t)hour10;
			}
		}
	}


	void rv3028_set_time(int32_t offsetSeconds)
	{
		int32_t timeVal;
		uint8_t data[7] = { 0, 0, 0, 0, 0, 0, 0 };
		int32_t temp;
		uint8_t hold;

		rv3028_read_time(&timeVal, NULL, Time_Format_Not_Specified);
		timeVal += offsetSeconds;

		data[0] = timeVal % 10;     /* seconds */
		temp = timeVal / 10;
		data[0] |= (temp % 6) << 4; /* 10s of seconds */
		temp /= 6;
		data[1] = temp % 10;        /* minutes */
		temp /= 10;
		data[1] |= (temp % 6) << 4; /* 10s of minutes */
		temp /= 6;
		hold = temp % 24;
		data[2] = hold % 10;        /* hours */
		hold /= 10;
		data[2] |= hold << 4;       /* 10s of hours */

		i2c_device_write(RV3028_BUS_BASE_ADDR, RTC_SECONDS, data, 3);

/*	temp /= 24; */

	}

/*
 *       1. Enter the correct password PW (PW = EEPW) to unlock write protection
 *       2. Disable automatic refresh by setting EERD = 1
 *       3. Edit Configuration settings in registers 35h to 37h (RAM)
 *       4. Update EEPROM (all Configuration RAM ? EEPROM) by setting EECMD = 00h followed by 11h
 *       5. Enable automatic refresh by setting EERD = 0
 *       6. Enter an incorrect password PW (PW ? EEPW) to lock the device
 */
	void rv3028_1s_sqw(BOOL enable)
	{
		if(enable)
		{
			uint8_t byte = 0x08; // EERD = 1
			i2c_device_write(RV3028_BUS_BASE_ADDR, RTC_CONTROL_1, &byte, 1);
			byte = 0xC5; // FD = 1 Hz
			i2c_device_write(RV3028_BUS_BASE_ADDR, RTC_EEPROM_CLKOUT, &byte, 1);
			byte = 0x00;
			i2c_device_write(RV3028_BUS_BASE_ADDR, RTC_EE_COMMAND, &byte, 1);
			byte = 0x11;
			i2c_device_write(RV3028_BUS_BASE_ADDR, RTC_EE_COMMAND, &byte, 1);
			byte = 0x00; // EERD = 0
			i2c_device_write(RV3028_BUS_BASE_ADDR, RTC_CONTROL_1, &byte, 1);
		}
		else
		{
//			uint8_t byte = 0x04;
//			i2c_device_write(RV3028_BUS_BASE_ADDR, RTC_EEPROM_CLKOUT, &byte, 1);
		}
	}


#endif  /* #ifdef INCLUDE_RV3028_SUPPORT */
