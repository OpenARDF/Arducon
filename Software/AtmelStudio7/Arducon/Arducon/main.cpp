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

/*#define SAMPLE_RATE 9630 */
#define SAMPLE_RATE 19260
/*#define SAMPLE_RATE 38520
 *#define SAMPLE_RATE 77040
 *#define SAMPLE_RATE 154080 */

/***********************************************************************
 * Global Variables & String Constants
 *
 * Identify each global with a "g_" prefix
 * Whenever possible limit globals' scope to this file using "static"
 * Use "volatile" for globals shared between ISRs and foreground loop
 ************************************************************************/
static uint8_t EEMEM ee_interface_eeprom_initialization_flag = EEPROM_UNINITIALIZED;
static char EEMEM ee_stationID_text[MAX_PATTERN_TEXT_LENGTH + 1];
static uint8_t EEMEM ee_id_codespeed;
static uint16_t EEMEM ee_clock_calibration;
/*static uint8_t EEMEM ee_override_DIP_switches; */
static uint8_t EEMEM ee_enable_LEDs;
static int16_t EEMEM ee_temp_calibration;
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


const int N = 200;
const float threshold = 4000000.;
const float sampling_freq = SAMPLE_RATE;
const float x_frequencies[4] = { 1209., 1336., 1477., 1633. };
const float y_frequencies[4] = { 697., 770., 852., 941. };
const char key[16] = { '1', '2', '3', 'A', '4', '5', '6', 'B', '7', '8', '9', 'C', '*', '0', '#', 'D' };
const char keyMorse[39] = { ' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U',
							'V', 'W', 'X', 'Y', 'Z', '<', '/', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };
char g_lastKey = '\0';
unsigned long g_last = 0;
unsigned long g_tick_count = 0;

Goertzel g_goertzel(N, sampling_freq);

static char g_tempStr[TEMP_STRING_LENGTH] = { '\0' };

/*
 * Local function prototypes
 */
void processKey(char key);
void initializeEEPROMVars(BOOL resetAll);
void permCallsign(char* call);
void handleLinkBusMsgs(void);

/***********************************************************************
 * ADC Conversion Complete ISR
 ************************************************************************/
ISR(ADC_vect)
{
	digitalWrite(PIN_CAL_OUT, ON);
	if(g_goertzel.DataPoint(ADCH))
	{
		ADCSRA &= ~(1 << ADIE); /* disable ADC interrupt */
	}
	digitalWrite(PIN_CAL_OUT, OFF);
}



/***********************************************************************
 * Timer/Counter2 Compare Match B ISR
 *
 * Handles periodic tasks not requiring precise timing.
 ************************************************************************/
ISR( TIMER2_COMPB_vect )
{
	g_tick_count++;
}   /* End of Timer 2 ISR */


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



#if COMPILE_FOR_ATMELSTUDIO7
	void loop(void);
	int main(void)
#else
	void setup()
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */
{
	initializeEEPROMVars(FALSE);
	linkbus_init(BAUD);                     /* Start the Link Bus serial comms */
	/* set pins as outputs */
	pinMode(PIN_LED, OUTPUT);       /* The amber LED: This led blinks when off cycle and blinks with code when on cycle. */
	digitalWrite(PIN_LED, OFF);
	pinMode(PIN_MORSE_KEY, OUTPUT); /* This pin is used to control the KEY line to the transmitter only active on cycle. */
	digitalWrite(PIN_MORSE_KEY, OFF);
	pinMode(PIN_AUDIO_OUT, OUTPUT);
	digitalWrite(PIN_AUDIO_OUT, OFF);

	/* Set unused pins as outputs pulled high */
	pinMode(PIN_UNUSED_7, INPUT_PULLUP);
	pinMode(PIN_UNUSED_8, INPUT_PULLUP);
	pinMode(PIN_UNUSED_10, INPUT_PULLUP);
	pinMode(PIN_UNUSED_12, INPUT_PULLUP);
	pinMode(A0, INPUT); /* Audio input */
	pinMode(A1, INPUT_PULLUP);
	pinMode(A2, INPUT_PULLUP);
	pinMode(A3, INPUT_PULLUP);
	pinMode(PIN_CAL_OUT, OUTPUT);
	digitalWrite(PIN_CAL_OUT, OFF);

	ADCSRA = 0;             /* clear ADCSRA register */
	ADCSRB = 0;             /* clear ADCSRB register */
	ADMUX |= (0 & 0x07);    /* set A0 analog input pin */
	ADMUX |= (1 << REFS0);  /* set reference voltage */
	ADMUX |= (1 << ADLAR);  /* left align ADC value to 8 bits from ADCH register */

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

	ADCSRA |= (1 << ADATE); /* enable auto trigger */
	ADCSRA |= (1 << ADIE);  /* enable interrupts when measurement complete */
	ADCSRA |= (1 << ADEN);  /* enable ADC */
	ADCSRA |= (1 << ADSC);  /* start ADC measurements */


	/**
	 * TIMER2 is for periodic interrupts to drive Morse code generation */
	/* Reset control registers */
	TCCR2A = 0;
	TCCR2B = 0;
	TCCR2A |= (1 << WGM21);                             /* set Clear Timer on Compare Match (CTC) mode with OCR2A setting the top */
#if CAL_SIGNAL_ON_PD3
		pinMode(PIN_CAL_OUT, OUTPUT);                   /* 601Hz Calibration Signal */
		TCCR2A |= (1 << COM2A0);                        /* Toggle OC2A (PB3) on compare match */
#endif /* CAL_SIGNAL_ON_PD3 */
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
/*	TCCR0A = 0x00; */
/*	TCCR0A |= (1 << WGM01);     / * Set CTC mode * / */
/*	TCCR0B = 0x00; */
/*	TCCR0B |= (1 << CS02);      / * Prescale 256 * / */
/*	OCR0A = DEFAULT_TONE_FREQUENCY; */
/*	TIMSK0 = 0x00; */
/*	TIMSK0 |= (1 << OCIE0A); */

	lb_send_string((char*)SW_REVISION, FALSE);


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

/*
 *  Here is the main loop
 * */
void loop()
{
	handleLinkBusMsgs();

	if(g_goertzel.SamplesReady())
	{
		float magnitudeX;
		float magnitudeY;
		static char lastKey = '\0';
		static int checkCount = 10; /* Set above the threshold to prevent an initial false key detect */
		static int quietCount = 0;
		int x = -1, y = -1;

		for(int i = 0; i < 4; i++)
		{
			g_goertzel.SetTargetFrequency(y_frequencies[i]);    /*initialize library function with the given sampling frequency no of samples and target freq */
			magnitudeY = g_goertzel.Magnitude2();               /*check them for target_freq */

			if(magnitudeY > threshold)                          /*if you're getting false hits or no hits adjust the threshold */
			{
				y = i;
				break;
			}
		}

		if(y >= 0)
		{
			for(int i = 0; i < 4; i++)
			{
				g_goertzel.SetTargetFrequency(x_frequencies[i]);    /*initialize library function with the given sampling frequency no of samples and target freq */
				magnitudeX = g_goertzel.Magnitude2();               /*check them for target_freq */

				if(magnitudeX > threshold)                          /*if you're getting false hits or no hits adjust the threshold */
				{
					x = i;
					break;
				}
			}

			if(x >= 0)
			{
				char newKey = key[4 * y + x];

				if(lastKey == newKey)
				{
					checkCount++;

					if(checkCount == 5)
					{
						quietCount = 0;
						g_lastKey = newKey;
						sprintf(g_tempStr, "\"%c\"\n", g_lastKey);
						lb_send_string(g_tempStr, FALSE);

						processKey(newKey);
/*            Serial.print("Mag X/Y=");
 *            Serial.print(magnitudeX);
 *            Serial.print(" / ");
 *            Serial.println(magnitudeY); */
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
				if(delta < 1500)
				{
					checkCount = 0;
				}

				quietCount = 0;
				g_last = g_tick_count;
				lastKey = '\0';
			}
		}

		ADCSRA |= (1 << ADIE);  /* enable ADC interrupt */
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

			case MESSAGE_OVERRIDE_DIP:
			{
			}
			break;

			case MESSAGE_LEDS:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					if((lb_buff->fields[FIELD1][1] == 'F') || (lb_buff->fields[FIELD1][0] == '0'))
					{
						g_enable_LEDs = FALSE;
						digitalWrite(PIN_LED,OFF);  /*  LED Off */
					}
					else
					{
						g_enable_LEDs = TRUE;
					}

					eeprom_update_byte(&ee_enable_LEDs,g_enable_LEDs);
//					g_LEDs_Timed_Out = !g_enable_LEDs;
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

			case MESSAGE_TRANSMITTER_ENABLE:
			{
			}
			break;

			case MESSAGE_GO:
			{
			}
			break;

			case MESSAGE_FACTORY_RESET:
			{
//				initializeEEPROMVars(TRUE);
//				softwareReset();
			}
			break;

			case MESSAGE_CLOCK_CAL:
			{
				if(lb_buff->fields[FIELD1][0])
				{
					uint16_t c = (uint16_t)atoi(lb_buff->fields[FIELD1]);

					if(c > 100)
					{
						g_clock_calibration = c;
						OCR1A = g_clock_calibration;
						eeprom_update_word(&ee_clock_calibration,g_clock_calibration);
					}
				}

				sprintf(g_tempStr,"Cal=%u\n",g_clock_calibration);
				lb_send_string(g_tempStr,FALSE);
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
//					g_time_needed_for_ID = (500 + timeRequiredToSendStrAtWPM(g_messages_text[STATION_ID],g_id_codespeed)) / 1000;
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
//							g_time_needed_for_ID = (500 + timeRequiredToSendStrAtWPM(g_messages_text[STATION_ID],g_id_codespeed)) / 1000;
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

			case MESSAGE_TEMP:
			{
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
Commands:
General (All foxes respond to these)
	C1 - Set callsign
	C2 - Set competition type
	C3 - Set clock date & time
	C4 - Synchronize transmit schedule immediately (ignoring clock time)
	C* - Start transmission schedule immediately (observe clock settings)
	C# - Cancel transmission schedule immediately
	C5 - Set start time
	C6 - Set finish time
	C7 nn nn nn nn- Lock DTMF access

Fox-specific
	D1 nn- Set unique fox ID#
	D2 nn nn nn nn - Set universal PIN number
	D2 nn - Report battery level
	D3 nn - Report all settings
	D* nn - Lock DTMF access
	D# nn - Unlock DTMF access

*/
void processKey(char key)
{
	static int state = STATE_IDLE;
	static int digits;
	static int value;
	static int callsignLength;
	static char callsign[MAX_PATTERN_TEXT_LENGTH + 1] = { '\0' };

	switch(state)
	{
		case STATE_IDLE:
		{
			callsignLength = 0;
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
		}
		break;

		case STATE_RECEIVING_CALLSIGN:
		{
			if(key == '#')
			{
				permCallsign(callsign);
				lb_send_string(callsign, FALSE);
				state = STATE_IDLE;
			}
			else if((key >= '0') && (key <= '9'))
			{
				if(digits)
				{
					value = value * 10 + (key - '0');
					if((value < 39) && (callsignLength < MAX_PATTERN_TEXT_LENGTH))
					{
						callsign[callsignLength] = keyMorse[value];
						callsignLength++;
						callsign[callsignLength] = '\0';
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
	}
}

/*
 * Set non-volatile variables to their values stored in EEPROM
 */
void initializeEEPROMVars(BOOL resetAll)
{
	uint8_t i;

	uint8_t initialization_flag = eeprom_read_byte(&ee_interface_eeprom_initialization_flag);

	if(!resetAll && (initialization_flag == EEPROM_INITIALIZED_FLAG))   /* EEPROM is up to date */
	{
		g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM, eeprom_read_byte(&ee_id_codespeed), MAX_CODE_SPEED_WPM);
		g_clock_calibration = eeprom_read_word(&ee_clock_calibration);
		g_temp_calibration = (int16_t)eeprom_read_word((uint16_t*)&ee_temp_calibration);
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
			eeprom_update_byte(&ee_id_codespeed, g_id_codespeed);
		}
		else
		{
			g_id_codespeed = CLAMP(MIN_CODE_SPEED_WPM, eeprom_read_byte(&ee_id_codespeed), MAX_CODE_SPEED_WPM);
		}

		if(resetAll || (eeprom_read_word(&ee_clock_calibration) == 0xFFFF))
		{
			g_clock_calibration = EEPROM_CLOCK_CALIBRATION_DEFAULT;
			eeprom_update_word(&ee_clock_calibration, g_clock_calibration); /* Calibration only gets set by a serial command */
		}
		else
		{
			g_clock_calibration = eeprom_read_word(&ee_clock_calibration);
		}

		if(resetAll || ((uint16_t)eeprom_read_word((uint16_t*)&ee_temp_calibration) == 0xFFFF))
		{
			g_temp_calibration = EEPROM_TEMP_CALIBRATION_DEFAULT;
			eeprom_update_word((uint16_t*)&ee_temp_calibration, (uint16_t)g_temp_calibration);
		}
		else
		{
			g_temp_calibration = (int16_t)eeprom_read_word((uint16_t*)&ee_temp_calibration);
		}

		if(resetAll || (eeprom_read_byte(&ee_enable_LEDs) == 0xFF))
		{
			g_enable_LEDs = EEPROM_ENABLE_LEDS_DEFAULT;
			eeprom_update_byte(&ee_enable_LEDs, g_enable_LEDs); /* Only gets set by a serial command */
		}
		else
		{
			g_enable_LEDs = eeprom_read_byte(&ee_enable_LEDs);
		}

		if(resetAll || (eeprom_read_byte(&ee_enable_start_timer) == 0xFF))
		{
			g_enable_start_timer = EEPROM_ENABLE_STARTTIMER_DEFAULT;
			eeprom_update_byte(&ee_enable_start_timer, g_enable_start_timer);   /* Only gets set by a serial command */
		}
		else
		{
			g_enable_start_timer = eeprom_read_byte(&ee_enable_start_timer);
		}

		if(resetAll || (eeprom_read_byte(&ee_enable_transmitter) == 0xFF))
		{
			g_enable_transmitter = EEPROM_ENABLE_TRANSMITTER_DEFAULT;
			eeprom_update_byte(&ee_enable_transmitter, g_enable_transmitter);   /* Only gets set by a serial command */
		}
		else
		{
			g_enable_transmitter = eeprom_read_byte(&ee_enable_transmitter);
		}

		if(resetAll || (eeprom_read_byte((uint8_t*)(&ee_stationID_text[0])) == 0xFF))
		{
			uint16_t i;
			strncpy(g_messages_text[STATION_ID], EEPROM_STATION_ID_DEFAULT, MAX_PATTERN_TEXT_LENGTH);

			for(i = 0; i < strlen(g_messages_text[STATION_ID]); i++)    /* Only gets set by a serial command */
			{
				eeprom_update_byte((uint8_t*)&ee_stationID_text[i], (uint8_t)g_messages_text[STATION_ID][i]);
			}
			eeprom_update_byte((uint8_t*)&ee_stationID_text[i], 0);
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

		eeprom_write_byte(&ee_interface_eeprom_initialization_flag, EEPROM_INITIALIZED_FLAG);
	}

	return;
}

void permCallsign(char* call)
{
	uint8_t i;

	strncpy(g_messages_text[STATION_ID], call, MAX_PATTERN_TEXT_LENGTH);

	for(i = 0; i < strlen(g_messages_text[STATION_ID]); i++)
	{
		eeprom_update_byte((uint8_t*)&ee_stationID_text[i], (uint8_t)g_messages_text[STATION_ID][i]);
	}

	eeprom_update_byte((uint8_t*)&ee_stationID_text[i], 0);
}

