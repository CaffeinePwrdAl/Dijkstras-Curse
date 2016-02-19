#include "Console.h"

ConsoleWriter::ConsoleWriter(SHORT width, SHORT height)
		:sBufferSize {width, height}
{
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// Wide characters
	_setmode(_fileno(stdout), _O_U16TEXT);

	SMALL_RECT sRect = { 0, 0, sBufferSize.X-1, sBufferSize.Y-1 };
	SetConsoleWindowInfo(hConsoleOut, TRUE, &sRect);

	CreateScreenBuffer();

	Foreground(BLACK, true);
	Background(BLACK, false);

	CONSOLE_CURSOR_INFO sCCI = {0};
	sCCI.bVisible = FALSE;
	SetConsoleCursorInfo(hConsoleOut, &sCCI);

	CONSOLE_FONT_INFOEX sCFIX = {0};
	sCFIX.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	sCFIX.FontWeight = 400;
	sCFIX.dwFontSize.X = 8;
	sCFIX.dwFontSize.Y = 16;
	wcscpy_s(sCFIX.FaceName, L"Courier New");		
	SetCurrentConsoleFontEx(hConsoleOut, FALSE, &sCFIX);
}

ConsoleWriter::~ConsoleWriter()
{
	SetConsoleActiveScreenBuffer(hConsoleOut);
}

bool ConsoleWriter::CreateScreenBuffer()
{
	// Allocate a new screen buffer
	hScreenBuffer = CreateConsoleScreenBuffer( 
		GENERIC_READ |           // read/write access 
		GENERIC_WRITE, 
		FILE_SHARE_READ | 
		FILE_SHARE_WRITE,        // shared 
		NULL,                    // default security attributes 
		CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE 
		NULL);                   // reserved; must be NULL 
	if (hScreenBuffer == INVALID_HANDLE_VALUE) 
	{
		printf("CreateConsoleScreenBuffer failed - (%d)\n", GetLastError()); 
		return false;
	}
 
	// Make the new screen buffer the active screen buffer. 
	if (!SetConsoleActiveScreenBuffer(hScreenBuffer) ) 
	{
		printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError()); 
		return false;
	}

	asConsoleBuffer = std::make_unique<CHAR_INFO[]>(sBufferSize.X * sBufferSize.Y);

	for (auto i = 0; i < sBufferSize.X * sBufferSize.Y; i++)
	{
		asConsoleBuffer[i].Attributes = colourCode;
		asConsoleBuffer[i].Char.UnicodeChar = L' ';
	}

	return true;
}

void ConsoleWriter::Foreground(ConsoleColour eColour, bool bLight)
{
	colourCode &= ~0x0F;
	colourCode |= eColour | (bLight ? FOREGROUND_INTENSITY : 0);
}
void ConsoleWriter::Background(ConsoleColour eColour, bool bLight)
{
	colourCode &= ~0xF0;
	colourCode |= (eColour | (bLight ? FOREGROUND_INTENSITY : 0)) << 4;
}

void ConsoleWriter::Rect(int x, int y, int w, int h, const wchar_t& wc)
{
	const int sx = min(max(x, 0),sBufferSize.X);
	const int sy = min(max(y, 0),sBufferSize.Y);
	const int ex = min(max(x+w, 0),sBufferSize.X);
	const int ey = min(max(y+h, 0),sBufferSize.Y);
	for (int j = sy; j < ey; j++)
	{
		for (int i = sx; i < ex; i++)
		{
			int nOffset = i + j * sBufferSize.X;
			asConsoleBuffer[nOffset].Attributes = colourCode;
			asConsoleBuffer[nOffset].Char.UnicodeChar = wc;
		}
	}
}

void ConsoleWriter::Print(int x, int y, const wchar_t* wszString)
{
	if (y < sBufferSize.Y)
	{
		COORD sCoord = { (SHORT) x, (SHORT) y };
		size_t nLen = wcslen(wszString);
		if (sBufferSize.X - x < nLen)
		{
			nLen = sBufferSize.X - x;
		}

		size_t nOffset = x + y * sBufferSize.X;
		for (auto i = 0; i < nLen; i++)
		{
			asConsoleBuffer[nOffset].Attributes = colourCode;
			asConsoleBuffer[nOffset].Char.UnicodeChar = wszString[i];
			nOffset++;
		}
	}
}

void ConsoleWriter::Printn(int x, int y, const wchar_t* wszString, int nChars)
{
	if (y < sBufferSize.Y)
	{
		COORD sCoord = { (SHORT) x, (SHORT) y };
		size_t nLen = min(wcslen(wszString), nChars);
		if (sBufferSize.X - x < nLen)
		{
			nLen = sBufferSize.X - x;
		}

		size_t nOffset = x + y * sBufferSize.X;
		for (auto i = 0; i < nLen; i++)
		{
			asConsoleBuffer[nOffset].Attributes = colourCode;
			asConsoleBuffer[nOffset].Char.UnicodeChar = wszString[i];
			nOffset++;
		}
	}
}

void ConsoleWriter::MLPrint(int x, int y, const wchar_t* wszString)
{
	if (y < sBufferSize.Y)
	{
		COORD sCoord = { (SHORT) x, (SHORT) y };
		size_t nLen = 1;
		size_t nOffset = x + y * sBufferSize.X;
		size_t nStartCh = 0;
		for (auto i = 0; i < nLen;)
		{
			const wchar_t wch = wszString[nStartCh + i];

			if (wch == L'\n')
			{
				y++;
				if (y >= sBufferSize.Y)
				{
					__debugbreak();
					break;
				}
				nOffset = x + y * sBufferSize.X;
				nStartCh += i+1;
				i = 0;
				continue;
			}

			if (i == 0) 
			{
				nLen = wcslen(wszString + nStartCh);
				if (sBufferSize.X - x <= nLen)
				{
					nLen = sBufferSize.X - x;
				}	
			}
			asConsoleBuffer[nOffset].Attributes = colourCode;
			asConsoleBuffer[nOffset].Char.UnicodeChar = wch;
			nOffset++;
			i++;
		}
	}
}

void ConsoleWriter::Swap()
{
	SMALL_RECT sWriteRegion;
	sWriteRegion.Top    = 0;
	sWriteRegion.Left   = 0;
	sWriteRegion.Bottom = sBufferSize.Y + 1;
	sWriteRegion.Right  = sBufferSize.X + 1;
	COORD sBufferCoord = {0, 0};
	WriteConsoleOutputW(hScreenBuffer, asConsoleBuffer.get(), sBufferSize, sBufferCoord, &sWriteRegion);
}
