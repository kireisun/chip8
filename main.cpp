//#include OpenGL graphics and input libraries
#include "cpuEmu.h" // my CPU core implementation
#include <ctime>
#include <cstdlib>

chip8 myChip8;

int main(int argc, char **argv)
{
    unsigned short opcode;
    unsigned char memory[4096];
    unsigned char V[16];
    unsigned short I;
    unsigned short pc;
    unsigned char gfx[64 * 32];
    unsigned char delay_timer;
    unsigned char sound_timer;
    unsigned short my_stack[16];
    unsigned short sp;
    unsigned char keyboard[16];
    // set up render system and register input callbacks

    setupGraphics();
    setupInput();

    // initialize the chip8 system and load the game into memory
    myChip8.initialize()
    myChip8.loadGame("pong")

    // emulation loop
    for(;;)
    {
        // emulate one cycle
        myChip8.emulateCycle();

        // if the draw flag is set, update the screen
        if(myChip8.drawFlag)
            drawGraphics();

        // store key press state (press and release)
        myChip8.setKeys();
    }

    return 0;
}

