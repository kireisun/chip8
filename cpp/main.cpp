#include "cpuEmu.h" // my CPU core implementation
#include "fonts.h" // include fonts
#include "typedefs.h" // typedefs definitions
#include <cstdio>
#include <GL\freeglut.h>
#include <GL\glew.h>

chip8 myChip8;

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("%s", "USAGE:\nchip8.exe [name of game to load] [window width] [window heigth]\n");
		exit(0);
	}
	
	// setup the chip8 machine and load game in memory

	myChip8.loadGamePath(argv[1]);
	myChip8.initialize();

	// setup OpenGL

	myChip8.windowModifier = 1;

	if (argc > 1)
	{
		myChip8.windowModifier = *argv[1];
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutInitWindowPosition(320, 320);
	glutInitWindowSize(myChip8.windowModifier * 64, myChip8.windowModifier * 32);
	glutCreateWindow("Chip8 Emulator");
	glewInit();
	glutDisplayFunc(myChip8.display);
	glutIdleFunc(myChip8.display);
	glutReshapeFunc(myChip8.reshape_display);
	
	myChip8.setupTexture();

	glutMainLoop();

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

