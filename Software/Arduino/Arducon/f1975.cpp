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

#include "f1975.h"
#include "linkbus.h"

#if COMPILE_FOR_ATMELSTUDIO7
#include "ardooweeno.h"
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */
	static uint8_t lower4;
	static uint8_t upper2;

void setAtten(tenthDB_t att)
{
	uint8_t pattern;
	att = MIN(att+2, MAX_ATTEN_TENTHS_DB); /* round up */
	att -= (att % 5); /* set to nearest multiple of 5 */
	att /= 5;

	lower4 = dB_low(att);
	upper2 = dB_high(att);

	pattern = PORTC & 0xF0;
	PORTC = pattern | lower4;

	pattern = PORTD & 0xFC;
	PORTD = pattern | upper2;
}

void setupPorts(void)
{
	linkbus_disable();
	pinMode(PIN_D0, OUTPUT);
	pinMode(PIN_D1, OUTPUT);
	pinMode(PIN_D2, OUTPUT);
	pinMode(PIN_D3, OUTPUT);
	pinMode(PIN_D4, OUTPUT);  /* Also RXD */
	pinMode(PIN_D5, OUTPUT);  /* Also TXD */
	pinMode(PIN_PWDN, OUTPUT);
	digitalWrite(PIN_PWDN, ON);

//	setAtten(0);
}
