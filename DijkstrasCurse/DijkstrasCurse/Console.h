#pragma once

#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <memory>
#include <Windows.h>
#include <WinCon.h>
#include <io.h>
#include <fcntl.h>

enum ConsoleColour
{
	BLACK	= 0,
	RED		= FOREGROUND_RED,
	GREEN   = FOREGROUND_GREEN,
	BLUE	= FOREGROUND_BLUE,
	CYAN	= BLUE | GREEN,
	YELLOW  = RED | GREEN,
	MAGENTA = RED | BLUE,
	WHITE   = RED | GREEN | BLUE,
};

class ConsoleWriter
{
public:
	ConsoleWriter(SHORT width=64, SHORT height=45);
	~ConsoleWriter();

public:
	void Foreground(ConsoleColour eColour, bool bLight);
	void Background(ConsoleColour eColour, bool bLight);

	void Rect(int x, int y, int w, int h, const wchar_t& wc);
	void Print(int x, int y, const wchar_t* wszString);
	void MLPrint(int x, int y, const wchar_t* wszString);
	void Swap();

protected:
	bool CreateScreenBuffer();

protected:
	HANDLE hConsoleOut;
	WORD colourCode;

	COORD sBufferSize;
	HANDLE hScreenBuffer;
	std::unique_ptr<CHAR_INFO[]> asConsoleBuffer;
};