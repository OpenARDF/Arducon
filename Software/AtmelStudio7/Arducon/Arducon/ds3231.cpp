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
 * ds3231.c
 *
 */

#include "defs.h"

#if INCLUDE_DS3231_SUPPORT

   #include "ds3231.h"
   #include <util/twi.h>
   #include <stdio.h>
   #include "i2c.h"

   #define RTC_SECONDS                     0x00
   #define RTC_MINUTES                     0x01
   #define RTC_HOURS                       0x02
   #define RTC_DAY                         0x03
   #define RTC_DATE                        0x04
   #define RTC_MONTH_CENT                  0x05
   #define RTC_YEAR                        0x06
   #define RTC_ALARM1_SECONDS              0x07
   #define RTC_ALARM1_MINUTES              0x08
   #define RTC_ALARM1_HOUR                 0x09
   #define RTC_ALARM1_DAY_DATE             0x0A
   #define RTC_ALARM2_MINUTES              0x0B
   #define RTC_ALARM2_HOUR                 0x0C
   #define RTC_ALARM2_DAY_DATE             0x0D
   #define RTC_CONTROL                     0x0E
   #define RTC_CONTROL_STATUS              0x0F
   #define RTC_AGING                       0x10
   #define RTC_TEMP_MSB                    0x11
   #define RTC_TEMP_LSB                    0x12

time_t epoch_from_ltm(tm *ltm);

uint8_t bcd2dec(uint8_t val)
{
	uint8_t result = 10 * (val >> 4) + (val & 0x0F);
	return( result);
}

uint8_t dec2bcd(uint8_t val)
{
	uint8_t result = val % 10;
	result |= (val / 10) << 4;
	return (result);
}

uint8_t char2bcd(char c[])
{
	uint8_t result = (c[1] - '0') + ((c[0] - '0') << 4);
	return( result);
}


/* Returns the UNIX epoch value for the character string passed in datetime. If datetime is null then it returns
the UNIX epoch for the time held in the DS3231 RTC. If error is not null then it holds 1 if an error occurred */
	time_t RTC_String2Epoch(bool *error, char *datetime)
	{
		time_t epoch = 0;
		uint8_t data[7] = { 0, 0, 0, 0, 0, 0, 0 };

		struct tm ltm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
		int16_t year = 100;                 /* start at 100 years past 1900 */
		uint8_t month;
		uint8_t date;
		uint8_t hours;
		uint8_t minutes;
		uint8_t seconds;

		if(datetime)                            /* String format "YYMMDDhhmmss" */
		{
			data[0] = char2bcd(&datetime[10]);  /* seconds in BCD */
			data[1] = char2bcd(&datetime[8]);   /* minutes in BCD */
			data[2] = char2bcd(&datetime[6]);   /* hours in BCD */
			/* data[3] =  not used */
			data[4] = char2bcd(&datetime[4]);   /* day of month in BCD */
			data[5] = char2bcd(&datetime[2]);   /* month in BCD */
			data[6] = char2bcd(&datetime[0]);   /* 2-digit year in BCD */

			hours = bcd2dec(data[2]); /* Must be calculated here */

			year += (int16_t)bcd2dec(data[6]);
			ltm.tm_year = year;                         /* year since 1900 */

			year += 1900;                               /* adjust year to calendar year */

			month = bcd2dec(data[5]);
			ltm.tm_mon = month - 1;                     /* mon 0 to 11 */

			date = bcd2dec(data[4]);
			ltm.tm_mday = date;                         /* month day 1 to 31 */

			ltm.tm_yday = 0;
			for(uint8_t mon = 1; mon < month; mon++)    /* months from 1 to 11 (excludes partial month) */
			{
				ltm.tm_yday += month_length(year, mon);;
			}

			ltm.tm_yday += (ltm.tm_mday - 1);

			seconds = bcd2dec(data[0]);
			minutes = bcd2dec(data[1]);

			ltm.tm_hour = hours;
			ltm.tm_min = minutes;
			ltm.tm_sec = seconds;

			epoch = epoch_from_ltm(&ltm);
		}
		else
		{
			epoch = RTC_get_epoch();
		}

		if(error)
		{
			*error = (epoch == 0);
		}

		return(epoch);
	}

	time_t epoch_from_ltm(tm *ltm)
	{
		time_t epoch = ltm->tm_sec + ltm->tm_min * 60 + ltm->tm_hour * 3600L + ltm->tm_yday * 86400L +
		(ltm->tm_year - 70) * 31536000L + ((ltm->tm_year - 69) / 4) * 86400L -
		((ltm->tm_year - 1) / 100) * 86400L + ((ltm->tm_year + 299) / 400) * 86400L;

		return(epoch);
	}

	time_t RTC_get_epoch(void)
	{
		time_t epoch = 0;
		uint8_t data[7] = { 0, 0, 0, 0, 0, 0, 0 };

		if(!i2c_device_read(DS3231_BUS_BASE_ADDR, RTC_SECONDS, data, 7))
		{
			struct tm ltm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			int16_t year = 100;                 /* start at 100 years past 1900 */
			uint8_t month;
			uint8_t date;
			uint8_t hours;
			uint8_t minutes;
			uint8_t seconds;

			uint8_t hour10;
			uint8_t hour;
			BOOL am_pm;
			BOOL twelvehour;

			am_pm = ((data[2] >> 5) & 0x01);
			hour10 = ((data[2] >> 4) & 0x01);
			hour = (data[2] & 0x0f);

			twelvehour = ((data[2] >> 6) & 0x01);

			if(!twelvehour && am_pm)
			{
				hour10 = 2;
			}

			hours = 10 * hour10 + hour;

			year += (int16_t)bcd2dec(data[6]);
			ltm.tm_year = year;                         /* year since 1900 */

			year += 1900;                               /* adjust year to calendar year */

			month = bcd2dec(data[5]);
			ltm.tm_mon = month - 1;                     /* mon 0 to 11 */

			date = bcd2dec(data[4]);
			ltm.tm_mday = date;                         /* month day 1 to 31 */

			ltm.tm_yday = 0;
			for(uint8_t mon = 1; mon < month; mon++)    /* months from 1 to 11 (excludes partial month) */
			{
				ltm.tm_yday += month_length(year, mon);;
			}

			ltm.tm_yday += (ltm.tm_mday - 1);

			seconds = bcd2dec(data[0]);
			minutes = bcd2dec(data[1]);

			ltm.tm_hour = hours;
			ltm.tm_min = minutes;
			ltm.tm_sec = seconds;

			epoch = epoch_from_ltm(&ltm);
		}

		return( epoch);
	}


	BOOL RTC_set_datetime(char *datetime)
	{
		BOOL failure = TRUE;
		uint8_t data[7] = { 0, 0, 0, 0, 0, 0, 0 };

		if(datetime)                            /* String format "YYMMDDhhmmss" */
		{
			data[0] = char2bcd(&datetime[10]);  /* seconds in BCD */
			data[1] = char2bcd(&datetime[8]);   /* minutes in BCD */
			data[2] = char2bcd(&datetime[6]);   /* hours in BCD 24-hour format */
			/* data[3] =  not used */
			data[4] = char2bcd(&datetime[4]);   /* day of month in BCD */
			data[5] = char2bcd(&datetime[2]);   /* month in BCD */
			data[6] = char2bcd(&datetime[0]);   /* 2-digit year in BCD */

			failure = i2c_device_write(DS3231_BUS_BASE_ADDR, RTC_SECONDS, data, 7);
		}

		return(failure);
	}


	BOOL RTC_1s_sqw(BOOL enable)
	{
		BOOL fail;

		if(enable)
		{
			uint8_t byte = 0x00;
			fail = i2c_device_write(DS3231_BUS_BASE_ADDR, RTC_CONTROL, &byte, 1);
		}
		else
		{
			uint8_t byte = 0x04;
			fail = i2c_device_write(DS3231_BUS_BASE_ADDR, RTC_CONTROL, &byte, 1);
		}

		return(fail);
	}

#ifdef SUPPORT_DS3231_AGING
	void ds3231_set_aging(int8_t* data)
	{
		i2c_device_write(DS3231_BUS_BASE_ADDR, RTC_AGING, (uint8_t*)data, 1);
	}

	int8_t ds3231_get_aging()
	{
		int8_t data[1];
		i2c_device_read(DS3231_BUS_BASE_ADDR, RTC_AGING, (uint8_t*)data, 1);
		return(data[0]);
	}
#endif // SUPPORT_DS3231_AGING

/* This simple synchronization approach works for all times except 12 midnight. If synchronization
results in the advancement to the next day, then one day would be lost. Instead of introducing that
error, this function merely fails to synchronize at midnight. */
BOOL ds3231_sync2nearestMinute()
{
	BOOL err = FALSE;
	uint8_t data[7] = { 0, 0, 0 };

	if(!i2c_device_read(DS3231_BUS_BASE_ADDR, RTC_SECONDS, data, 3))
	{
		uint8_t hours;
		uint8_t minutes;
		uint8_t seconds;

		uint8_t hour10;
		uint8_t hour;
		BOOL am_pm;
		BOOL twelvehour;

		seconds = bcd2dec(data[0]);
		minutes = bcd2dec(data[1]);
		am_pm = ((data[2] >> 5) & 0x01);
		hour10 = ((data[2] >> 4) & 0x01);
		hour = (data[2] & 0x0f);

		twelvehour = ((data[2] >> 6) & 0x01);

		if(!twelvehour && am_pm)
		{
			hour10 = 2;
		}

		hours = 10 * hour10 + hour;

		if(seconds > 30)
		{
			minutes++;

			if(minutes > 59)
			{
				minutes = 0;
				hours++;

				if(hours > 23) /* Don't attempt to synchronize at midnight */
				{
					err = TRUE;
				}
			}
		}

		if(!err)
		{
			data[0] = 0; /* seconds = 00 */
			data[1] = dec2bcd(minutes);
			data[2] = 0;

			if(twelvehour)
			{
				data[2] |= 0x40; /* set  12-hour bit */

				if(hours >= 12)
				{
					data[2] |= 0x20; /* set pm bit */
				}

				if(hours >= 10)
				{
					data[2] |= 0x10;
				}
			}
			else
			{
				if(hours >= 20)
				{
					data[2] |= 0x20; /* set 20 bit */
				}
				else if(hours >= 10)
				{
					data[2] |= 0x10; /* set 10 bit */
				}
			}

			data[2] |= hours % 10;
			err = i2c_device_write(DS3231_BUS_BASE_ADDR, RTC_SECONDS, data, 3);
		}
	}

	return err;
}

/**
 *   Converts an epoch (seconds since 1900)  into a string with format "yymmddhhmmss"
 */
// #define THIRTY_YEARS 946080000
// char* convertEpochToTimeString(unsigned long epoch, char* timeString)
//  {
//    struct tm  ts;
//
//    if(!timeString) return(timeString);
//
//    if (epoch < MINIMUM_EPOCH)
//    {
//         timeString[0] = '\0';
// 		return timeString;
//    }
//
//    time_t e = (time_t)epoch - THIRTY_YEARS;
//    // Format time, "yymmddhhmmss"
//    ts = *localtime(&e);
//    strftime(timeString, sizeof(timeString), "%y%m%d%H%M%S", &ts);
//
//    return timeString;
//  }


#endif  /* #ifdef INCLUDE_DS3231_SUPPORT */

