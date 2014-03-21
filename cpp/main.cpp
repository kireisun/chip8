#include "cpuEmu.h" // my CPU core implementation
#include "fonts.h" // include fonts
#include "typedefs.h"
#include <cstdio>
#include <Windows.h>


chip8 myChip8;

int main(int argc, char **argv)
{
	if(argc != 2)
	{
		printf("%s", "USAGE:\n-name of game to load\n");
		exit(0);
	};
	
	char *gameName(argv[1]);

	// set up render system and register input callbacks

	setupGraphics();
	setupInput();

	// initialize the chip8 system and load the game into memory
	myChip8.initialize();
	myChip8.loadGamePath(gameName);

		// emulation loop
	for (;;)
	{
		// emulate one cycle
		myChip8.emulateCycle();

		// if the draw flag is set, eupdate the screen
		if (myChip8.drawFlag)
			drawGraphics();

		// store key press state (press and release)
		myChip8.setKeys();
	}

	return 0;
}

