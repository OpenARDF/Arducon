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

EepromManager ee_mgr;

/*#define SAMPLE_RATE 9630 */
#define SAMPLE_RATE 19260
/*#define SAMPLE_RATE 38520
 *#define SAMPLE_RATE 77040
 *#define SAMPLE_RATE 154080 */

volatile int32_t g_seconds_since_sync = 0;  /* Total number of seconds since an active event was synchronized (started) */
volatile int32_t g_seconds_since_powerup = 0;
volatile Fox_t g_fox          = BEACON;     /* Sets Fox number not set by ISR. Set in startup and checked in main. */
volatile int g_active           = 0;        /* Disable active. set and clear in ISR. Checked in main. */

volatile BOOL g_on_the_air       = 0;       /* Used to indicate and control transmissions */
volatile int g_code_throttle    = 0;        /* Adjusts Morse code speed */

#if !INIT_EEPROM_ONLY
	const char g_morsePatterns[][5] = { "MO ", "MOE ", "MOI ", "MOS ", "MOH ", "MO5 ", "", "5", "S", "ME", "MI", "MS", "MH", "M5", "OE", "OI", "OS", "OH", "O5" };
#endif /* !INIT_EEPROM_ONLY */

volatile BOOL g_callsign_sent = TRUE;

volatile BOOL g_blinky_time = FALSE;
volatile BOOL g_LED_enunciating = FALSE;

volatile time_t g_current_epoch = 0;
volatile time_t g_event_start_epoch = 0;
volatile time_t g_event_finish_epoch = 0;
volatile int8_t g_utc_offset = 0;

volatile BOOL g_sendAMmodulation = FALSE;
volatile uint8_t g_AM_audio_frequency;
volatile BOOL g_AM_enabled = TRUE;
volatile BOOL g_sendAMmodulationConstantly = FALSE;
uint8_t g_dataModulation[SIZE_OF_DATA_MODULATION];

volatile BOOL g_transmissions_disabled = TRUE;
volatile int g_on_air_interval = 0;
volatile int g_cycle_period_seconds = 0;
volatile int g_fox_seconds_into_interval = 0;
volatile int g_fox_counter = 1;
volatile int g_number_of_foxes = 0;
volatile BOOL g_fox_transition = FALSE;
volatile int g_fox_id_offset = 0;   /* Used to handle fast and slow foxes in Sprint without unnecessary software complexity */
volatile int g_id_interval = 0;
volatile BOOL g_time_to_ID = FALSE;
volatile int g_fox_tone_offset = 1; /* Used to provide slightly different audio tones for different foxes for playback through the speaker */

volatile BOOL g_audio_tone_state = FALSE;
volatile int16_t g_sync_pin_timer = 0;
volatile ButtonStability_t g_sync_pin_stable = UNSTABLE;

volatile BOOL g_dtmf_detected = FALSE;
volatile uint8_t g_unlockCode[MAX_UNLOCK_CODE_LENGTH + 1];
int8_t g_temperature = 0;
uint16_t g_voltage = 0;
volatile ConfigurationState_t g_config_error = NULL_CONFIG;
volatile BOOL g_use_rtc_for_startstop = FALSE;

volatile BOOL g_DTMF_unlocked = TRUE;
volatile BOOL g_reset_button_held = FALSE;
volatile BOOL g_perform_EEPROM_reset = FALSE;

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
volatile uint8_t g_enable_transmitter = 1;
volatile uint8_t g_temperature_check_countdown = 0;
volatile uint8_t g_voltage_check_countdown = 0;
volatile int16_t g_rv3028_offset = EEPROM_RV3028_OFFSET_DEFAULT;

#define _N 201

#if !INIT_EEPROM_ONLY
	const int N = _N;
	const float threshold = 500000. * (_N / 100);
	const float sampling_freq = SAMPLE_RATE;
	const float x_frequencies[4] = { 1209., 1336., 1477., 1633. };
	const float y_frequencies[4] = { 697., 770., 852., 941. };
	const float mid_frequencies[7] = { 734., 811., 897., 1075., 1273., 1407., 1555. };
	const char key[16] = { '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D' };
	const int numKeys = sizeof(key) / sizeof(key[0]);
	const char keyMorse[39] = { ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
								'V', 'W', 'X', 'Y', 'Z', '<', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
	const int numMorseChars = sizeof(keyMorse) / sizeof(keyMorse[0]);
#endif  /* !INIT_EEPROM_ONLY */


char g_lastKey = '\0';
unsigned long g_last = 0;
unsigned long g_tick_count = 0;

#if !INIT_EEPROM_ONLY
	Goertzel g_goertzel(N, sampling_freq);
#endif  /* INIT_EEPROM_ONLY */

char g_tempStr[TEMP_STRING_LENGTH] = { '\0' };

/*
 * Local function prototypes
 */
void handleLinkBusMsgs(void);
void sendMorseTone(BOOL onOff);
void playStartingTone(uint8_t toneFreq);
void setupForFox(Fox_t* fox, EventAction_t action);
void setAMToneFrequency(uint8_t value);
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

#if !INIT_EEPROM_ONLY
	void processKey(char key);
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

	pinMode(PIN_LED1, OUTPUT);              /* This led blinks when off cycle and blinks with code when on cycle. */
	digitalWrite(PIN_LED1, OFF);

	pinMode(PIN_LED2, OUTPUT);
	digitalWrite(PIN_LED2, OFF);

	pinMode(PIN_CW_KEY_LOGIC, OUTPUT);  /* This pin is used to control the KEY line to the transmitter only active on cycle. */
	digitalWrite(PIN_CW_KEY_LOGIC, OFF);

	pinMode(PIN_CW_TONE_LOGIC, OUTPUT);
	digitalWrite(PIN_CW_TONE_LOGIC, OFF);

	pinMode(PIN_PTT_LOGIC, OUTPUT);
	digitalWrite(PIN_PTT_LOGIC, OFF);

	pinMode(PIN_AUDIO_INPUT, INPUT);    /* Receiver Audio sampling */
	pinMode(PIN_BATTERY_LEVEL, INPUT);  /* Battery voltage level */

	pinMode(PIN_MISO, INPUT_PULLUP);
	pinMode(PIN_MOSI, OUTPUT);
	digitalWrite(PIN_MOSI, OFF);

	linkbus_disable();

/*	pinMode(PIN_D0, OUTPUT);
 *	pinMode(PIN_D1, OUTPUT);
 *	pinMode(PIN_D2, OUTPUT);
 *	pinMode(PIN_D3, OUTPUT);
 *	pinMode(PIN_D4, OUTPUT);    / * Also RXD * /
 *	pinMode(PIN_D5, OUTPUT);    / * Also TXD * / */

/*	digitalWrite(PIN_D0, OFF);
 *	digitalWrite(PIN_D1, OFF);
 *	digitalWrite(PIN_D2, OFF);
 *	digitalWrite(PIN_D3, OFF);
 *	digitalWrite(PIN_D4, OFF);
 *	digitalWrite(PIN_D5, OFF); */
	DDRC |= 0x0F;
	PORTC &= 0xF0;
	DDRD |= 0x03;
	PORTD &= 0xFC;

	pinMode(PIN_PWDN, OUTPUT);
	digitalWrite(PIN_PWDN, ON);

	/* Set unused pins as inputs pulled high */
	pinMode(A4, INPUT_PULLUP);
	pinMode(A5, INPUT_PULLUP);

#if INIT_EEPROM_ONLY
		BOOL eepromErr = ee_mgr.initializeEEPROMVars(); /* Must happen after pins are configured due to I2C access */
#else
		i2c_init();
		BOOL eepromErr = ee_mgr.readNonVols();
		setUpSampling(AUDIO_SAMPLING, TRUE);
#endif

	cli();
	/*******************************************************************
	 * TIMER2 is for periodic interrupts to drive Morse code generation
	 * Reset control registers */
	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM21);                             /* set Clear Timer on Compare Match (CTC) mode with OCR2A setting the top */
	TCCR2B |= (1 << CS22) | (1 << CS21) | (1 << CS20);  /* 1024 Prescaler */

	OCR2A = 0x0C;                                       /* set frequency to ~300 Hz (0x0c) */
	OCR2B = 0x00;
	/* Use system clock for Timer/Counter2 */
	ASSR &= ~(1 << AS2);
	/* Reset Timer/Counter2 Interrupt Mask Register */
	TIMSK2 = 0;
	TIMSK2 |= (1 << OCIE2B);                    /* Output Compare Match B Interrupt Enable */

	/*******************************************************************
	 * Timer 1 is used for controlling the attenuator for AM generation
	 * set timer1 interrupt at 16 kHz */

	TCCR1A = 0;                                 /* set entire TCCR1A register to 0 */
	TCCR1B = 0;                                 /* same for TCCR1B */
	TCNT1 = 0;                                  /* initialize counter value to 0 */
	setAMToneFrequency(g_AM_audio_frequency);   /* For attenuator tone output */
/* turn on CTC mode */
	TCCR1B |= (1 << WGM12);
/* Set CS10 bit for no prescaling */
	TCCR1B |= (1 << CS10);
/* enable timer compare interrupt
 *	TIMSK1 |= (1 << OCIE1A); */


	/********************************************************************/
	/* Timer 0 is for audio Start tone generation and control
	 * Note: Do not use millis() or DELAY() after TIMER0 has been reconfigured here! */
	TCCR0A = 0x00;
	TCCR0A |= (1 << WGM01); /* Set CTC mode */
	TCCR0B = 0x00;
	TCCR0B |= (1 << CS02);  /* Prescale 256 */
	OCR0A = DEFAULT_TONE_FREQUENCY;
	TIMSK0 = 0x00;
	TIMSK0 |= (1 << OCIE0A);

	/*******************************************************************
	 * Sync button pin change interrupt */
	PCMSK2 = 0x00;
	PCMSK2 = (1 << PCINT20);    /* Enable PCINT20 */
	PCICR = 0x00;
	PCICR = (1 << PCIE2);       /* Enable pin change interrupt 2 */
	sei();                      /* Enable interrupts */

	if(!g_AM_enabled)
	{
		linkbus_init(BAUD);     /* Start the Link Bus serial comms */
	}

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

		digitalWrite(PIN_LED2, ON);
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

	g_current_epoch = rv3028_get_epoch();

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
		TIMSK1 |= (1 << OCIE1A);    /* start timer 1 interrupts */

#endif  /* #if INIT_EEPROM_ONLY */

	startEventNow(POWER_UP);

	/*******************************************************************
	 * INT0 is for external 1-second interrupts                         */
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
 * ADC Conversion Complete ISR
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
 * Pin Change Interrupt 2 ISR
 *
 * Handles pushbutton presses
 *
 ************************************************************************/
ISR(PCINT2_vect)
{
	BOOL pinVal = digitalRead(PIN_SYNC);

	g_sync_pin_timer = 0;

	if(pinVal)  /* Sync is high = button released */
	{
		if(g_transmissions_disabled)
		{
			if(g_sync_pin_stable == STABLE_LOW)
			{
				g_sync_pin_stable = UNSTABLE;
				digitalWrite(PIN_LED2, OFF);    /*  LED */
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


/***********************************************************************
 * USART Rx Interrupt ISR
 *
 * This ISR is responsible for reading characters from the USART
 * receive buffer to implement the Linkbus.
 *
 *      Message format:
 *              $id,f1,f2... fn;
 *              where
 *                      id = Linkbus MessageID
 *                      fn = variable length fields
 *                      ; = end of message flag
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
 * USART Tx UDRE ISR
 *
 * This ISR is responsible for filling the USART transmit buffer. It
 * implements the transmit function of the Linkbus.
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
 * Timer/Counter2 Compare Match B ISR
 *
 * Handles periodic tasks not requiring precise timing.
 ************************************************************************/
ISR( TIMER2_COMPB_vect )
{
	g_tick_count++;
	static uint16_t codeInc = 0;
	static uint8_t holdButtonState = HIGH;
	BOOL repeat = TRUE, finished = FALSE;

	uint8_t button = digitalRead(PIN_SYNC);

	if(g_reset_button_held && !button)
	{
		if(g_seconds_since_powerup == 3)
		{
			g_seconds_since_powerup = 4;
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

			if(button == LOW)
			{
				digitalWrite(PIN_LED2, ON);
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

	if(!g_transmissions_disabled && g_on_the_air)
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
						sendMorseTone(OFF);
					}

					digitalWrite(PIN_LED2, key);            /*  LED */
					digitalWrite(PIN_CW_KEY_LOGIC, key);    /* TX key line */
					g_sendAMmodulation = key;
					sendMorseTone(key);
				}
			}
			else
			{
				if(g_sync_pin_stable != STABLE_LOW)
				{
					digitalWrite(PIN_LED2, key);        /*  LED */
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

		if(g_LED_enunciating)
		{
			if(codeInc)
			{
				codeInc--;

				if(!codeInc)
				{
					key = makeMorse(NULL, &repeat, &finished);
					digitalWrite(PIN_LED2, key);    /*  LED */
					codeInc = g_code_throttle;
				}
			}
			else
			{
				codeInc = g_code_throttle;
			}
		}
		else
		{
			if(key)
			{
				key = OFF;
				if(g_sync_pin_stable != STABLE_LOW)
				{
					digitalWrite(PIN_LED2, OFF);    /*  LED Off */
				}
			}
		}

		digitalWrite(PIN_CW_KEY_LOGIC, OFF);    /* TX key line */
		g_sendAMmodulation = FALSE;
		sendMorseTone(OFF);
	}
}   /* End of Timer 2 ISR */


#if !INIT_EEPROM_ONLY
/***********************************************************************
 * Handle RTC 1-second interrupts
 **********************************************************************/
	ISR( INT0_vect )
	{
		static int id_countdown = 0;

		g_current_epoch++;

		g_seconds_since_powerup++;

		if(g_temperature_check_countdown)
		{
			g_temperature_check_countdown--;
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
				if(g_seconds_since_sync == 0)   /* sync just occurred */
				{
					id_countdown = g_id_interval;
					g_fox_counter = 1;
				}

				g_seconds_since_sync++; /* Total elapsed time counter */
				g_fox_seconds_into_interval++;

				if(id_countdown)
				{
					id_countdown--;
				}

				if(g_number_of_foxes && ((g_seconds_since_sync % g_on_air_interval) == 0))
				{
					g_fox_counter++;

					if(g_fox_counter > g_number_of_foxes)
					{
						g_fox_counter = 1;
						digitalWrite(PIN_LED2, OFF);
					}
					g_fox_transition = TRUE;
					g_fox_seconds_into_interval = 0;

					if(!id_countdown)
					{
						id_countdown = g_id_interval;
						g_time_to_ID = TRUE;
					}
				}
			}
		}
	}   /* end of INT0 ISR */
#endif /* INIT_EEPROM_ONLY */

/***********************************************************************
 * Timer0 interrupt generates an audio tone on the audio out pin.
 ************************************************************************/
ISR(TIMER0_COMPA_vect)
{
	static BOOL toggle = 0;

	toggle = !toggle;

	if(g_audio_tone_state)
	{
		if(toggle)
		{
			digitalWrite(PIN_CW_TONE_LOGIC, ON);
		}
		else
		{
			digitalWrite(PIN_CW_TONE_LOGIC, OFF);
		}
	}
	else
	{
		digitalWrite(PIN_CW_TONE_LOGIC, OFF);
	}
}

/***********************************************************************
 * Timer/Counter1 Compare Match A ISR
 *
 * Handles AM modulation tasks
 ************************************************************************/
ISR(TIMER1_COMPA_vect)  /* timer1 interrupt */
{
#if !INIT_EEPROM_ONLY
		if(g_AM_enabled)
		{
			static uint8_t index = 0;
			uint8_t port;
			static uint8_t controlPins = 0;

			if(g_sendAMmodulation || index || g_sendAMmodulationConstantly)
			{
/*		controlPins = eeprom_read_byte((uint8_t*)&ee_dataModulation[index++]); */
				controlPins = g_dataModulation[index++];
				if(index >= SIZE_OF_DATA_MODULATION)
				{
					index = 0;
				}

				port = PORTC & 0xF0;
				PORTC = port | dB_low(controlPins);

				port = PORTD & 0xFC;
				PORTD = port | dB_high(controlPins);
				controlPins = 0;
			}
			else if(controlPins != MAX_ATTEN_SETTING)
			{
				controlPins = MAX_ATTEN_SETTING;

				port = PORTC & 0xF0;
				PORTC = port | dB_low(controlPins);

				port = PORTD & 0xFC;
				PORTD = port | dB_high(controlPins);
			}
		}
#endif  /* INIT_EEPROM_ONLY */
}

/***********************************************************************
 *  Here is the main loop
 ************************************************************************/
void loop()
{
#if !INIT_EEPROM_ONLY
		static int time_for_id = 99;
		static BOOL id_set = TRUE;
		static BOOL proceed = FALSE;

		if(g_perform_EEPROM_reset)
		{
			g_perform_EEPROM_reset = FALSE;
			ee_mgr.resetEEPROMValues();
			linkbus_init(BAUD);
		}
#endif  /* !INIT_EEPROM_ONLY */

	handleLinkBusMsgs();

#if !INIT_EEPROM_ONLY
		if(!g_on_the_air || proceed)
		{
			/* At the appropriate time set the pattern to be sent and start transmissions */
			if(!g_transmissions_disabled)
			{
				proceed = FALSE;

				if((g_fox == BEACON) || (g_fox == FOXORING) || (g_fox == SPECTATOR) || (g_fox == (g_fox_counter + g_fox_id_offset)))
				{
					BOOL repeat;
					/* Choose the appropriate Morse pattern to be sent */
					if(g_fox == REPORT_BATTERY)
					{
						sprintf(g_tempStr, "|||%dR%d/%d", g_voltage / 100, (5 + (g_voltage % 100)) / 10, g_temperature);
						strcpy((char*)g_messages_text[PATTERN_TEXT], g_tempStr);
						repeat = FALSE;
					}
					else
					{
						strcpy((char*)g_messages_text[PATTERN_TEXT], g_morsePatterns[g_fox]);
						repeat = TRUE;
					}

					g_code_throttle = THROTTLE_VAL_FROM_WPM(g_pattern_codespeed);
					makeMorse((char*)g_messages_text[PATTERN_TEXT], &repeat, NULL);

					if(g_time_to_ID || (g_id_interval <= g_on_air_interval))
					{
						time_for_id = g_on_air_interval - (600 + timeRequiredToSendStrAtWPM((char*)g_messages_text[STATION_ID], g_id_codespeed)) / 1000;
						g_time_to_ID = FALSE;
					}
					else
					{
						time_for_id = g_on_air_interval + 99;   /* prevent sending ID */
					}

					id_set = FALSE;
					g_on_the_air = TRUE;
					g_callsign_sent = FALSE;
					g_fox_transition = FALSE;
					g_fox_tone_offset = g_fox_counter;
				}
			}
		}
		else
		{
			if(!id_set && (g_fox_seconds_into_interval == time_for_id)) /* Send the call sign at the right time */
			{
				g_code_throttle = THROTTLE_VAL_FROM_WPM(g_id_codespeed);
				BOOL repeat = FALSE;
				makeMorse((char*)g_messages_text[STATION_ID], &repeat, NULL);
				id_set = TRUE;
				g_callsign_sent = FALSE;
			}
			else if((g_fox >= SPRINT_S1) && (g_fox <= SPRINT_F5))
			{
				if(g_fox_transition)
				{
					g_fox_transition = FALSE;
					g_on_the_air = FALSE;
					proceed = TRUE;
				}
			}

			if((g_fox == BEACON) || (g_fox == FOXORING) || (g_fox == SPECTATOR))    /* Proceed as soon as the callsign has been sent */
			{
				if(g_callsign_sent)
				{
					proceed = TRUE;
				}
			}
			else if((g_fox >= SPRINT_S1) && (g_fox <= SPRINT_F5) && g_callsign_sent)
			{
				g_on_the_air = FALSE;
			}
			else if(g_fox != (g_fox_counter + g_fox_id_offset)) /* Turn off transmissions during times when this fox should be silent */
			{
				g_on_the_air = FALSE;
			}
		}


		if(g_goertzel.SamplesReady())
		{
			float magnitudeX;
			float magnitudeY;
#ifdef DEBUG_DTMF
				float magnitudeM;
#endif  /* DEBUG_DTMF */
			float largestX = 0;
			float largestY = 0;
			static char lastKey = '\0';
			static int checkCount = 10;                 /* Set above the threshold to prevent an initial false key detect */
			static int quietCount = 0;
			int x = -1, y = -1;

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
				g_temperature_check_countdown = 60;
			}
			else if(!g_voltage_check_countdown)
			{
				setUpSampling(VOLTAGE_SAMPLING, FALSE);
				g_voltage = getVoltage();
				setUpSampling(AUDIO_SAMPLING, FALSE);
				g_voltage_check_countdown = 10;
			}

			for(int i = 0; i < 4; i++)
			{
				g_goertzel.SetTargetFrequency(y_frequencies[i]);    /*initialize library function with the given sampling frequency no of samples and target freq */
				magnitudeY = g_goertzel.Magnitude2();               /*check them for target_freq */

				if(magnitudeY > largestY)                           /*if you're getting false hits or no hits adjust the threshold */
				{
					largestY = magnitudeY;
					if(magnitudeY > threshold)
					{
						y = i;
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

			if(y >= 0)
			{
				for(int i = 0; i < 4; i++)
				{
					g_goertzel.SetTargetFrequency(x_frequencies[i]);    /*initialize library function with the given sampling frequency no of samples and target freq */
					magnitudeX = g_goertzel.Magnitude2();               /*check them for target_freq */

					if(magnitudeX > largestX)                           /*if you're getting false hits or no hits adjust the threshold */
					{
						largestX = magnitudeX;
						if(magnitudeX > threshold)
						{
							x = i;
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

				if(x >= 0)
				{
					char newKey = key[4 * y + x];

					if(lastKey == newKey)
					{
						checkCount++;

						if(checkCount == 3)
						{
							g_dtmf_detected = TRUE;
							quietCount = 0;
							g_lastKey = newKey;

#ifdef DEBUG_DTMF
								if(lb_enabled())
								{
									sprintf(g_tempStr, "\"%c\"\n", g_lastKey);
									lb_send_string(g_tempStr, TRUE);
								}
#endif  /* DEBUG_DTMF */

							processKey(newKey);

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
										g_goertzel.SetTargetFrequency(mid_frequencies[i]);  /*initialize library function with the given sampling frequency no of samples and target freq */
										magnitudeM = g_goertzel.Magnitude2();               /*check them for target_freq */
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
			else    /* Quiet detected */
			{
				unsigned long delta = g_tick_count - g_last;

				/* Quieting must be detected at least 3 times in less than 5 seconds before another key can be accepted */
				if(quietCount++ > 2)
				{
					g_dtmf_detected = FALSE;
					if(g_transmissions_disabled && !g_LED_enunciating)
					{
						digitalWrite(PIN_LED1, OFF);
					}

					if(delta < 1500)
					{
						checkCount = 0;
					}

					quietCount = 0;
					g_last = g_tick_count;
					lastKey = '\0';
				}
			}

			ADCSRA |= (1 << ADIE);  /* enable interrupts when measurement complete */
			ADCSRA |= (1 << ADSC);  /* start ADC measurements */
		}
#endif  /* !INIT_EEPROM_ONLY */

	if(g_transmissions_disabled)
	{
		if(g_dtmf_detected)
		{
			BOOL repeat = FALSE;
			makeMorse(DTMF_DETECTED_BLINK_PATTERN, &repeat, NULL);
			g_code_throttle = THROTTLE_VAL_FROM_WPM(10);
			g_LED_enunciating = TRUE;
			g_config_error = NULL_CONFIG;   /* Trigger a new configuration enunciation */
		}
		else
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
					digitalWrite(PIN_LED2, OFF);    /* ensure LED is off */
				}
			}
		}
	}
}


ConfigurationState_t clockConfigurationCheck(void)
{
	if((g_event_finish_epoch < MINIMUM_EPOCH) || (g_event_finish_epoch < MINIMUM_EPOCH) || (g_current_epoch < MINIMUM_EPOCH))
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
	OCR0A = DEFAULT_TONE_FREQUENCY - g_fox_tone_offset;
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


/* The compiler does not seem to optimize large switch statements correctly
 * void __attribute__((optimize("O0"))) handleLinkBusMsgs() */
void handleLinkBusMsgs()
{
	LinkbusRxBuffer* lb_buff;
	BOOL send_ack = TRUE;

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
						int x = 0;
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

						if(x != BEACON)
						{
							c = CLAMP(SPECTATOR, x, SPRINT_F5);
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

			case MESSAGE_SET_AM_TONE:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					uint8_t toneVal = atol(lb_buff->fields[FIELD1]);
					g_AM_audio_frequency = CLAMP(MIN_AM_TONE_FREQUENCY, toneVal, MAX_AM_TONE_FREQUENCY);
					ee_mgr.updateEEPROMVar(Am_audio_frequency, (void*)&g_AM_audio_frequency);
					setAMToneFrequency(g_AM_audio_frequency);
				}

				sprintf(g_tempStr, "AM:%d\n", g_AM_audio_frequency);
				lb_send_string(g_tempStr, FALSE);
			}
			break;

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


			case MESSAGE_CODE_SPEED:
			{
				if(lb_buff->fields[FIELD1][0] == 'I')
				{
					if(lb_buff->fields[FIELD2][0])
					{
						uint8_t speed = atol(lb_buff->fields[FIELD2]);
						g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM, speed, MAX_CODE_SPEED_WPM);
						ee_mgr.updateEEPROMVar(Id_codespeed, (void*)&g_id_codespeed);

						if(g_messages_text[STATION_ID][0])
						{
							g_time_needed_for_ID = (600 + timeRequiredToSendStrAtWPM((char*)g_messages_text[STATION_ID], g_id_codespeed)) / 1000;
						}
					}
				}
				sprintf(g_tempStr, "ID: %d wpm\n", g_id_codespeed);
				lb_send_string(g_tempStr, FALSE);
			}
			break;

			case MESSAGE_VERSION:
			{
				ee_mgr.sendEEPROMString(TextVersion);
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
					}
					else
					{
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
/*						setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES); */
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
						reportTimeTill(g_event_start_epoch, g_event_finish_epoch, "Lasts: ", NULL);
						sprintf(g_tempStr, "Finish:%lu\n", g_event_finish_epoch);
/*						setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES); */
						startEventUsingRTC();
					}
					else
					{
						sprintf(g_tempStr, "Finish:%lu\n", g_event_finish_epoch);
					}

					doprint = true;
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
					if(clockConfigurationCheck() == CONFIGURATION_ERROR)
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
		if(send_ack)
		{
			lb_send_NewPrompt();
		}
	}
}

#if !INIT_EEPROM_ONLY

/*
 *  Command set:
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
	void processKey(char key)
	{
		static int state = STATE_SENTENCE_START;
		static int digits;
		static int value;
		static int stringLength;
		static char receivedString[MAX_PATTERN_TEXT_LENGTH + 1] = { '\0' };
		static BOOL setPasswordEnabled = FALSE;

		if(key == 'D')
		{
			g_DTMF_unlocked = FALSE;
			state = STATE_SHUTDOWN;
			return;
		}

		if(!g_DTMF_unlocked)
		{
			state = STATE_CHECK_PASSWORD;
		}
		else
		{
			if(key == '*')
			{
				state = STATE_SENTENCE_START;
			}
		}

		switch(state)
		{
			case STATE_SHUTDOWN:
			{
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
					else if(key != '*')
					{
						value = key - '0';
#if !INIT_EEPROM_ONLY
							if(g_AM_enabled)
							{
								setupPortsForF1975();
							}
#endif  /* !INIT_EEPROM_ONLY */
						state = STATE_TEST_ATTENUATOR;
					}
				}
			}
			break;

			case STATE_A:
			{
				state = STATE_SHUTDOWN;

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
				else if(key == '9')
				{
					state = STATE_SET_AM_TONE_FREQUENCY;
				}
				else if(key == 'B')
				{
					state = STATE_GET_BATTERY_VOLTAGE;
				}
				else
				{
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

					state = STATE_SHUTDOWN;
				}
				else if((key >= '0') && (key <= '9'))
				{
					if(digits)
					{
						value = value * 10 + (key - '0');
						if((value < numMorseChars) && (stringLength < MAX_PATTERN_TEXT_LENGTH))
						{
							receivedString[stringLength] = keyMorse[value];
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

			case STATE_SET_AM_TONE_FREQUENCY:
			{
				if(key == '#')
				{
					g_AM_audio_frequency = value;
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

			case STATE_GET_BATTERY_VOLTAGE:
			{
				if(key == '#')
				{
					Fox_t f = REPORT_BATTERY;
					setupForFox(&f, START_TRANSMISSIONS_NOW);
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
					digits = 0;
					state = STATE_SHUTDOWN;
				}
			}
			break;

			case STATE_TEST_ATTENUATOR:
			{
				if(key == '#')
				{
					if(value == 0)
					{
						setAtten(0);
						/* TIMSK1 |= (1 << OCIE1A); / * start timer 1 interrupts * / */
						g_sendAMmodulationConstantly = TRUE;
					}
					else if(value > 315)
					{
						g_sendAMmodulationConstantly = FALSE;
						/* TIMSK1 &= ~(1 << OCIE1A); / * stop timer 1 interrupts * / */
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
			}
			break;
		}
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

	cli();

	switch(g_fox)
	{
		case FOX_1:
		case FOX_2:
		case FOX_3:
		case FOX_4:
		case FOX_5:
		{
			g_on_air_interval = 60;
			g_cycle_period_seconds = 300;
			g_number_of_foxes = 5;
			g_fox_id_offset = 0;
			g_pattern_codespeed = 8;
			g_id_interval = 60;
		}
		break;

		case SPRINT_S1:
		case SPRINT_S2:
		case SPRINT_S3:
		case SPRINT_S4:
		case SPRINT_S5:
		{
			g_on_air_interval = 12;
			g_cycle_period_seconds = 60;
			g_number_of_foxes = 5;
			g_pattern_codespeed = SPRINT_SLOW_CODE_SPEED;
			g_fox_id_offset = SPRINT_S1 - 1;
			g_id_interval = 600;
		}
		break;

		case SPRINT_F1:
		case SPRINT_F2:
		case SPRINT_F3:
		case SPRINT_F4:
		case SPRINT_F5:
		{
			g_on_air_interval = 12;
			g_cycle_period_seconds = 60;
			g_number_of_foxes = 5;
			g_pattern_codespeed = SPRINT_FAST_CODE_SPEED;
			g_fox_id_offset = SPRINT_F1 - 1;
			g_id_interval = 600;
		}
		break;

		case REPORT_BATTERY:
		{
			g_on_air_interval = 30;
			g_cycle_period_seconds = 60;
			g_number_of_foxes = 2;
			g_pattern_codespeed = SPRINT_SLOW_CODE_SPEED;
			g_fox_id_offset = REPORT_BATTERY - 1;
			g_id_interval = 15;
		}
		break;


/* case BEACON: */
/* case SPECTATOR: */
		default:
		{
			g_on_air_interval = 600;
			g_number_of_foxes = 1;
			g_pattern_codespeed = 8;
			g_id_interval = 600;
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
		g_seconds_since_sync = 0;   /* Total elapsed time counter */
		g_fox_seconds_into_interval = 0;
		g_use_rtc_for_startstop = FALSE;
		g_transmissions_disabled = FALSE;
	}
	else if(action == START_TRANSMISSIONS_NOW)                      /* Immediately start transmitting, regardless RTC or time slot */
	{
		g_fox_counter = g_fox - g_fox_id_offset;
		g_seconds_since_sync = g_fox_counter * g_on_air_interval;   /* Total elapsed time counter */
		g_fox_seconds_into_interval = 0;
		g_use_rtc_for_startstop = FALSE;
		g_transmissions_disabled = FALSE;
	}
	else                                                                    /* if(action == START_EVENT_WITH_STARTFINISH_TIMES) */
	{
		if(g_event_start_epoch < g_current_epoch)                           /* timed event in progress */
		{
			g_seconds_since_sync = g_current_epoch - g_event_start_epoch;   /* Total elapsed time counter: synced at event start time */
			g_fox_seconds_into_interval = g_seconds_since_sync % g_cycle_period_seconds;
			g_fox_counter = CLAMP(1, 1 + (g_fox_seconds_into_interval / g_on_air_interval), g_number_of_foxes);
		}
		else                                                                /* event starts in the future */
		{
			g_seconds_since_sync = 0;                                       /* Total elapsed time counter */
			g_fox_counter = 1;
			g_fox_seconds_into_interval = 0;
		}


		g_use_rtc_for_startstop = TRUE;
		g_transmissions_disabled = TRUE;
	}

	g_fox_tone_offset = g_fox_counter;

	sendMorseTone(OFF);
	g_time_to_ID = FALSE;
	g_callsign_sent = TRUE;
	g_code_throttle    = 0;                 /* Adjusts Morse code speed */
	g_fox_transition = FALSE;

	g_on_the_air       = FALSE;             /* Controls transmitter Morse activity */

	g_config_error = NULL_CONFIG;           /* Trigger a new configuration enunciation */
	digitalWrite(PIN_LED2, OFF);            /*  LED Off - in case it was left on */

	digitalWrite(PIN_CW_KEY_LOGIC, OFF);    /* TX key line */
	g_sendAMmodulation = FALSE;
	g_LED_enunciating = FALSE;
	g_config_error = NULL_CONFIG;           /* Trigger a new configuration enunciation */

	sei();
}


/*
 * Read the temperature ADC value
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
 * Returns the most recent temperature reading
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
		 * for Arduino Uno ADC clock is 16 MHz and a conversion takes 13 clock cycles */

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
		 * with the internal reference of 1.1V.
		 * Channel 8 can not be selected with
		 * the analogRead function yet. */
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
		 * by dividing by 128. Assumes that the
		 * standard Arduino 16 MHz clock is in use. */
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
 *	lb_send_string((char*)"Sync OK\n", FALSE); */
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
		digitalWrite(PIN_LED2, OFF);    /*  LED Off */
	}
}

void startEventUsingRTC(void)
{
	setupForFox(NULL, START_EVENT_WITH_STARTFINISH_TIMES);
	g_current_epoch = rv3028_get_epoch();
	ConfigurationState_t state = clockConfigurationCheck();

	if(state != CONFIGURATION_ERROR)
	{
		reportTimeTill(g_current_epoch, g_event_start_epoch, "Starts in: ", "In progress\n");
		reportTimeTill(g_event_start_epoch, g_event_finish_epoch, "Lasts: ", NULL);
		if(g_event_start_epoch < g_current_epoch)
		{
			reportTimeTill(g_current_epoch, g_event_finish_epoch, "Time Remaining: ", NULL);
		}
	}
	else
	{
		reportConfigErrors();
	}
}

void reportConfigErrors(void)
{
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
		lb_send_string((char*)"Event running...\n", TRUE);
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

void setAMToneFrequency(uint8_t value)
{
	BOOL enableAM = TRUE;

	switch(value)
	{
		case 0:
		{
			enableAM = FALSE;
			OCR1A = 1000;
			linkbus_init(BAUD);
		}
		break;

		case 2:
		{
			OCR1A = 556;    /* For ~900 Hz tone output */
		}
		break;

		case 3:
		{
			OCR1A = 625;    /* For ~800 Hz tone output */
		}
		break;

		case 4:
		{
			OCR1A = 714;    /* For ~700 Hz tone output */
		}
		break;

		case 5:
		{
			OCR1A = 833;    /* For ~600 Hz tone output */
		}
		break;

		case 6:
		{
			OCR1A = 1000;   /* For ~500 Hz tone output */
		}
		break;

		default:
		{
			OCR1A = 500;    /* For ~1000 Hz tone output */
		}
		break;
	}

#if !INIT_EEPROM_ONLY
		if(enableAM)
		{
			setupPortsForF1975();
		}
#endif  /* INIT_EEPROM_ONLY */

	g_AM_enabled = enableAM;
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
