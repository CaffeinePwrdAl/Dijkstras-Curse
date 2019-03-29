#include <unistd.h>
#include <termios.h>
#include "console.h"

static const E8BColour g_asE8BDim[] =
{
	{0.0f, 0.0f, 0.0f}, /* E4B_BLACK    */
	{0.6f, 0.0f, 0.0f}, /* E4B_RED      */
	{0.0f, 0.4f, 0.0f}, /* E4B_GREEN    */
	{0.6f, 0.4f, 0.0f}, /* E4B_YELLOW   */
	{0.0f, 0.0f, 0.7f}, /* E4B_BLUE     */
	{0.6f, 0.0f, 0.7f}, /* E4B_MAGENTA  */
	{0.0f, 0.4f, 0.7f}, /* E4B_CYAN     */
	{0.6f, 0.4f, 0.7f}, /* E4B_WHITE    */ 
};
static const E8BColour g_asE8BBright[] =
{
	{0.2f, 0.2f, 0.2f}, /* E4B_BLACK    */
	{1.0f, 0.0f, 0.0f}, /* E4B_RED      */
	{0.0f, 1.0f, 0.0f}, /* E4B_GREEN    */
	{1.0f, 1.0f, 0.0f}, /* E4B_YELLOW   */
	{0.0f, 0.0f, 1.0f}, /* E4B_BLUE     */
	{1.0f, 0.0f, 1.0f}, /* E4B_MAGENTA  */
	{0.0f, 1.0f, 1.0f}, /* E4B_CYAN     */
	{1.0f, 1.0f, 1.0f}, /* E4B_WHITE    */ 
};

ConsoleWriter::ConsoleWriter(int width, int height)
	: bUse8BitColours(true)
{
	setlocale(LC_CTYPE,"");
//	wprintf(E4B_CLS());
//	Test4BitCodes();
//	Test8BitCodes();

	Foreground(RED, true);
	Background(BLACK, false);
	SetCurrentColour();
	wprintf(L"###\n");

	Foreground(RED, false);
	Background(CYAN, true);
	SetCurrentColour();
	wprintf(L"###\n");

	Foreground(WHITE, true);
	Background(BLACK, false);
	SetCurrentColour();
	wprintf(L"###\n");
	wprintf(L"\n");

	exit(42);

//	wprintf(E4B_CLS());
//	wprintf(E4B_RESET());
}

ConsoleWriter::~ConsoleWriter()
{
	wprintf(E4B_RESET());
	wprintf(E4B_CLS());
}

void ConsoleWriter::Foreground(ConsoleColour eColour, bool bLight)
{
	if (bUse8BitColours)
	{
		e8bColour.fg = (bLight)?(g_asE8BBright[eColour]):(g_asE8BDim[eColour]);
	}
	else
	{
		e4bColour.fg.colour = eColour;
		e4bColour.fg.bold = bLight;
	}
}
void ConsoleWriter::Background(ConsoleColour eColour, bool bLight)
{
	if (bUse8BitColours)
	{
		e8bColour.bg = (bLight)?(g_asE8BBright[eColour]):(g_asE8BDim[eColour]);
	}
	else
	{
		e4bColour.bg.colour = eColour;
		e4bColour.bg.bold = bLight;
	}
}

void ConsoleWriter::SetCurrentColour()
{
	if (bUse8BitColours)
	{
		E8BSetColour(e8bColour);
	}
	else
	{
		E4BSetColour(e4bColour);
	}
}

void ConsoleWriter::Rect(int x, int y, int w, int h, const wchar_t& wc)
{
	int i;
	wchar_t line[w];
	
	for (i = 0; i < w; line[i]=wc, i++);

	line[i] = L'\0';

	SetCurrentColour();

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
	SetCurrentColour();
	E4BMove(x+1,y);
	wprintf(wszString);
}

void ConsoleWriter::Printn(int x, int y, const wchar_t* wszString, int nChars)
{
	SetCurrentColour();
	E4BMove(x+1,y);
	wprintf(L"%.*s", nChars, wszString);
}

void ConsoleWriter::MLPrint(int x, int y, const wchar_t* wszString)
{
	SetCurrentColour();
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
