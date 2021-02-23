/**********************************************************************************************
 * Copyright (c) 2021 Digital Confections LLC
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
#if COMPILE_FOR_ATMELSTUDIO7
#include <string.h>
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

#ifdef INCLUDE_RV3028_SUPPORT

   #include "rv3028.h"
   #include <util/twi.h>
   #include <stdio.h>
   #include "i2c.h"
   #include "linkbus.h"

   #define RTC_SECONDS                     0x00
   #define RTC_MINUTES                     0x01
   #define RTC_HOURS                       0x02
   #define RTC_WEEKDAY                     0x03
   #define RTC_DATE                        0x04
   #define RTC_MONTH                       0x05
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

   #define EEbusy 7
   #define EERD   3
   #define EE_COMMAND_WRITE_ONE_BYTE      0x21
   #define EE_COMMAND_READ_ONE_BYTE       0x22
   #define EE_COMMAND_WRITE_ALL_EEPROM    0x11
   #define EE_COMMAND_READ_ALL_EEPROM     0x12

#if INIT_EEPROM_ONLY
	extern char g_tempStr[];
#endif // INIT_EEPROM_ONLY

	BOOL waitForEEPROMReady(void);
	uint8_t writeOneEEPROMByte(uint8_t rtc_ee_addr, uint8_t mask, uint8_t val);
	uint8_t readOneEEPROMByte(uint8_t rtc_ee_addr, uint8_t* val);
	void refreshRAMfromEEPROM(void);

	uint8_t bcd2dec(uint8_t val)
	{
		uint8_t result = 10 * (val >> 4) + (val & 0x0F);

		return( result);
	}

	uint8_t char2bcd(char c[])
	{
		uint8_t result = (c[1] - '0') + ((c[0] - '0') << 4);

		return( result);
	}

/*
 *  Handles setting a single RTC EEPROM register. The first argument holds the RTC register address.
 *  The second argument holds a mask field indicating which bits are being changed, where
 *  changing bits are indicated by a "1" bit in the mask. The third argument holds the bit values
 *  to be written.
 */
	uint8_t writeOneEEPROMByte(uint8_t rtc_ee_addr, uint8_t mask, uint8_t val)
	{
		uint8_t temp = 0;
		uint8_t hold;

		if(waitForEEPROMReady())
		{
			return( 1);
		}

		temp = (1 << EERD); /* EERD = 1 Disable automatic refresh - setting other bits to zero */
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_CONTROL_1, &temp, 1))
		{
			return( 1);
		}

		/* Save existing value stored in RAM - Note: we are assuming that RAM still holds unchanged EEPROM value */
		i2c_device_read(RV3028_I2C_SLAVE_ADDR, rtc_ee_addr, &hold, 1);
		hold &= ~mask;  /* Set mask 1-bits to zero */
		val &= mask;    /* Ensure only mask 1-bits get applied */
		val = hold | val;

		/* Set up single-byte write to EEPROM */

		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_ADDRESS, &rtc_ee_addr, 1))
		{
			return( 1);
		}

		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_DATA, &val, 1))
		{
			return( 1);
		}

		temp = 0x00;
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_COMMAND, &temp, 1))
		{
			return( 1);
		}

		if(waitForEEPROMReady())
		{
			return( 1);
		}

		temp = EE_COMMAND_WRITE_ONE_BYTE;
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_COMMAND, &temp, 1))
		{
			return( 1);
		}

		if(waitForEEPROMReady())
		{
			return( 1);
		}

		temp = 0;   /* EERD = 0 Re-enable automatic refresh - setting other bits to zero */
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_CONTROL_1, &temp, 1))
		{
			return( 1);
		}

		return( 0);
	}

	uint8_t readOneEEPROMByte(uint8_t rtc_ee_addr, uint8_t* val)
	{
		uint8_t temp = 0;

		if(!val)
		{
			return(1);
		}

		if(waitForEEPROMReady())
		{
			return( 1);
		}

		temp = (1 << EERD); /* EERD = 1 Disable automatic refresh - setting other bits to zero */
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_CONTROL_1, &temp, 1))
		{
			return( 1);
		}

		/* Set up single-byte read from EEPROM */

		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_ADDRESS, &rtc_ee_addr, 1))
		{
			return( 1);
		}

		temp = 0x00;
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_COMMAND, &temp, 1))
		{
			return( 1);
		}

		if(waitForEEPROMReady())
		{
			return( 1);
		}

		temp = EE_COMMAND_READ_ONE_BYTE;
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_COMMAND, &temp, 1))
		{
			return( 1);
		}

		if(waitForEEPROMReady())
		{
			return( 1);
		}

		temp = 0;   /* EERD = 0 Re-enable automatic refresh - setting other bits to zero */
		if(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_CONTROL_1, &temp, 1))
		{
			return( 1);
		}

		/* Read the value from RAM */
		i2c_device_read(RV3028_I2C_SLAVE_ADDR, rtc_ee_addr, val, 1);

		return(0);
	}

/* Note: this is not thread safe */
	time_t rv3028_get_epoch(bool *result, char *datetime)
	{
		uint8_t data[7] = { 0, 0, 0, 0, 0, 0, 0 };
		BOOL res = 0;
		time_t epoch = 0;

		if(datetime)                            /* String format "YYMMDDhhmmss" */
		{
			data[0] = char2bcd(&datetime[10]);  /* seconds in BCD */
			data[1] = char2bcd(&datetime[8]);   /* minutes in BCD */
			data[2] = char2bcd(&datetime[6]);   /* hours in BCD */
			/* data[3] =  not used */
			data[4] = char2bcd(&datetime[4]);   /* day of month in BCD */
			data[5] = char2bcd(&datetime[2]);   /* month in BCD */
			data[6] = char2bcd(&datetime[0]);   /* 2-digit year in BCD */

			struct tm ltm = { 0, 0, 0, 0, 0, 0, 0, 0, 0 };
			int16_t year = 100; /* start at 100 years past 1900 */
			uint8_t month;
			uint8_t date;
			uint8_t hours;
			uint8_t minutes;
			uint8_t seconds;

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
			hours = bcd2dec(data[2]);

			ltm.tm_hour = hours;
			ltm.tm_min = minutes;
			ltm.tm_sec = seconds;

			epoch = ltm.tm_sec + ltm.tm_min * 60 + ltm.tm_hour * 3600L + ltm.tm_yday * 86400L +
			(ltm.tm_year - 70) * 31536000L + ((ltm.tm_year - 69) / 4) * 86400L -
			((ltm.tm_year - 1) / 100) * 86400L + ((ltm.tm_year + 299) / 400) * 86400L;
		}
		else
		{
//			res = i2c_device_read(RV3028_I2C_SLAVE_ADDR, RTC_SECONDS, data, 7);
			epoch = rv3028_get_epoch();
			res = epoch == 0;
		}

		if(result)
		{
			*result = res ? 1 : 0;
		}

		return(epoch);
	}

#ifdef DATE_STRING_SUPPORT_ENABLED
		void rv3028_read_date_time(int32_t* val, char* buffer, TimeFormat format)
		{
			uint8_t data[7] = { 0, 0, 0, 0, 0, 0, 0 };
			uint8_t month;
			uint8_t date;
			uint16_t year = 2000;
			uint8_t seconds;
			uint8_t minutes;
			uint8_t hours;

			if(!i2c_device_read(RV3028_I2C_SLAVE_ADDR, RTC_SECONDS, data, 7))
			{
				seconds = bcd2dec(data[0]);
				minutes = bcd2dec(data[1]);
				hours = bcd2dec(data[2]);

				if(buffer)
				{
					switch(format)
					{
						case Minutes_Seconds:
						{
							sprintf(buffer, "%1d:%1d", minutes, seconds);
						}
						break;

						case Hours_Minutes_Seconds:
						{
							sprintf(buffer, "%1d:%1d:%1d", hours, minutes, seconds);

						}
						break;

						default:    /* Day_Month_Year_Hours_Minutes_Seconds: */
						{
							date = bcd2dec(data[4]);
							month = bcd2dec(data[5]);
							year += bcd2dec(data[6]);

							sprintf(buffer, "%4d-%02d-%02dT%1d:%1d:%1d", year, month, date, hours, minutes, seconds);
						}
						break;
					}
				}

				if(val)
				{
					*val = seconds + 60 * (int32_t)minutes +  3600 * (int32_t)hours;
				}
			}
		}
#endif  /* DATE_STRING_SUPPORT_ENABLED */

	BOOL rv3028_set_epoch(time_t epoch)
	{
		uint8_t data[5] = { 0, 0, 0, 0 };
		data[3] = (uint8_t)((epoch & 0xff000000) >> 24);
		data[2] = (uint8_t)((epoch & 0x00ff0000) >> 16);
		data[1] = (uint8_t)((epoch & 0x0000ff00) >> 8);
		data[0] = (uint8_t)((epoch & 0x000000ff));
		return(i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_UNIX_TIME_0, data, 4));
	}

	time_t rv3028_get_epoch(void)
	{
		time_t epoch = 0;
		uint8_t data[4] = { 0, 0, 0, 0 };
		if(!i2c_device_read(RV3028_I2C_SLAVE_ADDR, RTC_UNIX_TIME_0, data, 4))
		{
			epoch |= data[0];
			epoch |= ((time_t)data[1]) << 8;
			epoch |= ((time_t)data[2]) << 16;
			epoch |= ((time_t)data[3]) << 24;
		}

		return epoch;
	}


// 	BOOL rv3028_set_date_time(char * dateString)                                                                                         /* String format "YYMMDDhhmmss" */
// 	{
// 		BOOL fail = TRUE;
// 		uint8_t data[7] = { 0, 0, 0, 1, 0, 0, 0 };
// 		int length = strlen((const char*)dateString);
// 
// 		if(length >= 12)
// 		{
// 			data[0] = dateString[11] - '0';             /* seconds */
// 			data[0] += ((dateString[10] - '0') << 4);   /* 10s of seconds */
// 			data[1] = dateString[9] - '0';              /* minutes */
// 			data[1] += ((dateString[8] - '0') << 4);    /* 10s of minutes */
// 			data[2] = dateString[7] - '0';              /* hours */
// 			data[2] += ((dateString[6] - '0') << 4);    /* 10s of hours */
// 
// 			/*data[3] = Skip day of week */
// 
// 			data[4] = dateString[5] - '0';              /* day of month digit 1 */
// 			data[4] += ((dateString[4] - '0') << 4);    /* day of month */
// 			data[5] = dateString[3] - '0';              /* month digit 1 */
// 			data[5] += ((dateString[2] - '0') << 4);    /* month */
// 			data[6] = dateString[1] - '0';              /* year digit 1 */
// 			data[6] += ((dateString[0] - '0') << 4);    /* year - two digits */
// 
// 			fail = i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_SECONDS, data, 7);
// 		}
// 
// 		return( fail);
// 	}

			
	void refreshRAMfromEEPROM(void)
	{
		uint8_t status = FALSE;
		uint8_t temp = 0;
		status = i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_COMMAND, &temp, 1);
		waitForEEPROMReady();
		temp = 0x12;    /* Refresh RAM from EEPROM */
		status |= i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EE_COMMAND, &temp, 1);
		waitForEEPROMReady();
	}

	int16_t rv3028_get_offset_RAM()
	{
		uint8_t data[2];
		int16_t result;

		refreshRAMfromEEPROM(); /* Ensure we never return an altered RAM value */
		i2c_device_read(RV3028_I2C_SLAVE_ADDR, RTC_EEPROM_OFFSET, (uint8_t*)data, 2);
		result = data[0] << 1;
		if(data[1] & 0x80)
		{
			result++;
		}
		return(result);
	}

	void rv3028_set_offset_RAM(uint16_t val)
	{
		uint8_t data[2] = { 0, 0x10 };

		data[0] = val >> 1;
		if(val & 0x01)
		{
			data[1] = 0x90;
		}

		i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EEPROM_OFFSET, (uint8_t*)data, 2);
	}

#ifdef INIT_EEPROM_ONLY
/*
 *  Instructions for setting EEPROM values:
 *       1. Enter the correct password PW (PW = EEPW) to unlock write protection
 *       2. Disable automatic refresh by setting EERD = 1
 *       3. Edit Configuration settings in registers 35h to 37h (RAM)
 *       4. Update EEPROM (all Configuration RAM ? EEPROM) by setting EECMD = 00h followed by 11h
 *       5. Enable automatic refresh by setting EERD = 0
 *       6. Enter an incorrect password PW (PW ? EEPW) to lock the device
 *
 #define RTC_EE_ADDRESS                  0x25
 #define RTC_EE_DATA                     0x26
 #define RTC_EE_COMMAND                  0x27
 #define RTC_STATUS                      0x0E
 */
		uint8_t rv3028_1s_sqw(void)
		{
			uint8_t status = FALSE;
			uint8_t temp = 0;

#if INIT_EEPROM_ONLY
/* Ensure that no existing RTC RAM mirror settings differ from what is stored in EEPROM
by reading all EEPROM into the RAM mirror now */
			refreshRAMfromEEPROM();

			if(waitForEEPROMReady())
			{
				return( 1);
			}

			temp = 0xC5;    /* Enable 1Hz Output */
			uint8_t mask = 0xFF;    /* 11111111 */
			if(writeOneEEPROMByte(RTC_EEPROM_CLKOUT, mask, temp))
			{
				return( 1);
			}

			temp = 0x04;    /* Enable direct switching mode */
			mask = 0x06;    /* 00001100 */
			if(writeOneEEPROMByte(RTC_EEPROM_BACKUP, mask, temp))
			{
				return( 1);
			}
#endif // INIT_EEPROM_ONLY

			refreshRAMfromEEPROM();

/* Debug only */
#if INIT_EEPROM_ONLY
			uint8_t temp_byte;
			uint8_t hold_byte;
			for(temp_byte = 0x30; temp_byte <= 0x37; temp_byte++)
			{
				i2c_device_read(RV3028_I2C_SLAVE_ADDR, temp_byte, &hold_byte, 1);
				sprintf(g_tempStr, "\n0x%02X = 0x%02X", temp_byte, hold_byte);
				lb_send_string(g_tempStr, TRUE);
			}

			lb_send_NewLine();
			lb_send_NewPrompt();

			g_tempStr[0] = '\0';
#endif // INIT_EEPROM_ONLY
/* End debugging */

			if(!status)
			{
				temp = i2c_device_read(RV3028_I2C_SLAVE_ADDR, RTC_STATUS, &status, 1);
				status &= 0x6F; /* Mask off relevant flags: 01101111 */
			}
			else
			{
				status = 1 << RTC_STATUS_I2C_ERROR;
			}

			temp = 0x00;    /* Clear status flags */
			i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_STATUS, &temp, 1);

			return(status);
		}

		BOOL waitForEEPROMReady(void)
		{
			uint8_t status = 1;
			BOOL failure = FALSE;

			while(status && !failure)
			{
				failure |= i2c_device_read(RV3028_I2C_SLAVE_ADDR, RTC_STATUS, &status, 1);
				if(!failure)
				{
					status &= (1 << EEbusy);
				}
			}

			return(status);
		}


#endif  /* INIT_EEPROM_ONLY */

	void rv3028_32kHz_sqw(void)
	{
		/* Just set RAM value */
		uint8_t byte = 0xC0;    /* FD = 32.768 kHz */

		i2c_device_write(RV3028_I2C_SLAVE_ADDR, RTC_EEPROM_CLKOUT, &byte, 1);
	}

#endif  /* #ifdef INCLUDE_RV3028_SUPPORT */
