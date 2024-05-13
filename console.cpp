#include "console.h"

console::console() {

	AllocConsole();

	HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	int fd = _open_osfhandle(reinterpret_cast<intptr_t>(consoleHandle), _O_TEXT);
	FILE* consoleFile = _fdopen(fd, "w");
	*stdout = *consoleFile;
	setvbuf(stdout, nullptr, _IONBF, 0);

	// Redirect stdout to the console
	freopen_s(&consoleFile, "CONOUT$", "w", stdout);
}

console::~console() {
	
	// Flush and close stdout
	fflush(stdout);
	fclose(stdout);

	// Free the console
	FreeConsole();
}