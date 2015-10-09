#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <vector>
#include <string>
#include <memory>
#include <math.h>
#include <Windows.h>
#include <WinCon.h>

//void PrintString(const wchar_t* string)
//{
//	size_t nLength = wcslen(string);
//	const wchar_t* currentChar = string;
//	const wchar_t* startChar = string;
//	while (*currentChar != L'\0')
//	{
//		if (currentChar[0] == L'@' &&
//			currentChar[1] == L'{')
//		{
//			/* Print and parse character code */
//			wprintf_s(L"%.*s", currentChar-startChar, startChar);
//			currentChar += 2;
//
//			unsigned int uiAttrib;
//			if (swscanf_s(currentChar, L"%02x", &uiAttrib) == 1)
//			{
//				SetConsoleTextAttribute(hConsoleOut, uiAttrib);
//			}
//
//			while (*currentChar != L'}')
//			{
//				currentChar++;
//			}
//			currentChar++;
//			startChar = currentChar;
//		}
//
//		currentChar++;
//	}
//
//	wprintf_s(L"%.*s", currentChar - startChar, startChar);
//}

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
protected:
	HANDLE hConsoleOut;
	WORD colourCode;
public:
	ConsoleWriter()
	{
		//HWND hWnd = GetConsoleWindow();
		hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

		SMALL_RECT sRect = { 0, 0, 64, 40 };
		SetConsoleWindowInfo(hConsoleOut, TRUE, &sRect);

		Foreground(BLACK, true);
		Background(BLACK, false);

		CONSOLE_SCREEN_BUFFER_INFO sCSBI;
		GetConsoleScreenBufferInfo(hConsoleOut, &sCSBI);

		CONSOLE_CURSOR_INFO sCCI;
		sCCI.bVisible = FALSE;
		SetConsoleCursorInfo(hConsoleOut, &sCCI);
	}

public:
	void Foreground(ConsoleColour eColour, bool bLight)
	{
		colourCode &= ~0x0F;
		colourCode |= eColour | (bLight ? FOREGROUND_INTENSITY : 0);
	}
	void Background(ConsoleColour eColour, bool bLight)
	{
		colourCode &= ~0xF0;
		colourCode |= (eColour | (bLight ? FOREGROUND_INTENSITY : 0)) << 4;
	}

public:
	void Print(int x, int y, const wchar_t* wszString)
	{
		COORD sCoord = { x, y };
		SetConsoleTextAttribute(hConsoleOut, colourCode);
		SetConsoleCursorPosition(hConsoleOut, sCoord);
		wprintf(L"%s", wszString);
	}
};

class GridSprite
{
	wchar_t asChars[5][5];

	void Display(unsigned int edges)
	{

	}

};

struct Pos
{
	int x;
	int y;
};

enum Direction
{
	Up,
	Down,
	Left,
	Right,
};

enum GameCell
{
	SPACE,
	WALL,
	BLOB,
};

class GameBoard
{
protected:
	int width;
	int height;

	std::unique_ptr<GameCell[]> asGameBoard;

public:
	GameBoard(int w, int h)
		: width(w)
		, height(h)
		, asGameBoard(std::make_unique<GameCell[]>(w * h))
	{
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				bool bWall = (rand() % 2 == 0);
				bool bBlob = (rand() % 2 == 0);
				Cell(x, y) = (bWall)?(WALL):(bBlob)?(BLOB):(SPACE);
			}
		}
	}

	const GameCell& Cell(const int& x, const int& y) const
	{
		return asGameBoard[x + width * y];
	}

	GameCell& Cell(const int& x, const int& y)
	{
		return asGameBoard[x + width * y];
	}

	bool CanMove(const Pos& sPos, const Pos& sDir) {}

public:
	void Draw(ConsoleWriter& sConsole)
	{
		sConsole.Background(BLACK, false);
		sConsole.Foreground(BLACK, true);
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				sConsole.Print(x * 4, y * 2 + 0, L"+---+");
				sConsole.Print(x * 4, y * 2 + 1, L"|   |");
				sConsole.Print(x * 4, y * 2 + 2, L"+---+");
			}
		}

		sConsole.Background(BLACK, true);
		sConsole.Foreground(BLACK, false);
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				if (asGameBoard[x + y * width] == WALL)
				{
					sConsole.Print(x * 4 + 0, y * 2 + 0, L"+---+");
					sConsole.Print(x * 4 + 0, y * 2 + 1, L"|===|");
					sConsole.Print(x * 4 + 0, y * 2 + 2, L"+---+");
				}
			}
		}

		sConsole.Background(BLUE, false);
		sConsole.Foreground(BLUE, true);
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				if (asGameBoard[x + y * width] == BLOB)
				{
					//sConsole.Print(x * 4 + 1, y * 2 + 0, L"---");
					sConsole.Print(x * 4 + 1, y * 2 + 1, L"   ");
					//sConsole.Print(x * 4 + 1, y * 2 + 2, L"---");
				}
			}
		}
	}
protected:

};

int main(int argc, char** argv)
{
	ConsoleWriter sConsole;
	GameBoard sGame(10, 10);
	
	sGame.Draw(sConsole);

	sConsole.Foreground(WHITE, false);
	sConsole.Print(0, 39, L"");
	return 0;
}