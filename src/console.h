#pragma once

#include <stdint.h>
#include <memory>

#if defined(_WIN32)

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <Windows.h>
#include <WinCon.h>
#include <io.h>
#include <fcntl.h>

#else

#define E4B_WIDE_CHARS
#include "e4b.h" // My 4bit console escape code wrapper

#endif

enum ConsoleColour
{
#if defined(WIN32)
	BLACK	= 0,
	RED		= FOREGROUND_RED,
	GREEN   = FOREGROUND_GREEN,
	BLUE	= FOREGROUND_BLUE,
	CYAN	= BLUE | GREEN,
	YELLOW  = RED | GREEN,
	MAGENTA = RED | BLUE,
	WHITE   = RED | GREEN | BLUE,
#else
	BLACK   = E4B_BLACK,
	RED     = E4B_RED,
	GREEN   = E4B_GREEN,
	YELLOW  = E4B_YELLOW,
	BLUE    = E4B_BLUE,
	MAGENTA = E4B_MAGENTA,
	CYAN    = E4B_CYAN,
	WHITE   = E4B_WHITE,
#endif
};

class ConsoleWriter
{
public:
	ConsoleWriter(int width=64, int height=45);
	~ConsoleWriter();

public:
	void Foreground(ConsoleColour eColour, bool bLight);
	void Background(ConsoleColour eColour, bool bLight);

	void Rect(int x, int y, int w, int h, const wchar_t& wc);
	void Print(int x, int y, const wchar_t* wszString);
	void Printn(int x, int y, const wchar_t* wszString, int nChars);
	void MLPrint(int x, int y, const wchar_t* wszString);
	void Swap();

protected:
	bool CreateScreenBuffer();

protected:
#if defined(_WIN32)
	HANDLE hConsoleOut;
	WORD colourCode;

	COORD sBufferSize;
	HANDLE hScreenBuffer;
	std::unique_ptr<CHAR_INFO[]> asConsoleBuffer;
#else
	E4BColourPair currentColour;
#endif
};

class ConsoleReader
{
public:
	ConsoleReader();
	~ConsoleReader();

public:
	bool WaitForKeypress(wchar_t &ch);

protected:
#if defined(_WIN32)
	HANDLE hIn;
#else
	void SetTTY();
	void RestoreTTY();
#endif
};
