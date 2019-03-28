#include "console.h"

ConsoleWriter::ConsoleWriter(int width, int height)
		:sBufferSize {(SHORT)width, (SHORT)height}
{
	hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

	// Wide characters
	_setmode(_fileno(stdout), _O_U16TEXT);

	CreateScreenBuffer();

	// Hide the cursor
	CONSOLE_CURSOR_INFO sCCI = { 0 };
	sCCI.bVisible = FALSE;
	// Set the size of the cursor, and make the cursor invisible.
	// This is strange, because the cursor will only be hidden
	// if the size of the cursor is changed in combination with
	// setting bVisible to FALSE.
	// If you do not set the cursor size, the cursor will remain
	// visible, even though you've set bVisible = FALSE!
	sCCI.dwSize = 1;
	SetConsoleCursorInfo(hScreenBuffer, &sCCI);

	Foreground(BLACK, true);
	Background(BLACK, false);
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
	if (!SetConsoleActiveScreenBuffer(hScreenBuffer))
	{
		DWORD eError = GetLastError();
		fprintf(stderr, "SetConsoleActiveScreenBuffer failed - (%d)\n", eError);
		return false;
	}

	// Set the font details
	CONSOLE_FONT_INFOEX sCFIX = { 0 };
	sCFIX.cbSize = sizeof(CONSOLE_FONT_INFOEX);
	sCFIX.FontWeight = 400;
	sCFIX.dwFontSize.X = 10;
	sCFIX.dwFontSize.Y = 16;
	wcscpy_s(sCFIX.FaceName, L"Lucida Console");
	SetCurrentConsoleFontEx(hScreenBuffer, FALSE, &sCFIX);

	// Determine the minimum window size. We have to make sure the window is smaller
	// than the screen buffer size we're going to use, then once we create the screen
	// buffer we can resize up to that. If we don't do this step, we'll be truncated
	// if the starting console size was smaller then our desired screen buffer.
	CONSOLE_SCREEN_BUFFER_INFO sCSBI;
	GetConsoleScreenBufferInfo(hScreenBuffer, &sCSBI);
	SMALL_RECT sMinRect = sCSBI.srWindow;
	{
		if (!SetConsoleWindowInfo(hScreenBuffer, TRUE, &sMinRect))
		{
			DWORD eError = GetLastError();
			fprintf(stderr, "SetConsoleWindowInfo failed with current size? - (%d)\n", eError);
			return false;
		}

		// Shrink till it breaks, then go back one step - horrible, but I'm not
		// sure I can do this in a particularly nice way.
		do
		{
			sMinRect.Right -= 1;
		} while (SetConsoleWindowInfo(hScreenBuffer, TRUE, &sMinRect));
		sMinRect.Right += 1;
		do
		{
			sMinRect.Bottom -= 1;
		} while (SetConsoleWindowInfo(hScreenBuffer, TRUE, &sMinRect));
		sMinRect.Bottom += 1;

		if (!SetConsoleWindowInfo(hScreenBuffer, TRUE, &sMinRect))
		{
			DWORD eError = GetLastError();
			fprintf(stderr, "SetConsoleWindowInfo failed with final size - (%d)\n", eError);
			return false;
		}
	}

	// Now set the desired size for the screen buffer (must be greater than min, 
	// and greater than the current window size)
	sBufferSize.X = max(sBufferSize.X, sMinRect.Right - sMinRect.Left);
	sBufferSize.Y = max(sBufferSize.Y, sMinRect.Bottom - sMinRect.Top);
	if (!SetConsoleScreenBufferSize(hScreenBuffer, sBufferSize))
	{
		DWORD eError = GetLastError();
		fprintf(stderr, "SetConsoleScreenBufferSize failed - (%d)\n", eError);
		return false;
	}

	// Make the window the desired size now
	SMALL_RECT sRect = { 0, 0, sBufferSize.X - 1, sBufferSize.Y - 1 };
	if (!SetConsoleWindowInfo(hScreenBuffer, TRUE, &sRect))
	{
		DWORD eError = GetLastError();
		fprintf(stderr, "SetConsoleWindowInfo failed - (%d)\n", eError);
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

ConsoleReader::ConsoleReader()
{
	hIn = GetStdHandle(STD_INPUT_HANDLE);
}

ConsoleReader::~ConsoleReader()
{}

bool ConsoleReader::WaitForKeypress(wchar_t& ch)
{
	/* Wait is the game time step in ms - or INFINITE for no display update tick */
	DWORD nWaitStatus = WaitForSingleObject(hIn, 200);

	switch (nWaitStatus)
	{
	case WAIT_OBJECT_0:
	{
		INPUT_RECORD InRec;
		DWORD numRead;
		if (!ReadConsoleInput(hIn, &InRec, 1, &numRead))
		{
			return false;
		}

		if ((InRec.EventType == KEY_EVENT) &&
			InRec.Event.KeyEvent.bKeyDown)
		{
			ch = InRec.Event.KeyEvent.uChar.UnicodeChar;
			return true;
		}
	}
	break;

	case WAIT_TIMEOUT:
	{
		// Normal
	}
	break;

	default:
	case WAIT_FAILED:
	case WAIT_ABANDONED:
	{
		printf("Something went wrong: WaitForSingleObject -> 0x%x\n", nWaitStatus);
	}
	break;
	}

	return false;
}