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
/* linkbus.c
 *
 */

#include "linkbus.h"
#include "defs.h"
#include "eepromStrings.h"

#if COMPILE_FOR_ATMELSTUDIO7
#include <avr/eeprom.h>
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

#if COMPILE_FOR_ATMELSTUDIO7
#include <string.h>
#include <stdio.h>
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

/* Global Variables */
static volatile BOOL g_bus_disabled = TRUE;
static const char crlf[] = "\n";
static char lineTerm[8] = "\n";
static const char textPrompt[] = "> ";

extern char EEMEM ee_textHelp[]; /* Size is printed when this program is compiled and run with INIT_EEPROM_ONLY = TRUE */
extern char EEMEM ee_textVersion[];

static char g_tempMsgBuff[LINKBUS_MAX_MSG_LENGTH];

/* Local function prototypes */
BOOL linkbus_start_tx(void);
BOOL linkbus_send_text(char* text);

/* Module global variables */
static volatile BOOL linkbus_tx_active = FALSE; /* volatile is required to ensure optimizer handles this properly */
static LinkbusTxBuffer tx_buffer[LINKBUS_NUMBER_OF_TX_MSG_BUFFERS];
static LinkbusRxBuffer rx_buffer[LINKBUS_NUMBER_OF_RX_MSG_BUFFERS];

LinkbusTxBuffer* nextFullTxBuffer(void)
{
	BOOL found = TRUE;
	static uint8_t bufferIndex = 0;
	uint8_t count = 0;

	while(tx_buffer[bufferIndex][0] == '\0')
	{
		if(++count >= LINKBUS_NUMBER_OF_TX_MSG_BUFFERS)
		{
			found = FALSE;
			break;
		}

		bufferIndex++;
		if(bufferIndex >= LINKBUS_NUMBER_OF_TX_MSG_BUFFERS)
		{
			bufferIndex = 0;
		}
	}

	if(found)
	{
		return( &tx_buffer[bufferIndex]);
	}

	return(null);
}

LinkbusTxBuffer* nextEmptyTxBuffer(void)
{
	BOOL found = TRUE;
	static uint8_t bufferIndex = 0;
	uint8_t count = 0;

	while(tx_buffer[bufferIndex][0] != '\0')
	{
		if(++count >= LINKBUS_NUMBER_OF_TX_MSG_BUFFERS)
		{
			found = FALSE;
			break;
		}

		bufferIndex++;
		if(bufferIndex >= LINKBUS_NUMBER_OF_TX_MSG_BUFFERS)
		{
			bufferIndex = 0;
		}
	}

	if(found)
	{
		return( &tx_buffer[bufferIndex]);
	}

	return(null);
}

LinkbusRxBuffer* nextEmptyRxBuffer(void)
{
	BOOL found = TRUE;
	static uint8_t bufferIndex = 0;
	uint8_t count = 0;

	while(rx_buffer[bufferIndex].id != MESSAGE_EMPTY)
	{
		if(++count >= LINKBUS_NUMBER_OF_RX_MSG_BUFFERS)
		{
			found = FALSE;
			break;
		}

		bufferIndex++;
		if(bufferIndex >= LINKBUS_NUMBER_OF_RX_MSG_BUFFERS)
		{
			bufferIndex = 0;
		}
	}

	if(found)
	{
		return( &rx_buffer[bufferIndex]);
	}

	return(null);
}

LinkbusRxBuffer* nextFullRxBuffer(void)
{
	BOOL found = TRUE;
	static uint8_t bufferIndex = 0;
	uint8_t count = 0;

	while(rx_buffer[bufferIndex].id == MESSAGE_EMPTY)
	{
		if(++count >= LINKBUS_NUMBER_OF_RX_MSG_BUFFERS)
		{
			found = FALSE;
			break;
		}

		bufferIndex++;
		if(bufferIndex >= LINKBUS_NUMBER_OF_RX_MSG_BUFFERS)
		{
			bufferIndex = 0;
		}
	}

	if(found)
	{
		return( &rx_buffer[bufferIndex]);
	}

	return(null);
}


/***********************************************************************
 * linkbusTxInProgress(void)
 ************************************************************************/
BOOL linkbusTxInProgress(void)
{
	return(linkbus_tx_active);
}

BOOL linkbus_start_tx(void)
{
	BOOL success = !linkbus_tx_active;

	if(success) /* message will be lost if transmit is busy */
	{
		linkbus_tx_active = TRUE;
		UCSR0B |= (1 << UDRIE0);
	}

	return(success);
}

void linkbus_end_tx(void)
{
	if(linkbus_tx_active)
	{
		UCSR0B &= ~(1 << UDRIE0);
		linkbus_tx_active = FALSE;
	}
}

void linkbus_reset_rx(void)
{
	if(UCSR0B & (1 << RXEN0))   /* perform only if rx is currently enabled */
	{
		UCSR0B &= ~(1 << RXEN0);
/*    uint16_t s = sizeof(rx_buffer); // test */
		memset(rx_buffer, 0, sizeof(rx_buffer));
/*    if(s) s = 0; // test */
		UCSR0B |= (1 << RXEN0);
	}
}

void linkbus_init(uint32_t baud)
{
	memset(rx_buffer, 0, sizeof(rx_buffer));

	for(int bufferIndex = 0; bufferIndex < LINKBUS_NUMBER_OF_TX_MSG_BUFFERS; bufferIndex++)
	{
		tx_buffer[bufferIndex][0] = '\0';
	}

	/*Set baud rate */
	uint16_t myubrr = MYUBRR(baud);
	UBRR0H = (uint8_t)(myubrr >> 8);
	UBRR0L = (uint8_t)myubrr;
	/* Enable receiver and transmitter and related interrupts */
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);
/*  UCSR0B = (1<<RXEN0) | (1<<TXEN0);
 * Set frame format: 8data, 2stop bit */
	UCSR0C = (1 << USBS0) | (3 << UCSZ00);
	g_bus_disabled = FALSE;

#if INIT_EEPROM_ONLY
		saveEEPROM();
#endif  /* INIT_EEPROM_ONLY */
}

void linkbus_disable(void)
{
	uint8_t bufferIndex;

	g_bus_disabled = TRUE;
	UCSR0B = 0;
	linkbus_end_tx();
	memset(rx_buffer, 0, sizeof(rx_buffer));

	for(bufferIndex = 0; bufferIndex < LINKBUS_NUMBER_OF_TX_MSG_BUFFERS; bufferIndex++)
	{
		tx_buffer[bufferIndex][0] = '\0';
	}
}

void linkbus_enable(void)
{
	uint8_t bufferIndex;

	g_bus_disabled = FALSE;
	UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

	memset(rx_buffer, 0, sizeof(rx_buffer));

	for(bufferIndex = 0; bufferIndex < LINKBUS_NUMBER_OF_TX_MSG_BUFFERS; bufferIndex++)
	{
		tx_buffer[bufferIndex][0] = '\0';
	}
}


BOOL linkbus_send_text(char* text)
{
	BOOL err = TRUE;
	uint16_t tries = 200;

	if(g_bus_disabled)
	{
		return( err);
	}

	if(text)
	{
		LinkbusTxBuffer* buff = nextEmptyTxBuffer();

		while(!buff && tries)
		{
			while(linkbusTxInProgress() && tries)
			{
				if(tries)
				{
					tries--;    /* wait until transmit finishes */
				}
			}
			buff = nextEmptyTxBuffer();
		}

		if(buff)
		{
			sprintf(*buff, text);

			linkbus_start_tx();
			err = FALSE;
		}
	}

	return(err);
}


/***********************************************************************************
 *  Support for creating and sending various Terminal Mode Linkbus messages is provided below.
 ************************************************************************************/

void lb_send_NewPrompt(void)
{
	while(linkbus_send_text((char*)textPrompt))
	{
		;
	}
}

void lb_send_NewLine(void)
{
	linkbus_send_text((char*)crlf);
}

void lb_echo_char(uint8_t c)
{
	g_tempMsgBuff[0] = c;
	g_tempMsgBuff[1] = '\0';
	linkbus_send_text(g_tempMsgBuff);
}

BOOL lb_send_string(char* str, BOOL wait)
{
	BOOL err = FALSE;

	if(str == NULL)
	{
		return( TRUE);
	}

	if(strlen(str) > LINKBUS_MAX_TX_MSG_LENGTH)
	{
		return( TRUE);
	}

	strncpy(g_tempMsgBuff, str, LINKBUS_MAX_TX_MSG_LENGTH);

	if(wait)
	{
		while((err = linkbus_send_text(g_tempMsgBuff)))
		{
			;
		}
		while(!err && linkbusTxInProgress())
		{
			;
		}
	}
	else
	{
		err = linkbus_send_text(g_tempMsgBuff);
	}

	return( err);
}

void lb_send_value(uint16_t value, char* label)
{
	sprintf(g_tempMsgBuff, "> %s=%d%s", label, value, lineTerm);
	linkbus_send_text(g_tempMsgBuff);
}

/***********************************************************************
 * lb_send_Help(void)
 ************************************************************************/
void lb_send_Help(void)
{
	if(g_bus_disabled)
	{
		return;
	}

	lb_send_NewLine();
	sendEEPROMString(&ee_textVersion[0]);
	sendEEPROMString(&ee_textHelp[0]);
	lb_send_NewLine();
	lb_send_NewLine();
}
