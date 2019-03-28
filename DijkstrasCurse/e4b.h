#pragma once
#ifndef _ESCAPE_4BIT_H_
#define _ESCAPE_4BIT_H_

#define CONCAT(a, b) a ## b

#if defined(E4B_WIDE_CHARS)
	typedef wchar_t E4BChar;
	#define e4bprintf wprintf
	#define E4BW(x) CONCAT(L, x)
	#define PRI_WCHAR "%s"
#else
	typedef char E4BChar;
	#define e4bprintf printf
	#define E4BW(x) x
	#define PRI_WCHAR "%S"
#endif


/*
// 3/4 bit Ansi escape code based colouring
//
// Colours:           ESC SGR m
// Cursor Move (Abs): ESC row;col H
// Cursor Move (Rel): ESC n A/B/C/D = up/down/right/left n units
*/
#define E4B_PRE			"\x1b\x5b"
#define E4B_SGR			"m"
#define E4B_CUR			"H"
#define E4B_ED			"J"

					//  ESC[ n;nn  ;  n;nn   m   /0
#define E4B_MAXLEN		(2 + (4) + 1 + (4) + 1 + 1)


#define E4B(x)			E4BW(E4B_PRE x E4B_SGR)
#define E4B_CLS()		E4BW(E4B_PRE "2" E4B_ED)
#define E4B_RESET()		E4B("39;49")
#define E4B_RESETFG()	E4B("39")
#define E4B_RESETBG()	E4B("49")
#define E4B_RESETALL()	E4B("0")

#define E4B_BLACK    0
#define E4B_RED      1
#define E4B_GREEN    2
#define E4B_YELLOW   3
#define E4B_BLUE     4
#define E4B_MAGENTA  5
#define E4B_CYAN     6
#define E4B_WHITE    7
#define E4B_DONTCARE (-1)


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

static void E4BMove(int x, int y)
{
	e4bprintf(E4BW(E4B_PRE "%d;%d" E4B_CUR), y, x);
}

/*
// In wide character environments, the escape characters are still interpreted as
// a contiguous set of bytes (just with multiple bytes per character). This means
// the encoding is not the equivalent of just converting the byte character sequence
// to wide-characters ("blah" -> L"blah").
*/
static void E4BSetColour(const E4BColourPair &c)
{
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

static void Test4BitCodes()
{

/*	wprintf(L"\x1b[31mThis text has a red foreground using SGR.31.\r\n");
    wprintf(L"\x1b[1mThis text has a bright (bold) red foreground using SGR.1 to affect the previous color setting.\r\n");
    wprintf(L"\x1b[mThis text has returned to default colors using SGR.0 implicitly.\r\n");
    wprintf(L"\x1b[34;46mThis text shows the foreground and background change at the same time.\r\n");
    wprintf(L"\x1b[0mThis text has returned to default colors using SGR.0 explicitly.\r\n");
    wprintf(L"\x1b[31;32;33;34;35;36;101;102;103;104;105;106;107mThis text attempts to apply many colors in the same command. Note the colors are applied from left to right so only the right-most option of foreground cyan (SGR.36) and background bright white (SGR.107) is effective.\r\n");
    wprintf(L"\x1b[39mThis text has restored the foreground color only.\r\n");
    wprintf(L"\x1b[49mThis text has restored the background color only.\r\n");
*/

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
			e4bprintf(E4BW(PRI_WCHAR), L" \x2591\x2592\x2593\x2588");
			e4bprintf(E4B_RESETALL());
			e4bprintf(E4BW(" "));
		}
		e4bprintf(E4BW("\n"));
	}
	e4bprintf(E4BW("\n\n"));
}

#endif
