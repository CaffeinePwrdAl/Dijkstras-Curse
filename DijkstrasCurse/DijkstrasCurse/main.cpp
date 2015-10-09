#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <vector>
#include <string>
#include <memory>
#include <math.h>
#include <Windows.h>
#include <WinCon.h>
#include <io.h>
#include <fcntl.h>

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

	COORD sBufferSize;
	HANDLE hScreenBuffer;
	std::unique_ptr<CHAR_INFO[]> asConsoleBuffer;

protected:
	bool CreateScreenBuffer()
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

		return true;
	}

public:
	ConsoleWriter(SHORT width=64, SHORT height=40)
		:sBufferSize {width, height}
	{
		hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);

		// Wide characters
		_setmode(_fileno(stdout), _O_U16TEXT);

		SMALL_RECT sRect = { 0, 0, sBufferSize.X, sBufferSize.Y };
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

	~ConsoleWriter()
	{
		SetConsoleActiveScreenBuffer(hConsoleOut);
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
		if (y < sBufferSize.Y)
		{
			COORD sCoord = { (SHORT) x, (SHORT) y };
			//SetConsoleTextAttribute(hConsoleOut, colourCode);
			//SetConsoleCursorPosition(hConsoleOut, sCoord);

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
			}
		}
	}

	void Swap()
	{
		SMALL_RECT sWriteRegion = {0,0,0,0};
		COORD sBufferCoord = {0, 0};
		WriteConsoleOutputW(hScreenBuffer, asConsoleBuffer.get(), sBufferSize, sBufferCoord, &sWriteRegion);
		wprintf(L"%d %d", sWriteRegion.Right, sWriteRegion.Bottom);
	}
};

struct Pos
{
	int x;
	int y;
};

enum Direction
{
	None,
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

struct EdgeChar
{
	wchar_t norm;
	wchar_t term;
};

struct CornerChar
{
	wchar_t norm;
	wchar_t term;
	wchar_t midH;
	wchar_t midV;
};

struct GridSpriteData
{
	EdgeChar t;
	EdgeChar b;
	EdgeChar l;
	EdgeChar r;

	CornerChar tl;
	CornerChar tr;
	CornerChar bl;
	CornerChar br;

	wchar_t fill;
};

/* Draws grid cells using the ANSI single line drawing characters */
static const GridSpriteData g_sSingleLineBox = {
	{ 0x2500, 0x2500 },
	{ 0x2500, 0x2500 },
	{ 0x2502, 0x2502 },
	{ 0x2502, 0x2502 },
	{ 0x253C, 0x250C, 0x252C, 0x251C },
	{ 0x253C, 0x2510, 0x252C, 0x2524 },
	{ 0x253C, 0x2514, 0x2534, 0x251C },
	{ 0x253C, 0x2518, 0x2534, 0x2524 },
	L' ',
};

/* Draws grid cells using the ANSI single line drawing characters */
static const GridSpriteData g_sSnake = {
	{ 0x256C, 0x256C },
	{ 0x256C, 0x256C },
	{ 0x256C, 0x256C },
	{ 0x256C, 0x256C },
	{ 0x256C, 0x256C, 0x256C, 0x256C },
	{ 0x256C, 0x256C, 0x256C, 0x256C },
	{ 0x256C, 0x256C, 0x256C, 0x256C },
	{ 0x256C, 0x256C, 0x256C, 0x256C },
	L' ',
};

class GridSpriteRenderer
{
public:
	static void Render(int x, int y, int w, int h, 
					   bool bTop, bool bLeft, bool bBottom, bool bRight,
					   const GridSpriteData& sData, ConsoleWriter& sConsole)
	{
		wchar_t asChars[3][3];

		/* Corners */
		asChars[0][0] = (bTop)
							? (bLeft) ?(sData.tl.term):(sData.tl.midH) 
							: (bLeft) ?(sData.tl.midV):(sData.tl.norm);
		asChars[0][2] = (bTop)
							? (bRight)?(sData.tr.term):(sData.tr.midH)
							: (bRight)?(sData.tr.midV):(sData.tr.norm);
		asChars[2][0] = (bBottom)
							? (bLeft) ?(sData.bl.term):(sData.bl.midH) 
							: (bLeft) ?(sData.bl.midV):(sData.bl.norm);
		asChars[2][2] = (bBottom)
							? (bRight)?(sData.br.term):(sData.br.midH)
							: (bRight)?(sData.br.midV):(sData.br.norm);

		/* Edges */
		asChars[0][1] = (bTop)	 ?(sData.t.term):(sData.t.norm);
		asChars[2][1] = (bBottom)?(sData.b.term):(sData.b.norm);
		asChars[1][0] = (bLeft)  ?(sData.l.term):(sData.l.norm);
		asChars[1][2] = (bRight) ?(sData.r.term):(sData.r.norm);

		asChars[1][1] = sData.fill;
		
		wchar_t asLine[32];
		if (w > 31) return;
		
		for (int i = 0; i < h; i++)
		{
			int row;
			if (i == 0) row = 0;
			else if (i == h - 1) row = 2;
			else row = 1;

			asLine[0] = asChars[row][0];
			for (int j = 1; j < w - 1; j++)
			{
				asLine[j] = asChars[row][1];
			}
			asLine[w-1] = asChars[row][2];
			asLine[w]   = 0;
			sConsole.Print(x, y + i, asLine);
		}
	}
};

class BlobSpriteRenderer
{
public:
	static void Render(int x, int y, int w, int h, 
					   unsigned char ui8Neighbours,
					   const GridSpriteData& sData, ConsoleWriter& sConsole)
	{	
		//    01   02   04         -> 0x07
		//    08        10
		//    20   40   80         -> 0xE0
		//
		//    v         v
		//   0x29      0x94
				
		wchar_t asLine[32];
		if (w > 31) return;

		int i;
		for (i = 0; i < w - 1; i++)
		{
			asLine[i] = 0x2588;
		}
		asLine[i] = 0;

		for (int j = 0; j < h; j++)
		{
			int startX = 0;

			if (j == 0 && (ui8Neighbours & 0x02) == 0) continue;
			if (j == h - 1 && (ui8Neighbours & 0x40) == 0) continue;

			if (j == 0)
			{
				if ((ui8Neighbours & 0x01) == 0) startX = 1;
				if ((ui8Neighbours & 0x04) == 0) asLine[w - 1] = 0;
			}
			else if (j == h - 1)
			{
				if ((ui8Neighbours & 0x20) == 0) startX = 1;
				if ((ui8Neighbours & 0x80) == 0) asLine[w - 1] = 0;
			}
			else
			{
				if ((ui8Neighbours & 0x08) == 0) startX = 1;
				if ((ui8Neighbours & 0x10) == 0) asLine[w - 1] = 0;
			}

			sConsole.Print(x + startX, y + j, asLine + startX);
		}

		/*wchar_t dbg[3] = L"XX";
		wsprintf(dbg, L"%2X", (unsigned int)ui8Neighbours);
		sConsole.Print(x+1, y+1, dbg);*/
	}
};

class GameBoard
{
protected:
	int width;
	int height;

	int nCurrentBoard;
	std::unique_ptr<GameCell[]> asGameBoard[2];

public:
	GameBoard(int w, int h)
		: width(w)
		, height(h)
		, nCurrentBoard(0)
	{
		asGameBoard[0] = std::make_unique<GameCell[]>(w * h);
		asGameBoard[1] = std::make_unique<GameCell[]>(w * h);

		srand(42);

		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				bool bWall = (rand() % 5 == 0);
				WriteCell(x, y, (bWall)?(WALL):(SPACE));
			}
		}

		WriteCell(0,0, BLOB);
		/*Cell(0,1) = BLOB;
		Cell(0,2) = BLOB;
		Cell(0,3) = BLOB;
		Cell(1,3) = BLOB;
		Cell(0,4) = BLOB;
		Cell(1,4) = BLOB;
		Cell(1,0) = BLOB;
		Cell(2,0) = BLOB;
		Cell(3,0) = BLOB;
		Cell(4,0) = BLOB;
		Cell(5,0) = BLOB;
		Cell(5,1) = BLOB;
		Cell(5,2) = BLOB;
		Cell(5,3) = BLOB;
		Cell(4,3) = BLOB;

		Cell(8,8) = BLOB;
		Cell(9,9) = BLOB;*/

		SwitchBoard();
		SwitchBoard();

	}

	const GameCell Cell(const int& x, const int& y) const
	{
		if (x >= 0 && x < width && y >= 0 && y < height)
			return asGameBoard[!nCurrentBoard][x + width * y];
		return SPACE;
	}

	void WriteCell(const int& x, const int& y, const GameCell& eCell)
	{
		static GameCell eEmptyCell = SPACE;
		if (x >= 0 && x < width && y >= 0 && y < height)
			asGameBoard[nCurrentBoard][x + width * y] = eCell;
	}

	void SwitchBoard()
	{
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				asGameBoard[!nCurrentBoard][x + width * y] = asGameBoard[nCurrentBoard][x + width * y];
			}
		}
		nCurrentBoard = !nCurrentBoard;
	}

	bool CanMove(const Pos& sPos, const Pos& sDir) {}

	void KeyPress(wchar_t key)
	{
		Direction eDir = None;
		switch (key)
		{
			case L'w': 
				eDir = Up;
				break;
			case L's': 
				eDir = Down;
				break;
			case L'a': 
				eDir = Left;
				break;
			case L'd': 
				eDir = Right;
				break;
		}

		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				if (Cell(x,y) == BLOB)
				{
					int nx = x;
					int ny = y;
					switch (eDir)
					{
						case Up:	ny = y-1;	break;
						case Down:	ny = y+1;	break;
						case Left:	nx = x-1; 	break;
						case Right: nx = x+1; 	break;
					}
					if (Cell(nx, ny) != WALL)
					{
						WriteCell(nx, ny, BLOB);
					}
				}
			}
		}

		SwitchBoard();
	}

public:
	void Draw(ConsoleWriter& sConsole)
	{
		sConsole.Background(BLACK, false);
		sConsole.Foreground(BLACK, true);
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				GridSpriteRenderer::Render(x * 6, y * 4, 7, 5, 
										   y==0, x==0, y==height-1, x==width-1,
										   g_sSingleLineBox, sConsole);
			}
		}

		sConsole.Foreground(CYAN, false);
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				if (Cell(x,y) == WALL)
				{
					unsigned char ui8Neighbours = 0;
					ui8Neighbours |= (Cell(x-1, y-1) == WALL)?(0x01):(0);
					ui8Neighbours |= (Cell(x+0, y-1) == WALL)?(0x02):(0);
					ui8Neighbours |= (Cell(x+1, y-1) == WALL)?(0x04):(0);
					ui8Neighbours |= (Cell(x-1, y+0) == WALL)?(0x08):(0);
					ui8Neighbours |= (Cell(x+1, y+0) == WALL)?(0x10):(0);
					ui8Neighbours |= (Cell(x-1, y+1) == WALL)?(0x20):(0);
					ui8Neighbours |= (Cell(x-0, y+1) == WALL)?(0x40):(0);
					ui8Neighbours |= (Cell(x+1, y+1) == WALL)?(0x80):(0);

					BlobSpriteRenderer::Render(x * 6, y * 4, 7, 5, 
										   ui8Neighbours, g_sSnake, sConsole);
				}
			}
		}

		//sConsole.Background(BLUE, true);
		sConsole.Foreground(MAGENTA, true);
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				if (Cell(x,y) == BLOB)
				{
					unsigned char ui8Neighbours = 0;
					ui8Neighbours |= (Cell(x-1, y-1) == BLOB)?(0x01):(0);
					ui8Neighbours |= (Cell(x+0, y-1) == BLOB)?(0x02):(0);
					ui8Neighbours |= (Cell(x+1, y-1) == BLOB)?(0x04):(0);
					ui8Neighbours |= (Cell(x-1, y+0) == BLOB)?(0x08):(0);
					ui8Neighbours |= (Cell(x+1, y+0) == BLOB)?(0x10):(0);
					ui8Neighbours |= (Cell(x-1, y+1) == BLOB)?(0x20):(0);
					ui8Neighbours |= (Cell(x-0, y+1) == BLOB)?(0x40):(0);
					ui8Neighbours |= (Cell(x+1, y+1) == BLOB)?(0x80):(0);

					BlobSpriteRenderer::Render(x * 6, y * 4, 7, 5, 
										   ui8Neighbours, g_sSnake, sConsole);
				}
			}
		}
	}
protected:

};

int main(int argc, char** argv)
{
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

	ConsoleWriter sConsole;
	GameBoard sGame(10, 10);
	
	while (1)
	{
		if (WaitForSingleObject(hIn, INFINITE) != WAIT_OBJECT_0)
            break;

		INPUT_RECORD InRec;
        DWORD numRead;
        if (!ReadConsoleInput(hIn, &InRec, 1, &numRead))
            break;
 
        if ((InRec.EventType == KEY_EVENT) &&
            InRec.Event.KeyEvent.bKeyDown)
        {
			wchar_t ch = InRec.Event.KeyEvent.uChar.UnicodeChar;

			wchar_t astr[] = L"XX";
			wsprintf(astr, L"%02X", ch);
			sConsole.Foreground(WHITE, false);
			sConsole.Print(62, 38, astr);

			sGame.KeyPress(ch);

			if (ch == 0x1B) break;
        }

		sGame.Draw(sConsole);

		sConsole.Swap();
    }

	wchar_t chars[] = { 0x2588, 0x2593, 0x2592, 0x2591, 0};
	sConsole.Foreground(MAGENTA, true);
	sConsole.Print(40, 40, chars);

	sConsole.Foreground(WHITE, false);
	sConsole.Print(0, 39, L"");

	return 0;
}