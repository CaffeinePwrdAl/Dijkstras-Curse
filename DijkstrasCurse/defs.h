#define _USE_MATH_DEFINES
#include <math.h>

#include "Console.h"
#include <vector>

/* Size of Game Board */
const int GAME_SIZE_X = 10;
const int GAME_SIZE_Y = 11;

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

/*
 * Debug Defs
 *
 * Debug logging, when enabled is written into a pane along side the game.
 */

#if !defined (_DEBUG)

	const int DEBUG_AREA_X = 0;
	#define WRITE_DEBUG_LOG(...)

#else

	const int DEBUG_LINE_LEN = 80;
	const int DEBUG_AREA_X = DEBUG_LINE_LEN + 2;

	const int DEBUG_LOG_LEN = GAME_AREA_Y + SIDE_AREA_Y - 2;
	extern wchar_t g_astrDebugLog[DEBUG_LOG_LEN][DEBUG_LINE_LEN + 1];
	extern int g_nDebugLogNext;
	extern int g_nDebugLogStart;

	#define WRITE_DEBUG_LOG(...) \
		do \
		{ \
			swprintf_s(g_astrDebugLog[g_nDebugLogNext], DEBUG_LINE_LEN, __VA_ARGS__); \
			g_nDebugLogNext = (g_nDebugLogNext + 1) % DEBUG_LOG_LEN; \
			g_nDebugLogStart = (g_nDebugLogStart + 1) % DEBUG_LOG_LEN; \
		} while(0)

#endif

const int MAX_LINE_LENGTH = GAME_AREA_X + SIDE_AREA_X + DEBUG_AREA_X + 1;

struct Position
{
	int x;
	int y;
};

struct Level
{
	const wchar_t strTitle[GAME_AREA_X + 1];
	const wchar_t strMessage[GAME_AREA_X * 2 + 1];
	unsigned char ui8BonusStars;
	unsigned char ui8BonusSquares;
	unsigned char aui8Board[GAME_AREA_Y][GAME_AREA_X+1];
};

enum Direction
{
	None = 0,
	Up,
	Down,
	Left,
	Right,
};

static const wchar_t* g_asDirectionStrings[] =
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


