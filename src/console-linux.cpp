#include <unistd.h>
#include <termios.h>
#include "console.h"

ConsoleWriter::ConsoleWriter(int width, int height)
//		:sBufferSize {width, height}
{
	setlocale(LC_CTYPE,"");
	wprintf(E4B_CLS());
	Test4BitCodes();

	Foreground(WHITE, true);
	Background(BLACK, false);
	E4BSetColour(currentColour);
	wprintf(L"Hello!\n");

	wprintf(E4B_CLS());

	Rect(0, 0, 50, 50, L'*');

	Print(2, 2, L"A Normal Print");
	Printn(2, 4, L"A Normal Print", 8);
	MLPrint(2, 6, L"Hello there traveller...\n  Stay awhile and listen\n\nYou Died.");

	wprintf(E4B_RESET());

//	exit(0);
}

ConsoleWriter::~ConsoleWriter()
{
	//SetConsoleActiveScreenBuffer(hConsoleOut);
}

bool ConsoleWriter::CreateScreenBuffer()
{
	return true;
}

void ConsoleWriter::Foreground(ConsoleColour eColour, bool bLight)
{
	currentColour.fg.colour = eColour;
	currentColour.fg.bold = bLight;
}
void ConsoleWriter::Background(ConsoleColour eColour, bool bLight)
{
	currentColour.bg.colour = eColour;
	currentColour.bg.bold = bLight;
}

void ConsoleWriter::Rect(int x, int y, int w, int h, const wchar_t& wc)
{
	int i;
	wchar_t line[w];
	
	for (i = 0; i < w; line[i]=wc, i++);

	line[i] = L'\0';

	E4BSetColour(currentColour);

	const int sx = x+1;
	const int sy = y;
	const int ex = x+w;
	const int ey = y+h;
	for (int j = sy; j < ey; j++)
	{
		//for (int i = sx; i < ex; i++)
		{
			E4BMove(sx, j);
			wprintf(line);
		}
	}
}

void ConsoleWriter::Print(int x, int y, const wchar_t* wszString)
{
	E4BSetColour(currentColour);
	E4BMove(x+1,y);
	wprintf(wszString);
}

void ConsoleWriter::Printn(int x, int y, const wchar_t* wszString, int nChars)
{
	E4BSetColour(currentColour);
	E4BMove(x+1,y);
	wprintf(L"%.*s", nChars, wszString);
}

void ConsoleWriter::MLPrint(int x, int y, const wchar_t* wszString)
{
	E4BSetColour(currentColour);
	//if (y < sBufferSize.Y)
	{
		size_t nLen = 1;
		size_t nStartCh = 0;
		for (auto i = 0; i < nLen;)
		{
			const wchar_t wch = wszString[nStartCh + i];

			if (wch == L'\n' || wch == L'\0')
			{
				//E4BMove(x,y);
				//wprintf(L"%02d : %s", i, wszString + nStartCh);

				y++;
				//if (y >= sBufferSize.Y)
				{
				//	__debugbreak();
				//	break;
				}
				nStartCh += i + 1;
				i = 0;
				continue;
			}

			if (i == 0) 
			{
				nLen = wcslen(wszString + nStartCh);
				//if (sBufferSize.X - x <= nLen)
				{
				//	nLen = sBufferSize.X - x;
				}
			}
			
			E4BMove(x+i+1, y);
			wprintf(L"%c", wszString[nStartCh + i]);
			i++;
		}
	}
}

void ConsoleWriter::Swap()
{
}

ConsoleReader::ConsoleReader()
{
	SetTTY();
}

ConsoleReader::~ConsoleReader()
{
	RestoreTTY();
}

bool ConsoleReader::WaitForKeypress(wchar_t& ch)
{
	ch = getchar();

	return true;
}


static bool g_bInitialised = false;
static struct termios g_sOldTTY, g_sNewTTY;

void ConsoleReader::SetTTY()
{
	if (!g_bInitialised)
	{
		tcgetattr(STDIN_FILENO, &g_sOldTTY);

		g_sNewTTY = g_sOldTTY;

		// Don't wait for enter on keypress, and don't echo them
		// to the terminal.
		g_sNewTTY.c_lflag &= ~(ICANON | ECHO);

		tcsetattr(STDIN_FILENO, TCSANOW, &g_sNewTTY);

		g_bInitialised = true;
	}
}

void ConsoleReader::RestoreTTY()
{
	if (g_bInitialised)
	{
		tcsetattr(STDIN_FILENO, TCSANOW, &g_sOldTTY);
		g_bInitialised = false;
	}
}