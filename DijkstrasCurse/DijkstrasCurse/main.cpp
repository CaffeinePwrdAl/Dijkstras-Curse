#define _USE_MATH_DEFINES
#include <math.h>

#include "Console.h"
//#define WIN32_LEAN_AND_MEAN
//#define VC_EXTRALEAN
#include <vector>
//#include <string>
//#include <memory>
//
//#include <Windows.h>
//#include <WinCon.h>
//#include <io.h>
//#include <fcntl.h>

/* Size of Game Board */
const int GAME_SIZE_X = 10;
const int GAME_SIZE_Y = 11;

/* Size of Game Board Grid cell in chars */
const int CELL_SIZE_X = 6;
const int CELL_SIZE_Y = 4;

struct Position
{
	int x;
	int y;
};

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

#if defined (_DEBUG)
const int DEBUG_LINE_LEN = 80;
const int DEBUG_AREA_X = DEBUG_LINE_LEN + 2;
#else
const int DEBUG_AREA_X = 0;
#endif

const int MAX_LINE_LENGTH = GAME_AREA_X + SIDE_AREA_X + DEBUG_AREA_X + 1;

#if defined (_DEBUG)
const int DEBUG_LOG_LEN = GAME_AREA_Y + SIDE_AREA_Y - 2;
wchar_t g_astrDebugLog[DEBUG_LOG_LEN][DEBUG_LINE_LEN + 1];
int g_nDebugLogNext = 0;
int g_nDebugLogStart = DEBUG_LOG_LEN;
#define WRITE_DEBUG_LOG(...) \
	do \
	{ \
		swprintf_s(g_astrDebugLog[g_nDebugLogNext], DEBUG_LINE_LEN, __VA_ARGS__); \
		g_nDebugLogNext = (g_nDebugLogNext + 1) % DEBUG_LOG_LEN; \
		g_nDebugLogStart = (g_nDebugLogStart + 1) % DEBUG_LOG_LEN; \
	} while(0)
#else
#define WRITE_DEBUG_LOG(...)
#endif

struct Level
{
	const wchar_t strTitle[GAME_AREA_X + 1];
	const wchar_t strMessage[GAME_AREA_X * 2 + 1];
	unsigned char ui8BonusStars;
	unsigned char ui8BonusSquares;
	unsigned char aui8Board[GAME_AREA_Y][GAME_AREA_X+1];
};

const Level aui8Levels[] = 
{
	{
		L"T1: In the beginning...",
		L"When you move all your cells replicate in that direction.\n" \
		 "Reach the exit portal.",
		0, 0,
		{
			"B         ",
			"     B    ",
			"  B       ",
			"          ",
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
		0, 0,
		{
			"!!!!!!!!!!",
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
		0, 0,
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
			"xxxxxxxxxx",
		},
	},
	{
		L"T4: I'll take the high road, and I'll take the low road",
		L"Sometimes you'll explore many paths at the same time.\n" \
		 "  ... There might be bonus goals for covering extra squares.",
		0, 0,
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
"xxxxxxxxxx",
		},
	},
	{
		//01234567890123456789012345678901234567890123456789012345678|
		//0         1         2         3         4         5        |
		L"T5: The obvious path...\n",
		L"... is not always the right path.\n" \
		 "Bonus Goal: Cover all the open spaces",
		0, 40,
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
			"xxxxxxxxxx",
		},
	},
	{
		L"Section 1: Ready?",
		L"Now you know what you're doing, it's time to begin!",
		0, 0,
		{
			"xxxxxxxxxx",
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
		L"L1: 'The emergency exits are here, here, here...'",
		L"Bonus Goal: Collect 5 Stars",
		5, 0,
		{
			"!!!!!!! e!",
			"!!xxxx! x!",
			"!!  * * x!",
			"!!  x!! x!",
			"!! xxxx  !",
			"!x B  x  !",
			"!   x!  !!",
			"* * xx   !",
			"e!  *   *!",
			"!! *  !  !",
			"!x!! !x!e!",
		},
	},
	{
		L"L2: 'Left Brain/Right Brain'",
		L"Bonus Goal: Cover all the open spaces",
		0, 57,
		{
			"xxxxxxxxxx",
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
	},
	{
		L"L3: Slot Machine",
		L"Bonus Goal: Cover all the open spaces",
		0, 74,
		{
			"exxxxxxxBx",
			" xx x     ",
			" !     x  ",
			" !  x    x",
			" !    x   ",
			" ! x    x ",
			" !    xx  ",
			" xx x    x",
			"         x",
			"    x    x",
			"      xxxx",
		},
	},
	{
		L"Section 2: Ooooh, push it, push it real good",
		L"Yeah, Ok, I'll stop now.",
		0, 0,
		{
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"B   M     ",
			"xxxxxxxxex",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
			"xxxxxxxxxx",
		},
	},
	{
		L"M1: Like a knife through shark infested custard.",
		L"or is it a custard infested shark through knives?\n" \
		L"                    ... I can never remember.",
		0, 0,
		{	
			"B         ",
			"MMMMMMMMMM",
			"          ",
			"  !    !  ",
			"    !    !",
			"          ",
			"      !   ",
			"          ",
			" !   !   x",
			"   !     e",
			"         x",
		},
	},
	{
		L"M2: To Me, To You.",
		L"",
		0, 0,
		{	
			"Bx        ",
			" x        ",
			" x        ",
			" x       B",
			" x        ",
			" M        ",
			" xxxxxxxxx",
			"          ",
			" xxxxxxxx ",
			"!xxxxxxxx ",
			"xxxxxxxxxe",
		},
	},
	{
		L"M3: The Medusa Cascade",
		L"Bonus Goal: Cover all the open spaces",
		0, 61,
		{
			"Bx        ",
			" MB       ",
			" xxxxxxxx ",
			"       M  ",
			" xxxxxxxx ",
			"!!xx      ",
			"!!   xxxx ",
			"!x x   xx!",
			"xx x     !",
			"BM xxxxx !",
			"BBBxe    !",
		},
	},
	{
		L"M3: 'But they are the gatekeepers'",
		L"They are guarding all the doors, they are holding all the\n"\
		L"keys -- Morpheus.",
		0, 0,
		{
			"xQxxxx  M ",
			"xQxQx xQx ",
			"x x x x x ",
			"xQx xQxQxB",
			"xQxQxQx xx",
			"B  Q Q Q e",
			"xQxQx x xx",
			"x xQx x xx",
			"x x x x xx",
			"x x xxxxxx",
			"xxx xxxxxx",
		},
	},
};
const int numLevels = sizeof(aui8Levels) / sizeof(aui8Levels[0]);

struct Pos
{
	int x;
	int y;
};

enum Direction
{
	None = 0,
	Up,
	Down,
	Left,
	Right,
};

const wchar_t* g_asDirectionStrings[] =
{
	L"None",
	L"Up",
	L"Down",
	L"Left",
	L"Right",
};

enum GameCell
{
	SPACE,
	WALL,
	BLOB,
	SPIKES,
	EXIT,
	STAR,
	MOVEABLE,
	QUANTUM,
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
		
		wchar_t asLine[MAX_LINE_LENGTH];
		/*if (w+1 > 80)
		{
			__debugbreak();
			return;
		}*/
		
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

	// Per-level counters for bonus goals
	int nStars;
	int nSquares;

	// Frame Count
	int nFrame;

public:
	GameBoard(int w, int h)
		: width(w)
		, height(h)
		, nLevel(0)
		, nCurrentBoard(0)
		, eState(PLAYING)
		, nFrame(0)
	{
		asGameBoard[0] = std::make_unique<GameCell[]>(w * h);
		asGameBoard[1] = std::make_unique<GameCell[]>(w * h);

		srand(42);

		nLevel = 0;

		InitialiseTestLevel();
	}

	const GameCell Cell(const int& x, const int& y) const
	{
		if (x >= 0 && x < width && y >= 0 && y < height)
			return asGameBoard[!nCurrentBoard][x + width * y];
		return SPACE;
	}

	bool WriteCell(const int& x, const int& y, const GameCell& eCell)
	{
		static GameCell eEmptyCell = SPACE;
		if (x >= 0 && x < width && y >= 0 && y < height)
		{
			asGameBoard[nCurrentBoard][x + width * y] = eCell;
			return true;
		}
		else
		{
			return false;
		}
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
		nStars = 0;
		nSquares = 0;

		for (int x = 0; x < GAME_SIZE_X; x++)
		{
			for (int y = 0; y < GAME_SIZE_Y; y++)
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
					nSquares += 1;
					break;
				case '*':
					eCell = STAR;
					break;
				case 'M':
					eCell = MOVEABLE;
					break;
				case 'Q':
					eCell = QUANTUM;
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

protected:
	bool MoveInDir(Direction eDir, int& x, int& y) const
	{
		switch (eDir)
		{
		case Up:	y = y - 1; break;
		case Down:	y = y + 1; break;
		case Left:	x = x - 1; break;
		case Right: x = x + 1; break;
		}

		return (x >= 0 && x < width && y >= 0 && y < height);
	}

	bool MoveableBlockCanMove(const int& x, const int& y, const Direction& eDir)
	{
		bool bBlockedMove = true;

		int mx = x, my = y;

		/* Get coords of movement if move is on game board */
		if (MoveInDir(eDir, mx, my))
		{
			/* Check that the moveable block can move */
			const GameCell& eMvNeighbour = Cell(mx, my);
			switch (eMvNeighbour)
			{
				/* These are passable spaces */
				case SPACE:
				case STAR:
					bBlockedMove = false;
					break;

				/* Anything else cant be moved over */
			default:
				break;
			}
		}

		return (bBlockedMove == false);
	}

	bool QuantumBlockCanMove(const int& x, const int& y, const Direction& eDir)
	{
		bool bBlockedMove = true;

		int mx = x, my = y;

		/* Get coords of movement if move is on game board */
		if (MoveInDir(eDir, mx, my))
		{
			/* Check that the moveable block can move */
			const GameCell& eMvNeighbour = Cell(mx, my);
			switch (eMvNeighbour)
			{
				/* These are passable spaces */
			case SPACE:
			case STAR:
			case QUANTUM:
				bBlockedMove = false;
				break;

				/* Anything else cant be moved over */
			default:
				break;
			}
		}

		return (bBlockedMove == false);
	}

	bool MoveQuantumBlocks(const Direction& eDir)
	{
		bool bBlockedMove = false;
		std::vector<Position> asQuantumBlocks;

		WRITE_DEBUG_LOG(L"MoveQuantumBlocks():");

		WRITE_DEBUG_LOG(L"  Moving quantum blocks out of phase:");
		for (auto x = 0; x < width; x++)
		{
			for (auto y = 0; y < height; y++)
			{
				if (Cell(x, y) == QUANTUM)
				{
					WRITE_DEBUG_LOG(L"    Dephased Quantum block @ (%d, %d)", x, y);
					asQuantumBlocks.push_back({ x, y });
					WriteCell(x, y, SPACE);
				}
			}
		}

		WRITE_DEBUG_LOG(L"  Checking quantum blocks for free movement:");
		for (const auto& pos : asQuantumBlocks)
		{
			WRITE_DEBUG_LOG(L"    Check Quantum block @ (%d, %d) can move %s", pos.x, pos.y, g_asDirectionStrings[eDir]);
			if (!QuantumBlockCanMove(pos.x, pos.y, eDir))
			{
				WRITE_DEBUG_LOG(L"     -> Blocked!");
				bBlockedMove = true;
			}
			else
			{
				WRITE_DEBUG_LOG(L"     -> Can Move");
			}
		}

		if (bBlockedMove == false)
		{
			WRITE_DEBUG_LOG(L"Applying quantum block movement:");
			for (auto&& pos : asQuantumBlocks)
			{
				MoveInDir(eDir, pos.x, pos.y);
				WRITE_DEBUG_LOG(L"    Moved Quantum block to (%d, %d)", pos.x, pos.y);
			}
		}
		else
		{
			WRITE_DEBUG_LOG(L"Quantum blockage detected");
		}

		// Re-phase the quantum blocks
		WRITE_DEBUG_LOG(L"  Moving quantum blocks back into phase:");
		for (const auto& pos : asQuantumBlocks)
		{
			WriteCell(pos.x, pos.y, QUANTUM);
		}

		WRITE_DEBUG_LOG(L"");

		return bBlockedMove;
	}

public:
	void KeyPress(wchar_t key)
	{
		if (key == L'r')
		{
			eState = PLAYING;
			InitialiseTestLevel();
			SwitchBoard();
			return;
		}

		if (key == L'n')
		{
			eState = PLAYING;
			NextLevel();
			InitialiseTestLevel();
			SwitchBoard();
			return;
		}

		if (key == L'm')
		{
			nLevel = (nLevel + numLevels - 1) % (numLevels);
			eState = PLAYING;
			InitialiseTestLevel();
			SwitchBoard();
			return;
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
							bool bBlockedMove = true;
							bool bMovedQuantum = false;

							int nx = x;
							int ny = y;
							
							if (!MoveInDir(eDir, nx, ny))
							{
								/* Early exit - invalid square */
								continue;
							}

							const GameCell& eCell = Cell(nx, ny);
							switch (eCell)
							{
								/* Immovable */
								case WALL:
									break;
								case SPIKES:
									bDied = true;
									break;

								/* Doesn't count */
								case EXIT:
									bComplete = true;
									break;
								case BLOB:
									break;

								/* Quantum Moveable */
								case QUANTUM:
								{
									if (!bMovedQuantum)
									{
										bBlockedMove = MoveQuantumBlocks(eDir);
										bMovedQuantum = true;
									}
								}
								break;

								/* Moveable */
								case MOVEABLE:
								{
									int mx = nx, my = ny;
									/* If move is on game board */
									if (MoveInDir(eDir, mx, my))
									{
										/* Check that the moveable block can move */
										const GameCell& eMvNeighbour = Cell(mx, my);
										switch (eMvNeighbour)
										{
										/* These are passable spaces */
										case SPACE:
										case STAR:
											WriteCell(mx, my, MOVEABLE);
											bBlockedMove = false;
											break;

										/* Anything else cant be moved over */
										default:
											break;
										}
									}
								}
								break;

								/* All other cells can be moved to */
								default:
									bBlockedMove = false;
							}

							/* If movement was valid, update cell and counters */
							if (bBlockedMove == false)
							{
								WriteCell(nx, ny, BLOB);
								nSquares += 1;
								if (eCell == STAR)
								{
									nStars += 1;
								}
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

	static void WriteCounter(ConsoleWriter& sConsole, const int& nCount, const int& nMax, const int& x, const int& y)
	{
		wchar_t strCounter[25];
		wsprintf(strCounter, L"%03d / %03d", nCount, nMax);
		
		sConsole.Print(x, y, strCounter);
	}

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

		sConsole.Foreground(YELLOW, true);
		if (aui8Levels[nLevel].ui8BonusStars > 0)
		{
			WriteCounter(sConsole, nStars, aui8Levels[nLevel].ui8BonusStars, GAME_AREA_X - 12, 3);
		}

		if (aui8Levels[nLevel].ui8BonusSquares > 0)
		{
			WriteCounter(sConsole, nSquares, aui8Levels[nLevel].ui8BonusSquares, GAME_AREA_X - 12, 2);
		}

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
					const GameCell& eCell = Cell(x, y);
					const auto ypos = originY + y * CELL_SIZE_Y;
					if (eCell == SPIKES)
					{	
						sConsole.Background(BLACK, false);
						sConsole.Foreground(WHITE, true);
						sConsole.Print(xpos + 1, ypos + 1, L"\u25B2\u25B2\u25B2\u25B2\u25B2");
						sConsole.Print(xpos + 1, ypos + 2, L"\u25B2\u25B2\u25B2\u25B2\u25B2");
						sConsole.Print(xpos + 1, ypos + 3, L"\u25B2\u25B2\u25B2\u25B2\u25B2");

						sConsole.Foreground(WHITE, false);
						sConsole.Print(xpos + 2, ypos + 2, L"\u25B2");
						sConsole.Print(xpos + 4, ypos + 3, L"\u25B2");
						sConsole.Print(xpos + 5, ypos + 1, L"\u25B2");
					}
					else if (eCell == EXIT)
					{
						sConsole.Foreground(WHITE, true);
						sConsole.Background(BLACK, false);
						sConsole.Print(xpos + 1, ypos + 0,       L"\u2591\u2592\u2593\u2592\u2591");
						sConsole.Print(xpos + 0, ypos + 1, L"\u2591\u2592\u2554\u2550\u2557\u2592\u2591");
						sConsole.Print(xpos + 0, ypos + 2, L"\u2592\u2593\u2551\u25D8\u2551\u2593\u2592");
						sConsole.Print(xpos + 0, ypos + 3, L"\u2591\u2592\u255A\u2550\u255D\u2592\u2591");
						sConsole.Print(xpos + 1, ypos + 4,       L"\u2591\u2592\u2593\u2592\u2591");
					}
					else if (eCell == STAR)
					{
						sConsole.Foreground(YELLOW, false);
						sConsole.Background(BLACK, false);
						
						sConsole.Print(xpos + 2, ypos + 1, L".|,"); 
						sConsole.Print(xpos + 1, ypos + 2, L"-=O=-"); 
						sConsole.Print(xpos + 2, ypos + 3, L"'|`"); 
					}
					else if (eCell == MOVEABLE || eCell == QUANTUM)
					{
						sConsole.Foreground((eCell == MOVEABLE)?(WHITE):(BLUE), true);
						sConsole.Background(BLACK, false);

						/* Gradient pattern, 3 repeats */
						wchar_t *strGradient = L"\u2592\u2593\u2592\u2591\u2591\u2592\u2593\u2592\u2591\u2591\u2592\u2593\u2592\u2591\u2591";

						/* Animation test */
						int nOffset = 5;// + (nFrame % 5);

						sConsole.Printn(xpos + 1, ypos + 1, strGradient + nOffset, 5);
						sConsole.Printn(xpos + 1, ypos + 2, strGradient + nOffset - 1, 5);
						sConsole.Printn(xpos + 1, ypos + 3, strGradient + nOffset - 2, 5);
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
		nFrame++;
	}
};

int main(int argc, char** argv)
{
	HANDLE hIn = GetStdHandle(STD_INPUT_HANDLE);

	ConsoleWriter sConsole(GAME_AREA_X + SIDE_AREA_X + DEBUG_AREA_X,
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
	//system("pause");

	sConsole.Swap();
	
	bool bRunning = true;
	while (bRunning)
	{
		/* Wait is the game time step in ms - or INFINITE for no display update tick */
		DWORD nWaitStatus = WaitForSingleObject(hIn, INFINITE);	

		switch (nWaitStatus)
		{
			case WAIT_OBJECT_0:
			{
				INPUT_RECORD InRec;
				DWORD numRead;
				if (!ReadConsoleInput(hIn, &InRec, 1, &numRead))
				{
					bRunning = false;
					continue;
				}
				
				if ((InRec.EventType == KEY_EVENT) &&
					InRec.Event.KeyEvent.bKeyDown)
				{
					wchar_t ch = InRec.Event.KeyEvent.uChar.UnicodeChar;

					/*wchar_t astr[] = L"XX";
					wsprintf(astr, L"%02X", ch);
					sConsole.Foreground(WHITE, false);
					sConsole.Print(62, 38, astr);*/

					sGame.KeyPress(ch);

					if (ch == 0x1B)
					{
						bRunning = false;
						continue;
					}
				}
			}
			/* Fall through */

			case WAIT_TIMEOUT:
			{
				sGame.Draw(sConsole);

#if defined (_DEBUG)
				{
					sConsole.Foreground(WHITE, false);

					// Border around the debug area
					GridSpriteRenderer::Render(GAME_AREA_X + SIDE_AREA_X, 0,
						DEBUG_AREA_X - 1, GAME_AREA_Y + SIDE_AREA_Y - 1,
						true, true, true, true,
						g_sSingleLineBox, sConsole);

					// Dump the log buffer
					int entry = g_nDebugLogStart;
					for (auto i = 0; i < DEBUG_LOG_LEN; i++)
					{
						sConsole.Print(GAME_AREA_X + SIDE_AREA_X + 1, 1 + i, g_astrDebugLog[entry]);
						entry = (entry + 1) % DEBUG_LOG_LEN;
					}
				}
#endif
				sConsole.Swap();
			}
			break;

			default:
			case WAIT_ABANDONED:
			case WAIT_FAILED:
				printf("Something went wrong: WaitForSingleObject -> 0x%x\n", nWaitStatus);
				bRunning = false;
				continue;
		}

    }

	wchar_t chars[] = { 0x2588, 0x2593, 0x2592, 0x2591, 0};
	sConsole.Foreground(MAGENTA, true);
	sConsole.Print(40, 40, chars);

	sConsole.Foreground(WHITE, false);
	sConsole.Print(0, 39, L"");

	return 0;
}