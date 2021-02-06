/* 
* f1975.cpp
*
* Created: 2/4/2021 10:29:46 AM
* Author: Charles Scharlau
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
//	setAtten(0);
}
