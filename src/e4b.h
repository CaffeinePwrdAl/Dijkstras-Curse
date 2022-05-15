#pragma once
#ifndef _ESCAPE_4BIT_H_
#define _ESCAPE_4BIT_H_

#include <math.h>

#define CONCAT(a, b) a ## b

#if defined(E4B_WIDE_CHARS)
	typedef wchar_t E4BChar;
	#define e4bprintf wprintf
	#define E4BW(x) CONCAT(L, x)
	#define PRI_WCHAR L"%lc"
	#define PRI_WSTR  L"%ls"
#else
	typedef char E4BChar;
	#define e4bprintf printf
	#define E4BW(x) x
	#define PRI_WCHAR "%C"
	#define PRI_WSTR  "%S"
#endif


/*
// Ansi escape code based colouring
//
// Colours:           ESC SGR m
// Cursor Move (Abs): ESC row;col H
// Cursor Move (Rel): ESC n A/B/C/D = up/down/right/left n units
//
// In wide character environments, the escape characters are still interpreted as
// a contiguous set of bytes (just with multiple bytes per character). This means
// the encoding is not the equivalent of just converting the byte character sequence
// to wide-characters ("blah" -> L"blah").
*/

#define E4B(x)			E4BW(E4B_PRE x E4B_SGR)
#define E4B_CLS()		E4BW(E4B_PRE "2" E4B_ED)
#define E4B_RESET()		E4B("39;49")
#define E4B_RESETFG()	E4B("39")
#define E4B_RESETBG()	E4B("49")
#define E4B_RESETALL()	E4B("0")

#define E4B_PRE			"\x1b\x5b"
#define E4B_SGR			"m"
#define E4B_CUR			"H"
#define E4B_ED			"J"
#define E8B_FGCOL		"38;5;"
#define E8B_BGCOL		"48;5;"

#define E4B_BLACK    0
#define E4B_RED      1
#define E4B_GREEN    2
#define E4B_YELLOW   3
#define E4B_BLUE     4
#define E4B_MAGENTA  5
#define E4B_CYAN     6
#define E4B_WHITE    7
#define E4B_DONTCARE (-1)

/*
// 4bit Ansi escape code based colouring
*/
struct E4BColour
{
	int8_t colour;
	uint8_t bold;
};
static const E4BColour E4B_IGNORE { E4B_DONTCARE, false };

struct E4BColourPair
{
	E4BColour fg;
	E4BColour bg;
};

static void E4BSetColour(const E4BColourPair &c);

/*
// 8bit Ansi escape code based colouring
*/

struct E8BColour
{
	float r, g, b;
};
static const E8BColour E8B_IGNORE { -1, -1, -1 };

struct E8BColourPair
{
	E8BColour fg;
	E8BColour bg;
};

static void E8BSetColour(const E8BColourPair &c);

// Cursor positioning
static void E4BMove(int x, int y);



static void E4BMove(int x, int y)
{
	e4bprintf(E4BW(E4B_PRE "%d;%d" E4B_CUR), y, x);
}

static void E4BSetColour(const E4BColourPair &c)
{
	//                 ESC[ n;nn  ;  n;nn   m   /0
	#define E4B_MAXLEN (2 + (4) + 1 + (4) + 1 + 1)

	E4BChar str[E4B_MAXLEN] = E4BW(E4B_PRE);

	E4BChar *chr = str + 2;

	if (c.fg.colour != E4B_DONTCARE)
	{
		if (c.fg.bold != 0)
		{
			*chr++ = E4BW('1');
			*chr++ = E4BW(';');
		}
		*chr++ = E4BW('3');
		*chr++ = (c.fg.colour&7) + E4BW('0');
	}
	if (c.bg.colour != E4B_DONTCARE)
	{
		if (c.fg.colour != E4B_DONTCARE)
		{
			*chr++ = E4BW(';');
		}
		if (c.bg.bold != 0)
		{
			*chr++ = E4BW('1');
			*chr++ = E4BW('0');
		}
		else
		{
			*chr++ = E4BW('4');
		}
		*chr++ = (c.bg.colour&7) + E4BW('0');
	}
	*chr++ = E4BW('m');

#if defined(E4B_WIDE_CHARS)
	if ((chr - str) % 1 != 0)
	{
		*chr++ = L'.';
		*chr++ = L'.';
		*chr++ = L'.';
	}
#endif

//e4bprintf(L"\x5b\x31\x3b\x33\x35\x6d");
	e4bprintf(str);
}

static uint8_t E8BColourCode(const E8BColour &c)
{
	// Error Colour - red
	uint8_t colour = 9;

	// Error Colour - Bright Magenta (to big)
	if (c.r > 1 || c.g > 1 || c.b > 1)
	{
		return 13;
	}

	// Error Colour - Dark Magenta (negative)
	if (c.r < 0 || c.g < 0 || c.b < 0)
	{
		return 5;
	}
	
	// Greyscale
	if (c.r == c.g && c.r == c.b)
	{
		float grey = powf(c.r, 1.0f/2.2f);;//sqrt(c.r);
		/*
		// There are 24 contiguous greys (232 - 255) but also
		// there are some grey values in the colour cube too
		// basically have to do an interval based LUT to use them
		// all
		//
		//  Standard Colours
		//  (0)                                    (8)                 (7)             (15)
		//  00                                     80                  C0              FF
		//  v                                      ==                  v
		//  __ 08 12 1C 26 30 3A 44 4E 58 62 6C 76 80 8A 94 9E A8 B2 BC C6 D0 DA E4 EE __
		//  ^                           ^            ^           ^           ^
		//  00                          5F           87          AF          D7        FF
		//  (16)                       (59)        (102)       (145)       (188)      (231)
		//  Colour Cube 
		*/
		const uint8_t ug = (uint8_t)(grey * 255.0f);
		
		if (ug <= 0x04)						colour = 0;
		else if (ug >= 0x5B && ug <= 0x60)	colour = 59;
		else if (ug >= 0x84 && ug <= 0x88)	colour = 102;
		else if (ug >= 0xAB && ug <= 0xB0)	colour = 145;
		else if (ug >= 0xD4 && ug <= 0xD8)	colour = 188;
		else if (ug >= 0xF4)				colour = 15;
		else
		{
			const float min = 0x04 / 255.0f;
			const float max = 0xE8 / 255.0f;

			float v = (grey - min) / (max - min);
			if (v > 1) v = 1;

			colour = 232 + (uint8_t)(23 * v);
		}
	}
	// 6x6x6 Colour Cube
	else
	{
		uint8_t r, g, b;
		r = (uint8_t)(c.r * 5);
		g = (uint8_t)(c.g * 5);
		b = (uint8_t)(c.b * 5);
		colour = 16;
		colour += b + g * 6 + r * 36;
	}

	return colour;
}

#define E8B_COLOUR_IS_DONTCARE(c) (c.r == -1 || c.g == -1 || c.b == -1)

static void E8BSetColour(const E8BColourPair &c)
{
	bool bIgnoreFG = E8B_COLOUR_IS_DONTCARE(c.fg);
	if (!bIgnoreFG)
	{
		e4bprintf(E4B_PRE E8B_FGCOL E4BW("%d") E4B_SGR, E8BColourCode(c.fg));
	}

	if (!E8B_COLOUR_IS_DONTCARE(c.bg))
	{
		e4bprintf(E4B_PRE E8B_BGCOL E4BW("%d") E4B_SGR, E8BColourCode(c.bg));
	}
}

static void Test4BitCodes()
{
	E4BSetColour({{E4B_MAGENTA,1}, {E4B_DONTCARE, 0}});
	e4bprintf(E4BW("Test Sheet for 4bit Colour Escape Codes") E4B_RESETALL() E4BW("\n"));
	E4BSetColour({{E4B_WHITE,1}, {E4B_DONTCARE, 0}});
	e4bprintf(E4BW("=======================================") E4B_RESETALL() E4BW("\n\n"));

//	for (int8_t b = 0; b <= 7; b++)
	for (int8_t f = 0; f <= 7; f++)
	{
		int8_t b = f;
		const E4BColourPair colours[] =
		{
			{ { f, 1 }, 			{ b, 1 } },
			{ { f, 0 }, 			{ b, 1 } },
			{ { f, 0 }, 			{ b, 0 } },
			{ { f, 1 }, 			{ b, 0 } },
			
			{ { f, 1 },				{ E4B_WHITE, 1 } },
			{ { E4B_BLACK, 1 },		{ b, 1 } },
			{ { f, 0 },				{ E4B_BLACK, 1 } },
			{ { E4B_BLACK, 0 },		{ b, 0 } },

			{ { f, 1 }, 			{ E4B_DONTCARE, 0 } },
			{ { E4B_DONTCARE, 0 },	{ b, 1 } },
			{ { E4B_DONTCARE, 0 },	{ b, 0 } },
			{ { f, 0 },				{ E4B_DONTCARE, 0 } },
		};

		for (const auto &cp : colours)
		{
			E4BSetColour(cp);
			//e4bprintf(PRI_WSTR, L" \x2591\x2592\x2593\x2588");
			e4bprintf(PRI_WSTR, L"#@$£");
			e4bprintf(E4B_RESETALL());
			e4bprintf(E4BW(" "));
		}
		e4bprintf(E4BW("\n"));
	}
	e4bprintf(E4BW("\n\n"));
}

static void Test8BitCodes()
{
	E8BSetColour({{1.0f, 0.2f, 1.0f}, {-1, -1, -1}});
	e4bprintf(E4BW("Test Sheet for 8bit Colour Escape Codes") E4B_RESETALL() E4BW("\n"));
	E8BSetColour({{1.0f, 1.0f, 1.0f}, {-1, -1, -1}});
	e4bprintf(E4BW("=======================================") E4B_RESETALL() E4BW("\n\n"));

	for (int8_t r = 0; r < 3; r++)
	{
		for (int8_t g = 0; g < 6; g++)
		{
			for (int8_t rr = 1; rr <= 2; rr++)
			{
				for (int8_t b = 0; b < 6; b++)
				{
					float fr = (r*rr) / 5.0f;
					float fg = g / 5.0f;
					float fb = b / 5.0f;

					E8BSetColour({{fr, fg, fb}, {0.0f, 0.0f, 0.0f}});
					e4bprintf(PRI_WSTR, L"\x2591\x2592\x2593\x2588");
					e4bprintf(E4B_RESETALL());

					E8BSetColour({{1.0f, 1.0f, 1.0f}, {fr, fg, fb}});
					e4bprintf(PRI_WSTR, L"\x2591\x2592\x2593");
					e4bprintf(E4B_RESETALL());
				}
				e4bprintf(E4BW("  "));
			}
			e4bprintf(E4BW("\n"));
		}
		e4bprintf(E4BW("\n"));
	}

	for (uint8_t y = 0; y < 4; y++)
	{
		for (uint8_t x = 0; x < 80; x++)
		{
			float fx = (x / 79.0f);
			E8BSetColour({{fx, fx, fx}, {-1, -1, -1}});
			e4bprintf(E4BW("%S"), L"\x2588");
		}
		e4bprintf(E4BW("\n"));
	}
	
	e4bprintf(E4B_RESETALL() E4BW("\n\n"));
}

#endif
