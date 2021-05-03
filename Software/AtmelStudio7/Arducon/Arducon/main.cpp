/*
 *   MIT License
 *
 *   Copyright (c) 2021 DigitalConfections
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a copy
 *   of this software and associated documentation files (the "Software"), to deal
 *   in the Software without restriction, including without limitation the rights
 *   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *   copies of the Software, and to permit persons to whom the Software is
 *   furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in all
 *   copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *   SOFTWARE.
 */

#include "defs.h"
#include "linkbus.h"
#include "morse.h"
#include "rv3028.h"
#include "EepromManager.h"

#if !INIT_EEPROM_ONLY
#include "Goertzel.h"
#include "f1975.h"
#endif  /* INIT_EEPROM_ONLY */

#ifdef ATMEL_STUDIO_7
#include <avr/io.h>
#include <avr/eeprom.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ardooweeno.h"
#endif  /* ATMEL_STUDIO_7 */

#include <avr/wdt.h>

EepromManager ee_mgr;

/*#define SAMPLE_RATE 9630 */
#define SAMPLE_RATE 19260
/*#define SAMPLE_RATE 38520
 #define SAMPLE_RATE 77040
 #define SAMPLE_RATE 154080 */

volatile int32_t g_seconds_since_sync = 0;  /* Total number of seconds since an active event was synchronized (started) */
volatile int32_t g_seconds_since_powerup = 0;
volatile Fox_t g_fox          = BEACON;     /* Sets Fox number not set by ISR. Set in startup and checked in main. */
volatile int g_active           = 0;        /* Disable active. set and clear in ISR. Checked in main. */

volatile BOOL g_on_the_air       = 0;       /* Used to indicate and control transmissions */
volatile int g_code_throttle    = 0;        /* Adjusts Morse code speed */

volatile int g_dtmf_error_countdown = 0;

#if !INIT_EEPROM_ONLY
	const char g_morsePatterns[][5] = { "MO ", "MOE ", "MOI ", "MOS ", "MOH ", "MO5 ", "5", "S", "ME", "MI", "MS", "MH", "M5", "OE", "OI", "OS", "OH", "O5" };
#endif  /* !INIT_EEPROM_ONLY */

volatile BOOL g_callsign_sent = TRUE;

volatile BOOL g_blinky_time = FALSE;
volatile BOOL g_LED_enunciating = FALSE;

volatile time_t g_current_epoch = 0;
volatile time_t g_event_start_epoch = 0;
volatile time_t g_event_finish_epoch = 0;
volatile int8_t g_utc_offset = 0;
volatile BOOL g_ptt_periodic_reset_enabled;

volatile BOOL g_sendAMmodulation = FALSE;
volatile AM_Tone_Freq_t g_AM_audio_frequency;
volatile BOOL g_AM_enabled = TRUE;
volatile BOOL g_sendAMmodulationConstantly = FALSE;
uint8_t g_dataModulation[SIZE_OF_DATA_MODULATION];

volatile BOOL g_transmissions_disabled = TRUE;
volatile int g_on_air_interval_seconds = 0;
volatile int g_cycle_period_seconds = 0;
volatile int g_fox_counter = 1;
volatile int g_number_of_foxes = 0;
/*volatile BOOL g_fox_transition = FALSE; */
volatile int g_fox_id_offset = 0;   /* Used to handle fast and slow foxes in Sprint without unnecessary software complexity */
volatile int g_id_interval_seconds = 0;
volatile InitializeAction_t g_initialize_fox_transmissions = INIT_NOT_SPECIFIED;
volatile BOOL g_use_ptt_periodic_reset = FALSE;

volatile BOOL g_audio_tone_state = FALSE;
volatile int16_t g_sync_pin_timer = 0;
volatile ButtonStability_t g_sync_pin_stable = UNSTABLE;

volatile BOOL g_dtmf_detected = FALSE;
volatile uint8_t g_unlockCode[MAX_UNLOCK_CODE_LENGTH + 1];
volatile int8_t g_temperature = 0;
volatile uint16_t g_voltage = 0;
volatile ConfigurationState_t g_config_error = NULL_CONFIG;
volatile BOOL g_use_rtc_for_startstop = FALSE;

volatile BOOL g_DTMF_unlocked = TRUE;
volatile BOOL g_reset_button_held = FALSE;
volatile BOOL g_perform_EEPROM_reset = FALSE;

volatile uint16_t g_LED_timeout_countdown = LED_TIMEOUT_SECONDS;
static volatile BOOL g_enableHardwareWDResets = FALSE;
extern BOOL g_i2c_not_timed_out;

#ifndef ATMEL_STUDIO_7
	Fox_t operator++(volatile Fox_t &orig, int)
	{
		orig = static_cast < Fox_t > (orig + 1);    /* static_cast required because enum + int -> int */
		if(orig > INVALID_FOX)
		{
			orig = INVALID_FOX;
		}
		return( orig);
	}

	Fox_t operator +=(volatile Fox_t &a, int b)
	{
		a = static_cast < Fox_t > (a + b);  /* static_cast required because enum + int -> int */
		return( a);
	}
#endif  /* ATMEL_STUDIO_7 */

char g_messages_text[2][MAX_PATTERN_TEXT_LENGTH + 1] = { "\0", "\0" };
volatile uint8_t g_id_codespeed = EEPROM_ID_CODE_SPEED_DEFAULT;
volatile uint8_t g_pattern_codespeed = EEPROM_PATTERN_CODE_SPEED_DEFAULT;
volatile uint16_t g_time_needed_for_ID = 0;
volatile int16_t g_atmega_temp_calibration = EEPROM_TEMP_CALIBRATION_DEFAULT;
volatile uint8_t g_temperature_check_countdown = 0;
volatile uint8_t g_voltage_check_countdown = 0;
volatile int16_t g_rv3028_offset = EEPROM_RV3028_OFFSET_DEFAULT;

#if !INIT_EEPROM_ONLY
	const int N = Goertzel_N;
	const float threshold = 500000. * (Goertzel_N / 100);
	const float sampling_freq = SAMPLE_RATE;
	const float x_frequencies[4] = { 1209., 1336., 1477., 1633. };
	const float y_frequencies[4] = { 697., 770., 852., 941. };
#ifdef DEBUG_DTMF
		const float mid_frequencies[7] = { 734., 811., 897., 1075., 1273., 1407., 1555. };
#endif  /* DEBUG_DTMF */
#endif  /* !INIT_EEPROM_ONLY */

char g_lastKey = '\0';
volatile unsigned long g_tick_count = 0;
volatile unsigned int g_tone_duration_ticks = 0;
volatile unsigned int g_LED_Enunciation_holdoff = 0;
volatile unsigned int g_DTMF_sentence_in_progress_ticks = 0;

#if !INIT_EEPROM_ONLY
	Goertzel g_goertzel(N, sampling_freq);
#endif  /* INIT_EEPROM_ONLY */

char g_tempStr[TEMP_STRING_LENGTH] = { '\0' };

/*
 *  Local function prototypes
 */
void handleLinkBusMsgs(void);
void sendMorseTone(BOOL onOff);
void playStartingTone(uint8_t toneFreq);
void setupForFox(Fox_t* fox, EventAction_t action);

BOOL setAMToneFrequency(AM_Tone_Freq_t value);

uint16_t readADC();
float getTemp(void);
uint16_t getVoltage(void);
BOOL only_digits(char *s);
ConfigurationState_t clockConfigurationCheck(void);
void stopEventNow(EventActionSource_t activationSource);
void startEventNow(EventActionSource_t buttonActivated);
void startEventUsingRTC(void);
void reportConfigErrors(void);
/*char* convertEpochToTimeString(unsigned long epoch); */
BOOL reportTimeTill(time_t from, time_t until, const char* prefix, const char* failMsg);
time_t validateTimeString(char* str, time_t* epicVar, int8_t offsetHours);
void wdt_init(WDReset resetType);
char value2Morse(char value);
DTMF_key_t value2DTMFKey(uint8_t value);

#if !INIT_EEPROM_ONLY
	BOOL processDTMFdetection(DTMF_key_t key);
	void setUpSampling(ADCChannel_t channel, BOOL enableSampling);
#endif  /* !INIT_EEPROM_ONLY */

#ifdef ATMEL_STUDIO_7
	void loop(void);
	int main(void)
#else
	void setup()
#endif  /* ATMEL_STUDIO_7 */
{
	pinMode(PIN_SYNC, INPUT_PULLUP);

	pinMode(PIN_LED, OUTPUT);                             /* This is the enunciator LED */
	digitalWrite(PIN_LED, OFF);

	pinMode(PIN_CW_KEY_LOGIC, OUTPUT);  /* This pin is used to control the KEY line to the transmitter only active on cycle. */
	digitalWrite(PIN_CW_KEY_LOGIC, OFF);

	pinMode(PIN_CW_TONE_LOGIC, OUTPUT);
	digitalWrite(PIN_CW_TONE_LOGIC, OFF);

	pinMode(PIN_PTT_LOGIC, OUTPUT);
	digitalWrite(PIN_PTT_LOGIC, OFF);

	pinMode(PIN_AUDIO_INPUT, INPUT);    /* Receiver Audio sampling */
	pinMode(PIN_BATTERY_LEVEL, INPUT);  /* Battery voltage level */

	pinMode(PIN_PWDN, OUTPUT);
	digitalWrite(PIN_PWDN, ON);

	pinMode(PIN_SCL, INPUT_PULLUP);
	pinMode(PIN_SDA, INPUT_PULLUP);

	linkbus_disable();

#if SUPPORT_ONLY_80M
		/*	Set unused port pins */
		pinMode(PIN_UNUSED_1, OUTPUT);
		digitalWrite(PIN_UNUSED_1, OFF);

		pinMode(PIN_UNUSED_2, OUTPUT);
		digitalWrite(PIN_UNUSED_2, OFF);

		pinMode(PIN_UNUSED_3, OUTPUT);
		digitalWrite(PIN_UNUSED_3, OFF);

		pinMode(PIN_UNUSED_4, OUTPUT);
		digitalWrite(PIN_UNUSED_4, OFF);

		pinMode(PIN_UNUSED_5, OUTPUT);
		digitalWrite(PIN_UNUSED_5, OFF);

		pinMode(PIN_UNUSED_6, OUTPUT);
		digitalWrite(PIN_UNUSED_6, OFF);

		pinMode(PIN_UNUSED_7, OUTPUT);
		digitalWrite(PIN_UNUSED_7, OFF);

		pinMode(PIN_UNUSED_8, OUTPUT);
		digitalWrite(PIN_UNUSED_8, OFF);
#else
		/*	Set unused port pins */
		pinMode(PIN_UNUSED_1, OUTPUT);
		digitalWrite(PIN_UNUSED_1, OFF);

		pinMode(PIN_UNUSED_2, OUTPUT);
		digitalWrite(PIN_UNUSED_2, OFF);

		pinMode(PIN_UNUSED_3, OUTPUT);
		digitalWrite(PIN_UNUSED_3, OFF);

		/*	Set attenuator control port pins */
		DDRB |= 0x3F;
		PORTB &= 0xC0;
#endif  /* !SUPPORT_ONLY_80M */

#if INIT_EEPROM_ONLY
		BOOL eepromErr = ee_mgr.initializeEEPROMVars();                                 /* Must happen after pins are configured due to I2C access */
#else
		i2c_init();
		BOOL eepromErr = ee_mgr.readNonVols();
		setUpSampling(AUDIO_SAMPLING, TRUE);
#endif

	cli();

	/*******************************************************************
	 *  TIMER2 is for periodic interrupts to drive Morse code generation
	 *  Reset control registers */
	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM21);                             /* set Clear Timer on Compare Match (CTC) mode with OCR2A setting the top */
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  /* 1024 Prescaler */

#if F_CPU == 16000000UL
		OCR2A = 0x0C;                                   /* set frequency to ~300 Hz (0x0c) */
#else
		OCR2A = 0x06;                                   /* set frequency to ~300 Hz (0x0c) */
#endif

	OCR2B = 0x00;
	/* Use system clock for Timer/Counter2 */
	ASSR &= ~(1 << AS2);
	/* Reset Timer/Counter2 Interrupt Mask Register */
	TIMSK2 = 0;
	TIMSK2 |= (1 << OCIE2B);    /* Output Compare Match B Interrupt Enable */

	/*******************************************************************
	 *  Timer 1 is used for controlling the attenuator for AM generation
	 *  set timer1 interrupt at 16 kHz */

	TCCR1A = 0;                 /* set entire TCCR1A register to 0 */
	TCCR1B = 0;                 /* same for TCCR1B */
	TCNT1 = 0;                  /* initialize counter value to 0 */
#if !SUPPORT_ONLY_80M
		/* turn on CTC mode */
		TCCR1B |= (1 << WGM12);
		/* Set CS10 bit for no prescaling */
		TCCR1B |= (1 << CS10);
#endif  /* !SUPPORT_ONLY_80M */

	/********************************************************************/
	/* Timer 0 is for FM audio tone generation and control
	 *  Note: Do not use millis() or DELAY() after TIMER0 has been reconfigured here! */
	TCCR0A = 0x00;
	TCCR0A |= (1 << WGM01); /* Set CTC mode */
	TCCR0B = 0x00;
	TCCR0B |= (1 << CS02);  /* Prescale 256 */
	OCR0A = DEFAULT_TONE_FREQUENCY;
	TIMSK0 = 0x00;

	/*******************************************************************
	 *  Sync button pin change interrupt */
	PCMSK2 = 0x00;
	PCMSK1 = 0x00;
	PCMSK1 = (1 << PCINT11);    /* Enable PCINT11 */
	PCICR = 0x00;
	PCICR = (1 << PCIE1);       /* Enable pin change interrupt 1 */

#if SUPPORT_ONLY_80M
		setAMToneFrequency(AM_DISABLED);
#else
		/*******************************************************************
		*  AM attenuator initialization (also affects FM tone enable)     */
		setAMToneFrequency(g_AM_audio_frequency);   /* For attenuator tone output */
#endif /* !SUPPORT_ONLY_80M */

	sei();                                          /* Enable interrupts */

	linkbus_init(BAUD);                             /* Start the Link Bus serial comms */

#ifndef TRANQUILIZE_WATCHDOG
		wdt_init(WD_SW_RESETS);
		wdt_reset();                                /* HW watchdog */
#endif /* TRANQUILIZE_WATCHDOG */

	g_reset_button_held = !digitalRead(PIN_SYNC);

#if INIT_EEPROM_ONLY
		rv3028_1s_sqw(ON);

		if(eepromErr)
		{
			lb_send_string((char*)"EEPROM Erase Error!\n", TRUE);
		}
		else
		{
			ee_mgr.sendSuccessString();
		}

		digitalWrite(PIN_LED, ON);
		while(1)
		{
			;
		}
#else
		if(eepromErr)
		{
			lb_send_string((char*)"EEPROM Error!\n", TRUE);
		}

		uint8_t result = rv3028_1s_sqw(ON);
#endif  /* !INIT_EEPROM_ONLY */

#if !INIT_EEPROM_ONLY
		ee_mgr.send_Help();

		if(result & (1 << RTC_STATUS_I2C_ERROR))
		{
			sprintf(g_tempStr, "Err 1\n");
		}
		else if(result & (1 << RTC_STATUS_CLOCK_CORRUPT))   /* Power off occurred with no backup power */
		{
			sprintf(g_tempStr, "Err 2\n");
		}
		else if(result & (1 << RTC_STATUS_EVF_OCCURRED))
		{
			sprintf(g_tempStr, "Err 3\n");
		}
		else if(result & (1 << RTC_STATUS_BACKUP_SWITCHOVER_OCCURRED))
		{
			sprintf(g_tempStr, "RTC OK\n");
		}

		if(strlen(g_tempStr))
		{
			lb_send_string(g_tempStr, TRUE);
		}

		reportConfigErrors();
		lb_send_NewPrompt();

#endif  /* #if INIT_EEPROM_ONLY */

	startEventNow(POWER_UP);

	/*******************************************************************
	 *  INT0 is for external 1-second interrupts                         */
	EICRA  |= (1 << ISC01); /* Configure INT0 falling edge for RTC 1-second interrupts */
	EIMSK |= (1 << INT0);   /* Enable INT0 interrupts */

#ifdef ATMEL_STUDIO_7
		while(1)
		{
			loop();
		}
#endif  /* ATMEL_STUDIO_7 */
}


/***********************************************************************
 *  Watchdog Timeout ISR
 *
 *  The Watchdog timer helps prevent lockups due to hardware problems.
 *  It is especially helpful in this application for preventing I2C bus
 *  errors from locking up the foreground process.
 ************************************************************************/
ISR(WDT_vect)
{
	static uint8_t limit = 10;

	g_i2c_not_timed_out = FALSE;    /* unstick I2C */

	/* Don't allow an unlimited number of WD interrupts to occur without enabling
	 *  hardware resets. But a limited number might be required during hardware
	 *  initialization. */
	if(!g_enableHardwareWDResets && limit)
	{
		WDTCSR |= (1 << WDIE);  /* this prevents hardware resets from occurring */
	}

	if(limit)
	{
		limit--;
	}
}

/***********************************************************************
 *  Notice: Optimization must be enabled before watchdog can be set
 *  in C (WDCE). Use __attribute__ to enforce optimization level.
 ************************************************************************/
void __attribute__((optimize("O1"))) wdt_init(WDReset resetType)
{
	wdt_reset();

	if(MCUSR & (1 << WDRF))     /* If a reset was caused by the Watchdog Timer perform any special operations */
	{
		MCUSR &= (1 << WDRF);   /* Clear the WDT reset flag */
	}

	if(resetType == WD_DISABLE)
	{
		/* Clear WDRF in MCUSR */
		MCUSR &= ~(1 << WDRF);
		/* Write logical one to WDCE and WDE */
		/* Keep old prescaler setting to prevent unintentional
		 *   time-out */
		WDTCSR |= (1 << WDCE) | (1 << WDE);
		/* Turn off WDT */
		WDTCSR = 0x00;
		g_enableHardwareWDResets = FALSE;
	}
	else
	{
		if(resetType == WD_HW_RESETS)
		{
			WDTCSR |= (1 << WDCE) | (1 << WDE);
			WDTCSR = (1 << WDP3) | (1 << WDIE) | (1 << WDE);    /* Enable WD interrupt every 4 seconds, and hardware resets */
			/*	WDTCSR = (1 << WDP3) | (1 << WDP0) | (1 << WDIE) | (1 << WDE); // Enable WD interrupt every 8 seconds, and hardware resets */
		}
		else if(resetType == WD_SW_RESETS)
		{
			WDTCSR |= (1 << WDCE) | (1 << WDE);
			/*	WDTCSR = (1 << WDP3) | (1 << WDIE); // Enable WD interrupt every 4 seconds (no HW reset)
			 *  WDTCSR = (1 << WDP3) | (1 << WDP0)  | (1 << WDIE); // Enable WD interrupt every 8 seconds (no HW reset) */
			WDTCSR = (1 << WDP1) | (1 << WDP2)  | (1 << WDIE);  /* Enable WD interrupt every 1 seconds (no HW reset) */
		}
		else
		{
			WDTCSR |= (1 << WDCE) | (1 << WDE);
			WDTCSR = (1 << WDIE) | (1 << WDE);  /* Enable WD interrupt in 16ms, and hardware reset */
		}

		g_enableHardwareWDResets = (resetType != WD_SW_RESETS);
	}
}


/***********************************************************************
 *  ADC Conversion Complete ISR
 ************************************************************************/
ISR(ADC_vect)
{
#if !INIT_EEPROM_ONLY
		digitalWrite(PIN_MOSI, ON);
		if(g_goertzel.DataPoint(ADCH))
		{
			ADCSRA &= ~(1 << ADIE); /* disable ADC interrupt */
		}
		digitalWrite(PIN_MOSI, OFF);
#endif /* INIT_EEPROM_ONLY */
}


/***********************************************************************
 *  Pin Change Interrupt 2 ISR
 *
 *  Handles pushbutton presses
 *
 ************************************************************************/
ISR(PCINT1_vect)
{
	static BOOL holdPinVal = OFF;
	BOOL pinVal = digitalRead(PIN_SYNC);

	g_sync_pin_timer = 0;

	if(pinVal && !holdPinVal)   /* Sync is high = button released, and was low previously */
	{
		if(g_LED_timeout_countdown)
		{
			if(!g_perform_EEPROM_reset)
			{
				if(g_transmissions_disabled)
				{
					if(g_sync_pin_stable == STABLE_LOW)
					{
						g_sync_pin_stable = UNSTABLE;
						digitalWrite(PIN_LED, OFF); /*  LED */
						startEventNow(PUSHBUTTON);
					}
				}
				else
				{
					if(g_sync_pin_stable == STABLE_LOW)
					{
						g_sync_pin_stable = UNSTABLE;
						stopEventNow(PUSHBUTTON);
					}
				}
			}
		}
		else
		{
			g_config_error = NULL_CONFIG;   /* Trigger a new configuration enunciation */
		}

		g_LED_timeout_countdown = LED_TIMEOUT_SECONDS;
	}

	holdPinVal = pinVal;
}


/***********************************************************************
 *  USART Rx Interrupt ISR
 *
 *  This ISR is responsible for reading characters from the USART
 *  receive buffer to implement the Linkbus.
 *
 *       Message format:
 *               $id,f1,f2... fn;
 *               where
 *                       id = Linkbus MessageID
 *                       fn = variable length fields
 *                       ; = end of message flag
 *
 ************************************************************************/
ISR(USART_RX_vect)
{
	static char textBuff[LINKBUS_MAX_COMMANDLINE_LENGTH];
	static LinkbusRxBuffer* buff = NULL;
	static uint8_t charIndex = 0;
	static uint8_t field_index = 0;
	static uint8_t field_len = 0;
	static int msg_ID = 0;
	static BOOL receiving_msg = FALSE;
	uint8_t rx_char;

	rx_char = UDR0;

	if(!buff)
	{
		buff = nextEmptyRxBuffer();
	}

	if(buff)
	{
		static uint8_t ignoreCount = 0;
		rx_char = toupper(rx_char);

		if(ignoreCount)
		{
			rx_char = '\0';
			ignoreCount--;
		}
		else if(rx_char == 0x1B)    /* Ignore ESC sequences */
		{
			rx_char = '\0';

			if(charIndex < LINKBUS_MAX_MSG_FIELD_LENGTH)
			{
				rx_char = textBuff[charIndex];
			}

			ignoreCount = 2;        /* throw out the next two characters */
		}
		else if(rx_char == 0x0D)    /* Handle carriage return */
		{
			if(receiving_msg)
			{
				if(charIndex > 0)
				{
					buff->type = LINKBUS_MSG_QUERY;
					buff->id = (LBMessageID)msg_ID;

					if(field_index > 0) /* terminate the last field */
					{
						buff->fields[field_index - 1][field_len] = 0;
					}

					textBuff[charIndex] = '\0'; /* terminate last-message buffer */
				}

				lb_send_NewLine();
			}
			else
			{
				buff->id = INVALID_MESSAGE; /* print help message */
			}

			charIndex = 0;
			field_len = 0;
			msg_ID = MESSAGE_EMPTY;

			field_index = 0;
			buff = NULL;

			receiving_msg = FALSE;
		}
		else if(rx_char)
		{
			textBuff[charIndex] = rx_char;  /* hold the characters for re-use */

			if(charIndex)
			{
				if(rx_char == 0x7F)         /* Handle backspace */
				{
					charIndex--;
					if(field_index == 0)
					{
						msg_ID -= textBuff[charIndex];
						msg_ID /= 10;
					}
					else if(field_len)
					{
						field_len--;
						buff->fields[field_index - 1][field_len] = '\0';
					}
					else if(textBuff[charIndex] == ' ')
					{
						field_index--;
						field_len = strlen(buff->fields[field_index]);
					}
					else
					{
						buff->fields[field_index][0] = '\0';
						field_index--;
					}

					textBuff[charIndex] = '\0'; /* replace deleted char with null */

					if(charIndex == 0)
					{
						receiving_msg = FALSE;
					}
				}
				else
				{
					if(rx_char == ' ')
					{
						if((textBuff[charIndex - 1] == ' ') || ((field_index + 1) >= LINKBUS_MAX_MSG_NUMBER_OF_FIELDS))
						{
							rx_char = '\0';
						}
						else
						{
							if(field_index > 0)
							{
								buff->fields[field_index - 1][field_len] = '\0';
							}

							field_index++;
							field_len = 0;
							charIndex = MIN(charIndex + 1, (LINKBUS_MAX_COMMANDLINE_LENGTH - 1));
						}
					}
					else if(field_len < LINKBUS_MAX_MSG_FIELD_LENGTH)
					{
						if(field_index == 0)    /* message ID received */
						{
							msg_ID = msg_ID * 10 + rx_char;
							field_len++;
						}
						else
						{
							buff->fields[field_index - 1][field_len++] = rx_char;
							buff->fields[field_index - 1][field_len] = '\0';
						}

						charIndex = MIN(charIndex + 1, (LINKBUS_MAX_COMMANDLINE_LENGTH - 1));
					}
					else
					{
						rx_char = '\0';
					}
				}
			}
			else
			{
				if(rx_char == 0x7F) /* Handle Backspace */
				{
					if(msg_ID <= 0)
					{
						rx_char = '\0';
					}

					msg_ID = 0;
				}
				else if(rx_char == ' ') /* Handle Space */
				{
					rx_char = '\0';
				}
				else                    /* start of new message */
				{
					uint8_t i;
					field_index = 0;
					msg_ID = rx_char;

					/* Empty the field buffers */
					for(i = 0; i < LINKBUS_MAX_MSG_NUMBER_OF_FIELDS; i++)
					{
						buff->fields[i][0] = '\0';
					}

					receiving_msg = TRUE;
					charIndex++;
				}
			}

			if(rx_char)
			{
				lb_echo_char(rx_char);
			}
		}
	}
}   /* End of UART Rx ISR */


/***********************************************************************
 *  USART Tx UDRE ISR
 *
 *  This ISR is responsible for filling the USART transmit buffer. It
 *  implements the transmit function of the Linkbus.
 *
 ************************************************************************/
ISR(USART_UDRE_vect)
{
	static LinkbusTxBuffer* buff = 0;
	static uint8_t charIndex = 0;

	if(!buff)
	{
		buff = nextFullTxBuffer();
	}

	if((*buff)[charIndex])
	{
		/* Put data into buffer, sends the data */
		UDR0 = (*buff)[charIndex++];
	}
	else
	{
		charIndex = 0;
		(*buff)[0] = '\0';
		buff = nextFullTxBuffer();
		if(!buff)
		{
			linkbus_end_tx();
		}
	}
}   /* End of UART Tx ISR */


/***********************************************************************
 *  Timer/Counter2 Compare Match B ISR
 *
 *  Handles periodic tasks not requiring precise timing.
 ************************************************************************/
ISR( TIMER2_COMPB_vect )
{
	g_tick_count++;

	if(g_dtmf_error_countdown)
	{
		g_dtmf_error_countdown--;
	}

	if(g_LED_Enunciation_holdoff)
	{
		g_LED_Enunciation_holdoff--;
	}

	if(g_dtmf_detected)
	{
		g_tone_duration_ticks++;
		g_LED_Enunciation_holdoff = TIMER2_SECONDS_1;
	}
	else
	{
		g_tone_duration_ticks = 0;
	}

	if(g_DTMF_sentence_in_progress_ticks)
	{
		g_DTMF_sentence_in_progress_ticks--;
	}

	static uint16_t codeInc = 0;
	static uint8_t holdButtonState = HIGH;
	BOOL repeat = TRUE, finished = FALSE;

	uint8_t button = digitalRead(PIN_SYNC);

	if(g_reset_button_held && !button)
	{
		if(g_seconds_since_powerup < 5)
		{
			digitalWrite(PIN_LED, ON);  /* hold the LED on until the sync button is released */
		}
		else if(g_seconds_since_powerup == 5)
		{
			g_seconds_since_powerup = 6;
			g_perform_EEPROM_reset = TRUE;
		}
	}
	else
	{
		g_reset_button_held = FALSE;
	}

	if(button == holdButtonState)
	{
		if(g_sync_pin_timer < TIMER2_SECONDS_3)
		{
			g_sync_pin_timer++;
		}

		if(g_sync_pin_timer > TIMER2_MSECONDS_100)
		{
			g_sync_pin_stable = (button == HIGH) ? STABLE_HIGH : STABLE_LOW;

			if((button == LOW) && !g_reset_button_held)
			{
				digitalWrite(PIN_LED, ON);
			}
		}
	}
	else
	{
		holdButtonState = button;
		g_sync_pin_timer = 0;
	}


	static BOOL key = OFF;
	static uint16_t ptt_delay = 0;
	static uint16_t ptt_dropped = 0;

	if(!g_transmissions_disabled && g_on_the_air && !ptt_dropped)
	{
		if(!digitalRead(PIN_PTT_LOGIC))
		{
			digitalWrite(PIN_PTT_LOGIC, ON);
			ptt_delay = TIMER2_SECONDS_1;
		}
		else if(ptt_delay)
		{
			ptt_delay--;
		}
		else
		{
			if(codeInc)
			{
				codeInc--;

				if(!codeInc)
				{
					key = makeMorse(NULL, &repeat, &finished);

					if(!repeat && finished) /* ID has completed, so resume pattern */
					{
						key = OFF;
						g_callsign_sent = TRUE;

						if(g_use_ptt_periodic_reset)
						{
							g_on_the_air = FALSE;
							ptt_dropped = TIMER2_SECONDS_1;
						}
					}

					digitalWrite(PIN_LED, key);             /*  LED */
					digitalWrite(PIN_CW_KEY_LOGIC, key);    /* TX key line */
					g_sendAMmodulation = key;
					sendMorseTone(key);
				}
			}
			else
			{
				if(g_sync_pin_stable != STABLE_LOW)
				{
					digitalWrite(PIN_LED, key);         /*  LED */
				}

				digitalWrite(PIN_CW_KEY_LOGIC, key);    /* TX key line */
				g_sendAMmodulation = key;
				sendMorseTone(key);
				codeInc = g_code_throttle;
			}
		}
	}
	else
	{
		if(digitalRead(PIN_PTT_LOGIC))
		{
			if(!ptt_delay)
			{
				ptt_delay = 100;
			}
			else
			{
				ptt_delay--;

				if(!ptt_delay)
				{
					digitalWrite(PIN_PTT_LOGIC, OFF);   /* Unkey the microphone / de-energize transmitter */
				}
			}
		}
		else if(ptt_dropped)
		{
			ptt_dropped--;

			if(!ptt_dropped)
			{
				g_on_the_air = TRUE;
			}
		}

		if(g_LED_enunciating)
		{
			if(!g_LED_Enunciation_holdoff)
			{
				if(codeInc)
				{
					codeInc--;

					if(!codeInc)
					{
						key = makeMorse(NULL, &repeat, &finished);
						digitalWrite(PIN_LED, key); /*  LED */
						codeInc = g_code_throttle;
					}
				}
				else
				{
					codeInc = g_code_throttle;
				}
			}
		}
		else
		{
			if(key)
			{
				key = OFF;
				if(g_sync_pin_stable != STABLE_LOW)
				{
					digitalWrite(PIN_LED, OFF); /*  LED Off */
				}
			}
		}

		digitalWrite(PIN_CW_KEY_LOGIC, OFF);    /* TX key line */
		g_sendAMmodulation = FALSE;
		sendMorseTone(OFF);
	}
}                                               /* End of Timer 2 ISR */


#if !INIT_EEPROM_ONLY
/***********************************************************************
 *  Handle RTC 1-second interrupts
 **********************************************************************/
	ISR( INT0_vect )
	{
		static int32_t post_sync_seconds_to_send_ID = 0;
		static int seconds_into_cycle = 0;
		static BOOL fox_transition_occurred = FALSE;
		BOOL send_ID_now = FALSE;
		BOOL energizeTx = FALSE;

		g_current_epoch++;

		g_seconds_since_powerup++;

		if(g_temperature_check_countdown)
		{
			g_temperature_check_countdown--;
		}

		if(g_LED_timeout_countdown)
		{
			g_LED_timeout_countdown--;
		}

		if(g_voltage_check_countdown)
		{
			g_voltage_check_countdown--;
		}

		if(g_transmissions_disabled)
		{
			if(g_use_rtc_for_startstop)
			{
				if((g_current_epoch >= g_event_start_epoch) && (g_current_epoch < g_event_finish_epoch))    /* Event should be running */
				{
					g_LED_enunciating = FALSE;
					g_transmissions_disabled = FALSE;
					BOOL repeat = TRUE;
					makeMorse((char*)"\0", NULL, NULL);
					makeMorse((char*)g_messages_text[PATTERN_TEXT], &repeat, NULL);
					g_code_throttle = THROTTLE_VAL_FROM_WPM(g_pattern_codespeed);
				}
			}
		}
		else
		{
			if(g_use_rtc_for_startstop)
			{
				if(g_current_epoch >= g_event_finish_epoch) /* Event has ended */
				{
					g_use_rtc_for_startstop = FALSE;
					g_transmissions_disabled = TRUE;
				}
			}

			if(!g_transmissions_disabled)
			{
				int secondsForID = (500 + timeRequiredToSendStrAtWPM((char*)g_messages_text[STATION_ID], g_id_codespeed)) / 1000;

				if((g_seconds_since_sync == 0) && (g_initialize_fox_transmissions == INIT_NOT_SPECIFIED))   /* sync occurs now */
				{
					send_ID_now = FALSE;
					if(g_id_interval_seconds <= g_cycle_period_seconds)
					{
						int slot = MAX(1, g_fox - g_fox_id_offset);
						post_sync_seconds_to_send_ID =  (slot * g_on_air_interval_seconds) - secondsForID;
					}

					seconds_into_cycle = 0;
				}
				else
				{
					seconds_into_cycle = g_seconds_since_sync % g_cycle_period_seconds;

					if(g_id_interval_seconds)   /* This condition must be met or there will be no IDs sent */
					{
						if(g_initialize_fox_transmissions == INIT_EVENT_STARTING_NOW)
						{
							send_ID_now = FALSE;
							post_sync_seconds_to_send_ID = g_seconds_since_sync + g_on_air_interval_seconds - secondsForID;
							energizeTx = TRUE;
						}
						else if(g_initialize_fox_transmissions == INIT_EVENT_IN_PROGRESS_WITH_STARTFINISH_TIMES)
						{
							if(((g_fox - g_fox_id_offset) == g_fox_counter) || (g_number_of_foxes == 1))    /* This transmitter is on-the-air now */
							{
								int secondsLeftOfXmsn = g_on_air_interval_seconds - (seconds_into_cycle % g_on_air_interval_seconds);

								if(secondsLeftOfXmsn > secondsForID)
								{
									post_sync_seconds_to_send_ID = g_seconds_since_sync + secondsLeftOfXmsn - secondsForID;
								}
								else if(secondsLeftOfXmsn < secondsForID)
								{
									post_sync_seconds_to_send_ID = g_seconds_since_sync + secondsLeftOfXmsn + g_cycle_period_seconds - secondsForID;
								}
								else
								{
									send_ID_now = TRUE;
								}

								energizeTx = TRUE;
							}
							else
							{
								post_sync_seconds_to_send_ID = g_seconds_since_sync - seconds_into_cycle + ((g_fox - g_fox_id_offset) * g_on_air_interval_seconds) - secondsForID;
								if(post_sync_seconds_to_send_ID < g_seconds_since_sync)
								{
									post_sync_seconds_to_send_ID += g_id_interval_seconds;
								}
								send_ID_now = FALSE;
							}
						}
						else if(post_sync_seconds_to_send_ID == g_seconds_since_sync)
						{
							send_ID_now = TRUE;
							post_sync_seconds_to_send_ID = g_seconds_since_sync + g_id_interval_seconds;
						}
					}
				}

				/* Handle transitions from one fox to the next */
				if(g_number_of_foxes && ((g_seconds_since_sync % g_on_air_interval_seconds) == 0))
				{
					if(g_seconds_since_sync && !g_initialize_fox_transmissions)
					{
						g_fox_counter++;
					}

					fox_transition_occurred = TRUE;

					if(g_fox_counter > g_number_of_foxes)   /* End of cycle */
					{
						g_fox_counter = 1;
					}

					seconds_into_cycle = 0;
				}

				if(send_ID_now) /* Sending the call sign takes priority */
				{
					g_code_throttle = THROTTLE_VAL_FROM_WPM(g_id_codespeed);
					BOOL repeat = FALSE;
					if(g_messages_text[STATION_ID][0])
					{
						makeMorse((char*)g_messages_text[STATION_ID], &repeat, NULL);
					}
					else
					{
						makeMorse((char*)" ", &repeat, NULL);
					}
					g_callsign_sent = FALSE;
					g_on_the_air = TRUE;
				}
				else if((fox_transition_occurred && g_callsign_sent) || energizeTx)
				{
					digitalWrite(PIN_LED, OFF);
					fox_transition_occurred = FALSE;

					if((g_number_of_foxes > 1) && (g_fox != (g_fox_counter + g_fox_id_offset))) /* Turn off transmissions during times when this fox should be silent */
					{
						g_on_the_air = FALSE;
					}
					else
					{
						BOOL repeat;
						/* Choose the appropriate Morse pattern to be sent */
						if(g_fox == REPORT_BATTERY)
						{
							uint16_t v = g_voltage + 5;
							sprintf(g_tempStr, "|||%dR%d/%d", v / 100, (v % 100) / 10, g_temperature);
							strcpy((char*)g_messages_text[PATTERN_TEXT], g_tempStr);
							repeat = FALSE;
						}
						else
						{
							strcpy((char*)g_messages_text[PATTERN_TEXT], g_morsePatterns[g_fox]);
							repeat = TRUE;
						}

						g_code_throttle = THROTTLE_VAL_FROM_WPM(g_pattern_codespeed);
						makeMorse((char*)"\0", NULL, NULL);
						makeMorse((char*)g_messages_text[PATTERN_TEXT], &repeat, NULL);

						g_on_the_air = TRUE;
					}
				}

				g_initialize_fox_transmissions = INIT_NOT_SPECIFIED;
				g_seconds_since_sync++; /* Total elapsed time counter */
				seconds_into_cycle++;
			}
		}
	}                                   /* end of INT0 ISR */
#endif /* INIT_EEPROM_ONLY */

/***********************************************************************
 *  Timer0 interrupt generates an audio tone on the audio out pin.
 ************************************************************************/
ISR(TIMER0_COMPA_vect)
{
	static BOOL toggle = 0;

	if(!g_AM_enabled)
	{
		toggle = !toggle;

		if(g_audio_tone_state)
		{
			digitalWrite(PIN_CW_TONE_LOGIC, toggle);
		}
		else
		{
			digitalWrite(PIN_CW_TONE_LOGIC, OFF);
		}
	}
}

#if !SUPPORT_ONLY_80M
/***********************************************************************
 *  Timer/Counter1 Compare Match A ISR
 *
 *  Handles AM modulation tasks
 ************************************************************************/
	ISR(TIMER1_COMPA_vect)  /* timer1 interrupt */
	{
#if !INIT_EEPROM_ONLY
			if(g_AM_enabled)
			{
				static uint8_t index = 0;
				static uint8_t controlPins = 0;

				if(g_sendAMmodulation || index || g_sendAMmodulationConstantly)
				{
					/*		controlPins = eeprom_read_byte((uint8_t*)&ee_dataModulation[index++]); */
					controlPins = g_dataModulation[index++];
					if(index >= SIZE_OF_DATA_MODULATION)
					{
						index = 0;
					}

					PORTB = controlPins;
					controlPins = 0;
				}
				else if(controlPins != MAX_ATTEN_SETTING)
				{
					controlPins = MAX_ATTEN_SETTING;
					PORTB = controlPins;
				}
			}
#endif  /* INIT_EEPROM_ONLY */
	}
#endif  /* !SUPPORT_ONLY_80M */

#define CLIPPING_THRESHOLD 12000000.

/***********************************************************************
 *   Here is the main loop
 ************************************************************************/
void loop()
{
#if !INIT_EEPROM_ONLY
		int8_t dtmfX = -1;
		int8_t dtmfY = -1;
		float largestX;
		float largestY;
		BOOL dtmfDetected = FALSE;
		BOOL noiseDetected = FALSE;
		int clipCount = 0;
		BOOL dtmfEntryError = FALSE;
#endif  /* !INIT_EEPROM_ONLY */

#if !INIT_EEPROM_ONLY
		if(g_perform_EEPROM_reset)
		{
			ee_mgr.resetEEPROMValues();
			linkbus_init(BAUD);
			while(g_reset_button_held)
			{
				digitalWrite(PIN_LED, OFF); /*  LED */
			}

			g_perform_EEPROM_reset = FALSE;
		}

		dtmfEntryError = processDTMFdetection(NO_KEY);
#endif  /* !INIT_EEPROM_ONLY */

	handleLinkBusMsgs();

#ifndef TRANQUILIZE_WATCHDOG
		wdt_reset();    /* HW watchdog */
#endif /* TRANQUILIZE_WATCHDOG */


#if !INIT_EEPROM_ONLY
		if(!dtmfEntryError)
		{
			if(g_goertzel.SamplesReady())
			{
				static char lastKey = '\0';
				static int checkCount = 10; /* Initialize to a value above the threshold to prevent an initial false key detect */
				static int quietCount = 0;

				float magnitudeX;
				float magnitudeY;
#ifdef DEBUG_DTMF
					float magnitudeM;
#endif  /* DEBUG_DTMF */
				largestX = 0;
				largestY = 0;
				dtmfX = -1;
				dtmfY = -1;

				dtmfDetected = FALSE;
				noiseDetected = FALSE;
				clipCount = 0;

				if(!g_temperature_check_countdown)
				{
					setUpSampling(TEMPERATURE_SAMPLING, FALSE);
					int8_t temp = (int8_t)getTemp();
					if(temp != g_temperature)
					{
						g_temperature = temp;
						int8_t delta25 = temp > 25 ? temp - 25 : 25 - temp;
						int8_t adj = ee_mgr.readTemperatureTable(delta25);
						rv3028_set_offset_RAM(g_rv3028_offset + adj);
					}

					setUpSampling(AUDIO_SAMPLING, FALSE);
					g_temperature_check_countdown = TEMPERATURE_POLL_INTERVAL_SECONDS;
				}
				else if(!g_voltage_check_countdown)
				{
					setUpSampling(VOLTAGE_SAMPLING, FALSE);
					g_voltage = getVoltage();
					setUpSampling(AUDIO_SAMPLING, FALSE);
					g_voltage_check_countdown = VOLTAGE_POLL_INTERVAL_SECONDS;
				}

				for(int i = 0; i < 4; i++)
				{
					g_goertzel.SetTargetFrequency(y_frequencies[i]);    /* Initialize the object with the sampling frequency, # of samples and target freq */
					magnitudeY = g_goertzel.Magnitude2(&clipCount);     /* Check samples for presence of the target frequency */

					if(magnitudeY > largestY)                           /* Use only the greatest Y value */
					{
						largestY = magnitudeY;
						if(magnitudeY > threshold)                      /* Only consider Y above a certain threshold */
						{
							dtmfY = i;
						}
					}

#ifdef DEBUG_DTMF
						if(magnitudeY > threshold)
						{
							dtostrf((double)y_frequencies[i], 4, 0, s);
							sprintf(g_tempStr, "Y(%s)=", s);
							lb_send_string(g_tempStr, TRUE);
							dtostrf((double)magnitudeY, 4, 0, s);
							sprintf(g_tempStr, "%s\n", s);
							lb_send_string(g_tempStr, TRUE);
						}
#endif  /* DEBUG_DTMF */
				}

				if(dtmfY >= 0)
				{
					for(int i = 0; i < 4; i++)
					{
						g_goertzel.SetTargetFrequency(x_frequencies[i]);    /* Initialize the object with the sampling frequency, # of samples and target freq */
						magnitudeX = g_goertzel.Magnitude2(NULL);           /* Check samples for presence of the target frequency */

						if(magnitudeX > largestX)                           /* Use only the greatest X value */
						{
							largestX = magnitudeX;
							if(magnitudeX > threshold)                      /* Only consider X above a certain threshold */
							{
								dtmfX = i;
							}
						}

#ifdef DEBUG_DTMF
							if(magnitudeX > threshold)
							{
								dtostrf((double)x_frequencies[i], 4, 0, s);
								sprintf(g_tempStr, "X(%s)=", s);
								lb_send_string(g_tempStr, TRUE);
								dtostrf((double)magnitudeX, 4, 0, s);
								sprintf(g_tempStr, "%s\n", s);
								lb_send_string(g_tempStr, TRUE);
							}
#endif  /* DEBUG_DTMF */
					}

					if(g_DTMF_sentence_in_progress_ticks || (checkCount < 3) || (clipCount < 50))
					{
						if(dtmfX >= 0)
						{
							DTMF_key_t newKey = value2DTMFKey(4 * dtmfY + dtmfX);
							dtmfDetected = TRUE;

							/* If the same key is detected three times in a row with no silent periods between them then register a new keypress */
							if(lastKey == newKey)
							{
								if(checkCount < 10)
								{
									checkCount++;
								}

								if(checkCount == 3)
								{
									g_dtmf_detected = TRUE;
									quietCount = 0;
									g_lastKey = newKey;

/*#ifdef DEBUG_DTMF */
									if(lb_enabled())
									{
										sprintf(g_tempStr, "\"%c\"\n", g_lastKey);
										lb_send_string(g_tempStr, TRUE);
									}
/*#endif  / * DEBUG_DTMF * / */

									dtmfEntryError = processDTMFdetection(newKey);

#ifdef DEBUG_DTMF
										if(lb_enabled())
										{
											lb_send_string((char*)"Mag X/Y=", TRUE);
											dtostrf((double)largestX, 4, 0, s);
											sprintf(g_tempStr, "%s / ", s);
											lb_send_string(g_tempStr, TRUE);
											dtostrf((double)largestY, 4, 0, s);
											sprintf(g_tempStr, "%s\n", s);
											lb_send_string(g_tempStr, TRUE);

											for(int i = 0; i < 7; i++)
											{
												g_goertzel.SetTargetFrequency(mid_frequencies[i]);  /* Initialize the object with the sampling frequency, # of samples and target freq */
												magnitudeM = g_goertzel.Magnitude2(NULL);           /* Check samples for presence of the target frequency */
												dtostrf((double)mid_frequencies[i], 4, 0, s);
												sprintf(g_tempStr, "%s=", s);
												lb_send_string(g_tempStr, TRUE);
												dtostrf((double)magnitudeM, 4, 0, s);
												sprintf(g_tempStr, "%s\n", s);
												lb_send_string(g_tempStr, TRUE);
											}
										}
#endif  /* DEBUG_DTMF */
								}
							}

							lastKey = newKey;
						}
					}
					else
					{
						noiseDetected = TRUE;
						g_dtmf_detected = TRUE;

						digitalWrite(PIN_LED, ON);
/*					g_config_error = NULL_CONFIG;   / * Trigger a new configuration enunciation * / */

						/*#ifdef DEBUG_DTMF */
						if(lb_enabled())
						{

							sprintf(g_tempStr, "ClipCount=%d\n", clipCount);
							lb_send_string(g_tempStr, TRUE);

/*						char s[10];
 *						lb_send_string((char*)"Mag X/Y=", TRUE);
 *						dtostrf((double)largestX, 4, 0, s);
 *						sprintf(g_tempStr, "%s / ", s);
 *						lb_send_string(g_tempStr, TRUE);
 *						dtostrf((double)largestY, 4, 0, s);
 *						sprintf(g_tempStr, "%s\n", s);
 *						lb_send_string(g_tempStr, TRUE); */
						}
						/*#endif  / * DEBUG_DTMF * / */
					}
				}

				if(!dtmfDetected && !noiseDetected) /* Quiet detected */
				{
					static unsigned long lastQuiet = 0;
					unsigned long delta = g_tick_count - lastQuiet;

					/* Quieting must be detected at least 3 times in less than 2 seconds before another key can be accepted */
					if(quietCount++ > 2)
					{
						g_dtmf_detected = FALSE;
						if(g_transmissions_disabled && !g_LED_enunciating)
						{
							digitalWrite(PIN_LED, OFF);
						}

						if(delta < TIMER2_SECONDS_2)
						{
							checkCount = 0;
						}

						quietCount = 0;
						lastQuiet = g_tick_count;
						lastKey = '\0';
					}
				}
				else if(g_tone_duration_ticks >= TIMER2_SECONDS_5)  /* The most likely cause of such a long tone is loud noise */
				{
					g_dtmf_detected = FALSE;
					g_config_error = NULL_CONFIG;

					if(g_transmissions_disabled && !g_LED_enunciating)
					{
						digitalWrite(PIN_LED, OFF);
					}

					lastKey = '\0';
				}

				ADCSRA |= (1 << ADIE);  /* enable interrupts when measurement complete */
				ADCSRA |= (1 << ADSC);  /* start ADC measurements */
			}
		}

	if(!g_on_the_air)
	{
		if(g_dtmf_error_countdown)
		{
			g_dtmf_detected = FALSE;
		}
		else if(dtmfEntryError)
		{
			BOOL repeat = FALSE;
			makeMorse(DTMF_ERROR_BLINK_PATTERN, &repeat, NULL);
			g_code_throttle = THROTTLE_VAL_FROM_WPM(30);
			g_LED_enunciating = TRUE;
			g_dtmf_detected = FALSE;
			g_dtmf_error_countdown = TIMER2_SECONDS_3;
			g_LED_Enunciation_holdoff = 0;
		}
		else if(g_dtmf_detected)
		{
			digitalWrite(PIN_LED, ON);
			g_config_error = NULL_CONFIG;   /* Trigger a new configuration enunciation */
		}
		else if(g_transmissions_disabled)
		{
			if(g_LED_timeout_countdown && !g_LED_Enunciation_holdoff && !g_DTMF_sentence_in_progress_ticks)
			{
				ConfigurationState_t hold_config_err = g_config_error;
				g_config_error = clockConfigurationCheck();

				if(g_config_error != hold_config_err)
				{
					if(g_config_error == CONFIGURATION_ERROR)
					{
						BOOL repeat = TRUE;
						makeMorse(ERROR_BLINK_PATTERN, &repeat, NULL);
						g_code_throttle = THROTTLE_VAL_FROM_WPM(10);
						g_LED_enunciating = TRUE;
					}
					else if(g_config_error == WAITING_FOR_START)
					{
						BOOL repeat = TRUE;
						makeMorse(WAITING_BLINK_PATTERN, &repeat, NULL);
						g_code_throttle = THROTTLE_VAL_FROM_WPM(20);
						g_LED_enunciating = TRUE;
					}
					else if((g_config_error == SCHEDULED_EVENT_WILL_NEVER_RUN) || (g_config_error == SCHEDULED_EVENT_DID_NOT_START))
					{
						BOOL repeat = TRUE;
						makeMorse(ERROR_BLINK_PATTERN, &repeat, NULL);
						g_code_throttle = THROTTLE_VAL_FROM_WPM(10);
						g_LED_enunciating = TRUE;
					}
					else
					{
						g_LED_enunciating = FALSE;
						digitalWrite(PIN_LED, OFF); /* ensure LED is off */
					}
				}
			}
			else
			{
				g_LED_enunciating = FALSE;
			}
		}
		else
		{
			g_LED_enunciating = FALSE;
			digitalWrite(PIN_LED, OFF); /* ensure LED is off */
		}
	}
#endif  /* !INIT_EEPROM_ONLY */
}


ConfigurationState_t clockConfigurationCheck(void)
{
	if((g_event_finish_epoch < MINIMUM_EPOCH) || (g_event_start_epoch < MINIMUM_EPOCH) || (g_current_epoch < MINIMUM_EPOCH))
	{
		return(CONFIGURATION_ERROR);
	}

	if(g_event_finish_epoch <= g_event_start_epoch) /* Event configured to finish before it started */
	{
		return(CONFIGURATION_ERROR);
	}

	if(g_current_epoch > g_event_finish_epoch)  /* The scheduled event is over */
	{
		return(CONFIGURATION_ERROR);
	}

	if(g_current_epoch > g_event_start_epoch)       /* Event should be running */
	{
		if(g_transmissions_disabled)
		{
			return(SCHEDULED_EVENT_DID_NOT_START);  /* Event scheduled to be running isn't */
		}
		else
		{
			return(EVENT_IN_PROGRESS);              /* Event is running, so clock settings don't matter */
		}
	}
	else if(!g_use_rtc_for_startstop)
	{
		return(SCHEDULED_EVENT_WILL_NEVER_RUN);
	}

	return(WAITING_FOR_START);  /* Future event hasn't started yet */
}


void sendMorseTone(BOOL onOff)
{
	OCR0A = DEFAULT_TONE_FREQUENCY;
	g_audio_tone_state = onOff;
}

void playStartingTone(uint8_t toneFreq)
{
	if(toneFreq > 0)
	{
		OCR0A = toneFreq;
		g_audio_tone_state = ON;
	}
	else
	{
		OCR0A = DEFAULT_TONE_FREQUENCY;
		g_audio_tone_state = OFF;
	}
}


/* The compiler does not seem to always optimize large switch statements correctly
 * void __attribute__((optimize("O3"))) handleLinkBusMsgs() */
void handleLinkBusMsgs()
{
	LinkbusRxBuffer* lb_buff;

	while((lb_buff = nextFullRxBuffer()))
	{
		LBMessageID msg_id = lb_buff->id;

		switch(msg_id)
		{
			case MESSAGE_SET_FOX:
			{
				int c = (int)(lb_buff->fields[FIELD1][0]);

				if(c)
				{
					if(c == 'B')
					{
						c = BEACON;
					}
					else if(c == 'F')
					{
						c = FOXORING;
					}
					else if(c == 'C')
					{
						char t = lb_buff->fields[FIELD2][0];
						lb_buff->fields[FIELD2][1] = '\0';

						if(t == 'B')
						{
							t = '0';
						}

						if(isdigit(t))
						{
							c = CLAMP(BEACON, atoi(lb_buff->fields[FIELD2]), FOX_5);
						}
					}
					else if(c == 'S')
					{
						char x = 0;
						char t = lb_buff->fields[FIELD2][0];
						char u = lb_buff->fields[FIELD2][1];
						lb_buff->fields[FIELD2][2] = '\0';

						if(t == 'B')
						{
							x = BEACON;
						}
						else if(t == 'F')
						{
							if((u > '0') && (u < '6'))
							{
								x = SPRINT_F1 + (u - '1');
							}
						}
						else if(t == 'S')
						{
							if((u > '0') && (u < '6'))
							{
								x = SPRINT_S1 + (u - '1');
							}
							else
							{
								x = SPECTATOR;
							}
						}
						else if(u == 'F')
						{
							if((t > '0') && (t < '6'))
							{
								x = SPRINT_F1 + (t - '1');
							}
						}
						else if(u == 'S')
						{
							if((t > '0') && (t < '6'))
							{
								x = SPRINT_S1 + (t - '1');
							}
						}

						if((x >= SPECTATOR) && (x <= SPRINT_F5))
						{
							c = x;
						}
						else
						{
							c = BEACON;
						}
					}
					else
					{
						c = atoi(lb_buff->fields[FIELD1]);
					}

					if((c >= BEACON) && (c < INVALID_FOX))
					{
						Fox_t holdFox = (Fox_t)c;
						ee_mgr.updateEEPROMVar(Fox_setting, (void*)&holdFox);
						if(holdFox != g_fox)
						{
							setupForFox(&holdFox, START_NOTHING);
						}
					}
				}

				sprintf(g_tempStr, "Fox=%u\n", g_fox);
				lb_send_string(g_tempStr, FALSE);
			}
			break;

#if !SUPPORT_ONLY_80M
					case MESSAGE_SET_AM_TONE:
					{
						if(lb_buff->fields[FIELD1][0])
						{
							uint8_t toneVal = atol(lb_buff->fields[FIELD1]);
							g_AM_audio_frequency = (AM_Tone_Freq_t)CLAMP(MIN_AM_TONE_FREQUENCY, toneVal, MAX_AM_TONE_FREQUENCY);
							ee_mgr.updateEEPROMVar(Am_audio_frequency, (void*)&g_AM_audio_frequency);
							setAMToneFrequency(g_AM_audio_frequency);
						}

						sprintf(g_tempStr, "AM:%d\n", (uint8_t)g_AM_audio_frequency);
						lb_send_string(g_tempStr, FALSE);
					}
					break;
#endif  /* !SUPPORT_ONLY_80M */

			case MESSAGE_SYNC:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					if(lb_buff->fields[FIELD1][0] == '0')       /* Stop the event. Re-sync will occur on next start */
					{
						stopEventNow(PROGRAMMATIC);
					}
					else if(lb_buff->fields[FIELD1][0] == '1')  /* Start the event, re-syncing to a start time of now - same as a button press */
					{
						startEventNow(PROGRAMMATIC);
					}
					else if(lb_buff->fields[FIELD1][0] == '2')  /* Start the event at the programmed start time */
					{
						g_transmissions_disabled = TRUE;        /* Disable an event currently underway */
						startEventUsingRTC();
					}
					else if(lb_buff->fields[FIELD1][0] == '3')  /* Start the event at the programmed start time */
					{
						setupForFox(NULL, START_TRANSMISSIONS_NOW);
					}
					else
					{
						lb_send_string((char*)"err\n", TRUE);
					}
				}
				else
				{
					lb_send_string((char*)"err\n", TRUE);
				}
			}
			break;

			case MESSAGE_SET_STATION_ID:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					strcpy(g_tempStr, " "); /* Space before ID gets sent */
					strcat(g_tempStr, lb_buff->fields[FIELD1]);

					if(lb_buff->fields[FIELD2][0])
					{
						strcat(g_tempStr, " ");
						strcat(g_tempStr, lb_buff->fields[FIELD2]);
					}

					if(strlen(g_tempStr) <= MAX_PATTERN_TEXT_LENGTH)
					{
						strcpy((char*)g_messages_text[STATION_ID], g_tempStr);
						ee_mgr.updateEEPROMVar(StationID_text, (void*)g_tempStr);
					}
				}

				if(g_messages_text[STATION_ID][0])
				{
					g_time_needed_for_ID = (600 + timeRequiredToSendStrAtWPM((char*)g_messages_text[STATION_ID], g_id_codespeed)) / 1000;
				}

				sprintf(g_tempStr, "ID:%s\n", g_messages_text[STATION_ID]);
				lb_send_string(g_tempStr, TRUE);
			}
			break;


			case MESSAGE_CODE_SETTINGS:
			{
				if(lb_buff->fields[FIELD1][0] == 'S')
				{
					char x = lb_buff->fields[FIELD2][0];

					if(x)
					{
						uint8_t speed = atol(lb_buff->fields[FIELD2]);
						g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM, speed, MAX_CODE_SPEED_WPM);
						ee_mgr.updateEEPROMVar(Id_codespeed, (void*)&g_id_codespeed);

						if(g_messages_text[STATION_ID][0])
						{
							g_time_needed_for_ID = (600 + timeRequiredToSendStrAtWPM((char*)g_messages_text[STATION_ID], g_id_codespeed)) / 1000;
						}
					}

					sprintf(g_tempStr, "ID: %d wpm\n", g_id_codespeed);
				}
				else if(lb_buff->fields[FIELD1][0] == 'P')
				{
					char x = lb_buff->fields[FIELD2][0];

					if(x)
					{
						g_ptt_periodic_reset_enabled = ((x == '1') || (x == 'T') || (x == 'Y'));
						ee_mgr.updateEEPROMVar(Ptt_periodic_reset, (void*)&g_ptt_periodic_reset_enabled);
						g_use_ptt_periodic_reset = g_ptt_periodic_reset_enabled;
					}

					sprintf(g_tempStr, "DRP:%d\n", g_ptt_periodic_reset_enabled);
				}
				else
				{
					sprintf(g_tempStr, "err\n");
				}

				lb_send_string(g_tempStr, FALSE);
			}
			break;

			case MESSAGE_PASSWORD:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					strncpy(g_tempStr, lb_buff->fields[FIELD1], MAX_UNLOCK_CODE_LENGTH);
					g_tempStr[MAX_UNLOCK_CODE_LENGTH] = '\0';   /* truncate to no more than max characters */

					if(only_digits(g_tempStr) && (strlen(g_tempStr) >= MIN_UNLOCK_CODE_LENGTH))
					{
						strcpy((char*)g_unlockCode, g_tempStr);
						ee_mgr.updateEEPROMVar(UnlockCode, (void*)g_unlockCode);
					}
				}

				sprintf(g_tempStr, "PWD=%s\n", (char*)g_unlockCode);
				lb_send_string(g_tempStr, TRUE);
			}
			break;

			case MESSAGE_CLOCK:
			{
				BOOL doprint = false;

				if(lb_buff->fields[FIELD1][0] == 'T')   /* Current time format "YYMMDDhhmmss" */
				{
					strncpy(g_tempStr, lb_buff->fields[FIELD2], 12);
					g_tempStr[12] = '\0';               /* truncate to no more than 12 characters */

					time_t t = validateTimeString(g_tempStr, (time_t*)&g_current_epoch, -g_utc_offset);

					if(t)
					{
						rv3028_set_epoch(t);
						g_current_epoch = t;
						sprintf(g_tempStr, "Time:%lu\n", g_current_epoch);
						setupForFox(NULL, START_NOTHING);   /* Avoid timing problems if an event is already active */
					}
					else
					{
						g_current_epoch = rv3028_get_epoch();
						reportTimeTill(g_current_epoch, g_event_start_epoch, "Starts in: ", NULL);
						sprintf(g_tempStr, "UNIX Time:%lu\n", g_current_epoch);
					}

					doprint = true;
				}
				else if(lb_buff->fields[FIELD1][0] == 'S')  /* Event start time */
				{
					strcpy(g_tempStr, lb_buff->fields[FIELD2]);
					time_t s = validateTimeString(g_tempStr, (time_t*)&g_event_start_epoch, -g_utc_offset);

					if(s)
					{
						g_event_start_epoch = s;
						ee_mgr.updateEEPROMVar(Event_start_epoch, (void*)&g_event_start_epoch);
						g_event_finish_epoch = MAX(g_event_finish_epoch, (g_event_start_epoch + SECONDS_24H));
						ee_mgr.updateEEPROMVar(Event_finish_epoch, (void*)&g_event_finish_epoch);
						sprintf(g_tempStr, "Start:%lu\n", g_event_start_epoch);
						startEventUsingRTC();
					}
					else
					{
						sprintf(g_tempStr, "Start:%lu\n", g_event_start_epoch);
					}

					doprint = true;
				}
				else if(lb_buff->fields[FIELD1][0] == 'F')  /* Event finish time */
				{
					strcpy(g_tempStr, lb_buff->fields[FIELD2]);
					time_t f = validateTimeString(g_tempStr, (time_t*)&g_event_finish_epoch, -g_utc_offset);

					if(f)
					{
						g_event_finish_epoch = f;
						ee_mgr.updateEEPROMVar(Event_finish_epoch, (void*)&g_event_finish_epoch);
						sprintf(g_tempStr, "Finish:%lu\n", g_event_finish_epoch);
						lb_send_string(g_tempStr, TRUE);
						startEventUsingRTC();
					}
					else
					{
						sprintf(g_tempStr, "Finish:%lu\n", g_event_finish_epoch);
						doprint = true;
					}
				}
				else if(lb_buff->fields[FIELD1][0] == 'O')
				{
					if(lb_buff->fields[FIELD2][0])
					{
						int8_t offset = (uint16_t)atoi(lb_buff->fields[FIELD2]);

						offset = CLAMP(-24, offset, 24);

						g_utc_offset = offset;
						ee_mgr.updateEEPROMVar(Utc_offset, (void*)&g_utc_offset);
					}

					sprintf(g_tempStr, "Offset:%d\n", g_utc_offset);
					doprint = TRUE;
				}
				else if(lb_buff->fields[FIELD1][0] == 'C')  /* Test only - Set RTC offset value */
				{
					if(lb_buff->fields[FIELD2][0])
					{
						uint16_t c = (uint16_t)atoi(lb_buff->fields[FIELD2]);

						if(c < 512)
						{
							rv3028_set_offset_RAM(c);
						}
					}

					int a = rv3028_get_offset_RAM();
					sprintf(g_tempStr, "C=%d\n", a);
					doprint = true;
				}
				else
				{
					ConfigurationState_t cfg = clockConfigurationCheck();

					if((cfg != WAITING_FOR_START) && (cfg != EVENT_IN_PROGRESS))
					{
						reportConfigErrors();
					}
					else
					{
						reportTimeTill(g_current_epoch, g_event_start_epoch, "Starts in: ", "In progress\n");
						reportTimeTill(g_event_start_epoch, g_event_finish_epoch, "Lasts: ", NULL);
						if(g_event_start_epoch < g_current_epoch)
						{
							reportTimeTill(g_current_epoch, g_event_finish_epoch, "Time Remaining: ", NULL);
						}
					}
				}

				if(doprint)
				{
					lb_send_string(g_tempStr, TRUE);
				}
			}
			break;

			case MESSAGE_UTIL:
			{
				if(lb_buff->fields[FIELD1][0] == 'C')
				{
					if(lb_buff->fields[FIELD2][0])
					{
						int16_t v = atoi(lb_buff->fields[FIELD2]);

						if((v > -2000) && (v < 2000))
						{
							g_atmega_temp_calibration = v;
							ee_mgr.updateEEPROMVar(Atmega_temp_calibration, (void*)&g_atmega_temp_calibration);
						}
					}

					sprintf(g_tempStr, "T Cal= %d\n", g_atmega_temp_calibration);
					lb_send_string(g_tempStr, TRUE);
				}
#if !INIT_EEPROM_ONLY && !SUPPORT_ONLY_80M
					else if(lb_buff->fields[FIELD1][0] == 'Z')
					{
						cli();
						g_AM_enabled = FALSE;
						TIMSK0 |= (1 << OCIE0A);    /* Timer/Counter0 Output Compare Match A Interrupt Enable (CW Tone Output for FM) */
						setAtten(0);
						sei();
					}
#endif /* INIT_EEPROM_ONLY */

				sprintf(g_tempStr, "T=%dC\n", g_temperature);
				lb_send_string(g_tempStr, TRUE);

				sprintf(g_tempStr, "V=%d.%02dV\n", g_voltage / 100, g_voltage % 100);
				lb_send_string(g_tempStr, TRUE);
			}
			break;

			default:
			{
				ee_mgr.send_Help();
			}
			break;
		}

		lb_buff->id = (LBMessageID)MESSAGE_EMPTY;
		lb_send_NewPrompt();

		g_LED_timeout_countdown = LED_TIMEOUT_SECONDS;
		g_config_error = NULL_CONFIG;   /* Trigger a new configuration enunciation */
	}
}

#if !INIT_EEPROM_ONLY

/*
 *   Command set:
 *  *C1 c...c # - Set callsign ID to c...c
 *  *C2 n...n # - Set fox ID and format (0 = beacon, 1 = MOE Classic, ... )
 *  *C3 YYMMDDhhmmss # - Synchronize RTC to date/time = YYMMDDhhmmss
 *  *C4 tttttttt # - Set start date/time = tttttttt
 *  *C5 tttttttt # - Set finish date/time = tttttttt
 *  *C6 [A|B]n...n # - Set UTC offset hours A=ahead(+) B=behind(-)
 *  *C8 nn # - Set HT or 80m fox behavior (affects PTT timing, audio out?, attenuator control?, etc.)
 *  *C9 n # - Set AM modulation frequency n=1 => 1000Hz, n=2 => 900Hz, ... n=6 =>  500Hz
 *  *An # - n=0: stop transmissions, n=1: re-sync and start transmissions, n=3: re-start transmissions from stop
 *  *B nn  - Address the following command to only a specific fox type
 *  *D c...c # - Unlock Arducon (re-enable DTMF commands) where c...c is the password
 *  *D# - Lock Arducon (disables all DTMF commands except *Dc...c#)
 */
	BOOL processDTMFdetection(DTMF_key_t key)
	{
		static KeyprocessState_t state = STATE_SHUTDOWN;
		static int digits;
		static int value;
		static int stringLength;
		static char receivedString[MAX_PATTERN_TEXT_LENGTH + 1] = { '\0' };
		static BOOL setPasswordEnabled = FALSE;
		static unsigned int last_in_progress_ticks = 0;
		BOOL entryError = FALSE;

		g_config_error = NULL_CONFIG;   /* Trigger a new configuration enunciation */

		if(key == 'D')
		{
			g_DTMF_unlocked = FALSE;
			state = STATE_SHUTDOWN;
			return(entryError);
		}

		if(key == NO_KEY)
		{
			if(!g_DTMF_sentence_in_progress_ticks)
			{
				if((last_in_progress_ticks) && (state != STATE_SHUTDOWN))
				{
					entryError = TRUE;
				}

				state = STATE_SHUTDOWN;
			}

			last_in_progress_ticks = g_DTMF_sentence_in_progress_ticks;

			return(entryError);
		}

		g_LED_timeout_countdown = LED_TIMEOUT_SECONDS;

		if(!g_DTMF_unlocked)
		{
			state = STATE_CHECK_PASSWORD;
		}
		else
		{
			if(key == '*')
			{
				state = STATE_SENTENCE_START;
				g_DTMF_sentence_in_progress_ticks = TIMER2_SECONDS_10;
				return(entryError);
			}

			if(key == '#')
			{
				g_DTMF_sentence_in_progress_ticks = 0;
			}
			else if(state != STATE_SHUTDOWN)
			{
				g_DTMF_sentence_in_progress_ticks = TIMER2_SECONDS_10;
			}
		}

		switch(state)
		{
			case STATE_SHUTDOWN:
			{
				entryError = TRUE;
			}
			break;

			case STATE_SENTENCE_START:
			{
				stringLength = 0;
				value = 0;
				digits = 0;

				if(g_DTMF_unlocked)
				{
					if(key == 'A')
					{
						state = STATE_A;
					}
					else if(key == 'B')
					{
						state = STATE_RECEIVING_FOXES_TO_ADDRESS;
					}
					else if(key == 'C')
					{
						state = STATE_C;
					}
					else
					{
						entryError = TRUE;
						g_DTMF_sentence_in_progress_ticks = 0;
						state = STATE_SHUTDOWN;
					}
				}
			}
			break;

			case STATE_A:
			{
				if(key == '0')
				{
					state = STATE_PAUSE_TRANSMISSIONS;
				}
				else if(key == '1')
				{
					state = STATE_START_TRANSMISSIONS;
				}
				else if(key == '2')
				{
					state = STATE_START_TRANSMISSIONS_WITH_RTC;
				}
				else if(key == '3')
				{
					state = STATE_START_TRANSMITTING_NOW;
				}
				else if((key == '8') && setPasswordEnabled)
				{
					state = STATE_SET_PASSWORD;
				}
				else
				{
					entryError = TRUE;
					g_DTMF_sentence_in_progress_ticks = 0;
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_RECEIVING_FOXES_TO_ADDRESS:
			{
				if((key >= '0') && (key <= '9'))
				{
					value *= 10;
					value += key - '0';
					digits++;
				}

				if(digits >= 2)
				{
					state = STATE_SHUTDOWN;
					Fox_t holdFox = (Fox_t)value;

					if(holdFox == g_fox)
					{
						state = STATE_SENTENCE_START;
					}
				}
			}
			break;

			case STATE_PAUSE_TRANSMISSIONS:
			{
				if(key == '#')
				{
					stopEventNow(PROGRAMMATIC);
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_START_TRANSMISSIONS:
			{
				if(key == '#')
				{
					startEventNow(PROGRAMMATIC);
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_START_TRANSMISSIONS_WITH_RTC:
			{
				if(key == '#')
				{
					startEventUsingRTC();
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_START_TRANSMITTING_NOW:
			{
				if(key == '#')
				{
					setupForFox(NULL, START_TRANSMISSIONS_NOW);
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_C:
			{
				if(key == '1')
				{
					state = STATE_RECEIVING_CALLSIGN;
				}
				else if(key == '2')
				{
					state = STATE_RECEIVING_FOXFORMATANDID;
				}
				else if(key == '3') /* *C7YYMMDDhhmmss# Set RTC to this time and date */
				{
					state = STATE_RECEIVING_SET_CLOCK;
				}
				else if(key == '4')
				{
					state = STATE_RECEIVING_START_TIME;
				}
				else if(key == '5')
				{
					state = STATE_RECEIVING_FINISH_TIME;
				}
				else if(key == '6')
				{
					state = STATE_RECEIVING_UTC_OFFSET;
					digits = 1;
				}
#if !SUPPORT_ONLY_80M
					else if(key == '9')
					{
						state = STATE_SET_AM_TONE_FREQUENCY;
					}
#endif  /* !SUPPORT_ONLY_80M */
				else if(key == 'A')
				{
					state = STATE_SET_PTT_PERIODIC_RESET;
				}
				else if(key == 'B')
				{
					state = STATE_GET_BATTERY_VOLTAGE;
				}
				else
				{
					entryError = TRUE;
					g_DTMF_sentence_in_progress_ticks = 0;
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_SET_PASSWORD:
			{
				if(key == '#')
				{
					if(stringLength >= MIN_UNLOCK_CODE_LENGTH)
					{
						strcpy((char*)g_unlockCode, receivedString);
						ee_mgr.updateEEPROMVar(UnlockCode, (void*)g_unlockCode);
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					if(stringLength <= MAX_UNLOCK_CODE_LENGTH)
					{
						receivedString[stringLength++] = key;
						receivedString[stringLength] = '\0';
					}
				}
				else
				{
					entryError = TRUE;
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_RECEIVING_CALLSIGN:
			{
				if(key == '#')
				{
					if(strlen(receivedString) <= MAX_PATTERN_TEXT_LENGTH)
					{
						strcpy((char*)g_messages_text[STATION_ID], receivedString);
						ee_mgr.updateEEPROMVar(StationID_text, (void*)g_messages_text[STATION_ID]);
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					if(digits)
					{
						value = value * 10 + (key - '0');
						if((value < value2Morse(0x7F)) && (stringLength < MAX_PATTERN_TEXT_LENGTH))
						{
							receivedString[stringLength] = value2Morse(value);
							stringLength++;
							receivedString[stringLength] = '\0';
						}

						digits = 0;
					}
					else
					{
						value = key - '0';
						digits = 1;
					}
				}
				else
				{
					entryError = TRUE;
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_RECEIVING_FOXFORMATANDID:
			{
				if(key == '#')
				{
					if((value >= BEACON) && (value < INVALID_FOX))
					{
						Fox_t holdFox = (Fox_t)value;
						ee_mgr.updateEEPROMVar(Fox_setting, (void*)&holdFox);
						if(holdFox != g_fox)
						{
							setupForFox(&holdFox, START_NOTHING);
						}
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					value *= 10;
					value += key - '0';
				}
			}
			break;

			case STATE_RECEIVING_SET_CLOCK:
			{
				if(key == '#')
				{
					time_t t = validateTimeString(receivedString, (time_t*)&g_current_epoch, -g_utc_offset);

					if(t)
					{
						rv3028_set_epoch(t);
						g_current_epoch = t;
						setupForFox(NULL, START_NOTHING);   /* Avoid timing problems if an event is already active */
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					if(stringLength < MAX_DTMF_ARG_LENGTH)
					{
						receivedString[stringLength] = key;
						stringLength++;
						receivedString[stringLength] = '\0';
					}
				}
			}
			break;

			case STATE_RECEIVING_START_TIME:
			{
				if(key == '#')
				{
					time_t s = validateTimeString(receivedString, (time_t*)&g_event_start_epoch, -g_utc_offset);

					if(s)
					{
						g_event_start_epoch = s;
						ee_mgr.updateEEPROMVar(Event_start_epoch, (void*)&g_event_start_epoch);
						setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					if(stringLength < MAX_DTMF_ARG_LENGTH)
					{
						receivedString[stringLength] = key;
						stringLength++;
						receivedString[stringLength] = '\0';
					}
				}
			}
			break;

			case STATE_RECEIVING_FINISH_TIME:
			{
				if(key == '#')
				{
					time_t f = validateTimeString(receivedString, (time_t*)&g_event_finish_epoch, -g_utc_offset);

					if(f)
					{
						g_event_finish_epoch = f;
						ee_mgr.updateEEPROMVar(Event_finish_epoch, (void*)&g_event_finish_epoch);
						setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					if(stringLength < MAX_DTMF_ARG_LENGTH)
					{
						receivedString[stringLength] = key;
						stringLength++;
						receivedString[stringLength] = '\0';
					}
				}
			}
			break;

			case STATE_RECEIVING_UTC_OFFSET:
			{
				if(key == '#')
				{
					if((value >= 0) && (value < 24))
					{
						int8_t hold = value * digits;
						ee_mgr.updateEEPROMVar(Utc_offset, (void*)&hold);
						g_utc_offset = hold;
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					value *= 10;
					value += key - '0';
				}
				else if(key == 'B')
				{
					digits = -1;
				}
			}
			break;

#if !SUPPORT_ONLY_80M
					case STATE_SET_AM_TONE_FREQUENCY:
					{
						if(key == '#')
						{
							g_AM_audio_frequency = (AM_Tone_Freq_t)value;
							setAMToneFrequency(g_AM_audio_frequency);
							ee_mgr.updateEEPROMVar(Am_audio_frequency, (void*)&g_AM_audio_frequency);

							state = STATE_SHUTDOWN;
						}
						else if((key >= MIN_AM_TONE_FREQUENCY) && (key <= MAX_AM_TONE_FREQUENCY))
						{
							value = key - '0';
						}
					}
					break;
#endif  /* !SUPPORT_ONLY_80M */

			case STATE_SET_PTT_PERIODIC_RESET:
			{
				if(key == '#')
				{
					if((value >= 0) && (value <= 1))
					{
						g_ptt_periodic_reset_enabled = (uint8_t)value;
						g_use_ptt_periodic_reset = g_ptt_periodic_reset_enabled;
						ee_mgr.updateEEPROMVar(Ptt_periodic_reset, (void*)&g_ptt_periodic_reset_enabled);
					}
					else
					{
						entryError = TRUE;
					}

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '1'))
				{
					value = key - '0';
				}
			}
			break;

			case STATE_GET_BATTERY_VOLTAGE:
			{
				if(key == '#')
				{
					Fox_t f = REPORT_BATTERY;
					setupForFox(&f, START_TRANSMISSIONS_NOW);
				}
				else
				{
					entryError = TRUE;
				}

				state = STATE_SHUTDOWN;
			}
			break;

			case STATE_CHECK_PASSWORD:
			{
				if(g_unlockCode[digits++] == key)
				{
					if((size_t)digits == strlen((char*)g_unlockCode))
					{
						g_DTMF_unlocked = TRUE;
						setPasswordEnabled = TRUE;
					}
				}
				else
				{
					g_DTMF_sentence_in_progress_ticks = 0;
					digits = 0;
					state = STATE_SHUTDOWN;
					entryError = TRUE;
				}
			}
			break;

#if !SUPPORT_ONLY_80M
					case STATE_TEST_ATTENUATOR:
					{
						if(key == '#')
						{
							if(value == 0)
							{
								setAtten(0);
								g_sendAMmodulationConstantly = TRUE;
							}
							else if(value > 315)
							{
								g_sendAMmodulationConstantly = FALSE;
								setAtten(315);
							}
							else
							{
								setAtten(value);
							}

							state = STATE_SHUTDOWN;
						}
						else if((key >= '0') && (key <= '9'))
						{
							value *= 10;
							value += key - '0';
						}
						else
						{
							g_DTMF_sentence_in_progress_ticks = 0;
							state = STATE_SHUTDOWN;
							entryError = TRUE;
						}
					}
					break;
#endif  /* !SUPPORT_ONLY_80M */
		}

		return(entryError);
	}

#endif  /* #if !INIT_EEPROM_ONLY */


void setupForFox(Fox_t* fox, EventAction_t action)
{
	if(fox)
	{
		if(*fox != INVALID_FOX)
		{
			g_fox = *fox;
		}
	}

	g_current_epoch = rv3028_get_epoch();
	g_use_ptt_periodic_reset = FALSE;

	cli();

	switch(g_fox)
	{
		case FOX_1:
		case FOX_2:
		case FOX_3:
		case FOX_4:
		case FOX_5:
		{
			g_on_air_interval_seconds = 60;
			g_cycle_period_seconds = 300;
			g_number_of_foxes = 5;
			g_fox_id_offset = 0;
			g_pattern_codespeed = 8;
			g_id_interval_seconds = 300;
		}
		break;

		case SPRINT_S1:
		case SPRINT_S2:
		case SPRINT_S3:
		case SPRINT_S4:
		case SPRINT_S5:
		{
			g_on_air_interval_seconds = 12;
			g_cycle_period_seconds = 60;
			g_number_of_foxes = 5;
			g_pattern_codespeed = SPRINT_SLOW_CODE_SPEED;
			g_fox_id_offset = SPRINT_S1 - 1;
			g_id_interval_seconds = 600;
		}
		break;

		case SPRINT_F1:
		case SPRINT_F2:
		case SPRINT_F3:
		case SPRINT_F4:
		case SPRINT_F5:
		{
			g_on_air_interval_seconds = 12;
			g_cycle_period_seconds = 60;
			g_number_of_foxes = 5;
			g_pattern_codespeed = SPRINT_FAST_CODE_SPEED;
			g_fox_id_offset = SPRINT_F1 - 1;
			g_id_interval_seconds = 600;
		}
		break;

		case REPORT_BATTERY:
		{
			g_on_air_interval_seconds = 30;
			g_cycle_period_seconds = 60;
			g_number_of_foxes = 2;
			g_pattern_codespeed = SPRINT_SLOW_CODE_SPEED;
			g_fox_id_offset = REPORT_BATTERY - 1;
			g_id_interval_seconds = 60;
		}
		break;


		/* case BEACON:
		 * case SPECTATOR: */
		default:
		{
			g_use_ptt_periodic_reset = g_ptt_periodic_reset_enabled;
			g_number_of_foxes = 1;
			g_pattern_codespeed = 8;
			g_id_interval_seconds = g_ptt_periodic_reset_enabled ? 60 : 600;
			g_on_air_interval_seconds = g_id_interval_seconds;
			g_fox_id_offset = 0;
			g_cycle_period_seconds = g_id_interval_seconds;
		}
		break;
	}

	if(action == START_NOTHING)
	{
		g_use_rtc_for_startstop = FALSE;
		g_transmissions_disabled = TRUE;
	}
	else if(action == START_EVENT_NOW)
	{
		g_fox_counter = 1;
		g_seconds_since_sync = 0;                                               /* Total elapsed time since synchronization */
		g_use_rtc_for_startstop = FALSE;
		g_transmissions_disabled = FALSE;
	}
	else if(action == START_TRANSMISSIONS_NOW)                                  /* Immediately start transmitting, regardless RTC or time slot */
	{
		g_fox_counter = MAX(1, g_fox - g_fox_id_offset);
		g_seconds_since_sync = (g_fox_counter - 1) * g_on_air_interval_seconds; /* Total elapsed time since start of event */
		g_use_rtc_for_startstop = FALSE;
		g_transmissions_disabled = FALSE;
		g_initialize_fox_transmissions = INIT_EVENT_STARTING_NOW;
	}
	else                                                                    /* if(action == START_EVENT_WITH_STARTFINISH_TIMES) */
	{
		if(g_event_start_epoch < g_current_epoch)                           /* timed event in progress */
		{
			g_seconds_since_sync = g_current_epoch - g_event_start_epoch;   /* Total elapsed time counter: synced at event start time */
			g_fox_counter = CLAMP(1, 1 + ((g_seconds_since_sync % g_cycle_period_seconds) / g_on_air_interval_seconds), g_number_of_foxes);
			g_initialize_fox_transmissions = INIT_EVENT_IN_PROGRESS_WITH_STARTFINISH_TIMES;
		}
		else                                                                /* event starts in the future */
		{
			g_seconds_since_sync = 0;                                       /* Total elapsed time counter */
			g_fox_counter = 1;
		}

		g_use_rtc_for_startstop = TRUE;
		g_transmissions_disabled = TRUE;
	}

	sendMorseTone(OFF);
	g_code_throttle    = 0;                 /* Adjusts Morse code speed */
	g_on_the_air       = FALSE;             /* Controls transmitter Morse activity */

	g_config_error = NULL_CONFIG;           /* Trigger a new configuration enunciation */
	digitalWrite(PIN_LED, OFF);             /*  LED Off - in case it was left on */

	digitalWrite(PIN_CW_KEY_LOGIC, OFF);    /* TX key line */
	g_sendAMmodulation = FALSE;
	g_LED_enunciating = FALSE;
	g_config_error = NULL_CONFIG;           /* Trigger a new configuration enunciation */

	sei();
}


/*
 *  Read the temperature ADC value
 */
uint16_t readADC()
{
	/* Make sure the most recent ADC read is complete. */
	uint16_t c = MAX_UINT16;

	while((ADCSRA & (1 << ADSC)) && c)
	{
		c--;    /* Just wait for ADC to finish. */
	}
	uint16_t result = ADCW;
	/* Initiate another reading. */
	ADCSRA |= (1 << ADSC);
	return( result);
}

/*
 *  Returns the most recent temperature reading
 */
float getTemp(void)
{
	float offset = CLAMP(-200., (float)g_atmega_temp_calibration / 10., 200.);

	/* The offset in 1/10ths C (first term) was determined empirically */
	readADC();  /* throw away first reading */
	return(roundf(offset + (readADC() - 324.31) / 1.22));
}

uint16_t getVoltage(void)
{
	readADC();  /* throw away first reading */
	uint16_t hold = readADC();
	hold = (uint16_t)(((uint32_t)hold * 237) >> 7);
	return(hold);
}

void setUpSampling(ADCChannel_t channel, BOOL enableSampling)
{
	ADCSRA = 0; /* clear ADCSRA register */
	ADCSRB = 0; /* clear ADCSRB register */
	ADMUX = 0;

	if(channel == AUDIO_SAMPLING)
	{
		ADMUX |= 0x06;                          /* set A6 analog input pin */
		ADMUX |= (1 << REFS1) | (1 << REFS0);   /* set reference voltage to internal 1.1V */
		ADMUX |= (1 << ADLAR);                  /* left align ADC value to 8 bits from ADCH register */

		/* Sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
		 *  for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles */

#if F_CPU == 16000000UL
#if SAMPLE_RATE == 154080
				ADCSRA |= (1 << ADPS1) | (1 << ADPS0);                  /* 8 prescaler for 153800 sps */
#elif SAMPLE_RATE == 77040
				ADCSRA |= (1 << ADPS2);                                 /* 16 prescaler for 76900 sps */
#elif SAMPLE_RATE == 38520
				ADCSRA |= (1 << ADPS2) | (1 << ADPS0);                  /* 32 prescaler for 38500 sps */
#elif SAMPLE_RATE == 19260
				ADCSRA |= (1 << ADPS2) | (1 << ADPS1);                  /* 64 prescaler for 19250 sps */
#elif SAMPLE_RATE == 9630
				ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);   /* 128 prescaler for 9630 sps */
#else
#error "Select a valid sample rate."
#endif
#else
#if SAMPLE_RATE == 154080
				ADCSRA |= (1 << ADPS1);                 /* 4 prescaler for 153800 sps */
#elif SAMPLE_RATE == 77040
				ADCSRA |= (1 << ADPS1) | (1 << ADPS0);  /* 8 prescaler for 76900 sps */
#elif SAMPLE_RATE == 38520
				ADCSRA |= (1 << ADPS2);                 /* 16 prescaler for 38500 sps */
#elif SAMPLE_RATE == 19260
				ADCSRA |= (1 << ADPS2) | (1 << ADPS0);  /* 32 prescaler for 19260 sps */
#elif SAMPLE_RATE == 9630
				ADCSRA |= (1 << ADPS2) | (1 << ADPS1);  /* 64 prescaler for 9630 sps */
#else
#error "Select a valid sample rate."
#endif
#endif /* F_CPU == 16000000 */

		ADCSRA |= (1 << ADATE);     /* enable auto trigger */
		ADCSRA |= (1 << ADIE);      /* enable interrupts when measurement complete */
		ADCSRA |= (1 << ADEN);      /* enable ADC */

		if(enableSampling)
		{
			ADCSRA |= (1 << ADIE);  /* enable interrupts when measurement complete */
			ADCSRA |= (1 << ADSC);  /* start ADC measurements */
		}
	}
	else
	{
		/* The internal temperature has to be used
		 *  with the internal reference of 1.1V.
		 *  Channel 8 can not be selected with
		 *  the analogRead function yet. */
		/* Set the internal reference and mux. */
		ADMUX |= ((1 << REFS1) | (1 << REFS0));

		if(channel == TEMPERATURE_SAMPLING)
		{
			ADMUX |= (1 << MUX3);
		}
		else    /* channel == VOLTAGE_SAMPLING */
		{
			ADMUX |= (1 << MUX2) | (1 << MUX1) | (1 << MUX0);
		}

		/* Slow the ADC clock down to 125 KHz
		 *  by dividing by 128. Assumes that the
		 *  standard Arduino 16 MHz clock is in use. */
		ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
		ADCSRA |= (1 << ADEN);  /* enable the ADC */
		ADCSRA |= (1 << ADSC);  /* Start the ADC */
		readADC();
	}
}

BOOL only_digits(char *s)
{
	while(*s)
	{
		if(isdigit(*s++) == 0)
		{
			return( FALSE);
		}
	}

	return( TRUE);
}


void startEventNow(EventActionSource_t activationSource)
{
	ConfigurationState_t conf = clockConfigurationCheck();

	cli();
	if(activationSource == POWER_UP)
	{
		if(conf == CONFIGURATION_ERROR)
		{
			setupForFox(NULL, START_NOTHING);
		}
		else
		{
			setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);
		}
	}
	else if(activationSource == PROGRAMMATIC)
	{
		if(conf == CONFIGURATION_ERROR)                                                                                             /* Start immediately */
		{
			setupForFox(NULL, START_EVENT_NOW);
		}
		else if((conf == WAITING_FOR_START) || (conf == SCHEDULED_EVENT_WILL_NEVER_RUN) || (conf == SCHEDULED_EVENT_DID_NOT_START)) /* Start immediately */
		{
			setupForFox(NULL, START_EVENT_NOW);
		}
		else                                                                                                                        /*if((conf == EVENT_IN_PROGRESS) */
		{
			setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);                                                                  /* Let the RTC start the event */
		}
	}
	else                                                                                                                            /* PUSHBUTTON */
	{
		if(conf == CONFIGURATION_ERROR)                                                                                             /* No scheduled event */
		{
			setupForFox(NULL, START_EVENT_NOW);
		}
		else                                                                                                                        /* if(buttonActivated) */
		{
			if(conf == WAITING_FOR_START)
			{
				setupForFox(NULL, START_TRANSMISSIONS_NOW);                                                                         /* Start transmitting! */
			}
			else if(conf == SCHEDULED_EVENT_WILL_NEVER_RUN)
			{
				setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);                                                              /* rtc starts the event */
			}
			else                                                                                                                    /* Event should be running now */
			{
				setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);                                                              /* start the running event */
			}
		}
	}

	g_LED_enunciating = FALSE;
	sei();

	/*	g_current_epoch = rv3028_get_epoch();
	 *     lb_send_string((char*)"Sync OK\n", FALSE); */
}

void stopEventNow(EventActionSource_t activationSource)
{
	ConfigurationState_t conf = clockConfigurationCheck();

	cli();

	if(activationSource == PROGRAMMATIC)
	{
		setupForFox(NULL, START_NOTHING);
	}
	else    /* if(activationSource == PUSHBUTTON) */
	{
		if(conf == WAITING_FOR_START)
		{
			setupForFox(NULL, START_TRANSMISSIONS_NOW);
		}
		if(conf == SCHEDULED_EVENT_WILL_NEVER_RUN)
		{
			setupForFox(NULL, START_NOTHING);
		}
		else    /*if(conf == CONFIGURATION_ERROR) */
		{
			setupForFox(NULL, START_NOTHING);
		}
	}

	if(g_sync_pin_stable == STABLE_LOW)
	{
		digitalWrite(PIN_LED, OFF); /*  LED Off */
	}
}

void startEventUsingRTC(void)
{
	g_current_epoch = rv3028_get_epoch();
	ConfigurationState_t state = clockConfigurationCheck();

	if(state != CONFIGURATION_ERROR)
	{
		setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);
		reportTimeTill(g_current_epoch, g_event_start_epoch, "Starts in: ", "In progress\n");

		if(g_event_start_epoch < g_current_epoch)
		{
			reportTimeTill(g_current_epoch, g_event_finish_epoch, "Time Remaining: ", NULL);
		}
		else
		{
			reportTimeTill(g_event_start_epoch, g_event_finish_epoch, "Lasts: ", NULL);
		}
	}
	else
	{
		reportConfigErrors();
	}
}

void reportConfigErrors(void)
{
	g_current_epoch = rv3028_get_epoch();

	if(g_messages_text[STATION_ID][0] == '\0')
	{
		ee_mgr.sendEEPROMString(TextSetID);
	}

	if(g_current_epoch < MINIMUM_EPOCH) /* Current time is invalid */
	{
		ee_mgr.sendEEPROMString(TextSetTime);
	}

	if(g_event_finish_epoch < g_current_epoch)      /* Event has already finished */
	{
		if(g_event_start_epoch < g_current_epoch)   /* Event has already started */
		{
			ee_mgr.sendEEPROMString(TextSetStart);
		}

		ee_mgr.sendEEPROMString(TextSetFinish);
	}
	else if(g_event_start_epoch < g_current_epoch)  /* Event has already started */
	{
		if(g_event_start_epoch < MINIMUM_EPOCH)     /* Start in invalid */
		{
			ee_mgr.sendEEPROMString(TextSetStart);
		}
		else
		{
			lb_send_string((char*)"Event running...\n", TRUE);
		}
	}
}

BOOL reportTimeTill(time_t from, time_t until, const char* prefix, const char* failMsg)
{
	BOOL failure = FALSE;

	if(from >= until)   /* Negative time */
	{
		failure = TRUE;
		if(failMsg)
		{
			lb_send_string((char*)failMsg, TRUE);
		}
	}
	else
	{
		if(prefix)
		{
			lb_send_string((char*)prefix, TRUE);
		}
		time_t dif = until - from;
		uint16_t years = dif / YEAR;
		time_t hold = dif - (years * YEAR);
		uint16_t days = hold / DAY;
		hold -= (days * DAY);
		uint16_t hours = hold / HOUR;
		hold -= (hours * HOUR);
		uint16_t minutes = hold / MINUTE;
		uint16_t seconds = hold - (minutes * MINUTE);

		g_tempStr[0] = '\0';

		if(years)
		{
			sprintf(g_tempStr, "%d yrs ", years);
			lb_send_string(g_tempStr, TRUE);
		}

		if(days)
		{
			sprintf(g_tempStr, "%d days ", days);
			lb_send_string(g_tempStr, TRUE);
		}

		if(hours)
		{
			sprintf(g_tempStr, "%d hrs ", hours);
			lb_send_string(g_tempStr, TRUE);
		}

		if(minutes)
		{
			sprintf(g_tempStr, "%d min ", minutes);
			lb_send_string(g_tempStr, TRUE);
		}

		sprintf(g_tempStr, "%d sec", seconds);
		lb_send_string(g_tempStr, TRUE);

		lb_send_NewLine();
		g_tempStr[0] = '\0';
	}

	return( failure);
}

time_t validateTimeString(char* str, time_t * epicVar, int8_t offsetHours)
{
	time_t valid = 0;
	int len = strlen(str);
	time_t minimumEpoch = MINIMUM_EPOCH;
	uint8_t validationType = 0;

	if(epicVar == &g_event_start_epoch)
	{
		minimumEpoch = MAX(g_current_epoch, MINIMUM_EPOCH);
		validationType = 1;
	}
	else if(epicVar == &g_event_finish_epoch)
	{
		minimumEpoch = MAX(g_event_start_epoch, g_current_epoch);
		validationType = 2;
	}

	if((len == 12) && (only_digits(str)))
	{
		time_t ep = rv3028_get_epoch(NULL, str);    /* String format "YYMMDDhhmmss" */

		ep += (HOUR * offsetHours);

		if(ep > minimumEpoch)
		{
			valid = ep;
		}
		else
		{
			if(validationType == 1)         /* start time validation */
			{
				ee_mgr.sendEEPROMString(TextErrStartInPast);
			}
			else if(validationType == 2)    /* finish time validation */
			{
				if(ep < g_current_epoch)
				{
					ee_mgr.sendEEPROMString(TextErrFinishInPast);
				}
				else
				{
					ee_mgr.sendEEPROMString(TextErrFinishB4Start);
				}
			}
			else    /* current time validation */
			{
				ee_mgr.sendEEPROMString(TextErrTimeInPast);
			}
		}
	}
	else if(len)
	{
		ee_mgr.sendEEPROMString(TextErrInvalidTime);
	}

	return(valid);
}

BOOL setAMToneFrequency(AM_Tone_Freq_t value)
{
	BOOL enableAM = TRUE;

#if INIT_EEPROM_ONLY
	if(value) enableAM = FALSE; /* Remove compiler warning */
#else
	switch(value)
	{
		case AM_DISABLED:
		{
			enableAM = FALSE;
		}
		break;

		case AM_900Hz:
		{
#if F_CPU == 16000000UL
				OCR1A = 556;    /* For ~900 Hz tone output */
#else
				OCR1A = 278;
#endif
		}
		break;

		case AM_800Hz:
		{
#if F_CPU == 16000000UL
				OCR1A = 625;    /* For ~800 Hz tone output */
#else
				OCR1A = 312;
#endif
		}
		break;

		case AM_700Hz:
		{
#if F_CPU == 16000000UL
				OCR1A = 714;    /* For ~700 Hz tone output */
#else
				OCR1A = 357;
#endif
		}
		break;

		case AM_600Hz:
		{
#if F_CPU == 16000000UL
				OCR1A = 833;    /* For ~600 Hz tone output */
#else
				OCR1A = 416;
#endif
		}
		break;

		case AM_500Hz:
		{
#if F_CPU == 16000000UL
				OCR1A = 1000;   /* For ~500 Hz tone output */
#else
				OCR1A = 500;
#endif
		}
		break;

		case AM_1000Hz:
		default:
		{
#if F_CPU == 16000000UL
				OCR1A = 500;    /* For ~1000 Hz tone output */
#else
				OCR1A = 250;
#endif
		}
		break;
	}

	if(!OCR0A)
	{
		OCR0A = DEFAULT_TONE_FREQUENCY; /* Ensure that FM tone setting is initialized - even if it won't be used */

	}

 	if(!OCR1A)
 	{
		OCR1A = 1000;  /* Ensure that AM tone setting is initialized - even if it won't be used */
 	}

	cli();
	setupPortsForF1975(enableAM);

	if(enableAM)
	{
		TIMSK0 &= ~(1 << OCIE0A);   /* Timer/Counter0 Output Compare Match A Interrupt Disable (CW Tone Output for FM) */
		TIMSK1 |= (1 << OCIE1A);    /* Timer/Counter1 Output Compare Match A Interrupt Enable (CW Tone Output for AM) */
	}
	else
	{
		TIMSK0 |= (1 << OCIE0A);    /* Timer/Counter0 Output Compare Match A Interrupt Enable (CW Tone Output for FM) */
		TIMSK1 &= ~(1 << OCIE1A);   /* Timer/Counter1 Output Compare Match A Interrupt Disable (CW Tone Output for AM) */
	}

	g_AM_enabled = enableAM;
	sei();
#endif  /* INIT_EEPROM_ONLY */
	return(enableAM);
}


char value2Morse(char value)
{
	char morse = ' ';

	if(value == 0x7F)
	{
		return( 39);    /* Return the maximum value that will be accepted + 1 */

	}

	if((value >= 1) && (value <= 26))
	{
		morse = 'A' + value - 1;
	}
	else if((value >= 29) && (value <= 38))
	{
		morse = '0' + value - 29;
	}
	else if(value == 27)
	{
		morse = '<';
	}
	else if(value == 28)
	{
		morse = '/';
	}

	return( morse);
}

DTMF_key_t value2DTMFKey(uint8_t value)
{
	DTMF_key_t key = NO_KEY;

	if(value <= 2)
	{
		key = (DTMF_key_t)('1' + value);
	}
	else if((value >= 4) && (value <= 6))
	{
		key = (DTMF_key_t)('0' + value);
	}
	else if((value >= 8) && (value <= 10))
	{
		key = (DTMF_key_t)('/' + value);
	}
	else if(value == 3)
	{
		key = A_KEY;
	}
	else if(value == 7)
	{
		key = B_KEY;
	}
	else if(value == 11)
	{
		key = C_KEY;
	}
	else if(value == 12)
	{
		key = STAR_KEY;
	}
	else if(value == 13)
	{
		key = ZERO_KEY;
	}
	else if(value == 14)
	{
		key = POUND_KEY;
	}
	else if(value == 15)
	{
		key = D_KEY;
	}

	return( key);
}

/**
 *   Converts an epoch (seconds since 1900) and converts it into a string of format "yyyy-mm-ddThh:mm:ssZ containing UTC"
 *
 *  #define THIRTY_YEARS 946080000
 *  char* convertEpochToTimeString(unsigned long epoch)
 *  {
 *  struct tm  ts;
 *
 *  if (epoch < MINIMUM_EPOCH)
 *  {
 *       g_tempStr[0] = '\0';
 *   return g_tempStr;
 *  }
 *
 *  time_t e = (time_t)epoch - THIRTY_YEARS;
 *  // Format time, "ddd yyyy-mm-ddThh:mm:ss"
 *  ts = *localtime(&e);
 *  strftime(g_tempStr, sizeof(g_tempStr), "%Y-%m-%dT%H:%M:%SZ", &ts);
 *
 *  return g_tempStr;
 *  }
 */
