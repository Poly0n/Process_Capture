#include "Processes.h"

void SetColor()
{
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

	SetConsoleTextAttribute(
		hConsole,
		BACKGROUND_BLUE | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE
	);

	return;
}

int main() {

	SetColor();

	Processes myProcesses;

	myProcesses.execute();


	return 0;
}