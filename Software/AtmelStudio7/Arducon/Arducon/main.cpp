/*
 *  TODOs:
 *  1. Add LED feedback for decoded DTMF tones (fast blinking)
 *  2. Add DTMF and serial commands for
 *       - Synchronizing
 *       - Setting competition format
 *       - Setting fox ID
 *       - Addressing a command to a specific fox ID
 *	 - Addressing a command to a specific competition format
 *       - Setting and reading RTC
 */
/*
 *  MIT License
 *
 *  Copyright (c) 2020 DigitalConfections
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
#include "Goertzel.h"
#include "morse.h"
#include "rv3028.h"

#if COMPILE_FOR_ATMELSTUDIO7
#include <avr/io.h>
#include <avr/eeprom.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "ardooweeno.h"
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

#define MAX_PATTERN_TEXT_LENGTH 20
#define TEMP_STRING_LENGTH (MAX_PATTERN_TEXT_LENGTH + 10)
#define MAX_DTMF_ARG_LENGTH TEMP_STRING_LENGTH

/*#define SAMPLE_RATE 9630 */
#define SAMPLE_RATE 19260
/*#define SAMPLE_RATE 38520
 *#define SAMPLE_RATE 77040
 *#define SAMPLE_RATE 154080 */

volatile int32_t g_seconds_since_sync = 0;  /* Total elapsed time counter */
volatile Fox_t g_fox          = BEACON;     /* Sets Fox number not set by ISR. Set in startup and checked in main. */
volatile int g_active           = 0;        /* Disable active. set and clear in ISR. Checked in main. */

volatile int g_on_the_air       = 0;        /* Controls transmitter Morse activity */
volatile int g_code_throttle    = 0;        /* Adjusts Morse code speed */
const char g_morsePatterns[][5] = { "MO ", "MOE ", "MOI ", "MOS ", "MOH ", "MO5 ", "", "5", "S", "ME", "MI", "MS", "MH", "M5", "OE", "OI", "OS", "OH", "O5" };
volatile BOOL g_callsign_sent = FALSE;

volatile BOOL g_blinky_time = FALSE;

volatile int g_on_air_interval = 0;
volatile int g_fox_seconds_into_interval = 0;
volatile int g_fox_counter = 1;
volatile int g_number_of_foxes = 0;
volatile BOOL g_fox_transition = FALSE;
volatile int g_fox_id_offset = 0;
volatile int g_id_interval = 0;
volatile BOOL g_time_to_ID = FALSE;
volatile int g_startclock_interval = 60;
volatile int g_fox_tone_offset = 1;

volatile BOOL g_audio_tone_state = FALSE;
volatile uint8_t g_lastSeconds = 0x00;
volatile int16_t g_sync_pin_timer = 0;
volatile BOOL g_sync_pin_stable = FALSE;
volatile BOOL g_sync_enabled = TRUE;

volatile BOOL g_dtmf_detected = FALSE;
int8_t g_temperature = 0;

#if !COMPILE_FOR_ATMELSTUDIO7
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
 #endif  /* COMPILE_FOR_ATMELSTUDIO7 */

/***********************************************************************
 * Global Variables & String Constants
 *
 * Identify each global with a "g_" prefix
 * Whenever possible limit globals' scope to this file using "static"
 * Use "volatile" for globals shared between ISRs and foreground loop
 ************************************************************************/
static uint8_t EEMEM ee_interface_eeprom_initialization_flag = EEPROM_UNINITIALIZED;
static char EEMEM ee_stationID_text[MAX_PATTERN_TEXT_LENGTH + 1];
#define SIZE_OF_TEMPERATURE_TABLE 60
static uint16_t EEMEM ee_temperature_table[SIZE_OF_TEMPERATURE_TABLE];
static uint8_t EEMEM ee_id_codespeed;
static uint16_t EEMEM ee_clock_calibration;
static uint8_t EEMEM ee_fox_setting;
static uint8_t EEMEM ee_enable_LEDs;
static int16_t EEMEM ee_temp_calibration;
static int16_t EEMEM ee_rv3028_offset;
static uint8_t EEMEM ee_enable_start_timer;
static uint8_t EEMEM ee_enable_transmitter;

static char g_messages_text[2][MAX_PATTERN_TEXT_LENGTH + 1] = { "\0", "\0" };
static volatile uint8_t g_id_codespeed = EEPROM_ID_CODE_SPEED_DEFAULT;
static volatile uint8_t g_pattern_codespeed = EEPROM_PATTERN_CODE_SPEED_DEFAULT;
static volatile uint16_t g_time_needed_for_ID = 0;
static volatile uint16_t g_clock_calibration = EEPROM_CLOCK_CALIBRATION_DEFAULT;
static volatile int16_t g_temp_calibration = EEPROM_TEMP_CALIBRATION_DEFAULT;
/*static volatile uint8_t g_override_DIP_switches = EEPROM_OVERRIDE_DIP_SW_DEFAULT; */
static volatile uint8_t g_enable_LEDs;
static volatile uint8_t g_enable_start_timer;
static volatile uint8_t g_enable_transmitter;
static volatile uint8_t g_temperature_check_countdown = 60;
static volatile int16_t g_rv3028_offset = EEPROM_RV3028_OFFSET_DEFAULT;
extern BOOL g_allow_rv3028_eeprom_changes;

#define _N 201
const int N = _N;
const float threshold = 500000. * (_N / 100);
const float sampling_freq = SAMPLE_RATE;
const float x_frequencies[4] = { 1209., 1336., 1477., 1633. };
const float y_frequencies[4] = { 697., 770., 852., 941. };
const float mid_frequencies[7] = { 734., 811., 897., 1075., 1273., 1407., 1555. };
const char key[16] = { '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D' };
const int numKeys = sizeof(key) / sizeof(key[0]);
const char keyMorse[39] = { ' ','A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
							'V','W', 'X', 'Y', 'Z', '<', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
const int numMorseChars = sizeof(keyMorse) / sizeof(keyMorse[0]);

char g_lastKey = '\0';
unsigned long g_last = 0;
unsigned long g_tick_count = 0;

Goertzel g_goertzel(N, sampling_freq);

static char g_tempStr[TEMP_STRING_LENGTH] = { '\0' };
char s[12];

static volatile uint8_t g_LEDs_Timed_Out = FALSE;
/*
 * Local function prototypes
 */
void handleLinkBusMsgs(void);
void processKey(char key);
void initializeEEPROMVars(BOOL resetAll);
void permCallsign(char* call);
void permFox(Fox_t fox);
void sendMorseTone(BOOL onOff);
void playStartingTone(uint8_t toneFreq);
void setupForFox(Fox_t* fox);
void setUpTemp(void);
uint16_t readADC();
float getTemp(void);
void setUpAudioSampling(BOOL enableSampling);

#if COMPILE_FOR_ATMELSTUDIO7
	void loop(void);
	int main(void)
#else
	void setup()
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */
{
	/* set active pins */
	pinMode(PIN_LED1, OUTPUT);  /* The amber LED: This led blinks when off cycle and blinks with code when on cycle. */
	digitalWrite(PIN_LED1, OFF);

	pinMode(PIN_LED2, OUTPUT);
	digitalWrite(PIN_LED2, OFF);

	pinMode(PIN_CW_KEY_LOGIC, OUTPUT);  /* This pin is used to control the KEY line to the transmitter only active on cycle. */
	digitalWrite(PIN_CW_KEY_LOGIC, OFF);

	pinMode(PIN_CW_TONE_LOGIC, OUTPUT);
	digitalWrite(PIN_CW_TONE_LOGIC, OFF);

	pinMode(PIN_PTT_LOGIC, OUTPUT);
	digitalWrite(PIN_PTT_LOGIC, OFF);

	pinMode(PIN_CW_KEY_LOGIC, OUTPUT);
	digitalWrite(PIN_CW_KEY_LOGIC, OFF);

	pinMode(PIN_AUDIO_INPUT, INPUT);    /* Receiver Audio sampling */
	pinMode(PIN_BATTERY_LEVEL, INPUT);  /* Battery voltage level */

	pinMode(PIN_MISO, INPUT_PULLUP);
	pinMode(PIN_MOSI, OUTPUT);
	digitalWrite(PIN_MOSI, OFF);

	pinMode(PIN_D0, OUTPUT);
	pinMode(PIN_D1, OUTPUT);
	pinMode(PIN_D2, OUTPUT);
	pinMode(PIN_D3, OUTPUT);
	pinMode(PIN_D4, OUTPUT);    /* Also RXD */
	pinMode(PIN_D5, OUTPUT);    /* Also TXD */

	/* Set unused pins as outputs pulled high */
	pinMode(A4, INPUT_PULLUP);
	pinMode(A5, INPUT_PULLUP);

	initializeEEPROMVars(FALSE); /* Must happen after pins are configure due to I2C access */

	setUpAudioSampling(true);

	EICRA  |= (1 << ISC01); /* Configure INT0 falling edge for RTC 1-second interrupts */
	EIMSK |= (1 << INT0);   /* Enable INT0 interrupts */

	/**
	 * TIMER2 is for periodic interrupts to drive Morse code generation */
	/* Reset control registers */
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
	TIMSK2 |= (1 << OCIE2B);    /* Output Compare Match B Interrupt Enable */

	/* Timer 0 is for audio Start tone generation and control
	 * Note: Do not use millis() or DELAY() after TIMER0 has been reconfigured here! */
	TCCR0A = 0x00;
	TCCR0A |= (1 << WGM01);     /* Set CTC mode */
	TCCR0B = 0x00;
	TCCR0B |= (1 << CS02);      /* Prescale 256 */
	OCR0A = DEFAULT_TONE_FREQUENCY;
	TIMSK0 = 0x00;
	TIMSK0 |= (1 << OCIE0A);

	/* Sync button pin change interrupt */
	PCMSK2 = 0x00;
	PCMSK2 = (1 << PCINT19);    /* Enable PCINT19 */
	PCICR = 0x00;
	PCICR = (1 << PCIE2);       /* Enable pin change interrupt 2 */

	rv3028_1s_sqw();        /* Only needs to run once on a new board to program RTC */

	sei();                                                      /* Enable interrupts */

	linkbus_init(BAUD);                                         /* Start the Link Bus serial comms */
	lb_send_string((char*)SW_REVISION, FALSE);


	lb_send_string((char*)"\n\nStored Data:\n", TRUE);
	sprintf(g_tempStr, "  ID: %s\n", g_messages_text[STATION_ID]);
	lb_send_string(g_tempStr, TRUE);
	sprintf(g_tempStr, "  CAL: %u\n", g_clock_calibration);
	lb_send_string(g_tempStr, TRUE);
	sprintf(g_tempStr, "  LED: %s\n", g_enable_LEDs ? "ON" : "OFF");
	lb_send_string(g_tempStr, TRUE);
	sprintf(g_tempStr, "  STA: %s\n", g_enable_start_timer ? "ON" : "OFF");
	lb_send_string(g_tempStr, TRUE);
	sprintf(g_tempStr, "  TXE: %s\n", g_enable_transmitter ? "ON" : "OFF");
	lb_send_string(g_tempStr, TRUE);
	dtostrf((double)threshold, 4, 0, s);
	sprintf(g_tempStr, "Thresh=%s\n\n", s);
	lb_send_string(g_tempStr, TRUE);

	lb_send_NewPrompt();

	/*  Serial.print("SAMPLING_RATE = ");
	 *  Serial.println(_SAMPLING_FREQUENCY);
	 *  Serial.print(" N = ");
	 *  Serial.println(_N);
	 *  Serial.print("FREQ = ");
	 *  Serial.println(_TARGET_FREQUENCY);
	 *  Serial.print("k = ");
	 *  Serial.print(k);
	 *  Serial.print(" coeff = ");
	 *  Serial.println(g_coeff); */

#if COMPILE_FOR_ATMELSTUDIO7
		while(1)
		{
			loop();
		}
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */
}

/***********************************************************************
 * ADC Conversion Complete ISR
 ************************************************************************/
ISR(ADC_vect)
{
	digitalWrite(PIN_MOSI, ON);
	if(g_goertzel.DataPoint(ADCH))
	{
		ADCSRA &= ~(1 << ADIE); /* disable ADC interrupt */
	}
	digitalWrite(PIN_MOSI, OFF);
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

	if(pinVal)  /* Sync is high */
	{
		if(g_sync_pin_stable)
		{
/*			setupForFox(NULL); */
		}
	}

	g_sync_pin_timer = 0;
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
	static int blink_counter = 100;
	static int blink_count_direction = -1;
	static uint8_t hold_last10sec = 0;
	static int starting_blip = 0;
	static int starting_boop = 0;
	static BOOL playMorse = TRUE;
	BOOL repeat = TRUE, finished = FALSE;

	if(g_sync_enabled)
	{
		if(digitalRead(PIN_SYNC) == LOW)
		{
			if(g_sync_pin_timer < TIMER2_SECONDS_3)
			{
				g_sync_pin_timer++;
			}

			if(g_sync_pin_timer > TIMER2_SECONDS_1)
			{
				g_sync_pin_stable = TRUE;
				digitalWrite(PIN_LED2, HIGH);
			}
		}
	}

	if(g_dtmf_detected)
	{
		if(blink_counter < -BLINK_FAST)
		{
			blink_count_direction = 1;
			digitalWrite(PIN_LED1, OFF);
		}

		if(blink_counter > BLINK_FAST)
		{
			blink_count_direction = -1;
			digitalWrite(PIN_LED1, ON);
		}
	}
	else
	{
		blink_counter = BLINK_FAST;
		blink_count_direction = 1;
	}

	blink_counter += blink_count_direction;

	if(blink_counter < 0)
	{
		g_blinky_time = TRUE;
	}
	else
	{
		g_blinky_time = FALSE;
	}

	if(g_enable_start_timer)
	{
		if(hold_last10sec != g_lastSeconds)
		{
			hold_last10sec = g_lastSeconds;

			if(hold_last10sec > 0)
			{
				playMorse = FALSE;
				starting_blip = BLINK_SHORT;
			}
			else if(hold_last10sec == 0)
			{
				starting_blip = 0;
				starting_boop = TIMER2_SECONDS_2;
			}
		}

		if(starting_blip)
		{
			starting_blip--;

			if(starting_blip)
			{
				if(g_lastSeconds > 5)
				{
					playStartingTone(TONE_500Hz);
				}
				else
				{
					playStartingTone(TONE_600Hz);
				}
			}
			else
			{
				playStartingTone(0);
			}

		}
		else if(starting_boop)
		{
			starting_boop--;

			if(!starting_boop)
			{
				playStartingTone(0);
				playMorse = TRUE;
			}
			else
			{
				playStartingTone(TONE_400Hz);
			}
		}
	}

	static BOOL key = OFF;

	if(g_on_the_air > 0)
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
					if(playMorse)
					{
						sendMorseTone(OFF);
					}
				}

				if(key)
				{
					if(!g_LEDs_Timed_Out)
					{
						digitalWrite(PIN_LED2, HIGH);   /*  LED */
					}

					if(g_enable_transmitter)
					{
						digitalWrite(PIN_CW_KEY_LOGIC, HIGH);   /* TX key line */
					}
				}

				if(playMorse)
				{
					sendMorseTone(key);
				}
			}
		}
		else
		{
			if(!g_LEDs_Timed_Out && !g_sync_pin_stable)
			{
				digitalWrite(PIN_LED2, key);    /*  LED */
			}

			if(g_enable_transmitter)
			{
				digitalWrite(PIN_CW_KEY_LOGIC, key);    /* TX key line */
			}

			codeInc = g_code_throttle;

			if(playMorse)
			{
				sendMorseTone(key);
			}
		}
	}
	else if(!g_on_the_air)
	{
		if(key)
		{
			key = OFF;
			if(!g_sync_pin_stable)
			{
				digitalWrite(PIN_LED2, OFF);        /*  LED Off */
			}

			digitalWrite(PIN_CW_KEY_LOGIC, OFF);    /* TX key line */
		}

		if(playMorse)
		{
			sendMorseTone(OFF);
		}
	}
}   /* End of Timer 2 ISR */


/***********************************************************************
 * Handle RTC 1-second interrupts
 **********************************************************************/
ISR( INT0_vect )
{
	static int id_countdown = 0;

	if(g_seconds_since_sync == 0)   /* sync just occurred */
	{
		id_countdown = g_id_interval;
		g_fox_counter = 1;
		g_lastSeconds = 0;
	}

	g_seconds_since_sync++; /* Total elapsed time counter */
	g_fox_seconds_into_interval++;

	if(id_countdown)
	{
		id_countdown--;
	}

	if(g_temperature_check_countdown)
	{
		g_temperature_check_countdown--;
	}

	if(g_number_of_foxes && ((g_seconds_since_sync % g_on_air_interval) == 0))
	{
		g_fox_counter++;

		if(g_fox_counter > g_number_of_foxes)
		{
			g_fox_counter = 1;

			if(g_sync_enabled)
			{
				PCMSK2 &= ~(1 << PCINT19);  /* Disable PCINT19 */
				PCICR &= ~(1 << PCIE2);     /* Disable pin change interrupt 2 */
				pinMode(PIN_SYNC, INPUT);
				pinMode(PIN_SYNC, OUTPUT);  /* Set sync pin as output low */
				g_sync_enabled = FALSE;
			}

			g_LEDs_Timed_Out = TRUE;
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

	if(g_enable_start_timer && ((g_seconds_since_sync + 11) % g_startclock_interval <= 10))
	{
		g_lastSeconds = (uint8_t)((g_seconds_since_sync + 11) % g_startclock_interval);
	}
	else
	{
		g_lastSeconds = 0;
	}
}   /* end of Timer1 ISR */

/* This interrupt generates an audio tone on the audio out pin. */
SIGNAL(TIMER0_COMPA_vect)
{
	static BOOL toggle = 0;

	toggle = !toggle;

	if(g_audio_tone_state)
	{
		if(toggle)
		{
			digitalWrite(PIN_CW_TONE_LOGIC,ON);
		}
		else
		{
			digitalWrite(PIN_CW_TONE_LOGIC,OFF);
		}
	}
	else
	{
		digitalWrite(PIN_CW_TONE_LOGIC,OFF);
	}
}

/*
 *  Here is the main loop
 * */
void loop()
{
#if !INIT_EEPROM_ONLY
	static int time_for_id = 99;
	static BOOL id_set = TRUE;
	static BOOL proceed = FALSE;
#endif // !INIT_EEPROM_ONLY

	handleLinkBusMsgs();

#if !INIT_EEPROM_ONLY
	if(!g_on_the_air || proceed)
	{
		proceed = FALSE;

		/* Choose the appropriate Morse pattern to be sent */
		if(g_fox == FOX_DEMO)
		{
			strcpy(g_messages_text[PATTERN_TEXT],g_morsePatterns[g_fox_counter]);
		}
		else if(g_fox == SPRINT_DEMO)
		{
			strcpy(g_messages_text[PATTERN_TEXT],g_morsePatterns[g_fox_counter + 8]);
		}
		else
		{
			strcpy(g_messages_text[PATTERN_TEXT],g_morsePatterns[g_fox]);
		}

		/* At the appropriate time set the pattern to be sent and start transmissions */
		if((g_fox == FOX_DEMO) || (g_fox == SPRINT_DEMO) || (g_fox == BEACON) || (g_fox == FOXORING) || (g_fox == SPECTATOR) || (g_fox == (g_fox_counter + g_fox_id_offset)))
		{
			BOOL repeat = TRUE;
			g_code_throttle = THROTTLE_VAL_FROM_WPM(g_pattern_codespeed);
			makeMorse(g_messages_text[PATTERN_TEXT],&repeat,NULL);

			if(g_time_to_ID || (g_id_interval <= g_on_air_interval))
			{
				time_for_id = g_on_air_interval - (500 + timeRequiredToSendStrAtWPM(g_messages_text[STATION_ID],g_id_codespeed)) / 1000;
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
	else
	{
		if(!id_set && (g_fox_seconds_into_interval == time_for_id)) /* Send the call sign at the right time */
		{
			g_code_throttle = THROTTLE_VAL_FROM_WPM(g_id_codespeed);
			BOOL repeat = FALSE;
			makeMorse(g_messages_text[STATION_ID],&repeat,NULL);
			id_set = TRUE;
			g_callsign_sent = FALSE;
		}
		else if((g_fox >= SPRINT_S1) && (g_fox <= SPRINT_DEMO))
		{
			if(g_fox_transition)
			{
				g_fox_transition = FALSE;
				g_on_the_air = FALSE;
				proceed = TRUE;
			}
		}

		if((g_fox == FOX_DEMO) || (g_fox == SPRINT_DEMO))
		{
			if((g_callsign_sent) && g_fox_transition)   /* Ensure we've begun the next minute before proceeding */
			{
				proceed = TRUE;
			}
		}
		else if((g_fox == BEACON) || (g_fox == FOXORING) || (g_fox == SPECTATOR))   /* Proceed as soon as the callsign has been sent */
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
		else if((g_fox != g_fox_counter) && g_callsign_sent)    /* Turn off transmissions during minutes when this fox should be silent */
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
		static int checkCount = 10;                                                    /* Set above the threshold to prevent an initial false key detect */
		static int quietCount = 0;
		int x = -1,y = -1;

		if(!g_temperature_check_countdown)
		{

			setUpTemp();
			int8_t temp = (int8_t)getTemp();
			if(temp != g_temperature)
			{
				g_temperature = temp;
				int8_t delta25 = 25 - temp;
				int8_t adj = eeprom_read_byte((uint8_t*)&ee_temperature_table[delta25]);
				rv3028_set_offset_RAM(g_rv3028_offset+adj);
			}

			setUpAudioSampling(false);
			g_temperature_check_countdown = 60;
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
					dtostrf((double)y_frequencies[i],4,0,s);
					sprintf(g_tempStr,"Y(%s)=",s);
					lb_send_string(g_tempStr,TRUE);
					dtostrf((double)magnitudeY,4,0,s);
					sprintf(g_tempStr,"%s\n",s);
					lb_send_string(g_tempStr,TRUE);
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
/*					break; */
				}

#ifdef DEBUG_DTMF
					if(magnitudeX > threshold)
					{
						dtostrf((double)x_frequencies[i],4,0,s);
						sprintf(g_tempStr,"X(%s)=",s);
						lb_send_string(g_tempStr,TRUE);
						dtostrf((double)magnitudeX,4,0,s);
						sprintf(g_tempStr,"%s\n",s);
						lb_send_string(g_tempStr,TRUE);
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
						sprintf(g_tempStr,"\"%c\"\n",g_lastKey);
						lb_send_string(g_tempStr,TRUE);

						processKey(newKey);

#ifdef DEBUG_DTMF
							lb_send_string((char*)"Mag X/Y=",TRUE);
							dtostrf((double)largestX,4,0,s);
							sprintf(g_tempStr,"%s / ",s);
							lb_send_string(g_tempStr,TRUE);
							dtostrf((double)largestY,4,0,s);
							sprintf(g_tempStr,"%s\n",s);
							lb_send_string(g_tempStr,TRUE);

							for(int i = 0; i < 7; i++)
							{
								g_goertzel.SetTargetFrequency(mid_frequencies[i]);  /*initialize library function with the given sampling frequency no of samples and target freq */
								magnitudeM = g_goertzel.Magnitude2();               /*check them for target_freq */
								dtostrf((double)mid_frequencies[i],4,0,s);
								sprintf(g_tempStr,"%s=",s);
								lb_send_string(g_tempStr,TRUE);
								dtostrf((double)magnitudeM,4,0,s);
								sprintf(g_tempStr,"%s\n",s);
								lb_send_string(g_tempStr,TRUE);
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
/*			lb_send_string((char*)".", FALSE); */

			/* Quieting must be detected at least 3 times in less than 5 seconds before another key can be accepted */
			if(quietCount++ > 2)
			{
				g_dtmf_detected = FALSE;
				digitalWrite(PIN_LED1,OFF);

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

	if(!g_LEDs_Timed_Out)       /* flash LED */
	{
		if(g_dtmf_detected)
		{
			if(g_blinky_time)
			{
				digitalWrite(PIN_LED2,OFF);
			}
			else
			{
				digitalWrite(PIN_LED2,ON);
			}
		}
	}
#endif // !INIT_EEPROM_ONLY
}


void sendMorseTone(BOOL onOff)
{
	if(!g_lastSeconds)
	{
		OCR0A = DEFAULT_TONE_FREQUENCY - g_fox_tone_offset;
		g_audio_tone_state = onOff;
	}
	else
	{
		OCR0A = DEFAULT_TONE_FREQUENCY;
		g_audio_tone_state = OFF;
	}
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


/* The compiler does not seem to optimize large switch statements correctly */
void __attribute__((optimize("O0"))) handleLinkBusMsgs()
{
	LinkbusRxBuffer* lb_buff;
	BOOL send_ack = TRUE;

	while((lb_buff = nextFullRxBuffer()))
	{
		LBMessageID msg_id = lb_buff->id;

		switch(msg_id)
		{
			case MESSAGE_RESET:
			{
			}
			break;

			case MESSAGE_SET_FOX:
			{
				int c = (int)(lb_buff->fields[FIELD1][0]);

				if(c)
				{
					if(c == 'B')
					{
						c = BEACON;
					}
					else if(c == 'D')
					{
						char t = lb_buff->fields[FIELD2][0];

						if(t == 'S')
						{
							c = SPRINT_DEMO;
						}
						else
						{
							c = FOX_DEMO;
						}
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
							c = CLAMP(BEACON,atoi(lb_buff->fields[FIELD2]),FOX_5);
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
							c = CLAMP(SPECTATOR,x,SPRINT_F5);
						}
					}
					else if(c == 'N')
					{
						char t = lb_buff->fields[FIELD2][0];

						if(t == '2')
						{
							c = NO_CODE_START_TONES_2M;
						}
						else if(t == '5')
						{
							c = NO_CODE_START_TONES_5M;
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
						permFox(holdFox);
						if(holdFox != g_fox)
						{
							setupForFox(&holdFox);
						}
					}
				}

				sprintf(g_tempStr,"Fox=%u\n",g_fox);
				lb_send_string(g_tempStr,FALSE);
			}
			break;

			case MESSAGE_LEDS:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					if((lb_buff->fields[FIELD1][1] == 'F') || (lb_buff->fields[FIELD1][0] == '0'))
					{
						g_enable_LEDs = FALSE;
						digitalWrite(PIN_LED1,OFF); /*  LED Off */
					}
					else
					{
						g_enable_LEDs = TRUE;
					}

					eeprom_update_byte(&ee_enable_LEDs,g_enable_LEDs);
					g_LEDs_Timed_Out = !g_enable_LEDs;
				}

				sprintf(g_tempStr,"LED:%s\n",g_enable_LEDs ? "ON" : "OFF");
				lb_send_string(g_tempStr,FALSE);
			}
			break;

			case MESSAGE_STARTTONES_ENABLE:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					if((lb_buff->fields[FIELD1][1] == 'F') || (lb_buff->fields[FIELD1][0] == '0'))
					{
						g_enable_start_timer = FALSE;
					}
					else
					{
						g_enable_start_timer = TRUE;
					}

					eeprom_update_byte(&ee_enable_start_timer,g_enable_start_timer);
				}

				sprintf(g_tempStr,"STA:%s\n",g_enable_start_timer ? "ON" : "OFF");
				lb_send_string(g_tempStr,FALSE);
			}
			break;

			case MESSAGE_GO:
			{
				setupForFox(NULL);
				lb_send_string((char*)"Re-sync successful!\n",FALSE);
			}
			break;

			case MESSAGE_FACTORY_RESET:
			{
/*				initializeEEPROMVars(TRUE);
 *				softwareReset(); */
			}
			break;

			case MESSAGE_SET_STATION_ID:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					strcpy(g_tempStr," ");  /* Space before ID gets sent */
					strcat(g_tempStr,lb_buff->fields[FIELD1]);

					if(lb_buff->fields[FIELD2][0])
					{
						strcat(g_tempStr," ");
						strcat(g_tempStr,lb_buff->fields[FIELD2]);
					}

					if(strlen(g_tempStr) <= MAX_PATTERN_TEXT_LENGTH)
					{
						uint8_t i;
						strcpy(g_messages_text[STATION_ID],g_tempStr);

						for(i = 0; i < strlen(g_messages_text[STATION_ID]); i++)
						{
							eeprom_update_byte((uint8_t*)&ee_stationID_text[i],(uint8_t)g_messages_text[STATION_ID][i]);
						}

						eeprom_update_byte((uint8_t*)&ee_stationID_text[i],0);
					}
				}

				if(g_messages_text[STATION_ID][0])
				{
					g_time_needed_for_ID = (500 + timeRequiredToSendStrAtWPM(g_messages_text[STATION_ID],g_id_codespeed)) / 1000;
				}

				sprintf(g_tempStr,"ID:%s\n",g_messages_text[STATION_ID]);
				lb_send_string(g_tempStr,TRUE);
			}
			break;


			case MESSAGE_CODE_SPEED:
			{
				if(lb_buff->fields[FIELD1][0] == 'I')
				{
					if(lb_buff->fields[FIELD2][0])
					{
						uint8_t speed = atol(lb_buff->fields[FIELD2]);
						g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM,speed,MAX_CODE_SPEED_WPM);
						eeprom_update_byte(&ee_id_codespeed,g_id_codespeed);

						if(g_messages_text[STATION_ID][0])
						{
							g_time_needed_for_ID = (500 + timeRequiredToSendStrAtWPM(g_messages_text[STATION_ID],g_id_codespeed)) / 1000;
						}
					}
				}
				sprintf(g_tempStr,"ID:  %d wpm\n",g_id_codespeed);
				lb_send_string(g_tempStr,FALSE);
			}
			break;

			case MESSAGE_VERSION:
			{
				sprintf(g_tempStr,"SW Ver:%s\n",SW_REVISION);
				lb_send_string(g_tempStr,FALSE);
			}
			break;

			case MESSAGE_CLOCK:
			{
				BOOL doprint = false;

				if(lb_buff->fields[FIELD1][0] == 'T')
				{
					strcpy(g_tempStr,lb_buff->fields[FIELD2]);

					if(strlen(g_tempStr) <= MAX_PATTERN_TEXT_LENGTH)
					{
						strcat(g_tempStr,"Z");
						rv3028_set_date_time(g_tempStr);    /* "2018-03-23T18:00:00Z" */
					}

					time_t t = rv3028_get_epoch(NULL);
					sprintf(g_tempStr,"CLK:%lu\n",t);
					doprint = true;
				}
				else if(lb_buff->fields[FIELD1][0] == 'O')
				{
					if(lb_buff->fields[FIELD2][0])
					{
						uint16_t o = (uint16_t)atoi(lb_buff->fields[FIELD2]);

						if(o < 512)
						{
							rv3028_set_offset_RAM(o);
						}
					}

					int a = rv3028_get_offset_RAM();
					sprintf(g_tempStr,"C=%d\n",a);
					doprint = true;
				}
				else if(lb_buff->fields[FIELD1][0] == 'X')
				{
					EIMSK &= ~(1 << INT0);   /* Disable INT0 interrupts */
					rv3028_32kHz_sqw();
				}

				if(doprint)
				{
					lb_send_string(g_tempStr,TRUE);
				}
			}
			break;

			case MESSAGE_TEMP:
			{
				if(lb_buff->fields[FIELD1][0] == 'C')
				{
					if(lb_buff->fields[FIELD2][0])
					{
						int16_t v = atoi(lb_buff->fields[FIELD2]);

						if((v > -2000) && (v < 2000))
						{
							g_temp_calibration = v;
							eeprom_update_word((uint16_t*)&ee_temp_calibration,(int16_t)g_temp_calibration);
						}
					}

					sprintf(g_tempStr,"T Cal= %d\n",g_temp_calibration);
					lb_send_string(g_tempStr,FALSE);
				}

				sprintf(g_tempStr,"T=%d\n",g_temperature);
				lb_send_string(g_tempStr,TRUE);
			}
			break;

			default:
			{
				lb_send_Help();
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


/*
 *  Command set:
 *  C1 c...c # - Set callsign ID to c...c
 *  C2 n...n # - Set fox ID and format (0 = beacon, 1 = MOE Classic, ... )
 *  C4 tttttttt # - Set start date/time = tttttttt
 *  C5 tttttttt # - Set finish date/time = tttttttt
 *  C6# - Start competition immediately, synchronize now, ignore RTC start time
 *  C7 YYMMDDhhmmss # - Synchronize RTC to date/time = YYMMDDhhmmss
 *  C8 nn # - Set HT or 80m fox behavior (affects PTT timing, audio out?, attenuator control?, etc.)
 *  AA nn # - Address the following command to only those foxes in the specified competition format (used in case more than one competition's foxes might be listening)
 *
 */
void processKey(char key)
{
	static int state = STATE_SENTENCE_START;
	static int digits;
	static int value;
	static int stringLength;
	static char receivedString[MAX_PATTERN_TEXT_LENGTH + 1] = { '\0' };

	switch(state)
	{
		case STATE_SHUTDOWN:
		{
			if(key == '*')
			{
				state = STATE_SENTENCE_START;
			}
		}
		break;

		case STATE_SENTENCE_START:
		{
			stringLength = 0;
			value = 0;
			digits = 0;

			if(key == 'C')
			{
				state = STATE_C;
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
				state = STATE_RECEIVING_START_NOW;
			}
			else if(key == '7') /* *C7YYMMDDhhmmss# Set RTC to this time and date */
			{
				state = STATE_RECEIVING_SET_CLOCK;
			}
		}
		break;

		case STATE_RECEIVING_CALLSIGN:
		{
			if(key == '#')
			{
				permCallsign(receivedString);
				lb_send_string(receivedString,FALSE);
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
					permFox(holdFox);
					if(holdFox != g_fox)
					{
						setupForFox(&holdFox);
					}
				}
				state = STATE_SHUTDOWN;
			}
			else if((key >= '0') && (key <= '9'))
			{
				value += key - '0';
			}
		}
		break;

		case STATE_RECEIVING_START_TIME:
		{
			if(key == '#')
			{
				state = STATE_SHUTDOWN;
			}
			else if((key >= '0') && (key <= '9'))
			{

			}
		}
		break;

		case STATE_RECEIVING_FINISH_TIME:
		{
			if(key == '#')
			{
				state = STATE_SHUTDOWN;
			}
			else if((key >= '0') && (key <= '9'))
			{

			}
		}
		break;

		case STATE_RECEIVING_START_NOW:
		{
			if(key == '#')
			{
				state = STATE_SHUTDOWN;
			}
			else if((key >= '0') && (key <= '9'))
			{

			}
		}
		break;

		case STATE_RECEIVING_SET_CLOCK:
		{
			if(key == '#')
			{
				if(stringLength == 12)
				{
					/* string = "YYMMDDhhmmss" */
					sprintf(g_tempStr,"20%c%c-%c%c-%c%cT%c%c:%c%c:%c%c",receivedString[0],receivedString[1],receivedString[2],receivedString[3],receivedString[4],receivedString[5],receivedString[6],receivedString[7],receivedString[8],receivedString[9],receivedString[10],receivedString[11]);
					rv3028_set_date_time(g_tempStr);    /* "2018-03-23T18:00:00Z" */
					state = STATE_SHUTDOWN;
				}
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
	}
}

/*
 * Set non-volatile variables to their values stored in EEPROM
 */
void initializeEEPROMVars(BOOL resetAll)
{
	uint16_t i;
	uint8_t initialization_flag = eeprom_read_byte(&ee_interface_eeprom_initialization_flag);

	i2c_init(); /* Needs to happen here */

	if(!resetAll && (initialization_flag == EEPROM_INITIALIZED_FLAG))   /* EEPROM is up to date */
	{
		g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM,eeprom_read_byte(&ee_id_codespeed),MAX_CODE_SPEED_WPM);
		g_fox = CLAMP(BEACON,(Fox_t)eeprom_read_byte(&ee_fox_setting),INVALID_FOX);
		g_clock_calibration = eeprom_read_word(&ee_clock_calibration);
		g_temp_calibration = (int16_t)eeprom_read_word((uint16_t*)&ee_temp_calibration);
		g_rv3028_offset = (int16_t)eeprom_read_word((uint16_t*)&ee_rv3028_offset);
		g_enable_LEDs = eeprom_read_byte(&ee_enable_LEDs);
		g_enable_start_timer = eeprom_read_byte(&ee_enable_start_timer);
		g_enable_transmitter = eeprom_read_byte(&ee_enable_transmitter);

		for(i = 0; i < MAX_PATTERN_TEXT_LENGTH; i++)
		{
			g_messages_text[STATION_ID][i] = (char)eeprom_read_byte((uint8_t*)(&ee_stationID_text[i]));
			if(!g_messages_text[STATION_ID][i])
			{
				break;
			}
		}
	}
	else    /* EEPROM is missing some updates */
	{
		if(resetAll || (eeprom_read_byte(&ee_id_codespeed) == 0xFF))
		{
			g_id_codespeed = EEPROM_ID_CODE_SPEED_DEFAULT;
			eeprom_update_byte(&ee_id_codespeed,g_id_codespeed);
		}
		else
		{
			g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM,eeprom_read_byte(&ee_id_codespeed),MAX_CODE_SPEED_WPM);
		}

		if(resetAll || (eeprom_read_byte(&ee_fox_setting) == 0xFF))
		{
			g_fox = EEPROM_FOX_SETTING_DEFAULT;
			eeprom_update_byte(&ee_fox_setting,g_fox);
		}
		else
		{
			g_fox = CLAMP(BEACON,(Fox_t)eeprom_read_byte(&ee_fox_setting),INVALID_FOX);
		}

		if(resetAll || (eeprom_read_word(&ee_clock_calibration) == 0xFFFF))
		{
			g_clock_calibration = EEPROM_CLOCK_CALIBRATION_DEFAULT;
			eeprom_update_word(&ee_clock_calibration,g_clock_calibration);  /* Calibration only gets set by a serial command */
		}
		else
		{
			g_clock_calibration = eeprom_read_word(&ee_clock_calibration);
		}

		if(resetAll || ((uint16_t)eeprom_read_word((uint16_t*)&ee_temp_calibration) == 0xFFFF))
		{
			g_temp_calibration = EEPROM_TEMP_CALIBRATION_DEFAULT;
			eeprom_update_word((uint16_t*)&ee_temp_calibration,(uint16_t)g_temp_calibration);
		}
		else
		{
			g_temp_calibration = (int16_t)eeprom_read_word((uint16_t*)&ee_temp_calibration);
		}

		if((uint16_t)eeprom_read_word((uint16_t*)&ee_rv3028_offset) == 0xFFFF)
		{
			g_rv3028_offset = rv3028_get_offset_RAM();
			eeprom_update_word((uint16_t*)&ee_rv3028_offset,(uint16_t)g_rv3028_offset);
			g_allow_rv3028_eeprom_changes = TRUE; /* Allow 1-sec interrupt to get permanently saved within the RTC EEPROM */
		}
		else
		{
			g_rv3028_offset = (int16_t)eeprom_read_word((uint16_t*)&ee_rv3028_offset);
		}

		if(resetAll || (eeprom_read_byte(&ee_enable_LEDs) == 0xFF))
		{
			g_enable_LEDs = EEPROM_ENABLE_LEDS_DEFAULT;
			eeprom_update_byte(&ee_enable_LEDs,g_enable_LEDs);  /* Only gets set by a serial command */
		}
		else
		{
			g_enable_LEDs = eeprom_read_byte(&ee_enable_LEDs);
		}

		if(resetAll || (eeprom_read_byte(&ee_enable_start_timer) == 0xFF))
		{
			g_enable_start_timer = EEPROM_ENABLE_STARTTIMER_DEFAULT;
			eeprom_update_byte(&ee_enable_start_timer,g_enable_start_timer);    /* Only gets set by a serial command */
		}
		else
		{
			g_enable_start_timer = eeprom_read_byte(&ee_enable_start_timer);
		}

		if(resetAll || (eeprom_read_byte(&ee_enable_transmitter) == 0xFF))
		{
			g_enable_transmitter = EEPROM_ENABLE_TRANSMITTER_DEFAULT;
			eeprom_update_byte(&ee_enable_transmitter,g_enable_transmitter);    /* Only gets set by a serial command */
		}
		else
		{
			g_enable_transmitter = eeprom_read_byte(&ee_enable_transmitter);
		}

		if(resetAll || (eeprom_read_byte((uint8_t*)(&ee_stationID_text[0])) == 0xFF))
		{
			uint16_t i;
			strncpy(g_messages_text[STATION_ID],EEPROM_STATION_ID_DEFAULT,MAX_PATTERN_TEXT_LENGTH);

			for(i = 0; i < strlen(g_messages_text[STATION_ID]); i++)    /* Only gets set by a serial command */
			{
				eeprom_update_byte((uint8_t*)&ee_stationID_text[i],(uint8_t)g_messages_text[STATION_ID][i]);
			}
			eeprom_update_byte((uint8_t*)&ee_stationID_text[i],0);
		}
		else
		{
			for(i = 0; i < MAX_PATTERN_TEXT_LENGTH; i++)
			{
				g_messages_text[STATION_ID][i] = (char)eeprom_read_byte((uint8_t*)(&ee_stationID_text[i]));
				if(!g_messages_text[STATION_ID][i])
				{
					break;
				}
			}
		}

		/* Each correction pulse = 1 tick corresponds to 1 / (16384 × 64) = 0.9537 ppm.
		 * ppm frequency change = -0.035 * (T-T0)^2 (±10%)
		 * Table[0] = 25C, Table[1] = 24C or 26C, Table[2] = 23C or 27C, etc. */
		for(i = 0; i < SIZE_OF_TEMPERATURE_TABLE; i++)  /* Use 1-degree steps and take advantage of parabola symmetry for -35C to +85C coverage */
		{
			uint16_t val = (uint16_t)(((i * i) * 37L) / 1000L);
			eeprom_update_byte((uint8_t*)&ee_temperature_table[i],val);
		}

		eeprom_write_byte(&ee_interface_eeprom_initialization_flag,EEPROM_INITIALIZED_FLAG);
	}

	return;
}

void setupForFox(Fox_t* fox)
{
	cli();
	g_seconds_since_sync = 0;   /* Total elapsed time counter */
	g_on_the_air       = FALSE; /* Controls transmitter Morse activity */
	g_code_throttle    = 0;     /* Adjusts Morse code speed */
	g_callsign_sent = FALSE;

	g_on_air_interval = 0;
	g_fox_seconds_into_interval = 0;
	g_number_of_foxes = 0;
	g_fox_transition = FALSE;
	g_fox_id_offset = 0;
	g_id_interval = 0;
	g_time_to_ID = FALSE;
	g_audio_tone_state = OFF;
	sei();

	g_LEDs_Timed_Out = !g_enable_LEDs;
	digitalWrite(PIN_LED2,OFF); /*  LED Off - in case it was on in the middle of a transmission */

	g_fox = BEACON;

	if(fox)
	{
		if(*fox != INVALID_FOX)
		{
			g_fox = *fox;
		}
	}

	switch(g_fox)
	{
		case NO_CODE_START_TONES_2M:
		{
			g_startclock_interval = 120;
			g_enable_start_timer = TRUE;
		}
		break;

		case NO_CODE_START_TONES_5M:
		{
			g_startclock_interval = 300;
			g_enable_start_timer = TRUE;
		}
		break;

		case FOX_1:
		case FOX_2:
		case FOX_3:
		case FOX_4:
		case FOX_5:
		case FOX_DEMO:
		{
			g_on_air_interval = 60;
			g_number_of_foxes = 5;
			g_fox_id_offset = 0;
			g_pattern_codespeed = 8;
			g_id_interval = 60;
			g_startclock_interval = 300;
		}
		break;

		case SPRINT_S1:
		case SPRINT_F5:
		case SPRINT_DEMO:
		{
			g_on_air_interval = 12;
			g_number_of_foxes = 5;
			g_pattern_codespeed = ((g_fox == SPRINT_DEMO) || (g_fox <= SPRINT_S5)) ? 10 : 15;
			g_fox_id_offset = g_fox <= SPRINT_S5 ? SPRINT_S1 - 1 : SPRINT_F1 - 1;
			g_id_interval = 600;
			g_startclock_interval = 120;
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
			g_startclock_interval = (g_fox == SPECTATOR) ? 120 : 300;
		}
		break;
	}
}

void permCallsign(char* call)
{
	uint8_t i;

	strncpy(g_messages_text[STATION_ID],call,MAX_PATTERN_TEXT_LENGTH);

	for(i = 0; i < strlen(g_messages_text[STATION_ID]); i++)
	{
		eeprom_update_byte((uint8_t*)&ee_stationID_text[i],(uint8_t)g_messages_text[STATION_ID][i]);
	}

	eeprom_update_byte((uint8_t*)&ee_stationID_text[i],0);
}

void permFox(Fox_t fox)
{
	eeprom_update_byte((uint8_t*)&ee_fox_setting,(uint8_t)fox);
}


/*
 * Set up registers for measuring processor temperature
 */
void setUpTemp(void)
{
	/* The internal temperature has to be used
	 * with the internal reference of 1.1V.
	 * Channel 8 can not be selected with
	 * the analogRead function yet. */
	/* Set the internal reference and mux. */
	ADMUX = ((1 << REFS1) | (1 << REFS0) | (1 << MUX3));

	/* Slow the ADC clock down to 125 KHz
	 * by dividing by 128. Assumes that the
	 * standard Arduino 16 MHz clock is in use. */
	ADCSRA = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
	ADCSRA |= (1 << ADEN);  /* enable the ADC */
	ADCSRA |= (1 << ADSC);  /* Start the ADC */
	readADC();
}

/*
 * Read the temperature ADC value
 */
uint16_t readADC()
{
	/* Make sure the most recent ADC read is complete. */
	while((ADCSRA & (1 << ADSC)))
	{
		;   /* Just wait for ADC to finish. */
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
	float offset = CLAMP(-200.,(float)g_temp_calibration / 10.,200.);

	/* The offset in 1/10ths C (first term) was determined empirically */
	readADC();  /* throw away first reading */
	return(roundf(offset + (readADC() - 324.31) / 1.22));
}

void setUpAudioSampling(BOOL enableSampling)
{
	ADCSRA = 0;                             /* clear ADCSRA register */
	ADCSRB = 0;                             /* clear ADCSRB register */
	ADMUX = 0;
	ADMUX |= 0x06;                          /* set A6 analog input pin */
	ADMUX |= (1 << REFS1) | (1 << REFS0);   /* set reference voltage to internal 1.1V */
	ADMUX |= (1 << ADLAR);                  /* left align ADC value to 8 bits from ADCH register */

	/* sampling rate is [ADC clock] / [prescaler] / [conversion clock cycles]
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
