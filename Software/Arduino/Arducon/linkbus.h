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
/*
 * linkbus.h - a simple serial inter-processor communication protocol.
 */

#ifndef LINKBUS_H_
#define LINKBUS_H_

#include "defs.h"

#define LINKBUS_MAX_MSG_LENGTH 50
#define LINKBUS_MIN_MSG_LENGTH 2    /* shortest message: GO */
#define LINKBUS_MAX_MSG_FIELD_LENGTH 20
#define LINKBUS_MAX_MSG_NUMBER_OF_FIELDS 3
#define LINKBUS_NUMBER_OF_RX_MSG_BUFFERS 2
#define LINKBUS_MAX_TX_MSG_LENGTH 41
#define LINKBUS_NUMBER_OF_TX_MSG_BUFFERS 4

#define LINKBUS_MAX_COMMANDLINE_LENGTH ((1 + LINKBUS_MAX_MSG_FIELD_LENGTH) * LINKBUS_MAX_MSG_NUMBER_OF_FIELDS)

#define LINKBUS_POWERUP_DELAY_SECONDS 6

#define LINKBUS_MIN_TX_INTERVAL_MS 100

#define FOSC 16000000   /* Clock Speed */
#define BAUD 57600
#define MYUBRR(b) (FOSC / 16 / (b) - 1)

typedef enum
{
	EMPTY_BUFF,
	FULL_BUFF
} BufferState;

/*  Linkbus Messages
 *       Message formats:
 *               CMD [a1 [a2]]
 *
 *               where
 *                       CMD = command
 *                       a1, a2 = optional arguments or data fields
 *
 */

typedef enum
{
	MESSAGE_EMPTY = 0,

	/*	DUAL-BAND TX MESSAGE FAMILY (FUNCTIONAL MESSAGING) */
	MESSAGE_FACTORY_RESET = 'F' * 100 + 'A' * 10 + 'C',     /* Sets EEPROM back to defaults */
	MESSAGE_SET_FOX = 'F' * 100 + 'O' * 10 + 'X',           /* Override DIP switch settings using this value */
	MESSAGE_LEDS = 'L' * 100 + 'E' * 10 + 'D',              /* Turn on or off LEDs - accepts 1 or 0 or ON or OFF */
	MESSAGE_TEMP = 'T' * 100 + 'E' * 10 + 'M',              /* Temperature  data */
	MESSAGE_SET_STATION_ID = 'I' * 10 + 'D',                /* Sets amateur radio callsign text */
	MESSAGE_SYNC = 'S' * 100 + 'Y' * 10 + 'N',              /* Synchronizes transmissions */
	MESSAGE_CODE_SPEED = 'S' * 100 + 'P' * 10 + 'D',        /* Set Morse code speeds */
	MESSAGE_STARTTONES_ENABLE = 'S' * 100 + 'T' * 10 + 'A', /* Enables/disables the Starting Timer Tones */
	MESSAGE_CLOCK = 'C' * 100 + 'L' * 10 + 'K',             /* Set or read the RTC */
	/* UTILITY MESSAGES */
	MESSAGE_RESET = 'R' * 100 + 'S' * 10 + 'T',             /* Processor reset */
	MESSAGE_VERSION = 'V' * 100 + 'E' * 10 + +'R',          /* S/W version number */

	INVALID_MESSAGE = UINT16_MAX                            /* This value must never overlap a valid message ID */
} LBMessageID;

#define MESSAGE_VER_LABEL "VER"

typedef enum
{
	LINKBUS_MSG_UNKNOWN = 0,
	LINKBUS_MSG_COMMAND,
	LINKBUS_MSG_QUERY,
	LINKBUS_MSG_REPLY,
	LINKBUS_MSG_INVALID
} LBMessageType;

typedef enum
{
	FIELD1 = 0,
	FIELD2 = 1,
	FIELD3 = 2
} LBMessageField;

typedef enum
{
	BATTERY_BROADCAST = 0x0001,
	RSSI_BROADCAST = 0x0002,
	RF_BROADCAST = 0x0004,
	UPC_TEMP_BROADCAST = 0x0008,
	ALL_BROADCASTS = 0x000FF
} LBbroadcastType;

typedef enum
{
	NO_ID = 0,
	CONTROL_HEAD_ID = 1,
	RECEIVER_ID = 2,
	TRANSMITTER_ID = 3
} DeviceID;

typedef char LinkbusTxBuffer[LINKBUS_MAX_TX_MSG_LENGTH];

typedef struct
{
	LBMessageType type;
	LBMessageID id;
	char fields[LINKBUS_MAX_MSG_NUMBER_OF_FIELDS][LINKBUS_MAX_MSG_FIELD_LENGTH];
} LinkbusRxBuffer;

#define WAITING_FOR_UPDATE -1

/**
 */
void linkbus_init(uint32_t baud);

/**
 * Immediately turns off receiver and flushes receive buffer
 */
void linkbus_disable(void);

/**
 * Undoes linkbus_disable()
 */
void linkbus_enable(void);


/**
 */
void linkbus_end_tx(void);

/**
 */
void linkbus_reset_rx(void);

/**
 */
LinkbusTxBuffer* nextEmptyTxBuffer(void);

/**
 */
LinkbusTxBuffer* nextFullTxBuffer(void);

/**
 */
BOOL linkbusTxInProgress(void);

/**
 */
LinkbusRxBuffer* nextEmptyRxBuffer(void);

/**
 */
LinkbusRxBuffer* nextFullRxBuffer(void);

/**
 */
void lb_send_Help(void);

/**
 */
void lb_send_NewPrompt(void);

/**
 */
void lb_send_NewLine(void);

/**
 */
void lb_echo_char(uint8_t c);

/**
 */
BOOL lb_send_string(char* str, BOOL wait);

/**
 */
void lb_send_value(uint16_t value, char* label);

/** 
 */
BOOL lb_enabled(void);

#endif  /* LINKBUS_H_ */
