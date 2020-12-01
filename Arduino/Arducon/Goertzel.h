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
 *
 *  See https://www.embedded.com/the-goertzel-algorithm
 *
 */


/* ensure this library description is only included once */
#ifndef Goertzel_h
#define Goertzel_h

/* include types & constants of Wiring core API */
#include "defs.h"

#if COMPILE_FOR_ATMELSTUDIO7
#include "ardooweeno.h"
#else
#include "Arduino.h"
#endif  /* COMPILE_FOR_ATMELSTUDIO7 */

#define MAXN 205
#define ADCCENTER 512

/* library interface description */
class Goertzel
{
public:
Goertzel(float, float);
~Goertzel();
void SetTargetFrequency(float);
bool DataPoint(int);
float Magnitude2();
bool SamplesReady(void);

private:
void ProcessSample(int);
void ResetGoertzel(void);
};

#endif
