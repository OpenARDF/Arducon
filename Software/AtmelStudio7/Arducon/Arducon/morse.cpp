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

#include "morse.h"
#include <stddef.h>
#include <ctype.h>
#ifdef ATMEL_STUDIO_7
#include <avr/pgmspace.h>
#endif

MorseCharacter getMorseChar(char c);

#define SOLID_KEYDOWN 0xFF
#define INTER_CHAR_SPACE 0xFE
#define INTER_WORD_SPACE 0xFD

/*
 *  Load a string to send by passing in a pointer via the first argument.
 *  Call this function with a NULL argument at intervals of 1 element of time to generate Morse code.
 *  Once loaded with a string each call to this function returns a BOOL indicating whether a CW carrier should be sent
 *  Pass in a pointer to a BOOL in the second and third arguments:
 */
BOOL makeMorse(char* s, BOOL* repeating, BOOL* finished)
{
	static char* str = NULL;
	static char c = ' ';
	static BOOL repeat = TRUE;
	static MorseCharacter morseInProgress;
	static uint8_t charIndex;       /* letters, numbers, punctuation */
	static uint8_t symbolIndex;     /* dits and dahs */
	static uint8_t elementIndex;    /* units of time: dit = 1, dah = 3, intersymbol = 1, intercharacter = 3, etc. */
	static uint8_t addedSpace;      /* adds additional time to make an inter-character space */
	static BOOL completedString = FALSE;
	static BOOL carrierOn = FALSE;
	static BOOL holdKeyDown = FALSE;

	if(s)   /* load a new NULL-terminated string to send */
	{
		holdKeyDown = FALSE;

		if(repeating)
		{
			repeat = *repeating;
		}

		if(*s)
		{
			if(str != s) /* Passing in the same pointer will not change the state of Morse.c */
			{
				str = s;
				c = ' ';
				morseInProgress = getMorseChar(*str);
				charIndex = 0;
				symbolIndex = 0;
				elementIndex = 0;
				addedSpace = 0;
				completedString = FALSE;
				carrierOn = FALSE;
			}
		}
		else    /* a zero-length string shuts down makeMorse */
		{
			str = NULL;
			completedString = TRUE;
			if(finished)
			{
				*finished = TRUE;
			}

			carrierOn = FALSE;
		}

		return( OFF);
	}
	else if(str)
	{
		if(repeating)
		{
			*repeating = repeat;
		}

		if(completedString)
		{
			if(finished)
			{
				*finished = TRUE;
				holdKeyDown = FALSE;
			}
			return( OFF);
		}

		if(elementIndex)
		{
			elementIndex--;
		}
		else if(carrierOn && !holdKeyDown)  /* carrier is on, so turn it off and wait appropriate amount of space */
		{
			carrierOn = FALSE;
			/* wait one element = inter-symbol space */
			if(addedSpace)
			{
				elementIndex = addedSpace;
				addedSpace = 0;
			}
			/* wait inter-character space */
		}
		else    /* carrier is off, so turn it on and get next symbol */
		{
			if(symbolIndex >= morseInProgress.lengthInSymbols)
			{
				c = (*(str + ++charIndex));

				if(!c)  /* wrap to beginning of text */
				{
					if(repeat)
					{
						c = *str;
						charIndex = 0;
					}
					else
					{
						str = NULL;
						carrierOn = FALSE;
						completedString = TRUE;
						if(finished)
						{
							*finished = TRUE;
						}
						holdKeyDown = FALSE;
						return( OFF );
					}
				}

				morseInProgress = getMorseChar(c);
				symbolIndex = 0;
			}

			if(morseInProgress.pattern < INTER_WORD_SPACE)
			{
				BOOL isDah = morseInProgress.pattern & (1 << symbolIndex++);

				if(isDah)
				{
					elementIndex = 2;
				}
				else
				{
					elementIndex = 0;
				}

				carrierOn = TRUE;

				if(symbolIndex >= morseInProgress.lengthInSymbols)
				{
					addedSpace = 2;
				}
			}
			else
			{
				uint8_t sym = morseInProgress.lengthInSymbols;
				symbolIndex = 255;  /* ensure the next character gets read */
				carrierOn = FALSE;
				if(sym >= 4 )
				{
					elementIndex = morseInProgress.lengthInSymbols - 4;
				}
				else
				{
					elementIndex = 0;
				}
			}
		}

		/* Overrides for key on and key off special characters */
		if(c == '<')    /* constant tone */
		{
			holdKeyDown = TRUE;
		}
		else
		{
			holdKeyDown = FALSE;
		}
	}

	if(finished)
	{
		*finished = completedString;
	}

	if(holdKeyDown)
	{
		return(TRUE);
	}
	else
	{
		return( carrierOn);
	}
}

/**
 *  Returns the number of milliseconds required to send the string pointed to by the first argument at the WPM code speed
 *  passed in the second argument.
 */
uint16_t timeRequiredToSendStrAtWPM(char* str, uint16_t spd)
{
	uint8_t elements = 0;
	MorseCharacter m;
	char c;

	for(int i = 0; i < 20; i++)
	{
		c = str[i];
		if(!c)
		{
			break;
		}
		m = getMorseChar(c);
		if(m.pattern < INTER_WORD_SPACE)
		{
			elements += 3;
		}
		elements += m.lengthInElements;
	}

	return(elements * WPM_TO_MS_PER_DOT(spd));
}

/**
 *  Morse Code characters are defined as having three attributes:
 *  pattern = a sequence of up to 8 dit and dah symbols contained in an unsigned byte, sequentially read from LSB to MSB (first symbol is bit 0)
 *  lengthInSymbols = how many symbols (dits and dahs) the character contains; this is how many pattern bits are used to represent the character
 *  lengthInElements = how long (measured in "dit lengths") is the total character including all inter-symbol spaces.
 */
typedef struct {
	char c;
	MorseCharacter morse;
} MorseTableEntry;

static const MorseTableEntry MORSE_TABLE[] PROGMEM =
{
	{ 'A', { 0x02, 2, 5 } },
	{ 'B', { 0x01, 4, 9 } },
	{ 'C', { 0x05, 4, 11 } },
	{ 'D', { 0x01, 3, 7 } },
	{ 'E', { 0x00, 1, 1 } },
	{ 'F', { 0x04, 4, 9 } },
	{ 'G', { 0x03, 3, 9 } },
	{ 'H', { 0x00, 4, 7 } },
	{ 'I', { 0x00, 2, 3 } },
	{ 'J', { 0x0e, 4, 13 } },
	{ 'K', { 0x05, 3, 9 } },
	{ 'L', { 0x02, 4, 9 } },
	{ 'M', { 0x03, 2, 7 } },
	{ 'N', { 0x01, 2, 5 } },
	{ 'O', { 0x07, 3, 11 } },
	{ 'P', { 0x06, 4, 11 } },
	{ 'Q', { 0x0b, 4, 13 } },
	{ 'R', { 0x02, 3, 7 } },
	{ 'S', { 0x00, 3, 5 } },
	{ 'T', { 0x01, 1, 3 } },
	{ 'U', { 0x04, 3, 7 } },
	{ 'V', { 0x08, 4, 9 } },
	{ 'W', { 0x06, 3, 9 } },
	{ 'X', { 0x09, 4, 11 } },
	{ 'Y', { 0x0d, 4, 13 } },
	{ 'Z', { 0x03, 4, 11 } },
	{ '0', { 0x1f, 5, 19 } },
	{ '1', { 0x1e, 5, 17 } },
	{ '2', { 0x1c, 5, 15 } },
	{ '3', { 0x18, 5, 13 } },
	{ '4', { 0x10, 5, 11 } },
	{ '5', { 0x00, 5, 9 } },
	{ '6', { 0x01, 5, 11 } },
	{ '7', { 0x03, 5, 13 } },
	{ '8', { 0x07, 5, 15 } },
	{ '9', { 0x0f, 5, 17 } },
	{ '.', { 0x2a, 6, 17 } },
	{ ',', { 0x33, 6, 19 } },
	{ '?', { 0x0c, 6, 15 } },
	{ '\'', { 0x1e, 6, 19 } },
	{ '!', { 0x35, 6, 19 } },
	{ '/', { 0x09, 5, 13 } },
	{ '(', { 0x0d, 5, 15 } },
	{ ')', { 0x2d, 6, 19 } },
	{ '&', { 0x02, 5, 11 } },
	{ ':', { 0x07, 6, 17 } },
	{ ';', { 0x15, 6, 12 } },
	{ '=', { 0x11, 5, 13 } },
	{ '+', { 0x0a, 5, 13 } },
	{ '-', { 0x21, 6, 15 } },
	{ '_', { 0x2c, 6, 17 } },
	{ '"', { 0x12, 6, 15 } },
	{ '$', { 0x48, 7, 17 } },
	{ '@', { 0x16, 6, 17 } },
	{ '|', { 0xff, 7, 4 } },   /* adjusted by -3 to account for inter-character space */
	{ ' ', { 0xfe, 7, 7 } },
	{ '<', { 0x1f, 5, 19 } }
};

MorseCharacter getMorseChar(char c)
{
	c = toupper(c);

	for(uint8_t i = 0; i < (sizeof(MORSE_TABLE) / sizeof(MORSE_TABLE[0])); i++)
	{
		if((char)pgm_read_byte(&MORSE_TABLE[i].c) == c)
		{
			MorseCharacter morse;
			memcpy_P(&morse, &MORSE_TABLE[i].morse, sizeof(morse));
			return(morse);
		}
	}

	MorseCharacter morse = { 0x00, 0, 0 };
	return(morse);
}
