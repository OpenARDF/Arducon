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
 *
 * i2c.h
 */

#include "defs.h"

#ifndef I2C_H_
#define I2C_H_

#ifdef __cplusplus
	extern "C" {
#endif

#define I2C_TIMEOUT_SUPPORT /* limit number of tries for i2c success */
/* #define SUPPORT_I2C_CLEARBUS_FUNCTION */

#ifndef BOOL
	typedef uint8_t BOOL;
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE !FALSE
#endif

/**
 */
	void i2c_init(void);

/**
 */
	BOOL i2c_device_read(uint8_t slaveAddr, uint8_t addr, uint8_t data[], uint8_t bytes2read);

/**
 */
	BOOL i2c_device_write(uint8_t slaveAddr, uint8_t addr, uint8_t data[], uint8_t bytes2write);

#ifdef I2C_TIMEOUT_SUPPORT
/**
 */
	BOOL i2c_start(void);
#else
/**
 */
	void i2c_start(void);
#endif

/**
 */
void i2c_stop(void);

/**
 */
BOOL i2c_write_success(uint8_t, uint8_t);

/**
 */
uint8_t i2c_read_ack(void);

/**
 */
uint8_t i2c_read_nack(void);

/**
 */
BOOL i2c_status(uint8_t);

#ifdef SUPPORT_I2C_CLEARBUS_FUNCTION
/**
 */
	BOOL i2c_clearBus(void);
#endif  /* SUPPORT_I2C_CLEARBUS_FUNCTION */

#ifdef __cplusplus
	}
#endif

#endif  /* I2C_H_ */
