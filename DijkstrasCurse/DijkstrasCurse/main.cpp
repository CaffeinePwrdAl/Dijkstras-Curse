#define _USE_MATH_DEFINES
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <vector>
#include <string>
#include <iostream>
#include <math.h>
#include <Windows.h>
#include <WinCon.h>

CHAR_INFO* psScreenBuffer;

void PrintString(const wchar_t* string)
{
	size_t nLength = wcslen(string);
	const wchar_t* currentChar = string;
	const wchar_t* startChar = string;
	while (*currentChar != L'\0')
	{
		if (currentChar[0] == L'@' &&
			currentChar[1] == L'{')
		{
			/* Print and parse character code */
			wprintf_s(L"%.*s", currentChar-startChar, startChar);
			currentChar += 2;

			unsigned int uiAttrib;
			if (swscanf_s(currentChar, L"%02x", &uiAttrib) == 1)
			{
				HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hOutput, uiAttrib);
			}

			while (*currentChar != L'}')
			{
				currentChar++;
			}
			currentChar++;
			startChar = currentChar;
		}

		currentChar++;
	}

	wprintf_s(L"%.*s", currentChar - startChar, startChar);
}

bool InitialiseConsole()
{
	//HWND hWnd = GetConsoleWindow();
	HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hOutput, 0x0C);

	SMALL_RECT sRect = { 0, 0, 64, 64 };
	SetConsoleWindowInfo(hOutput, TRUE, &sRect);

	//SetConsoleTextAttribute(hOutput, 0x0C);

	CONSOLE_SCREEN_BUFFER_INFO sCSBI;
	GetConsoleScreenBufferInfo(hOutput, &sCSBI);
	
	CONSOLE_CURSOR_INFO sCCI;
	sCCI.bVisible = FALSE;
	SetConsoleCursorInfo(hOutput, &sCCI);

	return true;
}


struct Pos
{
	int x;
	int y;
};

class GameBoard
{
public:
	GameBoard(int w, int h) {}

public:
	bool CanMove(const Pos& sPos, const Pos& sDir) {}

protected:

};

int main(int argc, char** argv)
{
	InitialiseConsole();
	PrintString(L"@{07}You have chosen @{0D}Thermo-nuclear War@{07}. Do you wish to continue?\n@{0E}YES!\n\n@{07}");
	return 0;
}