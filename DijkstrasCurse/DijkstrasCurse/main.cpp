#define _USE_MATH_DEFINES
#include <math.h>

#include "Console.h"
//#define WIN32_LEAN_AND_MEAN
//#define VC_EXTRALEAN
//#include <vector>
//#include <string>
//#include <memory>
//
//#include <Windows.h>
//#include <WinCon.h>
//#include <io.h>
//#include <fcntl.h>

/* Size of Game Board */
const int GAME_SIZE_X = 10;
const int GAME_SIZE_Y = 10;

/* Size of Game Board Grid cell in chars */
const int CELL_SIZE_X = 6;
const int CELL_SIZE_Y = 4;

/*
 * Total dimensions of game area in chars
 *
 *   n*w + 1
 *   m*h + 1
 *
 * to account for the closing edge of characters.
 */
const int GAME_AREA_X = GAME_SIZE_X * CELL_SIZE_X + 1;
const int GAME_AREA_Y = GAME_SIZE_Y * CELL_SIZE_Y + 1;

const int SIDE_AREA_X = 0;
const int SIDE_AREA_Y = 5;

struct Level
{
	const wchar_t strTitle[GAME_AREA_X + 1];
	const wchar_t strMessage[GAME_AREA_X * 2 + 1];
	unsigned char aui8Board[GAME_AREA_Y][GAME_AREA_X+1];
};

const Level aui8Levels[] = 
{
	{
		L"T1: In the beginning...",
		L"When you move all your cells replicate in that direction.\n" \
		 "Reach the exit portal.",
		{
			"B         ",
			"     B    ",
			"  B       ",
			"        B ",
			"          ",
			"          ",
			"          ",
			"       xxx",
			"         e",
			"       xxx",
		},
		
	},

	{
		L"T2: Yes... Of course it has spikes!",
		L"But they only kill you if you move into them.\n" \
		 "You can survive if you reach the portal at the same time.",
		 {
			"B         ",
			"          ",
			"          ",
			"          ",
			"   xxx    ",
			"   x!!    ",
			"          ",
			"          ",
			"          ",
			"!!!!e!!!!!",
		},
	},
	{
		L"T3: Lone road",
		L"Sometimes there is only one path.",
		{
			"Bxxxxxxxxx",
			" x      xx",
			" x xxxx xx",
			" x xxxx xx",
			" x xxxx xx",
			" x xxxx xx",
			" x xxxx xx",
			" x xxxx xx",
			" x xxxx xx",
			"   xxxx  e",
		},
	},
	{
		L"T4: I'll take the high road, and I'll take the low road",
		L"Sometimes you'll explore many paths at the same time.\n" \
		 "  ... There might be bonus goals for covering extra squares.",
		{
			"Bxxxxxxxxx",
			" x      xx",
			" x xxxx xx",
			" x      xx",
			" x xxxx xx",
			" x      xx",
			" x xxxx xx",
			" x xxxx xx",
			" x xxxx xx",
			"   xxxx  e",
		},
	},
	{
		//01234567890123456789012345678901234567890123456789012345678|
		//0         1         2         3         4         5        |
		L"T5: The obvious path...\n",
		L"... is not always the right path.\n" \
		 "Bonus Goal: Cover all the open spaces",
		{
			"Bxxxxxxxxx",
			" x      xx",
			" x xxxx xx",
			"        xx",
			" x xxxx xx",
			" x  !!  xx",
			" x xxxx xx",
			" x xxxx xx",
			" x xxxx xx",
			"   xxxx  e",
		},
	},
	{
		L"Section 1: Ready?",
		L"Now you know what you're doing, it's time to begin!",
		{
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"B        e",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
		},
	},
	{
		L"L1: 'Left Brain/Right Brain'",
		L"Bonus Goal: Cover all the open spaces",
		{
			"    !    e",
			"  xxx  xxx",
			"  Bxx     ",
			" xxxxx xxx",
			"         e",
			"      xxxx",
			"          ",
			"     xx xx",
			"xxxx B  xx",
			"xxxxxxxxxx",
		},
	}
};
const int numLevels = sizeof(aui8Levels) / sizeof(aui8Levels[0]);

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
	SPIKES,
	EXIT,
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
		
		wchar_t asLine[80];
		if (w+1 > 80)
		{
			__debugbreak();
			return;
		}
		
		for (int i = 0; i <= h; i++)
		{
			int row;
			if (i == 0) row = 0;
			else if (i == h) row = 2;
			else row = 1;

			asLine[0] = asChars[row][0];
			for (int j = 1; j < w; j++)
			{
				asLine[j] = asChars[row][1];
			}
			asLine[w] = asChars[row][2];
			asLine[w+1]   = 0;
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
		if (w+1 > 31) return;

		for (int j = 0; j <= h; j++)
		{
			int startX = 0;

			int i;
			for (i = 0; i <= w; i++)
			{
				asLine[i] = 0x2588;
			}
			asLine[i] = 0;

			if (j == 0 && (ui8Neighbours & 0x02) == 0) continue;
			if (j == h && (ui8Neighbours & 0x40) == 0) continue;

			if (j == 0)
			{
				if ((ui8Neighbours & 0x01) == 0) startX = 1;
				if ((ui8Neighbours & 0x04) == 0) asLine[w] = 0;
			}
			else if (j == h)
			{
				if ((ui8Neighbours & 0x20) == 0) startX = 1;
				if ((ui8Neighbours & 0x80) == 0) asLine[w] = 0;
			}
			else
			{
				if ((ui8Neighbours & 0x08) == 0) startX = 1;
				if ((ui8Neighbours & 0x10) == 0) asLine[w] = 0;
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
	enum GameState
	{
		PLAYING,
		DIED,
		LEVEL_COMPLETE,
	};
	GameState eState;

protected:
	int width;
	int height;

	int nLevel;
	int nCurrentBoard;
	std::unique_ptr<GameCell[]> asGameBoard[2];

public:
	GameBoard(int w, int h)
		: width(w)
		, height(h)
		, nLevel(0)
		, nCurrentBoard(0)
		, eState(PLAYING)
	{
		asGameBoard[0] = std::make_unique<GameCell[]>(w * h);
		asGameBoard[1] = std::make_unique<GameCell[]>(w * h);

		srand(42);

		InitialiseTestLevel();

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

	void InitialiseTestLevel()
	{
		for (int x = 0; x < 10; x++)
		{
			for (int y = 0; y < 10; y++)
			{
				GameCell eCell = SPACE;
				switch (aui8Levels[nLevel].aui8Board[y][x])
				{
				case 'x':
					eCell = WALL;
					break;
				case 'e':
					eCell = EXIT;
					break;
				case '!':
					eCell = SPIKES;
					break;
				case 'B':
					eCell = BLOB;
					break;
				}
				WriteCell(x, y, eCell);
			}
		}

		SwitchBoard();
		SwitchBoard();
	}

	void NextLevel()
	{
		nLevel = (nLevel+1)%(numLevels);
	}

public:
	void KeyPress(wchar_t key)
	{
		if (key == L'n')
		{
			eState = LEVEL_COMPLETE;
		}

		switch (eState)
		{
			case LEVEL_COMPLETE:
			{
				eState = PLAYING;
				NextLevel();
				InitialiseTestLevel();
			}
			break;
			case DIED:
			{
				eState = PLAYING;
				InitialiseTestLevel();
			}
			break;
			case PLAYING:
			{
				bool bComplete = false;
				bool bDied = false;
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
							switch (Cell(nx, ny))
							{
							case EXIT:
								bComplete = true;
								break;
							case WALL:
								break;
							case SPIKES:
								bDied = true;
								break;
							default:
								WriteCell(nx, ny, BLOB);
							}
						}
					}
				}

				if (bComplete)
				{
					eState = LEVEL_COMPLETE;
				}
				else if (bDied)
				{
					eState = DIED;
				}
			}
			break;
		}

		SwitchBoard();
	}

public:
	void Draw(ConsoleWriter& sConsole)
	{
		const int originX = 0;
		const int originY = SIDE_AREA_Y;

		sConsole.Rect(0, 0,
					  GAME_AREA_X + SIDE_AREA_X,
					  SIDE_AREA_Y, L' ');


		sConsole.Foreground(MAGENTA, true);
		sConsole.MLPrint(1, 1, aui8Levels[nLevel].strTitle);

		sConsole.Foreground(WHITE, false);
		sConsole.MLPrint(1, 3, aui8Levels[nLevel].strMessage);

		if (eState == PLAYING || eState == LEVEL_COMPLETE)
		{

			sConsole.Background(BLACK, false);
			sConsole.Foreground(BLACK, true);
			for (auto x = 0; x < width; x++)
			{
				for (auto y = 0; y < height; y++)
				{
					GridSpriteRenderer::Render(originX + x * CELL_SIZE_X, 
											   originY + y * CELL_SIZE_Y,
											   CELL_SIZE_X,
											   CELL_SIZE_Y,
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

						BlobSpriteRenderer::Render(originX + x * CELL_SIZE_X,
												   originY + y * CELL_SIZE_Y,
												   CELL_SIZE_X,
											       CELL_SIZE_Y,
												   ui8Neighbours, g_sSnake, sConsole);
					}
				}
			}

			for (auto x = 0; x < width; x++)
			{
				const auto xpos = originX + x * CELL_SIZE_X;

				for (auto y = 0; y < height; y++)
				{
					const auto ypos = originY + y * CELL_SIZE_Y;
					if (Cell(x, y) == SPIKES)
					{							   
						sConsole.Foreground(WHITE, true);
						sConsole.Print(xpos + 1, ypos + 1, L"\u25B2\u25B2\u25B2\u25B2\u25B2");
						sConsole.Print(xpos + 1, ypos + 2, L"\u25B2\u25B2\u25B2\u25B2\u25B2");
						sConsole.Print(xpos + 1, ypos + 3, L"\u25B2\u25B2\u25B2\u25B2\u25B2");

						sConsole.Foreground(WHITE, false);
						sConsole.Print(xpos + 2, ypos + 2, L"\u25B2");
						sConsole.Print(xpos + 4, ypos + 3, L"\u25B2");
						sConsole.Print(xpos + 5, ypos + 1, L"\u25B2");
					}
					else if (Cell(x, y) == EXIT)
					{
						sConsole.Foreground(WHITE, true);
						sConsole.Background(BLACK, false);
						sConsole.Print(xpos + 1, ypos + 0,       L"\u2591\u2592\u2593\u2592\u2591");
						sConsole.Print(xpos + 0, ypos + 1, L"\u2591\u2592\u2554\u2550\u2557\u2592\u2591");
						sConsole.Print(xpos + 0, ypos + 2, L"\u2592\u2593\u2551\u25D8\u2551\u2593\u2592");
						sConsole.Print(xpos + 0, ypos + 3, L"\u2591\u2592\u255A\u2550\u255D\u2592\u2591");
						sConsole.Print(xpos + 1, ypos + 4,       L"\u2591\u2592\u2593\u2592\u2591");
					}
				}
			}

			//sConsole.Background(BLUE, true);
			sConsole.Foreground(MAGENTA, true);
		
			for (auto x = 0; x < width; x++)
			{
				for (auto y = 0; y < height; y++)
				{
					//if (x == 5 && y == 4 || y != 3 && y != 5) continue;
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

						BlobSpriteRenderer::Render(originX + x * CELL_SIZE_X,
												   originY + y * CELL_SIZE_Y,
												   CELL_SIZE_X,
											       CELL_SIZE_Y, 
												   ui8Neighbours, g_sSnake, sConsole);
					}
				}
			}
		}
		
		if (eState == LEVEL_COMPLETE || eState == DIED)
		{
			sConsole.Background(BLACK, false);
			sConsole.Foreground(BLACK, true);
			GridSpriteRenderer::Render(10, 10, 41, 20, true, true, true, true, g_sSingleLineBox, sConsole);

			if (eState == LEVEL_COMPLETE)
			{
				sConsole.Foreground(MAGENTA, true);
				sConsole.Print(12, 12, L"Level Complete!");
			}
			else if (eState == DIED)
			{
				sConsole.Foreground(RED, true);
				sConsole.Print(12, 12, L"You died!");
			}
			sConsole.Foreground(WHITE, false);
			sConsole.Print(12, 16, L"Press any key to continue");
		}
	}
};

int main(int argc, char** argv)
{
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

	ConsoleWriter sConsole(GAME_AREA_X + SIDE_AREA_X,
						   GAME_AREA_Y + SIDE_AREA_Y);
	GameBoard sGame(GAME_SIZE_X, GAME_SIZE_Y);

	{
		wchar_t awcShades[] = L"\u00A0\u2591\u2592\u2593";
		
		int row = (GAME_AREA_Y + SIDE_AREA_Y) - 28;
		sConsole.Foreground(BLACK, true);
		sConsole.Background(BLACK, false);
		for (int i = 0; i < 4; i++, row++)
		{
			sConsole.Rect(0, row, GAME_AREA_X + SIDE_AREA_X, 1, awcShades[i]);
		}		
		sConsole.Foreground(WHITE, false);
		sConsole.Background(BLACK, true);
		for (int i = 0; i < 8; i++, row++)
		{
			sConsole.Rect(0, row, GAME_AREA_X + SIDE_AREA_X, 1, awcShades[i/2]);
		}
		sConsole.Foreground(WHITE, true);
		sConsole.Background(WHITE, false);
		for (int i = 0; i < 12; i++, row++)
		{
			sConsole.Rect(0, row, GAME_AREA_X + SIDE_AREA_X, 1, awcShades[i/3]);
		}
		sConsole.Foreground(WHITE, true);
		sConsole.Background(WHITE, true);
		for (int i = 0; i < 4; i++, row++)
		{
			sConsole.Rect(0, row, GAME_AREA_X + SIDE_AREA_X, 1, awcShades[0]);
		}

		sConsole.Foreground(MAGENTA, true);
		sConsole.Background(BLACK, false);

		wchar_t asLogo[][GAME_AREA_X+1] = 
		{
			//0123456789012345678901234567890123456789012345678901234567890
			//0         1         2         3         4         5         6
			L"                      Dijkstras Curse                        ",
			L"                                                             ",
			L"                             .;:;.                           ",
			L"                       .   .;:##:;.                          ",
			L"                      .;. .;:####:;.                         ",
			L"                    .;:#:;.;:###::;.                         ",
			L"                      .;. .;:##:;.                           ",
			L"                       .   .;::;.                            ",
			L"                            .;;.                             ",
			L"",
			L"          A game by Alex Walters {@CaffeinePwrdAl}           ",
			L"",
			L"         Copyright (c)  Caffine Powered Games - 2015         ",
		};

		for (int i = 0; i < sizeof(asLogo)/sizeof(asLogo[0]); i++)
		{
			for (int j = 0; j < GAME_AREA_X; j++)
			{
				switch (asLogo[i][j])
				{
				case L'.': asLogo[i][j] = L'\u2591'; break;
				case L';': asLogo[i][j] = L'\u2592'; break;
				case L':': asLogo[i][j] = L'\u2593'; break;
				case L'#': asLogo[i][j] = L'\u2588'; break;
				}
			}
			sConsole.Print(0, 4 + i, asLogo[i]);
		}
	}

	sConsole.Swap();

	sGame.Draw(sConsole);
	system("pause");

	sConsole.Swap();
	
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

			/*wchar_t astr[] = L"XX";
			wsprintf(astr, L"%02X", ch);
			sConsole.Foreground(WHITE, false);
			sConsole.Print(62, 38, astr);*/

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