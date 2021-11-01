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


#ifndef __f1975_H__
#define __f1975_H__

#include "defs.h"


#ifdef __cplusplus
	extern "C" {
#endif


typedef uint16_t tenthDB_t;

/*
typedef enum{
		dB0 = 0x00,
		dB_5 = 0x01,
		dB1_0 = 0x02,
		dB1_5 = 0x03,
		dB2_0 = 0x04,
		dB2_5 = 0x05,
		dB3_0 = 0x06,
		dB3_5 = 0x07,
		dB4_0 = 0x08,
		dB4_5 = 0x09,
		dB5_0 = 0x0a,
		dB5_5 = 0x0b,
		dB6_0 = 0x0c,
		dB6_5 = 0x0d,
		dB7_0 = 0x0e,
		dB7_5 = 0x0f,
		dB8_0 = 0x10,
		dB8_5 = 0x11,
		dB9_0 = 0x12,
		dB9_5 = 0x13,
		dB10_0 = 0x14,
		dB10_5 = 0x15,
		dB11_0 = 0x16,
		dB11_5 = 0x17,
		dB12_0 = 0x18,
		dB12_5 = 0x19,
		dB13_0 = 0x1a,
		dB13_5 = 0x1b,
		dB14_0 = 0x1c,
		dB14_5 = 0x1d,
		dB15_0 = 0x1e,
		dB15_5 = 0x1f,
		dB16_0 = 0x20,
		dB16_5 = 0x21,
		dB17_0 = 0x22,
		dB17_5 = 0x23,
		dB18_0 = 0x24,
		dB18_5 = 0x25,
		dB19_0 = 0x26,
		dB19_5 = 0x27,
		dB20_0 = 0x28,
		dB20_5 = 0x29,
		dB21_0 = 0x2a,
		dB21_5 = 0x2b,
		dB22_0 = 0x2c,
		dB22_5 = 0x2d,
		dB23_0 = 0x2e,
		dB23_5 = 0x2f,
		dB24_0 = 0x30,
		dB24_5 = 0x31,
		dB25_0 = 0x32,
		dB25_5 = 0x33,
		dB26_0 = 0x34,
		dB26_5 = 0x35,
		dB27_0 = 0x36,
		dB27_5 = 0x37,
		dB28_0 = 0x38,
		dB28_5 = 0x39,
		dB29_0 = 0x3a,
		dB29_5 = 0x3b,
		dB30_0 = 0x3c,
		dB30_5 = 0x3d,
		dB31_0 = 0x3e,
		dB31_5 = 0x3f
	} dB_t;
*/

#define MAX_ATTEN_TENTHS_DB (tenthDB_t)315
#define MAX_ATTEN_SETTING 0x3F

void setAtten(tenthDB_t att);
void setupPortsForAttenuator(BOOL enable);

#ifdef __cplusplus
	}
#endif

#endif //__f1975_H__
