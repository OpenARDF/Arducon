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

#include "ardooweeno.h"
#include <util/delay.h>

void pinMode(uint8_t pin, uint8_t mode)
{
	switch(pin)
	{
		case D2:
		{
			if(mode == OUTPUT)
			{
				DDRD  |= (1 << PORTD2);
			}
			else
			{
				DDRD  &= ~(1 << PORTD2);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTD2);
				}
				else
				{
					PORTD &= ~(1 << PORTD2);
				}
			}
		}
		break;

		case D3:
		{
			if(mode == OUTPUT)
			{
				DDRD  |= (1 << PORTD3);
			}
			else
			{
				DDRD  &= ~(1 << PORTD3);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTD3);
				}
				else
				{
					PORTD &= ~(1 << PORTD3);
				}
			}
		}
		break;

		case D4:
		{
			if(mode == OUTPUT)
			{
				DDRD  |= (1 << PORTD4);
			}
			else
			{
				DDRD  &= ~(1 << PORTD4);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTD4);
				}
				else
				{
					PORTD &= ~(1 << PORTD4);
				}
			}
		}
		break;

		case D5:
		{
			if(mode == OUTPUT)
			{
				DDRD  |= (1 << PORTD5);
			}
			else
			{
				DDRD  &= ~(1 << PORTD5);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTD5);
				}
				else
				{
					PORTD &= ~(1 << PORTD5);
				}
			}
		}
		break;

		case D6:
		{
			if(mode == OUTPUT)
			{
				DDRD  |= (1 << PORTD6);
			}
			else
			{
				DDRD  &= ~(1 << PORTD6);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTD6);
				}
				else
				{
					PORTD &= ~(1 << PORTD6);
				}
			}
		}
		break;

		case D7:
		{
			if(mode == OUTPUT)
			{
				DDRD  |= (1 << PORTD7);
			}
			else
			{
				DDRD  &= ~(1 << PORTD7);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTD7);
				}
				else
				{
					PORTD &= ~(1 << PORTD7);
				}
			}
		}
		break;

		case D9:
		{
			if(mode == OUTPUT)
			{
				DDRB  |= (1 << PORTB1);
			}
			else
			{
				DDRB  &= ~(1 << PORTB1);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTB1);
				}
				else
				{
					PORTD &= ~(1 << PORTB1);
				}
			}
		}
		break;

		case D13:
		{
			if(mode == OUTPUT)
			{
				DDRB  |= (1 << PORTB5);
			}
			else
			{
				DDRB  &= ~(1 << PORTB5);

				if(mode == INPUT_PULLUP)
				{
					PORTD  |= (1 << PORTB5);
				}
				else
				{
					PORTD &= ~(1 << PORTB5);
				}
			}
		}
		break;

		case A0:
		{
			if(mode == OUTPUT)
			{
				DDRC  |= (1 << PORTC0);
			}
			else
			{
				DDRC  &= ~(1 << PORTC0);

				if(mode == INPUT_PULLUP)
				{
					PORTC  |= (1 << PORTC0);
				}
				else
				{
					PORTC &= ~(1 << PORTC0);
				}
			}
		}

		case A1:
		{
			if(mode == OUTPUT)
			{
				DDRC  |= (1 << PORTC1);
			}
			else
			{
				DDRC  &= ~(1 << PORTC1);

				if(mode == INPUT_PULLUP)
				{
					PORTC  |= (1 << PORTC1);
				}
				else
				{
					PORTC &= ~(1 << PORTC1);
				}
			}
		}

		case A2:
		{
			if(mode == OUTPUT)
			{
				DDRC  |= (1 << PORTC2);
			}
			else
			{
				DDRC  &= ~(1 << PORTC2);

				if(mode == INPUT_PULLUP)
				{
					PORTC  |= (1 << PORTC2);
				}
				else
				{
					PORTC &= ~(1 << PORTC2);
				}
			}
		}

		case A3:
		{
			if(mode == OUTPUT)
			{
				DDRC  |= (1 << PORTC3);
			}
			else
			{
				DDRC  &= ~(1 << PORTC3);

				if(mode == INPUT_PULLUP)
				{
					PORTC  |= (1 << PORTC3);
				}
				else
				{
					PORTC &= ~(1 << PORTC3);
				}
			}
		}

		case A4:
		{
			if(mode == OUTPUT)
			{
				DDRC  |= (1 << PORTC4);
			}
			else
			{
				DDRC  &= ~(1 << PORTC4);

				if(mode == INPUT_PULLUP)
				{
					PORTC  |= (1 << PORTC4);
				}
				else
				{
					PORTC &= ~(1 << PORTC4);
				}
			}
		}

		case A5:
		{
			if(mode == OUTPUT)
			{
				DDRC  |= (1 << PORTC5);
			}
			else
			{
				DDRC  &= ~(1 << PORTC5);

				if(mode == INPUT_PULLUP)
				{
					PORTC  |= (1 << PORTC5);
				}
				else
				{
					PORTC &= ~(1 << PORTC5);
				}
			}
		}

		default:
		{
		}
		break;
	}
}

BOOL digitalRead(uint8_t pin)
{
	int result = 0;

	switch(pin)
	{
		case D2:
		{
			result = PIND & (1 << PORTD2);
		}
		break;

		case D3:
		{
			result = PIND & (1 << PORTD3);
		}
		break;

		case D4:
		{
			result = PIND & (1 << PORTD4);
		}
		break;

		case D5:
		{
			result = PIND & (1 << PORTD5);
		}
		break;

		case D6:
		{
			result = PIND & (1 << PORTD6);
		}
		break;

		case D7:
		{
			result = PIND & (1 << PORTD7);
		}
		break;

		case D9:
		{
			result = PINB & (1 << PORTB1);
		}

		case D13:
		{
			result = PINB & (1 << PORTB5);
		}
		break;

		case A0:
		{
			result = PINC & (1 << PORTC0);
		}
		break;

		case A1:
		{
			result = PINC & (1 << PORTC1);
		}
		break;

		case A2:
		{
			result = PINC & (1 << PORTC2);
		}
		break;

		case A3:
		{
			result = PINC & (1 << PORTC3);
		}
		break;

		default:
		{
		}
		break;
	}

	return(result != 0);
}

void digitalWrite(uint8_t pin, uint8_t value)
{
	switch(pin)
	{
		case D2:
		{
			if(value)
			{
				PORTD  |= (1 << PORTD2);
			}
			else
			{
				PORTD  &= ~(1 << PORTD2);
			}
		}
		break;

		case D3:
		{
			if(value)
			{
				PORTD  |= (1 << PORTD3);
			}
			else
			{
				PORTD  &= ~(1 << PORTD3);
			}
		}
		break;

		case D4:
		{
			if(value)
			{
				PORTD  |= (1 << PORTD4);
			}
			else
			{
				PORTD  &= ~(1 << PORTD4);
			}
		}
		break;

		case D5:
		{
			if(value)
			{
				PORTD  |= (1 << PORTD5);
			}
			else
			{
				PORTD  &= ~(1 << PORTD5);
			}
		}
		break;

		case D6:
		{
			if(value)
			{
				PORTD  |= (1 << PORTD6);
			}
			else
			{
				PORTD  &= ~(1 << PORTD6);
			}
		}
		break;

		case D7:
		{
			if(value)
			{
				PORTD  |= (1 << PORTD7);
			}
			else
			{
				PORTD  &= ~(1 << PORTD7);
			}
		}
		break;

		case D9:
		{
			if(value)
			{
				PORTB  |= (1 << PORTB1);
			}
			else
			{
				PORTB  &= ~(1 << PORTB1);
			}
		}
		break;

		case D13:
		{
			if(value)
			{
				PORTB  |= (1 << PORTB5);
			}
			else
			{
				PORTB  &= ~(1 << PORTB5);
			}
		}
		break;

		case A0:
		{
			if(value)
			{
				PORTC  |= (1 << PORTC0);
			}
			else
			{
				PORTC  &= ~(1 << PORTC0);
			}
		}
		break;

		case A1:
		{
			if(value)
			{
				PORTC  |= (1 << PORTC1);
			}
			else
			{
				PORTC  &= ~(1 << PORTC1);
			}
		}
		break;

		case A2:
		{
			if(value)
			{
				PORTC  |= (1 << PORTC2);
			}
			else
			{
				PORTC  &= ~(1 << PORTC2);
			}
		}
		break;

		case A3:
		{
			if(value)
			{
				PORTC  |= (1 << PORTC3);
			}
			else
			{
				PORTC  &= ~(1 << PORTC3);
			}
		}
		break;

		default:
		{
		}
		break;
	}
}
